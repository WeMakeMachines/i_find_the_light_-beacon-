#include "sensors.h"

#include <Adafruit_VEML7700.h>
#include <DallasTemperature.h>
#include <OneWire.h>

Adafruit_VEML7700 veml = Adafruit_VEML7700();

const int oneWireBus = 4; // GPIO pin for DS18B20 data line
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void initLightSensor()
{
    // VEML7700
    if (!veml.begin())
    {
        Serial.println("Failed to initialise VEML7700 sensor!");
        while (1)
            ;
    }
    veml.setGain(VEML7700_GAIN_1_8);
    veml.setIntegrationTime(VEML7700_IT_800MS);
    Serial.println("VEML7700 sensor initialised.");
}

void initTempSensor()
{
    // DS18B20 3 pin temperature sensor
    sensors.begin();
    Serial.println("DS18B20 sensor initialised.");
}

SensorData pollSensors(Unit unit)
{
    int DS18B20_index = 0;
    float temperature;
    float lux = veml.readLux();
    sensors.requestTemperatures();

    switch (unit)
    {
    case Unit::Imperial:
        temperature = sensors.getTempFByIndex(DS18B20_index);
        break;
    default:
        temperature = sensors.getTempCByIndex(DS18B20_index);
    }

    return {
        lux,
        temperature,
    };
}