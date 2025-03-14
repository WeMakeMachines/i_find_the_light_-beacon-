#include "http.h"

#include <HTTPClient.h>

String httpPOSTRequest(const char *url, String json)
{
  HTTPClient http;
  String jsonResponse = "{}";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    jsonResponse = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  return jsonResponse;
}