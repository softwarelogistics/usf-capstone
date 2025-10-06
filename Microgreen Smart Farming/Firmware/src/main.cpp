#define TEMP_SNSR_BOARD_V3

#include <Arduino.h>
#include <NuvIoT.h>

#define HARDWARE_VERSION "1.0.0"
#define FIRMWARE_VERSION "0.1.0"
#define FIRMWARE_SKU "MG-Sensor"

void setup() {
  ioConfig.GPIO1Config = GPIO_CONFIG_DHT22;
  ioConfig.GPIO1Name = "dht22.1";

  //ioConfig.GPIO2Config = GPIO_CONFIG_DBS18;
  //ioConfig.GPIO2Name = "ds18b20.2";
  
  configureConsole();
  
  welcome(FIRMWARE_SKU, FIRMWARE_VERSION);
  state.init(FIRMWARE_SKU, FIRMWARE_VERSION, HARDWARE_VERSION, "gpio002", 010);
  initPins();

  probes.setup(&ioConfig);
}

void loop() {
  console.setVerboseLogging(true);
  probes.loop();
  probes.debugPrint();
  console.println("--");
  commonLoop();
  delay(1000);
}