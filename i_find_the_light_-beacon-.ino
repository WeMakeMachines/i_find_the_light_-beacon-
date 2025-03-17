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
const char *name = "0001"; // Must be unique for each beacon!

// DS1307 RTC
RTC_DS1307 rtc;

int retryDelay = 5000;
int retryCount = 0;

bool isFirstBoot()
{
  esp_reset_reason_t resetReason = esp_reset_reason();

  return (resetReason == ESP_RST_POWERON);
}

uint32_t getCurrentUnixTimeInSeconds()
{
  DateTime now = rtc.now();
  return now.unixtime();
}

uint32_t calcTimeDelta(uint32_t time_in, uint32_t time_out)
{
  if (time_out < time_in)
  {
    return 0;
  }
  return time_out - time_in;
}

void attemptHandshake()
{
  try
  {
    Serial.println("Handshaking with station...");
    HandshakeConfig config = httpRequestHandshake(name);

    setRtcDataAttr({config.beacon_id,
                    config.poll_interval_seconds,
                    config.schedule_start,
                    config.schedule_end,
                    config.unit});

    // adjust RTC with calibration timestamp from station
    uint32_t timestamp_seconds = config.rtc_calibration;
    DateTime dt(timestamp_seconds);
    rtc.adjust(dt);
  }
  catch (const StationAPI_ConnectionError &e)
  {
    // wait before attempting handshake again
    Serial.println(e.what());
    Serial.print("Will try again in ");
    Serial.print(retryDelay / 1000);
    Serial.println("s...");
    // don't increase retryCount forever
    if (retryCount < 11)
    {
      retryCount += 1;
    }
    // after some attemps, increase delay
    if (retryCount > 10)
    {
      retryDelay = API_HANDSHAKE_RETRY_DELAY;
    }
    delay(retryDelay);
    attemptHandshake();
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  if (isFirstBoot())
  {
    Serial.println("First time?");
    unsetRtcDataAttr();
  }
  Wire.begin(I2C_SDA, I2C_SCL);
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_RETRY_DELAY);
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

  // sleep before scheduled wake up
  uint32_t timeToSleep = 0;

  if (getScheduleStart() > 0 && calcTimeDelta(getCurrentUnixTimeInSeconds(), getScheduleStart()) > 0)
  {
    timeToSleep = calcTimeDelta(getCurrentUnixTimeInSeconds(), getScheduleStart());
    Serial.print("Will sleep for: ");
    Serial.print(timeToSleep);
    Serial.println(" seconds.");

    esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
    esp_wifi_stop();
    esp_deep_sleep_start();
  }

  if (getScheduleEnd() < getCurrentUnixTimeInSeconds())
  {
    Serial.print("Schedule end. Powering down.");
    esp_deep_sleep_start();
  }
}

void loop()
{
  esp_wifi_start();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_RETRY_DELAY);
    Serial.print(".");
  }

  SensorData data = pollSensors(getUnit());

  try
  {
    httpRequestReadings({
      beacon_id : getBeaconId(),
      lux : data.lux,
      temperature : data.temperature,
      timestamp : getCurrentUnixTimeInSeconds(),
      unit : getUnit()
    });

    Serial.println("Data transmitted!");
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

  Serial.print("Sleeping for ");
  Serial.print(getPollIntervalSeconds());
  Serial.println(" seconds.");
  esp_sleep_enable_timer_wakeup(getPollIntervalSeconds() * uS_TO_S_FACTOR);
  esp_wifi_stop();
  esp_deep_sleep_start();
}