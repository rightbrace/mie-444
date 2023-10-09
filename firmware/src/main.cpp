#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include "pins.h"

SoftwareSerial Bluetooth;

void setup() {

  // Setup pin modes
  initializePins();

  // Setup communication interfaces
  Serial.begin(9600);
  Wire.begin(9600); // Use default pins
  SPI.begin(); // Use default pins
  Bluetooth = SoftwareSerial(IPinBluetoothRX, OPinBluetoothTX);

}

void loop() {}
