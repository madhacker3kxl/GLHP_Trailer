//

#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include "AD5422/AD5422.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Change the precision to 12 bit
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress

DeviceA = {0x28, 0x20, 0x62, 0x28, 0x07, 0x00, 0x00, 0x2C},
DeviceB = {0x28, 0x19, 0x97, 0x26, 0x07, 0x00, 0x00, 0x36},
DeviceC = {0x28, 0x29, 0xAF, 0x27, 0x07, 0x00, 0x00, 0xEE},
DeviceD = {0x28, 0xA2, 0xAD, 0x28, 0x07, 0x00, 0x00, 0xE5};

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Calibration");

  // Start up the library
  sensors.begin();

  //Make sure it finds all the devices and the addresses matches

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // method 1: by index
  if (!sensors.getAddress(DeviceA, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(DeviceB, 1)) Serial.println("Unable to find address for Device 1");
  if (!sensors.getAddress(DeviceC, 2)) Serial.println("Unable to find address for Device 2");
  if (!sensors.getAddress(DeviceD, 3)) Serial.println("Unable to find address for Device 3");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(DeviceA);
  Serial.println();

  Serial.print("Device 1 Address: ");
  printAddress(DeviceB);
  Serial.println();

  Serial.print("Device 2 Address: ");
  printAddress(DeviceC);
  Serial.println();

  Serial.print("Device 3 Address: ");
  printAddress(DeviceD);
  Serial.println();

  // set the resolution to 12 bit
  sensors.setResolution(DeviceA, TEMPERATURE_PRECISION);
  sensors.setResolution(DeviceB, TEMPERATURE_PRECISION);
  sensors.setResolution(DeviceC, TEMPERATURE_PRECISION);
  sensors.setResolution(DeviceD, TEMPERATURE_PRECISION);

  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(DeviceA), DEC);
  Serial.println();

  Serial.print("Device 1 Resolution: ");
  Serial.print(sensors.getResolution(DeviceB), DEC);
  Serial.println();

  Serial.print("Device 2 Resolution: ");
  Serial.print(sensors.getResolution(DeviceC), DEC);
  Serial.println();

  Serial.print("Device 3 Resolution: ");
  Serial.print(sensors.getResolution(DeviceD), DEC);
  Serial.println();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

void loop(void)
{
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");

  float DevA[readings], DevB[readings], DevC[readings], DevD[readings],
        DevAT = 0, DevBT = 0, DevCT = 0, DevDT = 0;

  double AvgA = 0, AvgB = 0, AvgC = 0, AvgD = 0;

  for (int index = 0; index < readings; index++)
  {
    sensors.requestTemperatures();
    DevA [index] = sensors.getTempC(DeviceA);
    DevB [index] = sensors.getTempC(DeviceB);
    DevC [index] = sensors.getTempC(DeviceC);
    DevD [index] = sensors.getTempC(DeviceD);

    DevAT += DevA[index];
    DevBT += DevB[index];
    DevCT += DevC[index];
    DevDT += DevD[index];

    delay(3000);
  }
  Serial.println("DONE");

  AvgA = (DevAT / readings);
  AvgB = (DevBT / readings);
  AvgC = (DevCT / readings);
  AvgD = (DevDT / readings);

  // print the device information
  Serial.print("DeviceA: ");
  Serial.print(AvgA);
  Serial.println(" C");

  Serial.print("DeviceB: ");
  Serial.print(AvgB);
  Serial.println(" C");

  Serial.print("DeviceC: ");
  Serial.print(AvgC);
  Serial.println(" C");

  Serial.print("DeviceD: ");
  Serial.print(AvgD);
  Serial.println(" C");
}
