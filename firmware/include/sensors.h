#ifndef __SENSORS_H
#define __SENSORS_H

#include <Arduino.h>

#include "pins.h"

Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);

float ReadUltra(int sensor);
uint16_t ReadCompass();

// 300mm -> < 2ms of delay
const float MaxUltraDistance = 300;

float ReadUltra(int sensor) {
  if (sensor < 0 || sensor > 5) {
    Serial.println("Invalid sensor");
  }
  ShiftPinWrite(sensor, HIGH);
  delayMicroseconds(10);
  ShiftPinWrite(sensor, LOW);
  unsigned long uDelay = pulseIn(IPinUltraEcho, HIGH, MaxUltraDistance / (0.34027 / 2));
  return ((float) uDelay * (0.34027 / 2));
}

bool InitCompass() {
    return false;
}

uint16_t ReadCompass() {
  return 0;
}



#endif