#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(HardwareSerial& serial) : _serial(serial) {}

void CO2Sensor::begin(int rx, int tx, uint32_t baud, bool autoCal) {
  _serial.begin(baud, SERIAL_8N1, rx, tx);
  _mhz19.begin(_serial);
  _mhz19.autoCalibration(autoCal);
}

bool CO2Sensor::read(int& co2ppm, float& onboardTempC, int& errCode) {
  co2ppm       = _mhz19.getCO2();        // -1 on failure
  onboardTempC = _mhz19.getTemperature();
  errCode      = _mhz19.errorCode;
  return (co2ppm > 0);
}
