#pragma once
#include <Arduino.h>

// Ciclo di fertirrigazione: riempimento -> dosaggio -> miscelazione -> distribuzione.
// Se la vasca non basta, il ciclo si ripete a lotti finché ogni pianta ha avuto la sua dose.
namespace Cycle {
void begin();   // riprende un ciclo interrotto da un'interruzione di corrente
void update();
// day = aaaammgg per i cicli programmati, 0 per quelli manuali
bool start(float perPlantL, const float *recipe, uint32_t day, const char *why);
void abort(const char *why);
bool busy();
const char *phaseName();
}
