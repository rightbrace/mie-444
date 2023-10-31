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
  unsigned long uDelay = 0;
  ShiftPinWrite(sensor, HIGH);
  delay(10);
  ShiftPinWrite(sensor, LOW);
  switch (sensor) {
    case 0:
    case 1:
    case 2:
    case 3:
    uDelay = pulseIn(IPinUltraEcho, HIGH, 10000);
    break;

    case 4:
    uDelay = pulseIn(9, HIGH, 10000);
    break;

    default:
    Serial.println("Invalid sensor");

  }

  return ((float) uDelay * (0.34027 / 2.0));
}

bool InitCompass() {
    return false;
}

uint16_t ReadCompass() {
  return 0;
}



#endif