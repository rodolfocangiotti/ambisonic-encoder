//-----------------------------------------------------------------------------
// Encoder.h
// The Encoder class implements a 3rd order ambisonic encoder with FuMa
// channel ordering and MaxN normalization coefficients.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#pragma once

typedef int int32;
typedef unsigned int uint32;

class Encoder {
public:
  Encoder(uint32 inputBufferLength);
  ~Encoder();
  void initCoordinates(double inputTheta, double inputPhi);
  void changeCoordinates(double inputTheta, double inputPhi);
  double oneSampleProcessor(double inputSample, uint32 inputChannel);
private:
  double* buffer;
  uint32 bufferLength;
  double** thetaValues;
  double** phiValues;
  double plainValue;
  // OSSIA NON NORMALIZZATO...
  double wrappedValue;
  int32 intValueZero;
  int32 intValueOne;
  double fractionalValue;
  double previousTheta;
  double previousPhi;
  // previousTheta E previousPhi SONO VALORI NORMALIZZATI...
  const double CONST_W;
  const double CONST_L;
  const double CONST_M;
  const double CONST_N;
  const double CONST_O;
};
