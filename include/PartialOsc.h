#pragma once
#include <stdint.h>

namespace addsyn::internal {
	/*
	This class represents a Partial Sine for the additive engine
	It generates a sine wave at a given frequency with an amplitude
	It does not contain a state like pitch, pan, etc
	*/
	class PartialOsc {
	public:
		PartialOsc();
		~PartialOsc();
		void setFrequency(float frequency) noexcept;
		void setAmplitude(float amplitude) noexcept;
		void setSampleRate(float sampleRate) noexcept;
		void process(const uint32_t indexInBuffer, uint32_t channelCount, uint32_t framesPerWindow, float** window) noexcept; // per-window processing
		void prepareToPlay(float sampleRate, float frequency);
	protected:
		void updateAngle();
		// common params we might need in other oscs
		float sampleRate;
	private:
		float angle = 0.0f; // time bound
		float angleDelta = 0.0f;
		// parameters are prefixed with k
		float kFreq = 200.0f; // TODO is this the correct value
		float kAmp = 0.2f; // default amplitude is 80%
		uint32_t sampleOffset = 0;
	};
}