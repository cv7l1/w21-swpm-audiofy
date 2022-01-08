//
// Created by Jonathan on 08.01.2022.
//

#ifndef AUDIOFY_AUDIOMIXER_H
#define AUDIOFY_AUDIOMIXER_H

#include "al_player.h"
class AudioMixer {
public:
    void submitBuffer(_In_ AudioPlayBuffer<>* buffer);
    void process(_Out_ AudioPlayBuffer<>* buffer);
private:
    std::vector<AudioPlayBuffer<>*> submittedBuffers;

};


#endif //AUDIOFY_AUDIOMIXER_H
