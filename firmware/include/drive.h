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


void SetDriveCommand(int16_t leftSteps, int16_t rightSteps);
bool ExecuteNextDriveStep();
void stepWheel(int wheel, int dir);

// Assign a new drive command and start the robot in motion
void SetDriveCommand(int16_t leftSteps, int16_t rightSteps) {
  // Queue up all steps
  DriveCommandLeftSteps = leftSteps;
  DriveCommandRightSteps = rightSteps;
  // SO far we've done none
  DriveElapsedLeftSteps = 0;
  DriveElapsedRightSteps = 0;
  // Current state is driving
  DriveRunning = true;
}

// Step the wheels (as needed). Will not step each wheel more than once.
bool ExecuteNextDriveStep() {
  delayMicroseconds(1000);
  if (abs(RemainingLeftSteps) > abs(RemainingRightSteps)) {
    // If there's more left turn to execute
    stepWheel(LEFT_WHEEL, LeftWheelDir);
    DriveElapsedLeftSteps += LeftWheelDir;
  } else if (abs(RemainingRightSteps) > abs(RemainingLeftSteps)) {
    // If there's more right turn to execute
    stepWheel(RIGHT_WHEEL, RightWheelDir);
    DriveElapsedRightSteps += RightWheelDir;
  } else {
    // If there's the same amount of each wheel (in absolute terms)
    if (abs(RemainingLeftSteps) > 0) {
      // If both must step
      stepWheel(LEFT_WHEEL, LeftWheelDir);
      stepWheel(RIGHT_WHEEL, RightWheelDir);
      DriveElapsedLeftSteps += LeftWheelDir;
      DriveElapsedRightSteps += RightWheelDir;
    } else {
      // If both are done stepping, and we were running until now, send a halt message
      if (DriveRunning) {
        DriveRunning = false;
        SendHalted();
      }
      // Otherwise, do nothing
      // False indicates no step occured
      return false;
    }
  }
  // True indicates a step occured
  return true;
}

// wheel = LEFT | RIGHT
// dir = -1, 0, 1, where 0 results in no rotation
void stepWheel(int wheel, int dir) {

  if (dir == 0) return; // Do nothing
  // Motor driver expects 0 or 1
  dir = (dir + 1) / 2;

  if (wheel == LEFT_WHEEL) {
    digitalWrite(OPinStepperLeftDir, dir);
    delayMicroseconds(100);
    digitalWrite(OPinStepperLeftStep, HIGH);
    delayMicroseconds(500);
    digitalWrite(OPinStepperLeftStep, LOW);
    delayMicroseconds(500);
  } else if (wheel == RIGHT_WHEEL) {
    digitalWrite(OPinStepperRightDir, dir);
    delayMicroseconds(100);
    digitalWrite(OPinStepperRightStep, HIGH);
    delayMicroseconds(500);
    digitalWrite(OPinStepperRightStep, LOW);
    delayMicroseconds(500);
  }
}




#endif // __DRIVE_H