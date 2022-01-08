#ifndef AUDIOFY_PITCHEFFECT_H
#define AUDIOFY_PITCHEFFECT_H

#include "Effect.h"
#include "SoundTouchDLL.h"

class Pitch : public Effect {

public:
    Pitch(float p) : Effect::Effect() , pitch(p) {
    }

    ~Pitch() {
    }

    void applyEffect(AudioPlayBuffer<>& buffer, HANDLE h) override {
        soundtouch_setPitchSemiTones(h,pitch);
    }

private:
    float pitch;

};

#endif 

