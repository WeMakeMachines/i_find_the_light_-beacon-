#include "unit.h"
#include "esp32_rtc.h"
#include "sensors.h"
#include "station_api.h"
#include "wifi_config.h" // Make sure this has been configured!

#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <RTClib.h>

// this device unique reference
const char *name = "0001"; // Must be unique for each beacon

// DS1307 RTC
RTC_DS1307 rtc;

uint64_t getTimestampInMilliseconds()
{
  DateTime now = rtc.now();
  return static_cast<uint64_t>(now.unixtime()) * 1000ULL;
}

void setup()
{
  Serial.begin(115200);
  // Wire.begin(SDA, SCL)
  Wire.begin(32, 25);
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

  setRtcDataAttr({config.beacon_id,
                  config.poll_interval,
                  config.schedule_start,
                  config.schedule_end,
                  config.unit});

  initLightSensor();
  initTempSensor();

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }

  // adjust RTC with calibration timestamp from station
  uint32_t timestamp_seconds = static_cast<uint32_t>(config.rtc_calibration / 1000);
  DateTime dt(timestamp_seconds);
  rtc.adjust(dt);
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

  SensorData data = pollSensors(getUnit());

  httpRequestReadings({
    beacon_id : getBeaconId(),
    lux : data.lux,
    temperature : data.temperature,
    timestamp : getTimestampInMilliseconds(),
    unit : getUnit()
  });

  // Optionally, log data to the serial monitor for debugging
  Serial.print("Ambient Light: ");
  Serial.print(data.lux);
  Serial.print(" lux, Temperature: ");
  Serial.print(data.temperature);
  Serial.println(" °C");

  esp_sleep_enable_timer_wakeup(getPollInterval() * 1000000);
  esp_wifi_stop();
  esp_light_sleep_start();
}