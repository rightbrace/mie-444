#ifndef PHYSICAL_CONSTANTS_H
#define PHYSICAL_CONSTANTS_H


// Angles = PI*4/4, PI*3/4, PI*2/4, PI*1/4, PI*0/4
const float ScannerSines[] = {
  0, 0.70711, 1, 0.70711, 0
};
const float ScannerCosines[] = {
  -1, -0.70711, 0, 0.70711, 1
};

const float UltraRadius = 100.0;

const float WheelStepTravel = 485.775 / 400; // EMPRICAL RELATIONSHIP
const float WheelSeparation = 166.66;
const float WheelStepRotation = 90.0/109.0; // Empical relationship, degrees / step of each wheel

#endif