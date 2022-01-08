//
// Created by Jonathan on 08.01.2022.
//

#include "AudioMixer.h"
#include "samplerate.h"



void AudioMixer::mixTrack(AudioTrack* track)
{
	submittedBuffers.push_back(track);
	currentBufferPosition++;
	
	mix();
}

void AudioMixer::mix()
{
	AudioTrack* newTrack = submittedBuffers[currentBufferPosition];
	u64 sampleRate = newTrack->buffer.getAudioFormat().sampleRate;
	auto newTrackBuffer = newTrack->buffer;

	int sepCounter = 0;
	for (int currentPosition = newTrack->positionStart * sampleRate;
		currentPosition <= newTrack->positionEnd * sampleRate;
		++currentPosition) 
	{
		int a = 111; // first sample (-32768..32767)
		int b = 222; // second sample
		int m; // mixed result will go here

		// Make both samples unsigned (0..65535)
		a += 32768;
		b += 32768;

		// Pick the equation
		if ((a < 32768) || (b < 32768)) {
			// Viktor's first equation when both sources are "quiet"
			// (i.e. less than middle of the dynamic range)
			m = a * b / 32768;
		}
		else {
			// Viktor's second equation when one or both sources are loud
			m = 2 * (a + b) - (a * b) / 32768 - 65536;
		}

		// Output is unsigned (0..65536) so convert back to signed (-32768..32767)
		if (m == 65536) m = 65535;
		m -= 32768;
				
	}
			
}
