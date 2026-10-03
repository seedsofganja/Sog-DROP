#pragma once
#include "config.h"

enum OutputId : uint8_t {
  OUT_VALVE = 0,
  OUT_MIX,
  OUT_PLANT0,
  OUT_FERT0 = OUT_PLANT0 + NUM_PLANTS,
  OUT_COUNT = OUT_FERT0 + MAX_FERT
};

namespace Outputs {
void begin();
// maxMs = 0 usa il limite di sicurezza dell'uscita; valori maggiori vengono ridotti
void on(uint8_t id, uint32_t maxMs = 0);
void off(uint8_t id);
void allOff();
bool isOn(uint8_t id);
bool anyOn();
void update();  // spegne le uscite che superano il tempo massimo
String name(uint8_t id);
}
