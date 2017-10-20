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

#include "ambiEncoderController.h"
#include "ambiEncoderIDs.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ibstream.h"

namespace Steinberg {
namespace Vst {

//-----------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderController::initialize(FUnknown* context) {
  tresult result = EditController::initialize(context);
  if (result == kResultTrue) {
		Parameter* param;
		param = new RangeParameter(USTRING("Bypass"), kBypass, USTRING(""), 0, 1, 1);
		param->setPrecision(0);
		parameters.addParameter(param);
		param = new RangeParameter(USTRING("Azimuth (theta)"), kTheta, USTRING("deg."), -180.0, 180.0, 0.0);
		param->setPrecision(1);
		parameters.addParameter(param);
		param = new RangeParameter(USTRING("Elevation (phi)"), kPhi, USTRING("deg."), -90.0, 90.0, 0.0);
		param->setPrecision(1);
		parameters.addParameter(param);
  }
  return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API ambiEncoderController::setComponentState(IBStream* state) {
  // we receive the current state of the component (processor part)
  // we read only the gain and bypass value...
  if (state) {
    // read the bypass...
		int32 bypassState;
		if (state->read(&bypassState, sizeof(int32)) != kResultOk) {
      return kResultFalse;
    }
#if BYTEORDER == kBigEndian
    SWAP_32(bypassState)
#endif
		setParamNormalized(kBypass, bypassState ? 1 : 0);

		float thetaState = 0.0;
    if (state->read(&thetaState, sizeof(float)) != kResultOk) {
			return kResultFalse;
		}
#if BYTEORDER == kBigEndian
		SWAP_32(thetaState)
#endif
		setParamNormalized(kTheta, thetaState + 0.5);
    // NORMALIZZO LA VARIABILE LEGATA A kTheta...

    float phiState = 0.0;
		if (state->read(&phiState, sizeof(float)) != kResultOk) {
			return kResultFalse;
		}
#if BYTEORDER == kBigEndian
		SWAP_32(phiState)
#endif
		setParamNormalized(kPhi, phiState * 2.0 + 0.5);
	}

  return kResultOk;
}

} // namespace Vst
} // namespace Steinberg
