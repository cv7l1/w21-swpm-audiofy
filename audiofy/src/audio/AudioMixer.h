//
// Created by Jonathan on 08.01.2022.
//

#ifndef AUDIOFY_AUDIOMIXER_H
#define AUDIOFY_AUDIOMIXER_H

#include "al_player.h"
class AudioMixer {
public:
    void flush() {
        submittedBuffers.clear();
    }
    void submitBuffer(_In_ AudioPlayBuffer<>* buffer) {
        submittedBuffers.emplace_back(buffer);
    }
    void process(_Out_ AudioPlayBuffer<>* buffer);
private:
    
    void mix(std::vector<float*> tempBuffers);

    std::vector<AudioPlayBuffer<>*> submittedBuffers;
    AudioPlayBuffer<> outputBuffer;
    u32 commonSampleRate = 44100;
};


#endif //AUDIOFY_AUDIOMIXER_H
