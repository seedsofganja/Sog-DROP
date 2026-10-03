#include "clock.h"
#include <Wire.h>
#include "config.h"

namespace {
RTC_DS3231 rtc;
bool found = false;

// Ora legale UE: dall'ultima domenica di marzo all'ultima di ottobre, alle 01:00 UTC
bool euDst(const DateTime &utc) {
  uint16_t y = utc.year();
  DateTime start(y, 3, 31 - DateTime(y, 3, 31).dayOfTheWeek(), 1, 0, 0);
  DateTime end(y, 10, 31 - DateTime(y, 10, 31).dayOfTheWeek(), 1, 0, 0);
  return utc.unixtime() >= start.unixtime() && utc.unixtime() < end.unixtime();
}
}  // namespace

bool Clock::begin() {
  Wire.begin();
  found = rtc.begin();
  return found;
}

bool Clock::ok() { return found && !rtc.lostPower(); }

DateTime Clock::nowLocal() {
  DateTime utc = found ? rtc.now() : DateTime(2000, 1, 1);
  int32_t offset = TZ_OFFSET_MIN * 60 + ((USE_EU_DST && euDst(utc)) ? 3600 : 0);
  return DateTime(utc.unixtime() + offset);
}

void Clock::setUtc(uint32_t epoch) {
  if (found) rtc.adjust(DateTime(epoch));
}

uint32_t Clock::dayNumber(const DateTime &d) {
  return d.year() * 10000UL + d.month() * 100UL + d.day();
}

uint8_t Clock::weekdayMon0(const DateTime &d) { return (d.dayOfTheWeek() + 6) % 7; }

String Clock::format(const DateTime &d) {
  char buf[20];
  snprintf(buf, sizeof buf, "%04d-%02d-%02d %02d:%02d", d.year(), d.month(), d.day(), d.hour(),
           d.minute());
  return buf;
}
