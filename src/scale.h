#pragma once
#include <Arduino.h>

// Cella di carico sotto la vasca di miscelazione (HX711) + galleggiante di massimo
namespace Scale {
void begin();
void update();           // da chiamare a ogni loop
bool ok();               // letture recenti valide
bool calibrated();
float grams();           // contenuto netto della vasca
int32_t raw();
void tare();             // da eseguire con la vasca vuota
bool calibrate(float knownGrams);
bool tankFull();         // galleggiante di massimo scattato (o filo staccato)
}
