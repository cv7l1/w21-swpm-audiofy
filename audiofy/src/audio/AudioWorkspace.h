//
// Created by Jonathan on 04.01.2022.
//

#ifndef AUDIOFY_AUDIOWORKSPACE_H
#define AUDIOFY_AUDIOWORKSPACE_H

#include<list>
#include<map>
#include "al_player.h"


struct AudioPosition {
    float start;
    float end;
};

class AudioTrack {
    float lengthSec;
    std::map<float, AudioPlayBuffer<>> buffers;
    float currentPosition;
};

class AudioWorkspace {
    std::list<AudioTrack> audioTracks = std::list<AudioTrack>();

};


#endif //AUDIOFY_AUDIOWORKSPACE_H
