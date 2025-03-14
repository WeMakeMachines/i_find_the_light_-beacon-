#include "unit.h"
#include "i_find_the_light-beacon-.h"
#include "station_api.h"
#include "wifi_config.h" // Make sure this has been configured!

#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <Adafruit_VEML7700.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <RTClib.h>

// unique device reference
const char *name = "0001";

// DS1307 RTC
RTC_DS1307 rtc;

// DS18B20 3 pin temperature sensor
const int oneWireBus = 4; // GPIO pin for DS18B20 data line
OneWire oneWire(oneWireBus);
Adafruit_VEML7700 veml = Adafruit_VEML7700();
DallasTemperature sensors(&oneWire);

int poll_interval;
uint64_t rtc_calibrate_timestamp;
int beacon_id;
enum Unit unit;

uint64_t getTimestampInMilliseconds()
{
  DateTime now = rtc.now();
  return static_cast<uint64_t>(now.unixtime()) * 1000ULL;
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

  return {temperature, lux};
}

void setup()
{
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  HandshakeConfig config = httpRequestHandshake(name);

  poll_interval = config.poll_interval;
  rtc_calibrate_timestamp = config.rtc_calibration;
  beacon_id = config.beacon_id;
  unit = config.unit;

  // Initialise I2C for VEML7700 and DS1307
  Wire.begin(32, 25);
  if (!veml.begin())
  {
    Serial.println("Failed to initialise VEML7700 sensor!");
    while (1)
      ;
  }
  veml.setGain(VEML7700_GAIN_1_8);
  veml.setIntegrationTime(VEML7700_IT_800MS);
  Serial.println("VEML7700 sensor initialised.");

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }

  // adjust RTC with calibration timestamp from station
  uint32_t timestamp_seconds = static_cast<uint32_t>(rtc_calibrate_timestamp / 1000);
  DateTime dt(timestamp_seconds);
  rtc.adjust(dt);

  // Initialise DS18B20 sensor
  sensors.begin();
  Serial.println("DS18B20 sensor initialised.");
}

void loop()
{
  esp_wifi_start();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  SensorData data = pollSensors(unit);

  httpRequestReadings({
    beacon_id : beacon_id,
    lux : data.lux,
    temperature : data.temperature,
    timestamp : getTimestampInMilliseconds(),
    unit : unit
  });

  // Optionally, log data to the serial monitor for debugging
  Serial.print("Ambient Light: ");
  Serial.print(data.lux);
  Serial.print(" lux, Temperature: ");
  Serial.print(data.temperature);
  Serial.println(" °C");

  esp_sleep_enable_timer_wakeup(poll_interval * 1000000); // light sleep for 2 seconds
  esp_wifi_stop();
  esp_light_sleep_start();
}