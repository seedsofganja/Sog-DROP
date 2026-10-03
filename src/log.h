#pragma once
#include <ArduinoJson.h>

namespace Log {
void begin();
void info(const char *fmt, ...);
void warn(const char *fmt, ...);
void error(const char *fmt, ...);
void alertsToJson(JsonArray arr);  // ultimi avvisi ed errori (dal riavvio)
void clearAlerts();
bool hasAlerts();
}
