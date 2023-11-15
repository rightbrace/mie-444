#ifndef __GRIPPER_H
#define __GRIPPER_H

#include <Servo.h>

#include "pins.h"

Servo ServoA, ServoB;
void SetGripper(s16 pitch, s16 clamp);

bool InitGripper() {
  if (ServoA.attach(OPinServoA) == INVALID_SERVO) return false;
  if (ServoB.attach(OPinServoB) == INVALID_SERVO) return false;
  SetGripper(0, 0);
  return true;
}

void SetGripper(s16 pitch, s16 clamp) {
  ServoA.write(pitch+90);  
  ServoB.write(clamp+90);  
}

#endif