# I find the light (beacon)

## What am I?

- I am code that was designed to run on an ESP32 WEMOS LOLIN32 LITE
- I will connect to a WIFI network and register myself with the [I find the light station](https://github.com/WeMakeMachines/i_find_the_light)
- I periodically take light (lux) and temperature readings, and send these to the station

## Hardware

The code was designed to run on the following hardware;

- ESP32 WEMOS LOLIN32 LITE
- VEML7700 lux sensor
- RTC DS1307
- DS18B20 temperature sensor

## Setting up the development environment

Before you begin, you'll need to set up the Arduino IDE.

#### Install the board definitions

- **Tools** > **Board** > **Boards Manager** > Search for "esp32" by Espressif Systems and install.

#### Select the correct board

- **Tools** > **Board** > **esp32** > WEMOS LOLIN32 Lite

#### Libraries

- [Arduino_JSON 7.3.1](https://arduinojson.org/?utm_source=meta&utm_medium=library.properties)
- [Adafruit VEML7700 Library 2.1.6](https://github.com/adafruit/Adafruit_VEML7700)
- [OneWire 2.3.8](https://www.pjrc.com/teensy/td_libs_OneWire.html)
- [DallasTemperature 4.0.3](https://github.com/milesburton/Arduino-Temperature-Control-Library)
- [RTClib by Adafruit 2.1.4](https://github.com/adafruit/RTClib)
