#pragma once
#include <ArduinoJson.h>
#include <RTClib.h>

// Calendario: decide quando avviare un ciclo in base al piano settimanale
namespace Schedule {
int weekOf(const DateTime &d);        // 1..numWeeks, 0 prima dell'inizio, -1 dopo la fine
float litresPerIrrigation(int week);  // litri per pianta di ogni singola irrigazione
void update();
void statusToJson(JsonObject o);
}
