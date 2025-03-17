#include "rtc_DS1307.h"

#include <RTClib.h>
#include <cstdint>

// DS1307 RTC
RTC_DS1307 rtc_DS1307;

bool initRtc()
{
    return rtc_DS1307.begin();
}

void calibrateRtc(uint32_t time)
{
    // adjust RTC with calibration timestamp from station
    uint32_t timestamp_seconds = time;
    DateTime dt(timestamp_seconds);
    rtc_DS1307.adjust(dt);
}

uint32_t getCurrentUnixTimeInSeconds()
{
    DateTime now = rtc_DS1307.now();
    return now.unixtime();
}

uint32_t calcTimeDelta(uint32_t time_in, uint32_t time_out)
{
    if (time_out < time_in)
    {
        return 0;
    }
    return time_out - time_in;
}
