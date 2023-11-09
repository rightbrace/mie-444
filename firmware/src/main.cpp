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

// Running counts
unsigned long ScanLastTime = 0;

// Repeat a function until it either returns false or a given period has passed
#define loopFor(task, period) do {int __start = millis(); while (true) {if (task()) break; int __now = millis(); if (__now - __start > period) {break;}}} while(false);

void setup() {

  // Setup pin modes
  InitPins();

  // Setup communication interfaces

  Serial.begin(9600);
  Wire.begin(); // Use default pins
  Wire.setClock(400000);

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
  s16 x = str[2] - '0';
  x += (str[2] - '0') * 10;
  x += (str[1] - '0') * 100;
  x *= str[0] == '-' ? -1 : 1;
  return x;
}

void ProcessMessage() {
  if (str_match(4, (char*) incoming_msg, "HALT")) {
    // Set the current drive command to no steps
    SetDriveCommand(0, 0);
  } else if (str_match(4, (char*) incoming_msg, "DRIV")) {

    s16 distance = ParseInt4((char*) incoming_msg+4);
    s16 steps = (s16) ((float) distance / WheelStepTravel);
    SetDriveCommand(steps, steps);

  } else if (str_match(4, (char*) incoming_msg, "TURN")) {

    s16 bearing = ParseInt4((char*) incoming_msg+4);
    float bearing_rad = (float) bearing * PI / 180;
    // How much arc must the wheel move through?
    // (Assume both wheels step at once)
    float wheel_dist = bearing_rad * WheelSeparation / 2;
    float k = bearing < 0 ? 0.985 : 0.97;
    s16 steps = (s16) (k * (float) wheel_dist / WheelStepTravel);
    SetDriveCommand(-steps, steps);
  
  } else if (str_match(4, (char*) incoming_msg, "STEP")) {
    s16 left = ParseInt4((char*) incoming_msg+4);
    s16 right = ParseInt4((char*) incoming_msg+8);
    SetDriveCommand(left, right);
  } else if (str_match(4, (char*) incoming_msg, "PING")) {
    Serial.write("PONG........;");
  }

  Serial.write("ACK.........;");
}

void ReadBluetooth() {
  while (Serial.available()) {
    char chr = Serial.read();
    incoming_msg[msg_size++] = chr;

    // Check for terminator
    if (chr == ';') {
      if (msg_size == sizeof(incoming_msg)) {
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

void RangeScan() {
  for (int i = 0; i < 5; i++) {
    float r = ReadToF(i);
    float dx = ScannerCosines[i] * r;
    float dy = ScannerSines[i] * r;
    SendRange(i, (s16) dx, (s16) dy);
  }
  SendFloor(ReadIR());               
}

void loop(){
  
  ShiftPinWrite(6, (millis() / 500) % 2);
  ShiftPinWrite(7, Driving);

  ReadBluetooth();

  // Process drive instructions for a bit
  loopFor(ExecuteNextDriveStep, MinimumDriveIntervalms);

  // Scan surroundings
  if (millis() > ScanInterval + ScanLastTime && !Driving) {
    RangeScan();
    ScanLastTime = millis();
  }

}


