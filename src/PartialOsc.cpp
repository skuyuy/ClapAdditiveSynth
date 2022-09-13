#include "PartialOsc.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace
{
	static const float twoPi = 2.0f * (float) M_PI;
}

namespace addsyn::internal {
	PartialOsc::PartialOsc()
	{
	}

	PartialOsc::~PartialOsc()
	{
	}

	void PartialOsc::setFrequency(float frequency) noexcept
	{
		kFreq = frequency; // later we might want to ramp for smoothing
		updateAngle();
	}

	void PartialOsc::setAmplitude(float amplitude) noexcept
	{
		kAmp = amplitude;
	}

	void PartialOsc::setSampleRate(float _sampleRate) noexcept
	{
		sampleRate = _sampleRate;
		updateAngle();
	}

	void PartialOsc::process(const uint32_t indexInBuffer, uint32_t channelCount, uint32_t framesPerWindow, float** window) noexcept
	{
		// initialize frame
		//window[channelIndex][sampleIndex] = 0.0f;

		if (kAmp == 0.0f) // optimization
			return;

		auto oscValue = sinf(angle) * kAmp;
				
		for (uint32_t channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
			window[channelIndex][indexInBuffer] = oscValue;
		}

		angle += angleDelta;
		if (angle >= twoPi) {
			angle -= twoPi;
		}
	}

	void PartialOsc::prepareToPlay(float sampleRate, float frequency)
	{
		angleDelta = (twoPi  * frequency / sampleRate);
	}

	void PartialOsc::updateAngle()
	{
		angleDelta = (twoPi * kFreq / sampleRate);
	}
}