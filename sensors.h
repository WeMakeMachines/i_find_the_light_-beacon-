#ifndef SENSORS_H
#define SENSORS_H

#include "unit.h"

struct SensorData
{
    float lux;
    float temperature;
};

void initLightSensor();

void initTempSensor();

SensorData pollSensors(Unit unit);

#endif