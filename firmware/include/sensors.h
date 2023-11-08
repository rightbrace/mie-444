#ifndef __SENSORS_H
#define __SENSORS_H

#include <Arduino.h>

// #include <QMC5883LCompass.h>
#include "Adafruit_VL53L0X.h"


#include "pins.h"
#include "physical_constants.h"

// QMC5883LCompass compass;
Adafruit_VL53L0X tof;

float ReadUltra(int sensor);
s16 ReadCompass();

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
    Debug.println("Invalid sensor");

  }

  return ((float) uDelay * (0.34027 / 2.0)) + UltraRadius;
}

bool InitTOFs() {
  for (int i = 0; i < 5; i++) ShiftPinWrite(i, LOW);
  delay(10);
  for (int i = 0; i < 5; i++) ShiftPinWrite(i, HIGH);
  for (int i = 0; i < 5; i++) ShiftPinWrite(i, LOW);

  for (int i = 0; i < 5; i++) {
    ShiftPinWrite(i, HIGH);
    if (!tof.begin(0x10 | i)) return false;
    delay(10);
    tof.setMeasurementTimingBudgetMicroSeconds(30000);
    tof.configSensor(tof.VL53L0X_SENSE_HIGH_ACCURACY);
    tof.startRangeContinuous(35);
  }
  return true;
}

float ReadToF(uint8_t which) {
  VL53L0X_RangingMeasurementData_t data;
  
  tof.pMyDevice->I2cDevAddr = 0x10 | which;
  tof.rangingTest(&data);
  if (data.RangeMilliMeter > 999) return 0;
  // if (data.RangeStatus != 0) return 0;
  
  return ((float) data.RangeMilliMeter) + UltraRadius;
}

bool InitCompass() {
  // compass.init();
  // compass.setCalibrationOffsets(187.00, 1025.00, -1089.00);
  // compass.setCalibrationScales(0.92, 1.08, 1.01);
  // compass.setMagneticDeclination(-9, 59);
  // compass.setSmoothing(3, true);
  return true;
}

s16 ReadCompass() {
  // compass.read();
  // char dir[3];
  // compass.getDirection(dir, compass.getAzimuth());
  // Radio.write("ECHO");
  // Radio.write(dir);
  // Radio.write(".;");
  // return (s16) compass.getAzimuth();
  return 0;
}



#endif