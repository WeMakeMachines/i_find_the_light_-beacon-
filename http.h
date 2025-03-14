#ifndef HTTP_H
#define HTTP_H

#include <Arduino.h>

String httpPOSTRequest(const char *url, String json);

#endif