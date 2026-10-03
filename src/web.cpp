#include "web.h"
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include "clock.h"
#include "cycle.h"
#include "log.h"
#include "outputs.h"
#include "scale.h"
#include "schedule.h"
#include "storage.h"
#include "web_ui.h"

namespace {
WebServer server(80);

void sendJson(const JsonDocument &doc, int code = 200) {
  String s;
  serializeJson(doc, s);
  server.send(code, "application/json", s);
}

void sendOk() { server.send(200, "application/json", "{\"ok\":true}"); }

void sendError(const String &msg) {
  JsonDocument d;
  d["error"] = msg;
  sendJson(d, 400);
}

bool readBody(JsonDocument &doc) {
  if (!server.hasArg("plain")) return true;  // corpo vuoto = oggetto vuoto
  return !deserializeJson(doc, server.arg("plain"));
}

void handleStatus() {
  JsonDocument d;
  d["clockOk"] = Clock::ok();
  d["time"] = Clock::ok() ? Clock::format(Clock::nowLocal()) : String();
  d["scaleOk"] = Scale::ok();
  d["scaleCal"] = Scale::calibrated();
  d["grams"] = Scale::grams();
  d["raw"] = Scale::raw();
  d["tankFull"] = Scale::tankFull();
  d["phase"] = Cycle::phaseName();
  d["busy"] = Cycle::busy();
  d["skipNext"] = st.skipNext;
  d["fert0"] = (int)OUT_FERT0;
  JsonArray outs = d["outputs"].to<JsonArray>();
  JsonArray names = d["outNames"].to<JsonArray>();
  for (uint8_t i = 0; i < OUT_COUNT; i++) {
    outs.add(Outputs::isOn(i));
    names.add(Outputs::name(i));
  }
  if (Cycle::busy()) {
    JsonObject c = d["cycle"].to<JsonObject>();
    c["perPlantL"] = st.perPlantL;
    JsonArray del = c["delivered"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_PLANTS; i++) del.add(st.deliveredL[i]);
  }
  Schedule::statusToJson(d["schedule"].to<JsonObject>());
  Log::alertsToJson(d["alerts"].to<JsonArray>());
  sendJson(d);
}

void handleGetPlan() {
  JsonDocument d;
  Storage::planToJson(d.to<JsonObject>());
  sendJson(d);
}

void handleSetPlan() {
  JsonDocument d;
  if (!readBody(d)) return sendError("JSON non valido");
  static Plan p;
  String err;
  if (!Storage::planFromJson(d.as<JsonObjectConst>(), p, err)) return sendError(err);
  plan = p;
  Storage::savePlan();
  Log::info("Piano \"%s\" salvato (%u settimane, %s)", plan.name.c_str(), plan.numWeeks,
            plan.active ? "attivo" : "non attivo");
  sendOk();
}

void handleGetCalib() {
  JsonDocument d;
  Storage::calibToJson(d.to<JsonObject>());
  sendJson(d);
}

void handleSetCalib() {
  JsonDocument d;
  if (!readBody(d)) return sendError("JSON non valido");
  Storage::calibFromJson(d.as<JsonObjectConst>());
  Storage::saveCalib();
  sendOk();
}

void handleTime() {
  JsonDocument d;
  if (!readBody(d) || !d["epoch"].is<uint32_t>()) return sendError("Ora non valida");
  Clock::setUtc(d["epoch"].as<uint32_t>());
  Log::info("Ora impostata dal telefono");
  sendOk();
}

void handleManual() {
  JsonDocument d;
  if (!readBody(d)) return sendError("JSON non valido");
  if (Cycle::busy()) return sendError("Irrigazione in corso: premi STOP prima");
  int out = d["out"] | -1;
  int sec = d["sec"] | 0;
  if (out < 0 || out >= OUT_COUNT || sec < 1 || sec > 600) return sendError("Parametri non validi");
  if (out == OUT_VALVE && Scale::tankFull()) return sendError("Vasca piena (galleggiante)");
  Outputs::on(out, sec * 1000UL);
  Log::info("Manuale: %s per %d s", Outputs::name(out).c_str(), sec);
  sendOk();
}

void handleStop() {
  if (Cycle::busy()) Cycle::abort("fermata manuale");
  Outputs::allOff();
  sendOk();
}

void handleRun() {
  JsonDocument d;
  if (!readBody(d)) return sendError("JSON non valido");
  if (Cycle::busy()) return sendError("Irrigazione già in corso");
  float recipe[MAX_FERT] = {0};
  int week = Clock::ok() ? Schedule::weekOf(Clock::nowLocal()) : 0;
  if (week > 0) memcpy(recipe, plan.weeks[week - 1].fert, sizeof recipe);
  float litres = d["litres"] | Schedule::litresPerIrrigation(week);
  if (litres <= 0 || litres > 10) return sendError("Indica i litri per pianta (0-10)");
  if (!Cycle::start(litres, recipe, 0, "manuale"))
    return sendError("Avvio non riuscito: controlla la bilancia");
  sendOk();
}

void handleSkip() {
  JsonDocument d;
  if (!readBody(d)) return sendError("JSON non valido");
  st.skipNext = d["skip"] | false;
  Storage::saveState();
  sendOk();
}

void handleTare() {
  if (!Scale::ok()) return sendError("La bilancia non risponde");
  Scale::tare();
  Log::info("Bilancia: tara eseguita");
  sendOk();
}

void handleScaleCal() {
  JsonDocument d;
  if (!readBody(d)) return sendError("JSON non valido");
  float g = d["grams"] | 0.0f;
  if (!Scale::ok() || !Scale::calibrate(g))
    return sendError("Calibrazione non riuscita: hai fatto la tara e appoggiato il peso?");
  Log::info("Bilancia calibrata con %.0f g", g);
  sendOk();
}

void handleLog() {
  const char *path = server.hasArg("old") ? "/log.old" : "/log.txt";
  File f = LittleFS.open(path, "r");
  if (!f) {
    server.send(200, "text/plain; charset=utf-8", "(registro vuoto)");
    return;
  }
  server.streamFile(f, "text/plain; charset=utf-8");
  f.close();
}
}  // namespace

void Web::begin() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  server.on("/", HTTP_GET, [] { server.send(200, "text/html; charset=utf-8", INDEX_HTML); });
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/plan", HTTP_GET, handleGetPlan);
  server.on("/api/plan", HTTP_POST, handleSetPlan);
  server.on("/api/calib", HTTP_GET, handleGetCalib);
  server.on("/api/calib", HTTP_POST, handleSetCalib);
  server.on("/api/time", HTTP_POST, handleTime);
  server.on("/api/manual", HTTP_POST, handleManual);
  server.on("/api/stop", HTTP_POST, handleStop);
  server.on("/api/run", HTTP_POST, handleRun);
  server.on("/api/skip", HTTP_POST, handleSkip);
  server.on("/api/tare", HTTP_POST, handleTare);
  server.on("/api/scalecal", HTTP_POST, handleScaleCal);
  server.on("/api/log", HTTP_GET, handleLog);
  server.on("/api/alerts/clear", HTTP_POST, [] {
    Log::clearAlerts();
    sendOk();
  });
  server.onNotFound([] { server.send(404, "text/plain", "Non trovato"); });
  server.begin();
}

void Web::update() { server.handleClient(); }
