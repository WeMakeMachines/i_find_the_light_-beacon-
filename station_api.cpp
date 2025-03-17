#include "station_api.h"
#include "http.h"

#include <ArduinoJson.h>

HandshakeConfig httpRequestHandshake(const char *name)
{

  StaticJsonDocument<200> jsonPayload;
  StaticJsonDocument<200> jsonResponse;

  jsonPayload["name"] = name;
  String payloadAsString;
  serializeJson(jsonPayload, payloadAsString);
  String response = httpPOSTRequest(POST_HANDSHAKE_URL, payloadAsString);

  Serial.println("Received config from station");
  Serial.println(response);

  // Parse JSON response
  DeserializationError error = deserializeJson(jsonResponse, response);
  if (error)
  {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.f_str());
  }

  return {
      jsonResponse["beacon_id"],
      jsonResponse["rtc_calibration"],
      jsonResponse["poll_interval"],
      jsonResponse["schedule_start"],
      jsonResponse["schedule_end"],
      jsonResponse["unit"],
  };
}

void httpRequestReadings(Reading reading)
{
  StaticJsonDocument<200> payload;
  payload["beacon_id"] = reading.beacon_id;
  payload["lux"] = reading.lux;
  payload["temperature"] = reading.temperature;
  payload["timestamp"] = reading.timestamp;
  payload["unit"] = reading.unit;

  String payloadAsString;
  serializeJson(payload, payloadAsString);
  String response = httpPOSTRequest(POST_READINGS_URL, payloadAsString);
}