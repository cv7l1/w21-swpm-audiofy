//
// Created by Clemens on 07.01.2022.
//

#ifndef AUDIOFY_DEVELOP_SOUNDPROCESSOR_H
#define AUDIOFY_DEVELOP_SOUNDPROCESSOR_H

#include "effects/Effect.h"

class SoundProcessor {

public:
    SoundProcessor(AudioPlayBuffer<>* buffer);

    ~SoundProcessor();

    void addEffect(Effect* e);

    void removeEffect(Effect* e);

    void build();
    
    void process();

    HANDLE getHandle() { return soundtouchHandle; }

    AudioPlayBuffer<>* getBuffer();
    
    void reset() {
        soundtouch_destroyInstance(soundtouchHandle);
        soundtouchHandle = soundtouch_createInstance();
		soundtouch_setChannels(soundtouchHandle,2);
		soundtouch_setSampleRate(soundtouchHandle, buffer->getAudioFormat().sampleRate);
        effects.clear();
    }
private:
    std::vector<Effect*> effects = std::vector<Effect*>();
    AudioPlayBuffer<>* buffer;
    HANDLE soundtouchHandle;
};

#endif //AUDIOFY_DEVELOP_SOUNDPROCESSOR_H
