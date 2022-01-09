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
	mix();
	currentBufferPosition++;
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

	AudioTrack* newTrack = submittedBuffers[currentBufferPosition];

	newTrack->effectProcessor->build();

	u64 sampleRate = 44100;
	auto& newTrackBuffer = newTrack->getBuffer();

	u32 procSize = 6720;
	
	i32 samplesLeft = newTrack->file->audioInfo->getSampleCount();
	HANDLE h = newTrack->effectProcessor->getHandle();
	u32 samplesRecieved = 0;
	u32 position = 0;
	
	i16* currentPositionSource = newTrackBuffer.getRawData().data();

	i16* currentPositionDest = currentPositionSource;
	u32 samplesProcessed = 0;

	//Processing	
	while(samplesLeft > 0) {
		soundtouch_putSamples_i16(h, currentPositionSource, procSize / 2);
		do {
			samplesRecieved = soundtouch_receiveSamples_i16(h, currentPositionDest, procSize / 2);
			currentPositionDest += samplesRecieved * 2;
			samplesProcessed += samplesRecieved * 2;
		} while (samplesRecieved != 0);
		currentPositionSource += procSize;
		samplesLeft -= procSize;
	}
	
	soundtouch_flush(h);
	soundtouch_clear(h);

	//Mixing
	int sepCounter = 0;
	int startPosition = newTrack->positionStart * sampleRate * 2;
	int endPosition = newTrack->positionEnd * sampleRate * 2;

	if (newTrackBuffer.getCurrentBufferSize() <= endPosition - startPosition) {
		newTrackBuffer.getRawData().resize(endPosition - startPosition);
	}

	if (endPosition > outputBuffer.getCurrentBufferSize()) {
		outputBuffer.getRawData().resize(endPosition, 0);
	}
	
	for (int currentPosition = startPosition;
		currentPosition < endPosition;
		++currentPosition) 
	{
		int a = newTrackBuffer.getRawData()[sepCounter]; 
		int b = outputBuffer.getRawData()[currentPosition]; 

		outputBuffer.getRawData()[currentPosition] = ((a*newTrack->mixVol)+b)*0.5;
		sepCounter++;

	}
}
