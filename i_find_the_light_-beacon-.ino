#include "wifi_config.h" // Make sure this has been configured!

#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>

#include <ArduinoJson.h>
#include <Adafruit_VEML7700.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RTClib.h>

// unique device reference
String name = "0001";

// API endpoints
const char* api_POST_handshake = "http://192.168.50.1:3111/handshake";
const char* api_POST_readings = "http://192.168.50.1:3111/readings";

// device config
// node_id, timestamp, poll_interval
StaticJsonDocument<200> config;

enum Unit {
  Metric = 1,
  Imperial = 2
};

int poll_interval;
int rtc_calibrate_timestamp;
int node_id;
enum Unit unit;

// VEML7700 LUX sensor (the rectangular blue module with the rectangle diamond-looking bit)
Adafruit_VEML7700 veml = Adafruit_VEML7700();

// DS18B20 temperature sensor - this is a 3 pin temperature sensor, mounted upside-down - you can only see the 3 solder blobs from the top of the PCB
const int oneWireBus = 4;  // GPIO pin for DS18B20 data line
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// DS1307 RTC - The Real Time Clock module, this needs a CR2032 battery (coin-cell) installed to keeep the time.
RTC_DS1307 rtc;

// Variables to store sensor data
float lux = 0.0;
float temperature = 0.0;

int getTimestamp() {
  DateTime now = rtc.now();
  int timestamp = now.second();
  return timestamp;
}

String httpPOSTRequest(const char* url, String json) {
  HTTPClient http;
  String jsonResponse = "{}";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(json);

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    jsonResponse = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return jsonResponse;
}

// Function to poll data from sensors
void pollSensors(Unit unit) {
  int DS18B20_index = 0;
  lux = veml.readLux();
  sensors.requestTemperatures();

  switch (unit) {
    case Unit::Imperial:
      temperature = sensors.getTempFByIndex(DS18B20_index);
      break;
    default:
      temperature = sensors.getTempCByIndex(DS18B20_index);
  }
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

  node_id = config["node_id"];
  poll_interval = config["poll_interval"];
  rtc_calibrate_timestamp = config["timestamp"];

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
  reading["node_id"] = node_id;
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
