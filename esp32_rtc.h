#ifndef ESP32_RTC_H
#define ESP32_RTC_H

#include "esp_attr.h"
#include "unit.h"
#include <cstdint>

const int UNSET_BEACON_ID = -1;

// define RTCData used for handshake config
struct RTCData
{
    int beacon_id;
    int poll_interval_seconds;
    uint32_t schedule_start;
    uint32_t schedule_end;
    Unit unit;
};

extern RTC_DATA_ATTR RTCData rtc_data;

void unsetRtcDataAttr();

void setRtcDataAttr(RTCData new_rtc_values);

int getBeaconId();

int getPollIntervalSeconds();

uint32_t getScheduleStart();

uint32_t getScheduleEnd();

Unit getUnit();

#endif