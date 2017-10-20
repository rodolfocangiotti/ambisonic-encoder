//-----------------------------------------------------------------------------
// LICENSE
// (c) 2017, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "ambiEncoderProcessor.h"
#include "ambiEncoderIDs.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "Encoder.h"
#include "Ramp.h"

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
ambiEncoderProcessor::ambiEncoderProcessor(): bypass(true), theta(0.0), phi(0.0) {
  setControllerClass(ambiEncoderControllerUID);
  encoder = new Encoder(2048);
  encoder->initCoordinates(theta, phi);
  thetaSmoother = new Ramp(128);
  phiSmoother = new Ramp(128);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderProcessor::initialize(FUnknown* context) {
  tresult result = AudioEffect::initialize(context);
  if (result == kResultTrue) {
    addAudioInput(USTRING("AudioInput"), SpeakerArr::kMono);
    addAudioOutput(USTRING("AudioOutput"), SpeakerArr::kBFormat3rdOrder);
  }
  return result;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderProcessor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts) {
  // we only support one in and output bus and these buses must have the same number of channels
  if (numIns == 1 && numOuts == 1 && inputs[0] == SpeakerArr::kMono && outputs[0] == SpeakerArr::kBFormat3rdOrder) {
    return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
  }
  return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderProcessor::setActive(TBool state) {
  SpeakerArrangement arr;
  if (getBusArrangement(kOutput, 0, arr) != kResultTrue) {
    return kResultFalse;
  }
  int32 numChannels = SpeakerArr::getChannelCount(arr);
  if (numChannels == 0) {
    return kResultFalse;
  }
  return AudioEffect::setActive(state);
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderProcessor::process(ProcessData& data) {
  if (data.inputParameterChanges) {
    int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < numParamsChanged; index++) {
      IParamValueQueue* paramQueue = data.inputParameterChanges->getParameterData(index);
      if (paramQueue) {
        ParamValue value;
        int32 sampleOffset;
        int32 numPoints = paramQueue->getPointCount();
        switch (paramQueue->getParameterId()) {
        case kBypass:
          if (paramQueue->getPoint(numPoints - 1, sampleOffset, value) == kResultTrue)
            bypass = (value > 0.5);
          break;
        case kTheta:
          if (paramQueue->getPoint(numPoints - 1,  sampleOffset, value) == kResultTrue) {
            theta = value - 0.5;
          }
          break;
        case kPhi:
          if (paramQueue->getPoint(numPoints - 1,  sampleOffset, value) == kResultTrue) {
            phi = value * 0.5 - 0.25;
          }
          break;
        }
      }
    }
  }

  if (data.numSamples > 0) {
    SpeakerArrangement inArr;
    getBusArrangement(kInput, 0, inArr);
    int32 numInChannels = SpeakerArr::getChannelCount(inArr);

    int32 numOutChannels;
    if (bypass) {
      numOutChannels = numInChannels;
    } else {
      SpeakerArrangement outArr;
      getBusArrangement(kOutput, 0, outArr);
      numOutChannels = SpeakerArr::getChannelCount(outArr);
    }

    for (int32 channelIn = 0; channelIn < numInChannels; channelIn++) {
      float* inputChannel = data.inputs[0].channelBuffers32[channelIn];
      for (int32 channelOut = 0; channelOut < numOutChannels; channelOut++) {
        float* outputChannel = data.outputs[0].channelBuffers32[channelOut];
        for (int32 sample = 0; sample < data.numSamples; sample++) {
          float smoothedTheta = (float) thetaSmoother->oneSampleProcessor(theta);
          float smoothedPhi = (float) phiSmoother->oneSampleProcessor(phi);
          encoder->changeCoordinates(smoothedTheta, smoothedPhi);
          outputChannel[sample] = encoder->oneSampleProcessor(inputChannel[sample], channelOut);
        }
      }
    }
  }
  return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderProcessor::setState(IBStream* state) {
  if (!state) {
    return kResultFalse;
  }

  // called when we load a preset, the model has to be reloaded
  int32 savedBypass = 0;
  if (state->read(&savedBypass, sizeof(int32)) != kResultOk) {
    // could be an old version, continue
  }

  float savedTheta = 0.0;
  if (state->read(&savedTheta, sizeof(float)) != kResultOk) {
    return kResultFalse;
  }

  float savedPhi = 0.0;
  if (state->read(&savedPhi, sizeof(float)) != kResultOk) {
    return kResultFalse;
  }

#if BYTEORDER == kBigEndian
  SWAP_32(savedBypass)
  SWAP_32(savedTheta)
  SWAP_32(savedPhi)
#endif

  bypass = savedBypass > 0;
  theta = savedTheta;
  phi = savedPhi;

  return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderProcessor::getState(IBStream* state) {
  // here we need to save the model
  int32 toSaveBypass = bypass ? 1 : 0;
  float toSaveTheta = theta;
  float toSavePhi = phi;

#if BYTEORDER == kBigEndian
  SWAP_32(toSaveBypass)
  SWAP_32(toSaveTheta)
  SWAP_32(toSavePhi)
#endif

  state->write(&toSaveBypass, sizeof(int32));
  state->write(&toSaveTheta, sizeof(float));
  state->write(&toSavePhi, sizeof(float));

  return kResultOk;
}

} // namespace Vst
} // namespace Steinberg
