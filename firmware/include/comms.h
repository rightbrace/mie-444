#ifndef __COMMS_H
#define __COMMS_H

#include "pins.h"


void SendInt(s16 value) {
  if (value > 0) {
    Bluetooth.write("+");
  } else {
    Bluetooth.write("-");
  }
  value %= 1000;
  int hundreds = value / 100;
  value -= hundreds * 100;
  int tens = value / 10;
  value -= tens * 10;
  Bluetooth.write('0' + hundreds);
  Bluetooth.write('0' + tens);
  Bluetooth.write('0' + value);
}

void SendHalted() {
  Bluetooth.write("HALTxxxxxxxx;");
}

void SendFloor(int16_t value) {
  // FLORxx00;
  Bluetooth.write("FLOR");
  SendInt(value);
  Bluetooth.write("xxxx;");
}

void SendUltra(int16_t localX, int16_t localY) {
  // ULTRxxyy;
  Bluetooth.write("ULTR");
  SendInt(localX);
  SendInt(localY);
  Bluetooth.write(";");
}

void SendCompass(uint16_t bearing) {
  Bluetooth.write("COMPxxxxxxxx;");
  Bluetooth.write(bearing);
}

#endif // __COMMS_H