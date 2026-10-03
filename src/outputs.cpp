#include "outputs.h"

namespace {
bool state[OUT_COUNT];
uint32_t startedAt[OUT_COUNT];
uint32_t limitMs[OUT_COUNT];

uint8_t pinOf(uint8_t id) {
  if (id == OUT_VALVE) return PIN_VALVE;
  if (id == OUT_MIX) return PIN_MIX;
  if (id < OUT_FERT0) return PIN_PLANT[id - OUT_PLANT0];
  return PIN_FERT[id - OUT_FERT0];
}

uint32_t defaultLimit(uint8_t id) {
  if (id == OUT_VALVE) return MAX_ON_VALVE_MS;
  if (id == OUT_MIX) return MAX_ON_MIX_MS;
  if (id < OUT_FERT0) return MAX_ON_PLANT_MS;
  return MAX_ON_FERT_MS;
}

void write(uint8_t id, bool on) {
  digitalWrite(pinOf(id), (on != RELAY_ACTIVE_LOW) ? HIGH : LOW);
}
}  // namespace

void Outputs::begin() {
  for (uint8_t i = 0; i < OUT_COUNT; i++) {
    write(i, false);  // livello "spento" impostato prima di abilitare l'uscita
    pinMode(pinOf(i), OUTPUT);
    write(i, false);
    state[i] = false;
  }
}

void Outputs::on(uint8_t id, uint32_t maxMs) {
  if (id >= OUT_COUNT) return;
  uint32_t def = defaultLimit(id);
  limitMs[id] = (maxMs > 0 && maxMs < def) ? maxMs : def;
  startedAt[id] = millis();
  state[id] = true;
  write(id, true);
}

void Outputs::off(uint8_t id) {
  if (id >= OUT_COUNT) return;
  state[id] = false;
  write(id, false);
}

void Outputs::allOff() {
  for (uint8_t i = 0; i < OUT_COUNT; i++) off(i);
}

bool Outputs::isOn(uint8_t id) { return id < OUT_COUNT && state[id]; }

bool Outputs::anyOn() {
  for (uint8_t i = 0; i < OUT_COUNT; i++)
    if (state[i]) return true;
  return false;
}

void Outputs::update() {
  uint32_t now = millis();
  for (uint8_t i = 0; i < OUT_COUNT; i++) {
    if (state[i] && now - startedAt[i] >= limitMs[i]) off(i);
  }
}

String Outputs::name(uint8_t id) {
  if (id == OUT_VALVE) return "Valvola rubinetto";
  if (id == OUT_MIX) return "Pompa ricircolo";
  if (id < OUT_FERT0) return "Pompa pianta " + String(id - OUT_PLANT0 + 1);
  return "Fertilizzante " + String(id - OUT_FERT0 + 1);
}
