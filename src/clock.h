#pragma once
#include <RTClib.h>

namespace Clock {
bool begin();
bool ok();                 // RTC presente e ora impostata
DateTime nowLocal();       // ora locale (fuso + ora legale)
void setUtc(uint32_t epoch);
uint32_t dayNumber(const DateTime &d);   // aaaammgg
uint8_t weekdayMon0(const DateTime &d);  // 0 = lunedì ... 6 = domenica
String format(const DateTime &d);        // "aaaa-mm-gg hh:mm"
}
