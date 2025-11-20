#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>
#include <MHZ19.h>

class CO2Sensor {
public:
  explicit CO2Sensor(HardwareSerial& serial);
  void begin(int rx, int tx, uint32_t baud = 9600, bool autoCal = true);
  bool read(int& co2ppm, float& onboardTempC, int& errCode);

private:
  HardwareSerial& _serial;
  MHZ19 _mhz19;
};
