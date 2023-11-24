#ifndef __PINS_H
#define __PINS_H

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>


#define s16 int16_t

// Free pins:
// ~D9 
// D12 (SPI CIPO)

// Orange
const int OPinShiftData = 11;
// White
const int OPinShiftClock = 13;
// Purple
const int OPinShiftLatch = 10;

const int OPinStepperLeftStep = A0;
const int OPinStepperLeftDir = A1;
const int OPinStepperRightStep = A2;
const int OPinStepperRightDir = A3;

const int OPinServoA = 4;
const int OPinServoB = 6;

const int IOPinI2CData = 19;
const int OPinI2CClock = 18;

const int OPinGripperUltraTrigger = 2;
const int IPinGripperUltraEcho = 3;


const int IPinIR = 7;

const int OPinStatusA = 6;
const int OPinStatusB = 7;

void InitPins() {

  // Shift array
  pinMode(OPinShiftData, OUTPUT);
  pinMode(OPinShiftClock, OUTPUT);
  pinMode(OPinShiftLatch, OUTPUT);

  pinMode(9, INPUT);

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

  // Rear ultrasonic
  pinMode(OPinGripperUltraTrigger, OUTPUT);
  pinMode(IPinGripperUltraEcho, INPUT);

  // Gripper ultrasonic
  pinMode(OPinGripperUltraTrigger, OUTPUT);
  pinMode(IPinGripperUltraEcho, INPUT);

  // Black/White IR sensor  
  pinMode(IPinIR, INPUT);
  
}


uint8_t shiftState = 0;

void flushShiftRegister() {
  // digitalWrite(OPinUltraLatch, LOW);
  bitClear(PORTB, 2);

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(shiftState);
  SPI.endTransaction();
  // digitalWrite(OPinUltraLatch, HIGH);
  bitSet(PORTB, 2);

}

void ShiftPinWrite(int pin, int value) {
  shiftState &= ~(1<<pin);
  shiftState |= (value << pin);
  flushShiftRegister();
}

#endif // __PINS_H