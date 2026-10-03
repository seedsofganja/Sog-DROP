#pragma once
#include <ArduinoJson.h>
#include "config.h"

struct Week {
  float litres;           // litri per pianta nella settimana
  uint8_t days;           // giorni di irrigazione: bit0 = lunedì ... bit6 = domenica
  float fert[MAX_FERT];   // ml di concentrato per litro d'acqua
};

struct Plan {
  bool active;
  String name;
  uint16_t startY;
  uint8_t startM, startD;
  uint8_t hour, minute;
  uint8_t numFert;
  String fertNames[MAX_FERT];
  bool plantOn[NUM_PLANTS];
  uint8_t numWeeks;
  Week weeks[MAX_WEEKS];
};

struct Calib {
  int32_t scaleOffset;
  float scaleFactor;              // 0 = bilancia non calibrata
  float fertMlPerSec[MAX_FERT];   // 0 = pompa non calibrata
  uint16_t mixSeconds;
};

enum Phase : uint8_t { PH_IDLE, PH_FILL, PH_WAIT_RETRY, PH_DOSE, PH_MIX, PH_DISTRIBUTE };

// Stato persistente: permette di riprendere un ciclo dopo un'interruzione di corrente
struct CycleState {
  uint32_t lastDoneDay;          // aaaammgg dell'ultimo ciclo programmato gestito
  bool skipNext;
  float tankConc[MAX_FERT];      // ml/L di ogni fertilizzante nella soluzione in vasca

  Phase phase;
  uint32_t cycleDay;             // 0 = irrigazione manuale
  float perPlantL;               // litri da dare a ogni pianta in questo ciclo
  float recipe[MAX_FERT];        // ml/L richiesti
  float deliveredL[NUM_PLANTS];
  bool plantSkip[NUM_PLANTS];    // pianta esclusa o in guasto
  float batchPerPlantL;          // litri per pianta nel lotto corrente
  float residualL;               // soluzione già in vasca a inizio lotto
  float fillTargetG;
  float mixL;                    // volume del lotto dopo il riempimento
  float doseMl[MAX_FERT];
  uint8_t fertIdx;
  bool dosing;                   // pompa peristaltica avviata e non ancora confermata
  uint8_t plantIdx;
  bool plantRunning;
  float plantStartG;
  uint8_t retries;
};

extern Plan plan;
extern Calib calib;
extern CycleState st;

namespace Storage {
bool begin();
void loadAll();
bool savePlan();
bool saveCalib();
bool saveState();
void planToJson(JsonObject o);
bool planFromJson(JsonObjectConst o, Plan &out, String &err);
void calibToJson(JsonObject o);
void calibFromJson(JsonObjectConst o);
}
