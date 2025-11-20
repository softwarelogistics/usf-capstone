#include "TempSensor.h"

TempSensorDS18B20::TempSensorDS18B20(uint8_t oneWirePin)
  : _pin(oneWirePin), _oneWire(oneWirePin), _dallas(&_oneWire) {}

void TempSensorDS18B20::begin(uint8_t resolutionBits, bool waitForConv, bool enableInternalPullup) {
  if (enableInternalPullup) {
    pinMode(_pin, INPUT_PULLUP); 
  }
  _dallas.begin();
  _dallas.setWaitForConversion(waitForConv);
  _dallas.setResolution(resolutionBits);
}

void TempSensorDS18B20::scanTo(Print& out) const {  
  int count = const_cast<DallasTemperature&>(_dallas).getDeviceCount();
  out.println(String("DS18B20 count: ") + String(count));
  DeviceAddress addr;
  for (int i = 0; i < count; i++) {
    String line = String("  ROM[") + String(i) + String("] ");
    if (const_cast<DallasTemperature&>(_dallas).getAddress(addr, i)) {
      for (uint8_t j = 0; j < 8; j++) {
        if (addr[j] < 16) line += "0";
        line += String(addr[j], HEX);
      }
    } else {
      line += "<no address>";
    }
    out.println(line);
  }
}


bool TempSensorDS18B20::readFirst(float& tempC) {
  _dallas.requestTemperatures(); 
  tempC = _dallas.getTempCByIndex(0);
  return (tempC != DEVICE_DISCONNECTED_C);
}
