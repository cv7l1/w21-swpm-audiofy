//
// Created by Jonathan on 08.01.2022.
//

#ifndef AUDIOFY_AUDIOMIXER_H
#define AUDIOFY_AUDIOMIXER_H

#include "al_player.h"
#include "AudioWorkspace.h"
class AudioMixer {

public:
    AudioMixer(i32 initialOutputBufferSize = 0) : _initialOutputBufferSize(initialOutputBufferSize) {}

    AudioMixer(AudioTrack* initialTrack, i32 initialOutputBufferSize = 0) : _initialOutputBufferSize(initialOutputBufferSize) {
		outputBuffer.getRawData()
			.resize(_initialOutputBufferSize, 0);

    }

    void flush() {
        submittedBuffers.clear();
        outputBuffer.getRawData().resize(_initialOutputBufferSize);
        memset(outputBuffer.getRawData().data(), 0, _initialOutputBufferSize);
    }

    void submitBuffer(_In_ AudioTrack* track) {
        submittedBuffers.emplace_back(track);
    }
     
    void mixTrack(_In_ AudioTrack* track);
    

    void remixAll();

    AudioPlayBuffer<>* getOutputBuffer() { return &outputBuffer; }

private:
    void mix();

    std::vector<AudioTrack*> submittedBuffers;
    AudioPlayBuffer<> outputBuffer;
    i32 currentBufferPosition = -1;
    u32 _initialOutputBufferSize;
};


#endif //AUDIOFY_AUDIOMIXER_H
