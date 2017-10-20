//-----------------------------------------------------------------------------
// Ramp.cpp
// The Ramp class implements a linear ramp generator.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#include "Ramp.h"

typedef unsigned int uint32;

Ramp::Ramp(uint32 inputBlockSize): previousInput(0.0), output(0.0), previousOutput(0.0),
                                   incrementValue(0.0), cycleCounter(0), isRamping(false) {
  blockSize = inputBlockSize;
  // SI PREDILIGE UN VALORE DI blockSize EQUIVALENTE A 2^N ONDE EVITARE ERRORI DI APPROSSIMAZIONE...
}

Ramp::~Ramp() {

}

double Ramp::oneSampleProcessor(double inputSample) {
  if (inputSample - previousInput) {
    isRamping = true;
    incrementValue = (inputSample - previousInput) / blockSize;
    cycleCounter = blockSize;
  }
  //-----------------------
  previousInput = inputSample;
  //-----------------------
  if (isRamping) {
    output = previousOutput + incrementValue;
    previousOutput = output;
    cycleCounter -= 1;
    if (!cycleCounter) {
      isRamping = false;
    }
    return output;
  }
  //-----------------------
  output = inputSample;
  previousOutput = output;
  return output;
}
