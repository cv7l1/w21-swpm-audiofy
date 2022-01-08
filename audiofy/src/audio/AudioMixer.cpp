//
// Created by Jonathan on 08.01.2022.
//

#include "AudioMixer.h"
#include "samplerate.h"

void AudioMixer::process(AudioPlayBuffer<>* buffer)
{
	std::vector<float*> tempFloatBuffers;
	std::vector<float*> resampledBuffers;

	for (AudioPlayBuffer<>* mBuffer : submittedBuffers) {
		auto floatBuffer = static_cast<float*>(malloc(buffer->getCurrentBufferSize() * sizeof(float)));
		tempFloatBuffers.push_back(floatBuffer);
		src_short_to_float_array(mBuffer->getRawData().data(), floatBuffer, buffer->getCurrentBufferSize());

		if (mBuffer->getAudioFormat().sampleRate != commonSampleRate) {
			
		}
	}

		
	for (auto ptr : tempFloatBuffers) {
		free(ptr);
	}
}

void AudioMixer::mix(std::vector<float*> tempBuffers)
{
}
