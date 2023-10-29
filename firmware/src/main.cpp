#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "Adafruit_HMC5883_U.h"
#include "Servo.h"

#include "pins.h"
#include "drive.h"
#include "sensors.h"
#include "gripper.h"
#include "physical_constants.h"

#define s16 int16_t

#define DEBUG true


// Configuration

const int MinimumDriveIntervalms = 10;

// Repeat a function until it either returns false or a given period has passed
#define loopFor(task, period) do {int __start = millis(); while (true) {if (task()) break; int __now = millis(); if (__now - __start > period) {break;}}} while(false);

void setup() {

  // Setup pin modes
  InitPins();

  // Setup communication interfaces
  Serial.begin(9600);
  Wire.begin(); // Use default pins


  // Setup peripherals
  Serial.print("Initializing bluetooth... ");
  Serial.println(InitBluetooth() ? "OK" : "ERR");

  Serial.print("Initializing gripper... ");
  Serial.println(InitGripper() ? "OK" : "ERR");

  Serial.print("Initializing compass... ");
  Serial.println(InitCompass() ? "OK" : "ERR");


  for (int i = 0; i < 3; i++) {
    ShiftPinWrite(OPinStatusA, HIGH);
    delay(150);
    ShiftPinWrite(OPinStatusB, LOW);
    delay(150);
  }

  Serial.println("Initialization complete");

}

bool str_match(int len, char *a, const char *b) {
  for (int i = 0; i < len; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

u8 incoming_msg[9] = {0};
u8 msg_size = 0;

void process_msg() {
  if (str_match(4, (char*) incoming_msg, "STOP")) {

    // Set the current drive command to no steps
    SetDriveCommand(0, 0);

  } else if (str_match(4, (char*) incoming_msg, "DRIV")) {

    u16 distance = incoming_msg[4] | incoming_msg[5]<<8;
    s16 steps = (s16) ((float) distance / WheelStepTravel);
    SetDriveCommand(steps, steps);

  } else if (str_match(4, (char*) incoming_msg, "TURN")) {

    u16 bearing = incoming_msg[4] | incoming_msg[5]<<8;
    float bearing_rad = (float) bearing * PI / 180;
    // How much arc must the wheel move through?
    // (Assume both wheels step at once)
    float wheel_dist = bearing_rad * WheelSeparation / 2 / 2;
    s16 steps = (s16) ((float) wheel_dist / WheelStepTravel);
    SetDriveCommand(-steps, steps);

  } else if (str_match(4, (char*) incoming_msg, "COMP")) {
    u16 bearing = ReadCompass();
  } else if (str_match(4, (char*) incoming_msg, "FLOR")) {
    u8 value = digitalRead(IPinIR);

  } else if (str_match(3, (char*) incoming_msg, "SCAN")) {

    u8 n = incoming_msg[4];
    float distance = UltraRadius + ReadUltra(n);
    u16 x = (u16) (UltraCosines[n] * distance);
    u16 y = (u16) (UltraSines[n] * distance);
    
  }
}

void check_bluetooth() {
  while (Bluetooth.available()) {
    incoming_msg[msg_size++] = Bluetooth.read();
    // Check for terminator
    if (incoming_msg[msg_size-1] == ';') {

      // Confirm its at the end of the buffer, if so, all good
      // If not, then something went wrong. Just clear the buffer
      // and keep reading
      
      if (msg_size == sizeof(incoming_msg)) process_msg();

      // Clear buffer
      for (u8 i = 0; i < sizeof(incoming_msg); i++) incoming_msg[i] = 0;
      // Set pointer
      msg_size = 0;
    }
  }
}

void loop(){

  stepWheel(LEFT_WHEEL, 1);
  delay(40);

  // // Process drive instructions for a bit
  // loopFor(ExecuteNextDriveStep, MinimumDriveIntervalms);

  // if (Bluetooth.available())
  //    Serial.write(Bluetooth.read());
  // // Keep reading from Arduino Serial Monitor and send to Software Serial
  // if (Serial.available())
  //    Bluetooth.write(Serial.read());
}
