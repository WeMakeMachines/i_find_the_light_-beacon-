#include "unit.h"
#include "i_find_the_light-beacon-.h"
#include "http.h"
#include "wifi_config.h" // Make sure this has been configured!

#include <Wire.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <ArduinoJson.h>  
#include <Adafruit_VEML7700.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <RTClib.h>

// unique device reference
String name = "0001";

// API endpoints
const char* api_POST_handshake = "http://192.168.50.1:3111/handshake";
const char* api_POST_readings = "http://192.168.50.1:3111/readings";

// device config
// beacon_id, timestamp, poll_interval, unit
StaticJsonDocument<200> config;

// DS1307 RTC
RTC_DS1307 rtc;

// DS18B20 temperature sensor - this is a 3 pin temperature sensor, mounted upside-down - you can only see the 3 solder blobs from the top of the PCB
const int oneWireBus = 4;  // GPIO pin for DS18B20 data line
OneWire oneWire(oneWireBus);
Adafruit_VEML7700 veml = Adafruit_VEML7700();
DallasTemperature sensors(&oneWire);

int poll_interval;
int rtc_calibrate_timestamp;
int beacon_id;
enum Unit unit;

// Variables to store sensor data
float lux = 0.0;
float temperature = 0.0;

int getTimestamp() {
  DateTime now = rtc.now();
  int timestamp = now.second();
  return timestamp;
}

SensorData pollSensors(Unit unit) {
  int DS18B20_index = 0;
  float temperature;
  float lux = veml.readLux();
  sensors.requestTemperatures();

  switch (unit) {
    case Unit::Imperial:
      temperature = sensors.getTempFByIndex(DS18B20_index);
      break;
    default:
      temperature = sensors.getTempCByIndex(DS18B20_index);
  }

  return { temperature, lux };
}

void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  StaticJsonDocument<200> handshakePayload;
  handshakePayload["name"] = name;
  String payload;
  serializeJson(handshakePayload, payload);
  String response = httpPOSTRequest(api_POST_handshake, payload);
  Serial.println(response);

  // Parse JSON response
  DeserializationError error = deserializeJson(config, response);
  if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.f_str());
      return;
  }

  beacon_id = config["beacon_id"];
  poll_interval = config["poll_interval"];
  rtc_calibrate_timestamp = config["timestamp"];
  unit = validateUnit(config["unit"]);

  // Initialise I2C for VEML7700 and DS1307
  Wire.begin(32, 25);
  if (!veml.begin()) {
    Serial.println("Failed to initialise VEML7700 sensor!");
    while (1);
  }
  veml.setGain(VEML7700_GAIN_1_8);
  veml.setIntegrationTime(VEML7700_IT_800MS);
  Serial.println("VEML7700 sensor initialised.");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is not running! Setting time to default.");

    DateTime dt(rtc_calibrate_timestamp);
    rtc.adjust(dt);
  }

  // Initialise DS18B20 sensor
  sensors.begin();
  Serial.println("DS18B20 sensor initialised.");
}

void loop() {
  esp_wifi_start();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  pollSensors(unit);

  StaticJsonDocument<200> reading;
  reading["beacon_id"] = beacon_id;
  reading["lux"] = lux;
  reading["temperature"] = temperature;
  reading["timestamp"] = getTimestamp();

  // Convert JSON to a string
  String payload;
  serializeJson(reading, payload);
  String response = httpPOSTRequest(api_POST_readings, payload);

  Serial.println(response);

  // Optionally, log data to the serial monitor for debugging
  Serial.print("Ambient Light: ");
  Serial.print(lux);
  Serial.print(" lux, Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  esp_sleep_enable_timer_wakeup(poll_interval * 1000000); //light sleep for 2 seconds
  esp_wifi_stop();
  esp_light_sleep_start();
}
