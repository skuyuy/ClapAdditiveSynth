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

	float PartialOsc::tick() noexcept
	{
		// initialize frame
		//window[channelIndex][sampleIndex] = 0.0f;

		if (kAmp == 0.0f) // optimization
			return 0.0f;

		auto oscValue = sinf(angle) * kAmp;

		angle += angleDelta;
		if (angle >= twoPi) {
			angle -= twoPi;
		}

		return oscValue;
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