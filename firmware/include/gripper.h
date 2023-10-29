#ifndef __GRIPPER_H
#define __GRIPPER_H

#include <Servo.h>

#include "pins.h"

Servo ServoA, ServoB;

bool InitGripper() {
  if (ServoA.attach(OPinServoA) == INVALID_SERVO) return false;
  if (ServoB.attach(OPinServoB) == INVALID_SERVO) return false;
  return true;
}

void GripperDown() {
  // TODO
  ServoA.write(0);  
}

void GripperUp() {
  // TODO
  ServoA.write(0);  
}

void GripperOpen() {
  // TODO
  ServoB.write(0);  
}

void GripperClose() {
  // TODO
  ServoB.write(0);  
}

#endif