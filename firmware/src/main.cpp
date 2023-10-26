#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "Adafruit_HMC5883_U.h"
#include "Servo.h"

#include "pins.h"

#define LEFT_WHEEL 0
#define RIGHT_WHEEL 1


Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345);

Servo ServoA, ServoB;

SoftwareSerial Bluetooth(OPinBluetoothTX, IPinBluetoothRX);

void setup() {

  // Setup pin modes
  initializePins();

  // Setup communication interfaces
  Serial.begin(9600);
  Wire.begin(); // Use default pins

  Serial.print("Initializing bluetooth... ");
  Bluetooth.begin(38400);
  Serial.println("OK");

  // Setup Servo
  Serial.print("Initializing servos... ");
  ServoA.attach(OPinServoA);
  ServoB.attach(OPinServoB);
  Serial.println("OK");

  for (int i = 0; i < 3; i++) {
    shiftPinWrite(7, HIGH);
    delay(150);
    shiftPinWrite(7, LOW);
    delay(150);
  }
  Serial.println("Initialization complete");

}

void stepWheel(int wheel, int dir) {
  if (wheel == LEFT_WHEEL) {
    digitalWrite(OPinStepperLeftDir, dir);
    digitalWrite(OPinStepperLeftStep, HIGH);
    delayMicroseconds(500);
    digitalWrite(OPinStepperLeftStep, LOW);
    delayMicroseconds(500);

  } else if (wheel == RIGHT_WHEEL) {
    digitalWrite(OPinStepperRightDir, dir);
    digitalWrite(OPinStepperRightStep, HIGH);
    delayMicroseconds(500);
    digitalWrite(OPinStepperRightStep, LOW);
    delayMicroseconds(500);

  }
}

int readUltra(int sensor) {
  if (sensor < 0 || sensor > 5) {
    Serial.println("Invalid sensor");
  }
  shiftPinWrite(sensor, HIGH);
  delayMicroseconds(10);
  shiftPinWrite(sensor, LOW);
  unsigned long uDelay = pulseIn(IPinUltraEcho, HIGH);
  return (int) ((float) uDelay * 0.34027 / 2);
}



void loop(){
  if (Bluetooth.available())
     Serial.write(Bluetooth.read());
  // Keep reading from Arduino Serial Monitor and send to Software Serial
  if (Serial.available())
     Bluetooth.write(Serial.read());
}
