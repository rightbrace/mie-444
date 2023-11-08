#ifndef __COMMS_H
#define __COMMS_H

#include "pins.h"

#define WIRED_CONTROL false

#if WIRED_CONTROL
#define Radio Serial
SoftwareSerial Debug = SoftwareSerial(OPinBluetoothTX, IPinBluetoothRX);
#else
SoftwareSerial Radio = SoftwareSerial(OPinBluetoothTX, IPinBluetoothRX);
#define Debug Serial
#endif

#define s16 int16_t

bool InitBluetooth() {
  // Bluetooth auto-inits now
  return true;
}

void SendInt(s16 value) {
  if (value >= 0) {
    Radio.write("+");
  } else {
    Radio.write("-");
    value = -value;
  }
  value %= 1000;
  int hundreds = value / 100;
  value -= hundreds * 100;
  int tens = value / 10;
  value -= tens * 10;
  Radio.write('0' + hundreds);
  Radio.write('0' + tens);
  Radio.write('0' + value);
}

void SendHalted() {
  Radio.write("HALT........;");
}

void SendRange(u8 which, int16_t localX, int16_t localY) {
  // ULTRxxyy;
  Radio.write("RNG");
  Radio.write('0' + which);
  SendInt(localX);
  SendInt(localY);
  Radio.write(";");
}

void SendCompass(s16 bearing) {
  Radio.write("COMP");
  SendInt(bearing);
  Radio.write("....;");
}

#endif // __COMMS_H