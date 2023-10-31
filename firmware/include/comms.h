#ifndef __COMMS_H
#define __COMMS_H

#include "pins.h"

#define USE_SERIAL true

#if USE_SERIAL
#define Radio Serial
SoftwareSerial Debug = SoftwareSerial(OPinBluetoothTX, IPinBluetoothRX);
#else
SoftwareSerial Radio = SoftwareSerial(OPinBluetoothTX, IPinBluetoothRX);
#define Debug Serial
#endif

#define s16 int16_t

bool InitBluetooth() {
  // Bluetooth = SoftwareSerial(OPinBluetoothTX, IPinBluetoothRX);
  #if !USE_SERIAL
  delay(4000);
  digitalWrite(OPinBluetoothEN, LOW);
  Radio.begin(38400);
  Radio.write("AT+RESET\r\n");
  Radio.end();
  Radio.begin(9600);
  delay(1000);
  while (Radio.available()) Radio.read();
  #endif
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

void SendUltra(u8 which, int16_t localX, int16_t localY) {
  // ULTRxxyy;
  Radio.write("ULT");
  Radio.write('0' + which);
  SendInt(localX);
  SendInt(localY);
  Radio.write(";");
}

void SendCompass(uint16_t bearing) {
  Radio.write("COMP........;");
  Radio.write(bearing);
}

#endif // __COMMS_H