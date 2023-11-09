#ifndef __DRIVE_H
#define __DRIVE_H

#include "pins.h"
#include "comms.h"

#define LEFT_WHEEL 0
#define RIGHT_WHEEL 1

bool DriveRunning = false;
int16_t DriveCommandLeftSteps = 0;
int16_t DriveCommandRightSteps = 0;
int16_t DriveElapsedLeftSteps = 0;
int16_t DriveElapsedRightSteps = 0;

#define RemainingLeftSteps (DriveCommandLeftSteps - DriveElapsedLeftSteps)
#define RemainingRightSteps (DriveCommandRightSteps - DriveElapsedRightSteps)

#define LeftWheelDir (DriveCommandLeftSteps == 0 ? 0 : DriveCommandLeftSteps > 0 ? 1 : -1)
#define RightWheelDir (DriveCommandRightSteps == 0 ? 0 : DriveCommandRightSteps > 0 ? 1 : -1)

#define Driving (!((DriveCommandLeftSteps == DriveElapsedLeftSteps) && (DriveCommandRightSteps == DriveElapsedRightSteps)))

void SetDriveCommand(int16_t leftSteps, int16_t rightSteps);
bool ExecuteNextDriveStep();
void stepWheels(int wheel, int dir);

// Assign a new drive command and start the robot in motion
void SetDriveCommand(int16_t leftSteps, int16_t rightSteps) {
  // Queue up all steps
  DriveCommandLeftSteps = (s16) ((float) leftSteps * 1.15);
  DriveCommandRightSteps = (s16) ((float) rightSteps * 1.15);
  // SO far we've done none
  DriveElapsedLeftSteps = 0;
  DriveElapsedRightSteps = 0;
  // Current state is driving
  DriveRunning = true;
}

// Step the wheels (as needed). Will not step each wheel more than once.
bool ExecuteNextDriveStep() {

  float progress = (float) (DriveElapsedLeftSteps) / (float) (DriveCommandLeftSteps);
  float delay = 3000 + 10000 * (2.25 - (5 * progress * (1- progress) + 1)); 

  if (DriveCommandLeftSteps/abs(DriveCommandLeftSteps) != DriveCommandRightSteps / abs(DriveCommandRightSteps)) {
    delay = 5000 + 10000 * (2.25 - (3 * progress * (1- progress) + 1)); 
  }

  int leftStep = 0;
  int rightStep = 0;

  if (abs(DriveCommandLeftSteps) > 0 && DriveElapsedLeftSteps != DriveCommandLeftSteps)
    leftStep = DriveCommandLeftSteps / abs(DriveCommandLeftSteps);

  if (abs(DriveCommandRightSteps) > 0 && DriveElapsedRightSteps != DriveCommandRightSteps)
    rightStep = DriveCommandRightSteps / abs(DriveCommandRightSteps);

  if (leftStep != 0 || rightStep != 0) {
    delayMicroseconds(int(delay));
    stepWheels(leftStep, rightStep);
    DriveElapsedLeftSteps += leftStep;
    DriveElapsedRightSteps += rightStep;
  } else {
    if (DriveRunning) {
      DriveRunning = false;
      SendHalted();
    }
    return false;
  }
  return true;
}

// wheel = LEFT | RIGHT
// dir = -1, 0, 1, where 0 results in no rotation
void stepWheels(int leftDir, int rightDir) {

  bool stepLeft = leftDir != 0;
  bool stepRight = rightDir != 0;

  // Motor driver expects 0 or 1
  leftDir = (leftDir + 1) / 2;
  rightDir = (rightDir + 1) / 2;

  // Set directions
  digitalWrite(OPinStepperLeftDir, leftDir);
  digitalWrite(OPinStepperRightDir, rightDir);

  if (stepLeft) digitalWrite(OPinStepperLeftStep, HIGH);
  if (stepRight) digitalWrite(OPinStepperRightStep, HIGH);
  delayMicroseconds(500);
  if (stepLeft) digitalWrite(OPinStepperLeftStep, LOW);
  if (stepRight) digitalWrite(OPinStepperRightStep, LOW);
  delayMicroseconds(500);

}




#endif // __DRIVE_H