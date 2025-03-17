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
    int poll_interval;
    uint64_t schedule_start;
    uint64_t schedule_end;
    Unit unit;
};

extern RTC_DATA_ATTR RTCData rtc_data;

void unsetRtcDataAttr();

void setRtcDataAttr(RTCData new_rtc_values);

int getBeaconId();

int getPollInterval();

uint64_t getScheduleStart();

uint64_t getScheduleEnd();

Unit getUnit();

#endif