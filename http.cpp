#include "http.h"

#include <HTTPClient.h>

HttpResponse httpPOSTRequest(const char *url, String json)
{
  HTTPClient http;
  String jsonResponse = "{}";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(json);

  if (httpResponseCode == 200)
  {
    jsonResponse = http.getString();
  }

  http.end();

  return {httpResponseCode, jsonResponse};
}