#include "esp32_rtc.h"

RTC_DATA_ATTR RTCData rtc_data = {-1, -1, 0, 0};

void setRtcDataAttr(RTCData new_rtc_values)
{
    rtc_data = new_rtc_values;
}

RTCData readRtcDataAtt()
{
    return rtc_data;
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