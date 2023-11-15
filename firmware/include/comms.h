#ifndef __COMMS_H
#define __COMMS_H

#include "pins.h"


#define s16 int16_t

bool InitBluetooth() {
  // Bluetooth auto-inits now
  return true;
}

void SendInt(s16 value) {
  if (value >= 0) {
    Serial.write('+');
  } else {
    Serial.write('-');
    value = -value;
  }
  value %= 1000;
  int hundreds = value / 100;
  value -= hundreds * 100;
  int tens = value / 10;
  value -= tens * 10;
  Serial.write('0' + hundreds);
  Serial.write('0' + tens);
  Serial.write('0' + value);
}

void SendInt5(s16 value) {
  if (value >= 0) {
    Serial.write('+');
  } else {
    Serial.write('-');
    value = -value;
  }
  value %= 10000;
  int thousands = value / 1000;
  value -= thousands * 1000;
  int hundreds = value / 100;
  value -= hundreds * 100;
  int tens = value / 10;
  value -= tens * 10;
  Serial.write('0' + thousands);
  Serial.write('0' + hundreds);
  Serial.write('0' + tens);
  Serial.write('0' + value);
}

void SendHalted() {
  Serial.write("HALT........;");
}

void SendRange(u8 which, int16_t localX, int16_t localY) {
  if (abs(localX) > 999 || abs(localY) > 999 || (localX == 0 && localY == 0)) {
    return;
  }
  Serial.write("RNG");
  Serial.write('0' + which);
  SendInt(localX);
  SendInt(localY);
  Serial.write(";");
}

void SendCompass(s16 bearing) {
  Serial.write("COMP");
  SendInt(bearing);
  Serial.write("....;");
}

void SendFloor(bool state) {
  Serial.write("FLOR");
  if (state) {
    Serial.write("+0010000;");
  } else {
    Serial.write("+0000000;");
  }
}

void SendBearing(s16 bearing) {
  Serial.write("BRNG");
  SendInt(bearing);
  Serial.write("....;");
}

void SendPosition(s16 x, s16 y) {
  Serial.write("PN");
  SendInt5(x);
  SendInt5(y);
  Serial.write(";");
}

#endif // __COMMS_H