//-----------------------------------------------------------------------------
// Encoder.cpp
// The Encoder class implements a 3rd order ambisonic encoder with FuMa
// channel ordering and MaxN normalization coefficients.
// © 2017, Rodolfo Cangiotti. Some rights reserved.
//-----------------------------------------------------------------------------

#include "Encoder.h"
#include "macros.h"
#include <cmath>

typedef int int32;
typedef unsigned int uint32;

Encoder::Encoder(uint32 inputBufferLength): plainValue(0.0), wrappedValue(0.0),
                                            intValueZero(0), intValueOne(0), fractionalValue(0.0),
                                            previousTheta(0.0), previousPhi(0.0),
                                            CONST_W(1.0 / sqrt(2.0)),
                                            CONST_L(sqrt(945.0 / 1792.0)), CONST_M(sqrt(945.0 / 1792.0)),
                                            CONST_N(sqrt(105.0 / 35.0) * 1.5), CONST_O(sqrt(105.0 / 35.0) * 1.5) {
  buffer = new double[inputBufferLength + 1];
  for (uint32 i = 0; i < inputBufferLength; i++) {
    buffer[i] = sin(2.0 * M_PI * i / inputBufferLength);
  }
  buffer[inputBufferLength] = 0.0;
  // GUARD POINT...
  bufferLength = inputBufferLength;

  thetaValues = new double*[2];
  for (uint32 i = 0; i < 2; i++) {
    thetaValues[i] = new double[3];
  }
  // ISTANZIO UNA MATRICE 2x3...
  phiValues = new double*[2];
  for (uint32 i = 0; i < 2; i++) {
    phiValues[i] = new double[3];
  }
}

Encoder::~Encoder() {
  delete[] buffer;
  delete[] thetaValues;
  delete[] phiValues;
}

void Encoder::initCoordinates(double inputTheta, double inputPhi) {
  for (uint32 i = 0; i < 2; i++) {
    for (uint32 j = 0; j < 2; j++) {
      for (uint32 k = 0; k < 3; k++) {
        if (i == 0) {
          plainValue = inputTheta * bufferLength;
        } else if (i == 1) {
          plainValue = inputPhi * bufferLength;
        }
        if (j == 0) {
          wrappedValue = wrap((j + 1) * plainValue, (unsigned int) 0, bufferLength);
        } else if (j == 1) {
          wrappedValue = wrap((j + 1) * plainValue + bufferLength * 0.25, (unsigned int) 0, bufferLength);
        }
        intValueZero = (int32) wrappedValue;
        intValueOne = intValueZero + 1;
        fractionalValue = wrappedValue - intValueZero;
        if (i == 0) {
          thetaValues[j][k] = buffer[intValueZero] * (1 - fractionalValue) + buffer[intValueOne] * fractionalValue;
        } else if (i == 1) {
          phiValues[j][k] = buffer[intValueZero] * (1 - fractionalValue) + buffer[intValueOne] * fractionalValue;
        }
      }
    }
  }
  previousTheta = inputTheta;
  previousPhi = inputPhi;
}

void Encoder::changeCoordinates(double inputTheta, double inputPhi) {
  if (inputTheta != previousTheta) {
    plainValue = inputTheta * bufferLength;
    // SCALO IL VALORE NORMALIZZATO ALLA DIMENSIONE DEL BUFFER...
    for (uint32 i = 0; i < 2; i++) {
      for (uint32 j = 0; j < 3; j++) {
        if (i == 0) {
          wrappedValue = wrap((j + 1) * plainValue, (unsigned int) 0, bufferLength);
        } else if (i == 1) {
          wrappedValue = wrap((j + 1) * plainValue + bufferLength * 0.25, (unsigned int) 0, bufferLength);
          // CALCOLO IL COSENO UTILIZZANDO LO STESSO BUFFER...
        }
        intValueZero = (int32) wrappedValue;
        intValueOne = intValueZero + 1;
        fractionalValue = wrappedValue - intValueZero;
        thetaValues[i][j] = buffer[intValueZero] * (1 - fractionalValue) + buffer[intValueOne] * fractionalValue;
      }
    }
  }
  if (inputPhi != previousPhi) {
    plainValue = inputPhi * bufferLength;
    for (uint32 i = 0; i < 2; i++) {
      for (uint32 j = 0; j < 3; j++) {
        if (i == 0) {
          wrappedValue = wrap((j + 1) * plainValue, (unsigned int) 0, bufferLength);
        } else if (i == 1) {
          wrappedValue = wrap((j + 1) * plainValue + bufferLength * 0.25, (unsigned int) 0, bufferLength);
        }
        intValueZero = (int32) wrappedValue;
        intValueOne = intValueZero + 1;
        fractionalValue = wrappedValue - intValueZero;
        phiValues[i][j] = buffer[intValueZero] * (1 - fractionalValue) + buffer[intValueOne] * fractionalValue;
      }
    }
  }
  previousTheta = inputTheta;
  previousPhi = inputPhi;
}

double Encoder::oneSampleProcessor(double inputSample, uint32 inputChannel) {
  if (inputChannel == 0) {
    return inputSample * CONST_W;
  } // W...
  if (inputChannel == 1) {
    return inputSample * thetaValues[1][0] * phiValues[1][0];
  } // X...
  if (inputChannel == 2) {
    return inputSample * thetaValues[0][0] * phiValues[1][0];
  } // Y...
  if (inputChannel == 3) {
    return inputSample * phiValues[0][0];
  } // Z...
  if (inputChannel == 4) {
    return inputSample * (3.0 * phiValues[0][0] * phiValues[0][0] - 1) * 0.5;
  } // R...
  if (inputChannel == 5) {
    return inputSample * thetaValues[1][0] * phiValues[0][1];
  } // S...
  if (inputChannel == 6) {
    return inputSample * thetaValues[0][0] * phiValues[0][1];
  } // T...
  if (inputChannel == 7) {
    return inputSample * thetaValues[1][1] * phiValues[1][0] * phiValues[1][0];
  } // U...
  if (inputChannel == 8) {
    return inputSample * thetaValues[0][1] * phiValues[1][0] * phiValues[1][0];
  } // V...
  if (inputChannel == 9) {
    return inputSample * phiValues[0][0] * (5.0 * phiValues[0][0] * phiValues[0][0] - 3.0) * 0.5;
  } // K...
  if (inputChannel == 10) {
    return inputSample * thetaValues[1][0] * (5.0 * phiValues[0][0] * phiValues[0][0] - 1.0) * phiValues[1][0] * CONST_L;
  } // L...
  if (inputChannel == 11) {
    return inputSample * thetaValues[0][0] * (5.0 * phiValues[0][0] * phiValues[0][0] - 1.0) * phiValues[1][0] * CONST_M;
  } // M...
  if (inputChannel == 12) {
    return inputSample * thetaValues[1][1] * phiValues[0][0] * phiValues[1][0] * phiValues[1][0] * CONST_N;
  } // N...
  if (inputChannel == 13) {
    return inputSample * thetaValues[0][1] * phiValues[0][0] * phiValues[1][0] * phiValues[1][0] * CONST_O;
  } // O...
  if (inputChannel == 14) {
    return inputSample * thetaValues[1][2] * phiValues[1][0] * phiValues[1][0] * phiValues[1][0];
  } // P...
  if (inputChannel == 15) {
    return inputSample * thetaValues[0][2] * phiValues[1][0] * phiValues[1][0] * phiValues[1][0];
  } // Q...
  return -10;
}
