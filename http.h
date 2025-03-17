#ifndef HTTP_H
#define HTTP_H

#include <Arduino.h>

struct HttpResponse
{
    int httpResponseCode;
    String json;
};

HttpResponse httpPOSTRequest(const char *url, String json);

#endif