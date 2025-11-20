// ==============================
//  MGS-Sensor (ESP32)
//  MH-Z19B (UART2) + DHT22 + DS18B20
// ==============================

#include <Arduino.h>
#include <NuvIoT.h>
#include <WiFi.h>


#include "Config.h"
#include "CO2Sensor.h"
#include "DHT22Sensor.h"
#include "TempSensor.h"

// ---------- UART2 for MH-Z19B ----------
HardwareSerial co2Serial(2);
//HardwareSerial co2(1);
co2.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17


// ---------- Sensor instances ----------
CO2Sensor          co2(co2Serial);
DHT22Sensor        dht(DHT22_PIN);
TempSensorDS18B20  ds(ONE_WIRE_BUS);

// ---------- Timing ----------
static unsigned long lastSampleMs = 0;

// ---------- Helpers ----------
static void scanDallasOnce() {

  ds.scanTo(Serial);
}



void setup() {
  // ---------- Console / banner ----------
  configureConsole();
  welcome(FIRMWARE_SKU, FIRMWARE_VERSION);
  console.println("Booting MGS-Sensor (NuvIoT Wi-Fi/MQTT)...");
  console.println(
    String("Pins: RXD2=") + String(RXD2) +
    " TXD2=" + String(TXD2) +
    " DS18=" + String(ONE_WIRE_BUS) +
    " DHT22=" + String(DHT22_PIN)
  );

  // ---------- NuvIoT device state ----------
  state.init(FIRMWARE_SKU, FIRMWARE_VERSION, HARDWARE_VERSION, "wfmq001", 10);

  initPins();

  // ---------- Probes ----------
  ioConfig.GPIO1Config = GPIO_CONFIG_NONE;
  ioConfig.GPIO1Name   = "unused";
  probes.setup(&ioConfig);

  sysConfig.DeviceId      = "mgs-001";            // set to your device ID
  sysConfig.Commissioned  = true;
  sysConfig.WiFiEnabled   = true;
  sysConfig.WiFiSSID      = "";       // <-- fill in
  sysConfig.WiFiPWD       = "";   // <-- fill in
  sysConfig.SrvrHostName  = "test.mosquitto.org";
  sysConfig.Port          = 1883;
  sysConfig.SrvrUID       = "";                  
  sysConfig.SrvrPWD       = "";                  


  // DS18B20
  ds.begin(/*resolutionBits*/12, /*waitForConv*/true, /*enableInternalPullup*/true);
  console.println(String("OneWire (DS18B20) on GPIO ") + String(ONE_WIRE_BUS));
  // Also mirror the bus scan to Serial so you can see raw addresses
  Serial.begin(115200);
  delay(200);
  scanDallasOnce();

  // MH-Z19B
  co2.begin(RXD2, TXD2, 9600, /*autoCal*/true);
  console.println(String("MH-Z19B ready on UART2 (") + String(RXD2) + "/" + String(TXD2) + ")");

  // DHT22
  dht.begin();
  console.println(String("DHT22 ready on GPIO ") + String(DHT22_PIN));
}

void loop() {
  // ---------- NuvIoT service loops ----------
  probes.loop();
  commonLoop();


  const unsigned long now = millis();
  if (now - lastSampleMs >= SAMPLE_PERIOD_MS) {
    lastSampleMs = now;

    // --- MH-Z19B CO2 ---
    int co2ppm = -1, err = 0;
    float tCO2 = NAN;
    if (co2.read(co2ppm, tCO2, err)) {
      console.println(String("CO2: ") + String(co2ppm) +
                      " ppm | CO2-sensor Temp: " + String(tCO2, 1) + " °C");
    } else {
      console.println(String("[warn] MH-Z19 read error, code=") + String(err));
    }

    // --- DHT22 temp/humidity ---
    float tDHT = NAN, hDHT = NAN;
    String dhtErr;
    if (dht.read(tDHT, hDHT, dhtErr)) {
      console.println(String("DHT22: ") + String(tDHT, 1) +
                      " °C | Humidity: " + String(hDHT, 1) + " %");
    } else {
      console.println(String("[warn] DHT22: ") + dhtErr);
    }

    // --- DS18B20 temperature ---
    float tDS = NAN;
    if (ds.readFirst(tDS)) {
      console.println(String("DS18B20: ") + String(tDS, 2) + " °C");
    } else {
      console.println("[warn] DS18B20: disconnected");
    }

    console.println("--");
  }

  delay(10);  
}
