//-----------------------------------------------------------------------------
// Ramp.h
// The Ramp class implements a linear ramp generator.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#pragma once

typedef unsigned int uint32;

class Ramp {
public:
  Ramp(uint32 inputBlockSize);
  ~Ramp();
  double oneSampleProcessor(double inputSample);
private:
  uint32 blockSize;
  double previousInput;
  double output;
  double previousOutput;
  double incrementValue;
  uint32 cycleCounter;
  bool isRamping;
};
