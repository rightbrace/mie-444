#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "pins.h"
#include "drive.h"
#include "sensors.h"
#include "gripper.h"
#include "physical_constants.h"


// Configuration
const int MinimumDriveIntervalms = 10;
const int ScanInterval = 500;
const int SafetyCheckInterval = 100;

// Running counts
unsigned long ScanLastTime = 0;
unsigned long SafetyCheckLastTime = 0;
bool SafetyCheckEnabled = false;


// Repeat a function until it either returns false or a given period has passed
#define loopFor(task, period) do {int __start = millis(); while (true) {if (task()) break; int __now = millis(); if (__now - __start > period) {break;}}} while(false);

void setup() {

  // Setup pin modes
  InitPins();

  // Setup communication interfaces

  Serial.begin(9600);
  Wire.begin(); // Use default pins
  // Wire.setClock(400000);


  if (!InitBluetooth() || 
      !InitGripper() ||
      !InitTOFs()
  ) {
    while (true) {
      ShiftPinWrite(7, HIGH);
      delay(150);
      ShiftPinWrite(7, LOW);
      delay(150);
    }
  }
}

bool str_match(int len, char *a, const char *b) {
  for (int i = 0; i < len; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

u8 incoming_msg[13] = {0};
u8 msg_size = 0;

s16 ParseInt4(char *str) {
  s16 x = str[3] - '0';
  x += (str[2] - '0') * 10;
  x += (str[1] - '0') * 100;
  x *= str[0] == '-' ? -1 : 1;
  return x;
}

void Halt() {
  SetDriveCommand(0, 0);
}

void ProcessMessage() {
  if (str_match(4, (char*) incoming_msg, "HALT")) {
    // Set the current drive command to no steps
    Halt();
  } else if (str_match(4, (char*) incoming_msg, "DRIV")) {
    s16 distance = ParseInt4((char*) incoming_msg+4);
    s16 steps = (s16) ((float) distance / WheelStepTravel);
    SetDriveCommand(steps, steps);
  } else if (str_match(4, (char*) incoming_msg, "TURN")) {
    s16 bearing = ParseInt4((char*) incoming_msg+4);
    s16 steps = (s16) ((float) bearing / WheelStepRotation);
    SetDriveCommand(-steps, steps);
  } else if (str_match(4, (char*) incoming_msg, "STEP")) {
    s16 left = ParseInt4((char*) incoming_msg+4);
    s16 right = ParseInt4((char*) incoming_msg+8);
    SetDriveCommand(left, right);
  } else if (str_match(4, (char*) incoming_msg, "GRIP")) {
    s16 pitch = ParseInt4((char*) incoming_msg + 4);
    s16 clamp = ParseInt4((char*) incoming_msg + 8);
    SetGripper(pitch, clamp);
  } else if (str_match(4, (char*) incoming_msg, "SAFE")) {
    s16 value = ParseInt4((char*) incoming_msg + 4);
    SafetyCheckEnabled = value != 0;
    if (SafetyCheckEnabled) {
      Serial.write("SAFE+001....;");
    } else {
      Serial.write("SAFE+000....;");
    }
  } else if (str_match(4, (char*) incoming_msg, "PING")) {
    Serial.write("PONG........;");
  }

}

void ReadBluetooth() {
  while (Serial.available()) {
    char chr = Serial.read();
    incoming_msg[msg_size++] = chr;

    // Check for terminator
    if (chr == ';') {
      if (msg_size == sizeof(incoming_msg)) {
        Serial.write("ACK.........;");
        ProcessMessage();
      } else {
        Serial.write("INVALID HAVE: '");
        for (int i = 0; i < sizeof(incoming_msg); i++)
          Serial.write((char) incoming_msg[i]);
        Serial.write("'\n");
      }
    }

    if (msg_size >= sizeof(incoming_msg) || chr == ';') {
      // Clear buffer
      for (u8 i = 0; i < sizeof(incoming_msg); i++) incoming_msg[i] = 0;
      // Set pointer
      msg_size = 0;
    }
  }
}

#define timeit(name, expr) do {auto __start = millis(); expr; Serial.print(name); Serial.print(millis() - __start); Serial.print(";");} while(false);

// Check space around walls of robot
bool SafetyCheck() {
  const float emergencyStopMargin = 40;
  if (ReadToF(0) < emergencyStopMargin) return false;
  if (ReadToF(2) < emergencyStopMargin) return false;
  if (ReadToF(4) < emergencyStopMargin) return false;
  return true;
}

void RangeScan() {
  SendBearing(RobotBearing);  
  SendPosition(RobotX, RobotY);     
  int radii[5] = {0};
  for (int i = 0; i < 5; i++) {
    float r;
    r = ReadToF(i);
    // timeit("tof", r = ReadToF(i));
    float dx = ScannerCosines[i] * r;
    float dy = ScannerSines[i] * r;
    radii[i] = (s16) r;
    SendRange(i, (s16) dx, (s16) dy);
  }
  SendRange(6, 0, (s16) ReadGripperUltra());
  SendFloor(ReadIR());
  
  /*
  const float variance = 5;
  int numStraight = 0;

  for (int i = 0; i < 4; i++) {

    float target_angle = i % 2 ? 45 : 90;

    float a = (float) radii[i];
    float b = (float) radii[i+1];
    if (a == 0 || b == 0)
      continue;

    // if (b < a) {
    //   float _ = b;
    //   b = a;
    //   a = _;
    // }

    Serial.println("----");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(radii[i]);
    Serial.print(i+1);
    Serial.print(": ");
    Serial.println(radii[i+1]);
    float ang = 180 / M_PI * asin(b / (M_SQRT2 * sqrt(a*a + b*b - 2 * a * b / M_SQRT2)));
    Serial.print("angle = ");
    Serial.println(ang );
    Serial.print("target = ");
    Serial.println(target_angle);

    if (ang > target_angle - variance && ang < target_angle + variance) {
      numStraight ++;
      Serial.println ("(straight)");
    }
  }

  Serial.println();
  Serial.println(numStraight);

  */

}

void loop(){
  
  ShiftPinWrite(6, (millis() / 500) % 2);
  ShiftPinWrite(7, Driving);

  ReadBluetooth();

  // Process drive instructions for a bit
  loopFor(ExecuteNextDriveStep, MinimumDriveIntervalms);

  // Intermittently check wall safety
  if (SafetyCheckEnabled && Driving && millis() > SafetyCheckInterval + SafetyCheckLastTime) {
    if (!SafetyCheck()) {
      Halt();
      Serial.write("SAFETYSTOP..;");
    }
    SafetyCheckLastTime = millis();
  }

  // Scan surroundings
  if (millis() > ScanInterval + ScanLastTime && !Driving) {
    RangeScan();
    ScanLastTime = millis();
  }

}


