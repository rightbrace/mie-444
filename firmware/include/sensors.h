#ifndef __SENSORS_H
#define __SENSORS_H

#include <Arduino.h>
#include "Adafruit_VL53L0X.h"
#include "HMC5883L_Simple.h"


#include "pins.h"
#include "physical_constants.h"

Adafruit_VL53L0X tof;
HMC5883L_Simple compass;


bool ReadIR() {
  return digitalRead(IPinIR);
}

float ReadGripperUltra(){
  digitalWrite(OPinGripperUltraTrigger, HIGH);
  delay(10);
  digitalWrite(OPinGripperUltraTrigger, LOW);
  auto uDelay = pulseIn(IPinGripperUltraEcho, HIGH, 4000);
  auto dist = ((float) uDelay * (0.34027 / 2.0)) + UltraRadius;
  if (dist < 125) return 0;
  return dist;
}


bool InitTOFs() {

  // Cycle the TOF XSHUT pins simultaneously
  shiftState &= 0b11100000;
  flushShiftRegister();
  delay(10);
  shiftState |= 0b00011111;
  flushShiftRegister();
  delay(10);
  shiftState &= 0b11100000;
  flushShiftRegister();

  for (int i = 0; i < 6; i++) {
    ShiftPinWrite(i, HIGH);
    if (!tof.begin(0x10 | i)) return false;
    delay(10);
    tof.setMeasurementTimingBudgetMicroSeconds(30000);
    tof.configSensor(tof.VL53L0X_SENSE_HIGH_SPEED);
    tof.startRangeContinuous(35);
    delay(100);
  }
  return true;
}

float ReadToF(uint8_t which) {
  VL53L0X_RangingMeasurementData_t data;
  // Never got around to vendoring the dependancy properly, so cloning from git won't reflect the change,
  // but this field needs to not be marked private in the library. That's the only change
  tof.pMyDevice->I2cDevAddr = 0x10 | which;
  tof.MyDevice.I2cDevAddr = 0x10 | which;
  tof.rangingTest(&data);
  return ((float) data.RangeMilliMeter) + UltraRadius;
}

bool InitCompass() {
  compass.SetDeclination(23, 35, 'E');
  compass.SetSamplingMode(COMPASS_SINGLE);
  compass.SetScale(COMPASS_SCALE_130);
  compass.SetOrientation(COMPASS_HORIZONTAL_X_NORTH);
  return true;
}

// From Quercus
s16 ReadCompass()
{
  float _heading = compass.GetHeadingDegrees();
  // Invert the heading degrees
  _heading = 360 - _heading;
  // Use our non-linear function to estimate the heading
  if ((_heading >= 0.0) && (_heading < 45.5)) {
    return (s16) (1.977 * _heading);
  } else if ((_heading >= 45.5) && (_heading < 96.3)) {
    return (s16) ((1.773 * _heading) + 9.257);
  } else if ((_heading >= 96.3) && (_heading < 305.53)) {
    return (s16) ((0.430 * _heading) + 138.589);
  } else if ((_heading >= 305.53) && (_heading <= 360.0)) {
    return (s16) ((1.661 * _heading) - 237.618);
  }
  return 0;
}

#endif
