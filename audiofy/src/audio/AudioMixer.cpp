//
// Created by Jonathan on 08.01.2022.
//

#include "samplerate.h"
#include "AudioMixer.h"
ay_AudioMixer::ay_AudioMixer() : _initialOutputBufferSize(0) {}

ay_AudioMixer::ay_AudioMixer(AudioTrack* initialTrack, i32 initialOutputBufferSize) : _initialOutputBufferSize(initialOutputBufferSize) {
	outputBuffer.getRawData()
		.resize(_initialOutputBufferSize, 0);
}

void ay_AudioMixer::flush() {
	submittedBuffers.clear();
	outputBuffer.getRawData().resize(_initialOutputBufferSize);
	memset(outputBuffer.getRawData().data(), 0, _initialOutputBufferSize);
	outputBuffer.setAudioFormat(AudioFormatInfo<>::PCMDefault());
}

void ay_AudioMixer::submitBuffer(_In_ AudioTrack* track) {
	submittedBuffers.emplace_back(track);
}

void ay_AudioMixer::mixTrack(_In_ AudioTrack* track)
{
	submittedBuffers.push_back(track);
	currentBufferPosition++;
	mix();
}

void ay_AudioMixer::remixAll()
{ 
	memset(outputBuffer.getRawData().data(), 0, outputBuffer.getCurrentBufferSize());
	currentBufferPosition = 0;
		
	for (int i = 0; i < submittedBuffers.size(); ++i) {
		mix();
		currentBufferPosition++;
	}
}

AudioPlayBuffer<>& ay_AudioMixer::getOutputBuffer() { return outputBuffer; }

void ay_AudioMixer::mix()
{
	auto pos = submittedBuffers.begin();
	std::advance(pos, currentBufferPosition);

	AudioTrack* newTrack = *pos;
	u64 sampleRate = 44100;

	auto& newTrackBuffer = newTrack->getBuffer();

	int sepCounter = 0;
	int startPosition = newTrack->positionStart * sampleRate * 2;
	int endPosition = newTrack->positionEnd * sampleRate * 2;

	if (endPosition > outputBuffer.getCurrentBufferSize()) {
		outputBuffer.getRawData().resize(endPosition, 0);
	}

	for (int currentPosition = startPosition;
		currentPosition < endPosition;
		++currentPosition) 
	{
		int a = newTrackBuffer.getRawData()[sepCounter]; 
		int b = outputBuffer.getRawData()[currentPosition]; 

		outputBuffer.getRawData()[currentPosition] = (a/3+b)/2;
		sepCounter++;

	}
}
