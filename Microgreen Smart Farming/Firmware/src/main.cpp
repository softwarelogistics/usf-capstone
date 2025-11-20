#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>
#include <WiFi.h>
#include <Wire.h>
#include <BH1750.h>
#include <MHZ19.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include "SparkFun_Soil_Moisture_Sensor.h"  // SOIL

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Version Info
#define HARDWARE_VERSION "1.0.0"
#define FIRMWARE_VERSION "0.1.0"
#define FIRMWARE_SKU "MGS-Sensor"

// Timing
#define SAMPLE_PERIOD_MS 2000
#define SEND_RATE_MS     30000
#define CO2_WARMUP_MS    10000

// === Sensors ===
BH1750 lightMeter(0x23);
MHZ19 co2Sensor;
HardwareSerial co2Serial(2);

// === DS18B20 CONFIG (GPIO 26) ===
#define DS18B20_PIN 26
OneWire oneWire(DS18B20_PIN);
DallasTemperature dsSensors(&oneWire);

// === BME680 ===
Adafruit_BME680 bme;
bool bmeFound = false;
uint8_t bmeAddress = 0;

// === SOIL MOISTURE (Qwiic, I2C) ===
SparkFunSoilMoistureSensor soilSensor;
bool soilFound = false;

unsigned long lastSampleMs = 0;
unsigned long bootMs = 0;

void setup() {
  configureConsole();
  configureFileSystem();
  sysConfig.load();
  // === DHT22 GPIO ===
  configPins.Gpio1 = 25;
  ioConfig.GPIO1Config = GPIO_CONFIG_DHT22;
  ioConfig.GPIO1Name   = "DHT22 Temp";

  ioConfig.GPIO2Config = GPIO_CONFIG_DHT22_HUMIDITY;
  ioConfig.GPIO2Name   = "DHT22 Humidity";


  delay(1000);

  console.println("=== MGS-Sensor Booting ===");

  // === Device + WiFi ===
  if(sysConfig.DeviceId == NULL || sysConfig.DeviceId == "") 
      sysConfig.DeviceId = "etd001";
  sysConfig.Commissioned = true;
  sysConfig.WiFiEnabled = true;
  if(sysConfig.WiFiSSID == NULL || sysConfig.WiFiSSID == "") 
      sysConfig.WiFiSSID = "Ashford House";
  if(sysConfig.WiFiPWD == NULL || sysConfig.WiFiPWD == "") 
      sysConfig.WiFiPWD  = "ItsComingHome";

  sysConfig.SrvrHostName = "usf.iotharsh.iothost.net";
  sysConfig.Port = 1883;
  sysConfig.SrvrUID = "capstone";
  sysConfig.SrvrPWD = "Test1234";
  sysConfig.SrvrType = "mqtt";
  sysConfig.SendUpdateRateMS = SEND_RATE_MS;

  welcome(FIRMWARE_SKU, FIRMWARE_VERSION);
  state.init(FIRMWARE_SKU, FIRMWARE_VERSION, HARDWARE_VERSION, "gpio002", 010);

  initPins();
  probes.setup(&ioConfig);

  // === I2C (BH1750, BME680, Soil Sensor) ===
  Wire.begin(21, 22);

  // === BH1750 ===
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    console.println("BH1750 initialized successfully.");
  } else {
    console.println("[ERROR] BH1750 init failed.");
  }

  // === BME680 Auto-detect ===
  console.println("Scanning for BME680...");

  if (bme.begin(0x76, &Wire)) {
    bmeAddress = 0x76;
    bmeFound = true;
    console.println("BME680 initialized at 0x76.");
  }
  else if (bme.begin(0x77, &Wire)) {
    bmeAddress = 0x77;
    bmeFound = true;
    console.println("BME680 initialized at 0x77.");
  }
  else {
    console.println("[ERROR] BME680 not detected at 0x76 or 0x77!");
    bmeFound = false;
  }

  if (bmeFound) {
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);
  }

  // === SOIL MOISTURE INIT ===
  console.println("Initializing Soil Moisture sensor...");

  if (soilSensor.begin()) {
    soilFound = true;
    console.println("Soil Moisture sensor initialized.");
  } else {
    soilFound = false;
    console.println("[ERROR] Soil Moisture sensor NOT detected on I2C.");
  }

  // === MH-Z19B CO2 ===
  co2Serial.begin(9600, SERIAL_8N1, 16, 17);
  co2Sensor.begin(co2Serial);
  co2Sensor.autoCalibration(false);

  console.println("[info] MH-Z19B warming up...");
  bootMs = millis();

  // === DS18B20 ===
  dsSensors.begin();
  console.println("[info] DS18B20 initialized.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSampleMs >= SAMPLE_PERIOD_MS) {
    lastSampleMs = now;

    // === Wi-Fi ===
    if (WiFi.status() == WL_CONNECTED) {
      static bool printedIP = false;
      if (!printedIP) {
        console.println(String("Wi-Fi connected, IP: ") + WiFi.localIP().toString());
        printedIP = true;
      }
    } else {
      console.println("[info] Connecting Wi-Fi…");
    }

    // === BH1750 ===
    float lux = lightMeter.readLightLevel();
    console.println(String("Light: ") + String(lux, 1) + " lx");
    ioValues.setValue(2, String(lux, 1));

    // === DHT22 ===
    float humidity = probes.getHumidity(0);
    ioValues.setValue(1, String(humidity, 1));

    float temperature = probes.getTemperature(0);
    ioValues.setValue(0, String(temperature, 1));

    // === DS18B20 ===
    dsSensors.requestTemperatures();
    float dsTemp = dsSensors.getTempCByIndex(0);

    if (dsTemp > -100 && dsTemp < 150) {
      console.println(String("DS18B20 Temp: ") + dsTemp + " °C");
      ioValues.setValue(4, String(dsTemp, 2));
    } else {
      console.println("[ERROR] DS18B20 read failed.");
    }

    // === BME680 ===
    if (bmeFound) {
      if (bme.performReading()) {
        float bmeTemp = bme.temperature;
        float bmeHum  = bme.humidity;
        float bmePres = bme.pressure / 100.0;
        float bmeGas  = bme.gas_resistance / 1000.0;

        console.println(
          String("BME680 -> Temp: ") + bmeTemp +
          " °C, Hum: " + bmeHum +
          " %, Pres: " + bmePres +
          " hPa, Gas: " + bmeGas + " KΩ"
        );

        ioValues.setValue(10, String(bmeTemp, 2));
        ioValues.setValue(11, String(bmeHum, 2));
        ioValues.setValue(12, String(bmePres, 2));
        ioValues.setValue(13, String(bmeGas, 2));
      }
      else {
        console.println("[ERROR] BME680 read failed.");
      }
    }

    // === SOIL MOISTURE READ ===
    if (soilFound) {
      // Blink LED on the board so you SEE it's alive
      soilSensor.LEDOn();
      delay(20);
      soilSensor.LEDOff();

      uint16_t soilRaw   = soilSensor.readMoistureValue();
      float    soilPct   = soilSensor.readMoisturePercentage();
      float    soilRatio = soilSensor.readMoistureRatio();

      console.println(
        String("Soil -> raw: ") + soilRaw +
        ", percent: " + String(soilPct, 1) +
        "%, ratio: " + String(soilRatio, 3)
      );

      ioValues.setValue(5, String(soilPct, 1));

      const float TOO_DRY_PERCENT = 30.0;
      const float TOO_WET_PERCENT = 70.0;

      if (soilPct < TOO_DRY_PERCENT) {
        console.println("[Soil] STATUS: TOO DRY (needs watering)");
      } else if (soilPct > TOO_WET_PERCENT) {
        console.println("[Soil] STATUS: TOO WET (risk of mold/root rot)");
      } else {
        console.println("[Soil] STATUS: IDEAL RANGE");
      }
    }

    // === CO2 ===
    if (now - bootMs > CO2_WARMUP_MS) {
      int co2ppm = co2Sensor.getCO2();
      if (co2ppm > 0) {
        console.println(String("CO2: ") + co2ppm + " ppm");
        ioValues.setValue(3, String(co2ppm));
      } else {
        console.println("[ERROR] CO2 read failed.");
      }
    } else {
      console.println("[info] CO2 warming up...");
    }
  }

  console.setVerboseLogging(true);
  probes.loop();
  probes.debugPrint();
  console.println("--");

  commonLoop();
  delay(1000);
}
