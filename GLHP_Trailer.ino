#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Definitions for DS18B20
//Device Addresses:
//DeviceA = {0x28, 0x20, 0x62, 0x28, 0x07, 0x00, 0x00, 0x2C}
//DeviceB = {0x28, 0x19, 0x97, 0x26, 0x07, 0x00, 0x00, 0x36}
//DeviceC = {0x28, 0x29, 0xAF, 0x27, 0x07, 0x00, 0x00, 0xEE}
//DeviceD = {0x28, 0xA2, 0xAD, 0x28, 0x07, 0x00, 0x00, 0xE5}

#define One_Wire_Bus 8 //Data wire is plugged into port 
#define TEMPERATURE_PRECISION 12 //Sensor precision

// Setup a oneWire instance to communicate OneWire devices
OneWire oneWire(One_Wire_Bus);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress 
  DeviceA = {0x28, 0x20, 0x62, 0x28, 0x07, 0x00, 0x00, 0x2C}, 
  DeviceB = {0x28, 0x19, 0x97, 0x26, 0x07, 0x00, 0x00, 0x36};

//Definitions for AD5422BREZ
#define latch 10 //Latch pin for DAC

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
