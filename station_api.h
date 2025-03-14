#ifndef STATION_API_H
#define STATION_API_H

#define POST_HANDSHAKE_URL "http://192.168.50.1:3111/handshake"
#define POST_READINGS_URL "http://192.168.50.1:3111/readings"

#include "unit.h"
#include <cstdint>

struct HandshakeConfig {
  int beacon_id;
  uint64_t rtc_calibration;
  int poll_interval;
  uint64_t schedule_start;
  uint64_t schedule_end;
  Unit unit;
};

struct Reading {
  int beacon_id;
  float lux;
  float temperature;
  uint64_t timestamp;
  Unit unit;
};

HandshakeConfig httpRequestHandshake(const char* name);

void httpRequestReadings(Reading reading);

#endif