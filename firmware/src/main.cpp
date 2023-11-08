#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>

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

  Debug.begin(38400);
  Radio.begin(38400);
  Wire.begin(); // Use default pins
  Wire.setClock(400000);


  // Setup peripherals
  Debug.print("Initializing bluetooth... ");
  Debug.println(InitBluetooth() ? "OK" : "ERR");

  Debug.print("Initializing gripper... ");
  Debug.println(InitGripper() ? "OK" : "ERR");

  Debug.print("Initializing ToFs...");
  Debug.println(InitTOFs() ? "OK" : "ERR");

  Debug.println("Initialization complete");

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

void process_msg() {
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
    s16 steps = (s16) ((float) wheel_dist / WheelStepTravel);
    SetDriveCommand(-steps, steps);

  } else if (str_match(4, (char*) incoming_msg, "PING")) {
    Radio.write("PONG........;");
    while (true) {}
  }

  Radio.write("ACK.........;");
}

void check_bluetooth() {
  while (Radio.available()) {
    char chr = Radio.read();
    incoming_msg[msg_size++] = chr;

    // Check for terminator
    if (chr == ';') {
      Debug.print("End of transmission: ");
      for (int i = 0; i < sizeof(incoming_msg); i++)
        Debug.print((char) incoming_msg[i]);
      Debug.println();
      Debug.print("msg_size = ");
      Debug.println(msg_size);

      // Confirm its at the end of the buffer, if so, all good
      // If not, then something went wrong. Just clear the buffer
      // and keep reading
      if (msg_size == sizeof(incoming_msg)) {
        process_msg();
      } else {
        Debug.println("Invalid command");
        Radio.write("Invalid command, have: '");
        for (int i = 0; i < sizeof(incoming_msg); i++)
          Radio.write((char) incoming_msg[i]);
        Radio.write("'\n");
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
    Debug.print(i);
    Debug.print(" ");
    Debug.println(r);
    float dx = UltraCosines[i] * r;
    float dy = UltraSines[i] * r;
    SendRange(i, (s16) dx, (s16) dy);
  }
}

void loop(){
  
  ShiftPinWrite(6, (millis() / 500) % 2);
  ShiftPinWrite(7, Driving);

  // Process drive instructions for a bit
  check_bluetooth();
  loopFor(ExecuteNextDriveStep, MinimumDriveIntervalms);

  // Scan surroundings
  if (millis() > ScanInterval + ScanLastTime && !Driving) {
    RangeScan();
    ScanLastTime = millis();
  }

}


