#include <Arduino.h>

#ifndef __PINS_H
#define __PINS_H

// Free pins:
// ~D3 ~D9 
// D12 (SPI CIPO)

const int OPinUltraData = 11;
const int OPinUltraClock = 13;
const int OPinUltraLatch = 10;
const int IPinUltraEcho = 8;

const int OPinStepperLeftStep = A0;
const int OPinStepperLeftDir = A1;
const int OPinStepperRightStep = A2;
const int OPinStepperRightDir = A3;

const int OPinServoA = 5;
const int OPinServoB = 6;

const int IOPinI2CData = 19;
const int OPinI2CClock = 18;

const int OPinBluetoothTX = 2;
const int IPinBluetoothRX = 4;

const int IPinIR = 7;

void initializePins() {

  // Ultrasonic array
  pinMode(OPinUltraData, OUTPUT);
  pinMode(OPinUltraClock, OUTPUT);
  pinMode(OPinUltraLatch, OUTPUT);
  pinMode(IPinUltraEcho, INPUT);

  // Stepper motor drivers
  pinMode(OPinStepperLeftStep, OUTPUT);
  pinMode(OPinStepperLeftDir, OUTPUT);
  pinMode(OPinStepperRightStep, OUTPUT);
  pinMode(OPinStepperRightDir, OUTPUT);

  // Servo signals
  pinMode(OPinServoA, OUTPUT);
  pinMode(OPinServoB, OUTPUT);

  // I2C bus
  pinMode(IOPinI2CData, OUTPUT);
  pinMode(OPinI2CClock, OUTPUT);

  // Bluetooth software serial
  pinMode(OPinBluetoothTX, OUTPUT);
  pinMode(IPinBluetoothRX, INPUT);

  // Black/White IR sensor  
  pinMode(IPinIR, INPUT);
  
}

#endif // __PINS_H