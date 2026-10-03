#include <Arduino.h>
#include <esp_task_wdt.h>
#include "clock.h"
#include "config.h"
#include "cycle.h"
#include "log.h"
#include "outputs.h"
#include "scale.h"
#include "schedule.h"
#include "storage.h"
#include "web.h"

namespace {
// LED: acceso fisso durante l'irrigazione, lampeggio veloce se ci sono avvisi,
// lampeggio lento se è tutto regolare
void updateLed() {
  uint32_t period = Cycle::busy() ? 0 : (Log::hasAlerts() || !Clock::ok()) ? 200 : 1000;
  digitalWrite(PIN_LED, period == 0 || (millis() / period) % 2 ? HIGH : LOW);
}
}  // namespace

void setup() {
  Outputs::begin();  // per prima cosa: tutte le pompe e la valvola spente
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);

  bool fsOk = Storage::begin();
  bool rtcOk = Clock::begin();
  Storage::loadAll();
  Log::begin();
  Log::info("Avvio sistema");
  if (!fsOk) Log::error("Memoria interna non disponibile");
  if (!rtcOk) Log::error("Orologio RTC non trovato: controlla i collegamenti");
  else if (!Clock::ok()) Log::error("Orologio da impostare (batteria RTC scarica o primo avvio)");

  Scale::begin();
  Web::begin();
  Cycle::begin();

  esp_task_wdt_init(WDT_TIMEOUT_S, true);  // riavvio automatico se il programma si blocca
  esp_task_wdt_add(NULL);
}

void loop() {
  esp_task_wdt_reset();
  Scale::update();
  // sicurezza: con il galleggiante alzato la valvola resta chiusa, qualunque cosa accada
  if (Scale::tankFull() && Outputs::isOn(OUT_VALVE)) Outputs::off(OUT_VALVE);
  Outputs::update();
  Cycle::update();
  Schedule::update();
  Web::update();
  updateLed();
}
