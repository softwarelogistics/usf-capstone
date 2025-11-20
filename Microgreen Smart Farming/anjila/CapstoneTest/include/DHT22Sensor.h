#pragma once
#include <Arduino.h>

class DHT;

class DHT22Sensor {
public:
  explicit DHT22Sensor(uint8_t pin);
  void begin();
  bool read(float &temperatureC, float &humidity, String &errorMsg);

private:
  uint8_t _pin;
  DHT* _dht;   
};
