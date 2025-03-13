#ifndef IFTL_H
#define IFTL_H

enum Unit {
  Metric = 1,
  Imperial = 2
};

struct SensorData {
  float temperature;
  float lux;
};

#endif