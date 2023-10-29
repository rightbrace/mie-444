#ifndef __COMMS_H
#define __COMMS_H

#include "pins.h"

void SendHalted() {
  Bluetooth.write("HALTxxxx;");
}

void SendFloor(int16_t value) {
  // FLORxx00;
  Bluetooth.write("FLOR");
  Bluetooth.write((uint8_t) value);
  Bluetooth.write((uint8_t) 0);
  Bluetooth.write((uint8_t) 0);
  Bluetooth.write((uint8_t) 0);
  Bluetooth.write(";");
}

void SendUltra(int16_t localX, int16_t localY) {
  // ULTRxxyy;
  Bluetooth.write("ULTR");
  Bluetooth.write((uint8_t) localX & 0xFF); // LSB
  Bluetooth.write((uint8_t) (localX >> 8)); // MSB
  Bluetooth.write((uint8_t) localY & 0xFF); // LSB
  Bluetooth.write((uint8_t) (localY >> 8)); // MSB
  Bluetooth.write(";");
}

void SendCompass(uint16_t bearing) {
  Bluetooth.write("COMPxxxx;");
  Bluetooth.write(bearing);
}

#endif // __COMMS_H