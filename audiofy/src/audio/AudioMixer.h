//
// Created by Jonathan on 08.01.2022.
//

#pragma once

#include "AudioWorkspace.h"
class ay_AudioMixer {
public:
    ay_AudioMixer();

    ay_AudioMixer(AudioTrack* initialTrack, i32 initialOutputBufferSize = 0);

    void flush();

    void submitBuffer(_In_ AudioTrack* track);

    void remove(_In_ AudioTrack* track) { 
        submittedBuffers.remove(track);
        delete track;
    }
    void mixTrack(_In_ AudioTrack* track);
    

    void remixAll();

    AudioPlayBuffer<>& getOutputBuffer();
    std::list<AudioTrack*> submittedBuffers;

private:
    void mix();

    AudioPlayBuffer<> outputBuffer;
    i32 currentBufferPosition = 0;
    u32 _initialOutputBufferSize;

};


