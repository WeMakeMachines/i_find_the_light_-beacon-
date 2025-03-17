#include "i_find_the_light-beacon-.h"

#include "unit.h"
#include "esp32_rtc.h"
#include "sensors.h"
#include "station_api.h"
#include "wifi_config.h" // Make sure this has been configured!

#include <Wire.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <stdexcept>

#include <RTClib.h>

// this device unique reference
const char *name = "0001"; // Must be unique for each beacon

// DS1307 RTC
RTC_DS1307 rtc;

bool isFirstBoot()
{
  esp_reset_reason_t resetReason = esp_reset_reason();

  return (resetReason == ESP_RST_POWERON);
}

uint64_t getTimestampInMilliseconds()
{
  DateTime now = rtc.now();
  return static_cast<uint64_t>(now.unixtime()) * 1000ULL;
}

void attemptHandshake()
{
  try
  {
    Serial.println("Handshaking with station...");
    HandshakeConfig config = httpRequestHandshake(name);

    setRtcDataAttr({config.beacon_id,
                    config.poll_interval,
                    config.schedule_start,
                    config.schedule_end,
                    config.unit});

    // adjust RTC with calibration timestamp from station
    uint32_t timestamp_seconds = static_cast<uint32_t>(config.rtc_calibration / 1000);
    DateTime dt(timestamp_seconds);
    rtc.adjust(dt);
  }
  catch (const StationAPI_ConnectionError &e)
  {
    Serial.println(e.what());
    Serial.println("Will try again in 5s...");
    // wait before attempting handshake again
    delay(5000);
    attemptHandshake();
  }
}

void setup()
{
  Serial.begin(115200);
  if (isFirstBoot())
  {
    Serial.println("First time?");
    unsetRtcDataAttr();
  }
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

  Serial.print("Beacon ID: ");
  Serial.println(getBeaconId());

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }

  // check if config has already been set
  if (getBeaconId() == UNSET_BEACON_ID)
  {
    Serial.println("Beacon ID not set.");
    attemptHandshake();
  }

  initLightSensor();
  initTempSensor();
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

  try
  {
    httpRequestReadings({
      beacon_id : getBeaconId(),
      lux : data.lux,
      temperature : data.temperature,
      timestamp : getTimestampInMilliseconds(),
      unit : getUnit()
    });

    Serial.print("Lux: ");
    Serial.println(data.lux);
    Serial.print("Temperature: ");
    Serial.print(data.temperature);
    if (getUnit() == Unit::Metric)
    {
      Serial.println(" °C");
    }
    else
    {
      Serial.println(" °F");
    }
  }
  catch (const StationAPI_ConnectionError &e)
  {
    Serial.println("Could not upload data to server.");
    Serial.println(e.what());
  }

  esp_sleep_enable_timer_wakeup(getPollInterval() * uS_TO_S_FACTOR);
  esp_wifi_stop();
  esp_deep_sleep_start();
}