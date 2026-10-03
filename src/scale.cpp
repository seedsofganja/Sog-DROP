#include "scale.h"
#include <HX711.h>
#include "config.h"
#include "storage.h"

namespace {
HX711 hx;
int32_t samples[3];
uint8_t count = 0;
uint8_t next = 0;
int32_t filtered = 0;
uint32_t lastAt = 0;

int32_t median3(int32_t a, int32_t b, int32_t c) {
  return max(min(a, b), min(max(a, b), c));
}
}  // namespace

void Scale::begin() {
  hx.begin(PIN_HX711_DOUT, PIN_HX711_SCK);
  pinMode(PIN_FLOAT_MAX, INPUT_PULLUP);
}

void Scale::update() {
  if (!hx.is_ready()) return;
  int32_t v = hx.read();
  samples[next] = v;
  next = (next + 1) % 3;
  if (count < 3) count++;
  // la mediana scarta i picchi dovuti ai disturbi delle pompe
  filtered = count < 3 ? v : median3(samples[0], samples[1], samples[2]);
  lastAt = millis();
}

bool Scale::ok() { return lastAt != 0 && millis() - lastAt < 2000; }

bool Scale::calibrated() { return calib.scaleFactor != 0; }

float Scale::grams() {
  if (!calibrated()) return 0;
  return (filtered - calib.scaleOffset) / calib.scaleFactor;
}

int32_t Scale::raw() { return filtered; }

void Scale::tare() {
  calib.scaleOffset = filtered;
  Storage::saveCalib();
}

bool Scale::calibrate(float knownGrams) {
  if (knownGrams <= 0) return false;
  float factor = (filtered - calib.scaleOffset) / knownGrams;
  if (fabsf(factor) < 1) return false;  // nessuna variazione: peso non appoggiato?
  calib.scaleFactor = factor;
  Storage::saveCalib();
  return true;
}

bool Scale::tankFull() { return digitalRead(PIN_FLOAT_MAX) == HIGH; }
