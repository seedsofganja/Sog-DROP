#include "cycle.h"
#include "log.h"
#include "outputs.h"
#include "scale.h"
#include "storage.h"

namespace {
enum Step : uint8_t { S_START, S_RUN, S_SETTLE };
Step step = S_START;
uint32_t stepAt = 0;
uint32_t runMs = 0;
float stepStartG = 0;
float lastProgressG = 0;
uint32_t lastProgressAt = 0;
uint32_t bootAt = 0;

void setPhase(Phase p) {
  st.phase = p;
  step = S_START;
  stepAt = millis();
  Storage::saveState();
}

uint8_t activePlants() {
  uint8_t n = 0;
  for (uint8_t i = 0; i < NUM_PLANTS; i++)
    if (!st.plantSkip[i]) n++;
  return n;
}

float remainingPerPlant() {
  float rem = 0;
  for (uint8_t i = 0; i < NUM_PLANTS; i++)
    if (!st.plantSkip[i]) rem = max(rem, st.perPlantL - st.deliveredL[i]);
  return rem;
}

void finish() {
  Outputs::allOff();
  String summary;
  for (uint8_t i = 0; i < NUM_PLANTS; i++) {
    if (!plan.plantOn[i]) continue;
    summary += " P" + String(i + 1) + "=" + String(st.deliveredL[i], 2) + "L";
  }
  Log::info("Irrigazione completata:%s", summary.c_str());
  st.phase = PH_IDLE;
  if (st.cycleDay) st.lastDoneDay = st.cycleDay;
  Storage::saveState();
}

void progressReset(float g) {
  lastProgressG = g;
  lastProgressAt = millis();
}

void beginBatch() {
  uint8_t n = activePlants();
  float rem = remainingPerPlant();
  if (n == 0 || rem < 0.01f) {
    finish();
    return;
  }
  st.residualL = max(0.0f, Scale::grams() / 1000);
  float base = max(st.residualL, TANK_RESERVE_L);
  st.batchPerPlantL = min(rem, (TANK_MAX_L - base) / n);
  if (st.batchPerPlantL < 0.02f) {
    Cycle::abort("vasca già piena, impossibile preparare la soluzione");
    return;
  }
  float totalL = base + st.batchPerPlantL * n;
  // il concentrato occupa volume: si aggiunge meno acqua
  float fertMl = 0;
  for (uint8_t f = 0; f < MAX_FERT; f++)
    fertMl += max(0.0f, st.recipe[f] * totalL - st.tankConc[f] * st.residualL);
  st.fillTargetG = totalL * 1000 - fertMl;
  st.retries = 0;
  setPhase(PH_FILL);
}

void goDose() {
  float g = Scale::grams();
  float totalL = g / 1000;
  uint8_t n = activePlants();
  float base = max(st.residualL, TANK_RESERVE_L);
  // se il galleggiante ha interrotto il riempimento, il lotto si riduce
  st.batchPerPlantL = min(st.batchPerPlantL, max(0.0f, (totalL - base) / n));
  if (st.batchPerPlantL < 0.02f) {
    Cycle::abort("acqua insufficiente in vasca dopo il riempimento");
    return;
  }
  st.mixL = totalL;
  for (uint8_t f = 0; f < MAX_FERT; f++) {
    // la soluzione rimasta dal lotto precedente è stata diluita dall'acqua nuova
    st.tankConc[f] = totalL > 0 ? st.tankConc[f] * st.residualL / totalL : 0;
    st.doseMl[f] = max(0.0f, (st.recipe[f] - st.tankConc[f]) * totalL);
  }
  st.fertIdx = 0;
  st.dosing = false;
  setPhase(PH_DOSE);
}

void goDistribute() {
  st.plantIdx = 0;
  st.plantRunning = false;
  setPhase(PH_DISTRIBUTE);
}

void fillFault(const char *why) {
  Outputs::off(OUT_VALVE);
  st.retries++;
  if (st.retries > FILL_MAX_RETRIES) {
    Cycle::abort(why);
    return;
  }
  Log::warn("Riempimento: %s. Nuovo tentativo tra %lu minuti (%u/%u)", why,
            (unsigned long)(FILL_RETRY_MS / 60000), st.retries, FILL_MAX_RETRIES);
  setPhase(PH_WAIT_RETRY);
}

void plantDone(uint8_t p, float g, const char *fault) {
  float delivered = max(0.0f, st.plantStartG - g) / 1000;
  st.deliveredL[p] += delivered;
  if (fault) {
    Log::error("Pianta %u: %s (erogati %.2f L in questo lotto)", p + 1, fault, delivered);
    st.plantSkip[p] = true;
  }
  st.plantRunning = false;
  st.plantIdx++;
  Storage::saveState();
  step = S_START;
}

void updateFill(float g, uint32_t now) {
  switch (step) {
    case S_START:
      if (g >= st.fillTargetG - 5) {
        goDose();
      } else if (Scale::tankFull()) {
        Log::warn("Galleggiante di massimo scattato prima del previsto");
        goDose();
      } else {
        Outputs::on(OUT_VALVE);
        progressReset(g);
        step = S_RUN;
      }
      break;
    case S_RUN:
      if (g >= st.fillTargetG || Scale::tankFull()) {
        if (g < st.fillTargetG) Log::warn("Galleggiante di massimo scattato durante il riempimento");
        Outputs::off(OUT_VALVE);
        step = S_SETTLE;
        stepAt = now;
      } else if (!Outputs::isOn(OUT_VALVE)) {
        fillFault("superato il tempo massimo di riempimento");
      } else if (g - lastProgressG >= PROGRESS_MIN_G) {
        progressReset(g);
      } else if (now - lastProgressAt > NO_PROGRESS_MS) {
        fillFault("il peso non aumenta, manca acqua dal rubinetto?");
      }
      break;
    case S_SETTLE:
      if (now - stepAt > SETTLE_MS) goDose();
      break;
  }
}

void updateDose(float g, uint32_t now) {
  switch (step) {
    case S_START: {
      while (st.fertIdx < MAX_FERT && st.doseMl[st.fertIdx] < 0.1f) st.fertIdx++;
      if (st.fertIdx >= MAX_FERT) {
        goDistribute();  // ogni dosaggio è già stato seguito da una miscelazione
        break;
      }
      uint8_t f = st.fertIdx;
      if (calib.fertMlPerSec[f] <= 0) {
        Log::error("Fertilizzante %u non calibrato: saltato", f + 1);
        st.fertIdx++;
        break;
      }
      runMs = st.doseMl[f] / calib.fertMlPerSec[f] * 1000;
      // se la corrente salta durante il dosaggio, al riavvio questo fertilizzante
      // viene saltato: meglio una dose in meno che una doppia
      st.dosing = true;
      Storage::saveState();
      stepStartG = g;
      Outputs::on(OUT_FERT0 + f, runMs + 1000);
      stepAt = now;
      step = S_RUN;
      break;
    }
    case S_RUN:
      if (now - stepAt >= runMs) {
        Outputs::off(OUT_FERT0 + st.fertIdx);
        step = S_SETTLE;
        stepAt = now;
      }
      break;
    case S_SETTLE:
      if (now - stepAt > SETTLE_MS) {
        uint8_t f = st.fertIdx;
        float added = g - stepStartG;
        if (st.doseMl[f] >= 5 && added < st.doseMl[f] * 0.5f) {
          Log::warn("Fertilizzante %u: dosati %.1f ml ma il peso è salito solo di %.0f g. "
                    "Tanica vuota o tubo staccato?", f + 1, st.doseMl[f], added);
        } else {
          Log::info("Fertilizzante %u: %.1f ml", f + 1, st.doseMl[f]);
        }
        st.tankConc[f] += st.doseMl[f] / st.mixL;
        st.dosing = false;
        st.fertIdx++;
        setPhase(PH_MIX);
      }
      break;
  }
}

void updateDistribute(float g, uint32_t now) {
  uint8_t p = st.plantIdx;
  float targetG = st.batchPerPlantL * 1000;
  switch (step) {
    case S_START:
      while (st.plantIdx < NUM_PLANTS && st.plantSkip[st.plantIdx]) st.plantIdx++;
      p = st.plantIdx;
      if (p >= NUM_PLANTS) {
        beginBatch();  // lotto finito: ne serve un altro?
        break;
      }
      if (!st.plantRunning) {
        st.plantStartG = g;
        st.plantRunning = true;
        Storage::saveState();
      }
      // dopo un riavvio plantStartG è quello salvato: si conta quanto già erogato
      if (st.plantStartG - g >= targetG - 2) {
        plantDone(p, g, nullptr);
        break;
      }
      Outputs::on(OUT_PLANT0 + p);
      progressReset(g);
      step = S_RUN;
      break;
    case S_RUN:
      if (st.plantStartG - g >= targetG) {
        Outputs::off(OUT_PLANT0 + p);
        step = S_SETTLE;
        stepAt = now;
      } else if (!Outputs::isOn(OUT_PLANT0 + p)) {
        plantDone(p, g, "superato il tempo massimo della pompa");
      } else if (lastProgressG - g >= PROGRESS_MIN_G) {
        progressReset(g);
      } else if (now - lastProgressAt > NO_PROGRESS_MS) {
        Outputs::off(OUT_PLANT0 + p);
        plantDone(p, g, "il peso non scende, pompa bloccata o tubo ostruito?");
      } else if (g < TANK_RESERVE_L * 500) {
        Outputs::off(OUT_PLANT0 + p);
        plantDone(p, g, "vasca quasi vuota");
      }
      break;
    case S_SETTLE:
      if (now - stepAt > SETTLE_MS) plantDone(p, g, nullptr);
      break;
  }
}
}  // namespace

void Cycle::begin() {
  bootAt = millis();
  step = S_START;
  stepAt = millis();
  if (st.phase == PH_IDLE) return;
  Log::warn("Riavvio durante un'irrigazione: riprendo dalla fase \"%s\"", phaseName());
  if (st.phase == PH_DOSE && st.dosing) {
    Log::warn("Fertilizzante %u interrotto durante il dosaggio: saltato", st.fertIdx + 1);
    st.dosing = false;
    st.fertIdx++;
    Storage::saveState();
  }
}

bool Cycle::start(float perPlantL, const float *recipe, uint32_t day, const char *why) {
  if (busy()) return false;
  if (!Scale::ok() || !Scale::calibrated()) {
    Log::error("Irrigazione (%s) non avviata: bilancia non pronta o non calibrata", why);
    return false;
  }
  st.cycleDay = day;
  st.perPlantL = perPlantL;
  for (uint8_t f = 0; f < MAX_FERT; f++) st.recipe[f] = f < plan.numFert ? recipe[f] : 0;
  for (uint8_t i = 0; i < NUM_PLANTS; i++) {
    st.deliveredL[i] = 0;
    st.plantSkip[i] = !plan.plantOn[i];
  }
  st.dosing = false;
  st.plantRunning = false;
  String r;
  for (uint8_t f = 0; f < plan.numFert; f++) r += " " + String(st.recipe[f], 1);
  Log::info("Avvio irrigazione (%s): %.2f L per pianta, ricetta ml/L:%s", why, perPlantL,
            r.c_str());
  beginBatch();
  return true;
}

void Cycle::abort(const char *why) {
  Outputs::allOff();
  Log::error("Irrigazione interrotta: %s", why);
  st.phase = PH_IDLE;
  st.dosing = false;
  st.plantRunning = false;
  if (st.cycleDay) st.lastDoneDay = st.cycleDay;  // niente nuovi tentativi oggi
  Storage::saveState();
}

bool Cycle::busy() { return st.phase != PH_IDLE; }

const char *Cycle::phaseName() {
  switch (st.phase) {
    case PH_IDLE: return "in attesa";
    case PH_FILL: return "riempimento";
    case PH_WAIT_RETRY: return "attesa acqua dal rubinetto";
    case PH_DOSE: return "dosaggio fertilizzanti";
    case PH_MIX: return "miscelazione";
    case PH_DISTRIBUTE: return "irrigazione piante";
  }
  return "?";
}

void Cycle::update() {
  if (st.phase == PH_IDLE) return;
  uint32_t now = millis();

  if (!Scale::ok()) {
    if (now - bootAt > 10000) abort("la bilancia non risponde");
    return;
  }
  float g = Scale::grams();

  switch (st.phase) {
    case PH_FILL:
      updateFill(g, now);
      break;
    case PH_WAIT_RETRY:
      if (now - stepAt > FILL_RETRY_MS) setPhase(PH_FILL);
      break;
    case PH_DOSE:
      updateDose(g, now);
      break;
    case PH_MIX:
      if (step == S_START) {
        Outputs::on(OUT_MIX);
        step = S_RUN;
        stepAt = now;
      } else if (now - stepAt >= calib.mixSeconds * 1000UL) {
        Outputs::off(OUT_MIX);
        setPhase(PH_DOSE);  // prossimo fertilizzante o distribuzione
      }
      break;
    case PH_DISTRIBUTE:
      updateDistribute(g, now);
      break;
    default:
      break;
  }
}
