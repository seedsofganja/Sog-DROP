#include "storage.h"
#include <LittleFS.h>

Plan plan;
Calib calib;
CycleState st;

namespace {
const char *PLAN_PATH = "/plan.json";
const char *CALIB_PATH = "/calib.json";
const char *STATE_PATH = "/state.json";

bool readJson(const char *path, JsonDocument &doc) {
  File f = LittleFS.open(path, "r");
  if (!f) return false;
  DeserializationError e = deserializeJson(doc, f);
  f.close();
  return !e;
}

// LittleFS rende effettiva la scrittura solo alla chiusura del file:
// un'interruzione di corrente lascia la versione precedente intatta.
bool writeJson(const char *path, const JsonDocument &doc) {
  File f = LittleFS.open(path, "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

void defaultPlan(Plan &p) {
  p.active = false;
  p.name = "Nuovo piano";
  p.startY = 2026;
  p.startM = 1;
  p.startD = 1;
  p.hour = 7;
  p.minute = 0;
  p.numFert = 3;
  for (uint8_t i = 0; i < MAX_FERT; i++) p.fertNames[i] = "Fert. " + String(i + 1);
  for (uint8_t i = 0; i < NUM_PLANTS; i++) p.plantOn[i] = true;
  p.numWeeks = 1;
  p.weeks[0] = {1.0f, 0b0010101, {0, 0, 0, 0, 0}};  // lun, mer, ven
}

void defaultCalib(Calib &c) {
  c.scaleOffset = 0;
  c.scaleFactor = 0;
  for (uint8_t i = 0; i < MAX_FERT; i++) c.fertMlPerSec[i] = 0;
  c.mixSeconds = 20;
}

template <typename T, size_t N>
void toArray(JsonArray a, const T (&v)[N]) {
  for (size_t i = 0; i < N; i++) a.add(v[i]);
}

template <typename T, size_t N>
void fromArray(JsonArrayConst a, T (&v)[N]) {
  for (size_t i = 0; i < N; i++) v[i] = a[i] | v[i];
}
}  // namespace

bool Storage::begin() { return LittleFS.begin(true); }

void Storage::loadAll() {
  defaultPlan(plan);
  defaultCalib(calib);
  memset(&st, 0, sizeof st);

  JsonDocument doc;
  if (readJson(PLAN_PATH, doc)) {
    String err;
    Plan p;
    if (planFromJson(doc.as<JsonObjectConst>(), p, err)) plan = p;
  }

  doc.clear();
  if (readJson(CALIB_PATH, doc)) {
    calib.scaleOffset = doc["scaleOffset"] | 0;
    calib.scaleFactor = doc["scaleFactor"] | 0.0f;
    calibFromJson(doc.as<JsonObjectConst>());
  }

  doc.clear();
  if (readJson(STATE_PATH, doc)) {
    st.lastDoneDay = doc["lastDoneDay"] | 0;
    st.skipNext = doc["skipNext"] | false;
    fromArray(doc["tankConc"], st.tankConc);
    st.phase = (Phase)(doc["phase"] | 0);
    st.cycleDay = doc["cycleDay"] | 0;
    st.perPlantL = doc["perPlantL"] | 0.0f;
    fromArray(doc["recipe"], st.recipe);
    fromArray(doc["deliveredL"], st.deliveredL);
    fromArray(doc["plantSkip"], st.plantSkip);
    st.batchPerPlantL = doc["batchPerPlantL"] | 0.0f;
    st.residualL = doc["residualL"] | 0.0f;
    st.fillTargetG = doc["fillTargetG"] | 0.0f;
    st.mixL = doc["mixL"] | 0.0f;
    fromArray(doc["doseMl"], st.doseMl);
    st.fertIdx = doc["fertIdx"] | 0;
    st.dosing = doc["dosing"] | false;
    st.plantIdx = doc["plantIdx"] | 0;
    st.plantRunning = doc["plantRunning"] | false;
    st.plantStartG = doc["plantStartG"] | 0.0f;
    st.retries = doc["retries"] | 0;
  }
}

bool Storage::savePlan() {
  JsonDocument doc;
  planToJson(doc.to<JsonObject>());
  return writeJson(PLAN_PATH, doc);
}

bool Storage::saveCalib() {
  JsonDocument doc;
  calibToJson(doc.to<JsonObject>());
  return writeJson(CALIB_PATH, doc);
}

bool Storage::saveState() {
  JsonDocument doc;
  doc["lastDoneDay"] = st.lastDoneDay;
  doc["skipNext"] = st.skipNext;
  toArray(doc["tankConc"].to<JsonArray>(), st.tankConc);
  doc["phase"] = (uint8_t)st.phase;
  doc["cycleDay"] = st.cycleDay;
  doc["perPlantL"] = st.perPlantL;
  toArray(doc["recipe"].to<JsonArray>(), st.recipe);
  toArray(doc["deliveredL"].to<JsonArray>(), st.deliveredL);
  toArray(doc["plantSkip"].to<JsonArray>(), st.plantSkip);
  doc["batchPerPlantL"] = st.batchPerPlantL;
  doc["residualL"] = st.residualL;
  doc["fillTargetG"] = st.fillTargetG;
  doc["mixL"] = st.mixL;
  toArray(doc["doseMl"].to<JsonArray>(), st.doseMl);
  doc["fertIdx"] = st.fertIdx;
  doc["dosing"] = st.dosing;
  doc["plantIdx"] = st.plantIdx;
  doc["plantRunning"] = st.plantRunning;
  doc["plantStartG"] = st.plantStartG;
  doc["retries"] = st.retries;
  return writeJson(STATE_PATH, doc);
}

void Storage::planToJson(JsonObject o) {
  char buf[12];
  o["active"] = plan.active;
  o["name"] = plan.name;
  snprintf(buf, sizeof buf, "%04u-%02u-%02u", plan.startY, plan.startM, plan.startD);
  o["start"] = buf;
  snprintf(buf, sizeof buf, "%02u:%02u", plan.hour, plan.minute);
  o["time"] = buf;
  o["numFert"] = plan.numFert;
  toArray(o["fertNames"].to<JsonArray>(), plan.fertNames);
  toArray(o["plants"].to<JsonArray>(), plan.plantOn);
  JsonArray weeks = o["weeks"].to<JsonArray>();
  for (uint8_t i = 0; i < plan.numWeeks; i++) {
    JsonObject w = weeks.add<JsonObject>();
    w["l"] = plan.weeks[i].litres;
    w["d"] = plan.weeks[i].days;
    toArray(w["f"].to<JsonArray>(), plan.weeks[i].fert);
  }
}

bool Storage::planFromJson(JsonObjectConst o, Plan &out, String &err) {
  defaultPlan(out);
  out.active = o["active"] | false;
  out.name = o["name"] | "Piano";
  out.name = out.name.substring(0, 30);

  int y, m, d, hh, mm;
  if (sscanf(o["start"] | "", "%d-%d-%d", &y, &m, &d) != 3 || y < 2020 || y > 2100 || m < 1 ||
      m > 12 || d < 1 || d > 31) {
    err = "Data di inizio non valida";
    return false;
  }
  if (sscanf(o["time"] | "", "%d:%d", &hh, &mm) != 2 || hh < 0 || hh > 23 || mm < 0 || mm > 59) {
    err = "Ora di irrigazione non valida";
    return false;
  }
  out.startY = y;
  out.startM = m;
  out.startD = d;
  out.hour = hh;
  out.minute = mm;

  out.numFert = o["numFert"] | 3;
  if (out.numFert < 1 || out.numFert > MAX_FERT) {
    err = "Numero di fertilizzanti non valido";
    return false;
  }
  JsonArrayConst names = o["fertNames"];
  for (uint8_t i = 0; i < MAX_FERT; i++) {
    String n = names[i] | out.fertNames[i].c_str();
    out.fertNames[i] = n.substring(0, 15);
  }
  fromArray(o["plants"], out.plantOn);

  JsonArrayConst weeks = o["weeks"];
  if (weeks.size() < 1 || weeks.size() > MAX_WEEKS) {
    err = "Il piano deve avere da 1 a " + String(MAX_WEEKS) + " settimane";
    return false;
  }
  out.numWeeks = weeks.size();
  for (uint8_t i = 0; i < out.numWeeks; i++) {
    Week &w = out.weeks[i];
    w.litres = weeks[i]["l"] | 0.0f;
    w.days = (weeks[i]["d"] | 0) & 0x7F;
    if (w.litres < 0 || w.litres > 20) {
      err = "Settimana " + String(i + 1) + ": litri non validi";
      return false;
    }
    for (uint8_t f = 0; f < MAX_FERT; f++) {
      w.fert[f] = f < out.numFert ? (weeks[i]["f"][f] | 0.0f) : 0;
      if (w.fert[f] < 0 || w.fert[f] > 50) {
        err = "Settimana " + String(i + 1) + ": dose fertilizzante non valida";
        return false;
      }
    }
  }
  return true;
}

void Storage::calibToJson(JsonObject o) {
  o["scaleOffset"] = calib.scaleOffset;
  o["scaleFactor"] = calib.scaleFactor;
  toArray(o["fertMlPerSec"].to<JsonArray>(), calib.fertMlPerSec);
  o["mixSeconds"] = calib.mixSeconds;
}

// Non tocca i dati della bilancia, che si impostano solo con tara e calibrazione
void Storage::calibFromJson(JsonObjectConst o) {
  JsonArrayConst rates = o["fertMlPerSec"];
  for (uint8_t i = 0; i < MAX_FERT; i++) {
    float r = rates[i] | calib.fertMlPerSec[i];
    if (r >= 0 && r < 100) calib.fertMlPerSec[i] = r;
  }
  uint16_t mix = o["mixSeconds"] | calib.mixSeconds;
  calib.mixSeconds = min<uint16_t>(mix, 120);
}
