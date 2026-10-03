#include "schedule.h"
#include "clock.h"
#include "cycle.h"
#include "log.h"
#include "storage.h"

namespace {
uint32_t lastCheck = 0;
bool clockWarned = false;

bool isIrrigationDay(const DateTime &d, int week) {
  if (week < 1) return false;
  return (plan.weeks[week - 1].days >> Clock::weekdayMon0(d)) & 1;
}

bool timeReached(const DateTime &d) {
  return d.hour() * 60 + d.minute() >= plan.hour * 60 + plan.minute;
}
}  // namespace

int Schedule::weekOf(const DateTime &d) {
  DateTime start(plan.startY, plan.startM, plan.startD);
  DateTime day(d.year(), d.month(), d.day());
  int32_t days = ((int32_t)day.unixtime() - (int32_t)start.unixtime()) / 86400;
  if (days < 0) return 0;
  int week = days / 7 + 1;
  return week > plan.numWeeks ? -1 : week;
}

float Schedule::litresPerIrrigation(int week) {
  if (week < 1 || week > plan.numWeeks) return 0;
  const Week &w = plan.weeks[week - 1];
  int n = __builtin_popcount(w.days);
  return n ? w.litres / n : 0;
}

void Schedule::update() {
  if (millis() - lastCheck < 5000) return;
  lastCheck = millis();
  if (!plan.active || Cycle::busy()) return;

  if (!Clock::ok()) {
    if (!clockWarned) Log::error("Ora non impostata: le irrigazioni programmate sono sospese");
    clockWarned = true;
    return;
  }
  clockWarned = false;

  DateTime now = Clock::nowLocal();
  uint32_t today = Clock::dayNumber(now);
  if (st.lastDoneDay == today || !timeReached(now)) return;

  int week = weekOf(now);
  float litres = litresPerIrrigation(week);
  if (!isIrrigationDay(now, week) || litres <= 0) return;

  if (st.skipNext) {
    Log::info("Irrigazione di oggi saltata su richiesta");
    st.skipNext = false;
    st.lastDoneDay = today;
    Storage::saveState();
    return;
  }

  char why[24];
  snprintf(why, sizeof why, "settimana %d", week);
  if (!Cycle::start(litres, plan.weeks[week - 1].fert, today, why)) {
    st.lastDoneDay = today;  // errore già registrato: non riprovare ogni 5 secondi
    Storage::saveState();
  }
}

void Schedule::statusToJson(JsonObject o) {
  o["planActive"] = plan.active;
  o["planName"] = plan.name;
  o["numWeeks"] = plan.numWeeks;
  if (!Clock::ok()) return;

  DateTime now = Clock::nowLocal();
  int week = weekOf(now);
  o["week"] = week;
  if (!plan.active) return;

  DateTime today(now.year(), now.month(), now.day());
  for (int k = 0; k < 7 * MAX_WEEKS; k++) {
    DateTime d(today.unixtime() + k * 86400UL);
    int w = weekOf(d);
    if (w < 0) break;
    float litres = litresPerIrrigation(w);
    if (!isIrrigationDay(d, w) || litres <= 0) continue;
    if (k == 0 && st.lastDoneDay == Clock::dayNumber(now)) continue;
    char buf[20];
    snprintf(buf, sizeof buf, "%02d/%02d/%04d %02u:%02u", d.day(), d.month(), d.year(), plan.hour,
             plan.minute);
    o["next"] = buf;
    o["nextL"] = litres;
    break;
  }
}
