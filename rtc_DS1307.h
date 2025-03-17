#ifndef RTC_DS1307_H
#define RTC_DS1307_H

#include <cstdint>

bool initRtc();

void calibrateRtc(uint32_t time);

uint32_t getCurrentUnixTimeInSeconds();

uint32_t calcTimeDelta(uint32_t time_in, uint32_t time_out);

#endif