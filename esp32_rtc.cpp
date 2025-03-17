#include "esp32_rtc.h"

RTC_DATA_ATTR RTCData rtc_data;

void unsetRtcDataAttr()
{
    rtc_data = {UNSET_BEACON_ID, 0, 0, 0};
}

void setRtcDataAttr(RTCData new_rtc_values)
{
    rtc_data = new_rtc_values;
}

int getBeaconId()
{
    return rtc_data.beacon_id;
}

int getPollInterval()
{
    return rtc_data.poll_interval;
}

uint64_t getScheduleStart()
{
    return rtc_data.schedule_start;
}

uint64_t getScheduleEnd()
{
    return rtc_data.schedule_end;
}

Unit getUnit()
{
    return rtc_data.unit;
};