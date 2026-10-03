#include "log.h"
#include <LittleFS.h>
#include <stdarg.h>
#include "clock.h"
#include "config.h"

namespace {
const char *LOG_PATH = "/log.txt";
const char *LOG_OLD = "/log.old";
constexpr uint8_t MAX_ALERTS = 10;

struct Alert {
  String time;
  String msg;
  bool error;
};
Alert alerts[MAX_ALERTS];
uint8_t alertCount = 0;
uint8_t alertNext = 0;

String timestamp() {
  if (Clock::ok()) return Clock::format(Clock::nowLocal());
  return "T+" + String(millis() / 1000) + "s";
}

void write(const char *level, bool alert, bool isError, const char *fmt, va_list ap) {
  char msg[200];
  vsnprintf(msg, sizeof msg, fmt, ap);
  String ts = timestamp();
  String line = ts + " " + level + " " + msg;
  Serial.println(line);

  File f = LittleFS.open(LOG_PATH, "a");
  if (f) {
    f.println(line);
    size_t size = f.size();
    f.close();
    if (size > LOG_MAX_BYTES) {
      LittleFS.remove(LOG_OLD);
      LittleFS.rename(LOG_PATH, LOG_OLD);
    }
  }

  if (alert) {
    alerts[alertNext] = {ts, msg, isError};
    alertNext = (alertNext + 1) % MAX_ALERTS;
    if (alertCount < MAX_ALERTS) alertCount++;
  }
}
}  // namespace

void Log::begin() {}

void Log::info(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  write("INFO", false, false, fmt, ap);
  va_end(ap);
}

void Log::warn(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  write("AVVISO", true, false, fmt, ap);
  va_end(ap);
}

void Log::error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  write("ERRORE", true, true, fmt, ap);
  va_end(ap);
}

void Log::alertsToJson(JsonArray arr) {
  // dal più recente al più vecchio
  for (uint8_t i = 0; i < alertCount; i++) {
    const Alert &a = alerts[(alertNext + MAX_ALERTS - 1 - i) % MAX_ALERTS];
    JsonObject o = arr.add<JsonObject>();
    o["t"] = a.time;
    o["m"] = a.msg;
    o["e"] = a.error;
  }
}

void Log::clearAlerts() {
  alertCount = 0;
  alertNext = 0;
}

bool Log::hasAlerts() { return alertCount > 0; }
