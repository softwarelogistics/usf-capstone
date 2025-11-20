#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TempSensorDS18B20 {
public:
  explicit TempSensorDS18B20(uint8_t oneWirePin);
  void begin(uint8_t resolutionBits = 12, bool waitForConv = true, bool enableInternalPullup = true);
  void scanTo(Print& out) const;  
  bool readFirst(float& tempC);

private:
  uint8_t _pin;
  OneWire _oneWire;
  DallasTemperature _dallas;
};
