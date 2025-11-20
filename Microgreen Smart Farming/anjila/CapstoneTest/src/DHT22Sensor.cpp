#include "DHT22Sensor.h"
#include <DHT.h> 

DHT22Sensor::DHT22Sensor(uint8_t pin) : _pin(pin), _dht(nullptr) {}

void DHT22Sensor::begin() {
  if (!_dht) _dht = new DHT(_pin, DHT22);
  _dht->begin();
}

bool DHT22Sensor::read(float &temperatureC, float &humidity, String &errorMsg) {
  if (!_dht) { errorMsg = "DHT22 not initialized"; return false; }
  humidity = _dht->readHumidity();
  temperatureC = _dht->readTemperature();
  if (isnan(humidity) || isnan(temperatureC)) {
    errorMsg = "Failed to read from DHT22";
    return false;
  }
  return true;
}
