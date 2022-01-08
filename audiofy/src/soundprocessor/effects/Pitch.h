#ifndef AUDIOFY_PITCHEFFECT_H
#define AUDIOFY_PITCHEFFECT_H

#include "Effect.h"
#include "SoundTouchDLL.h"

class Pitch : public Effect {

public:
    Pitch::Pitch(float p) : pitch(p) {
    }

    ~Pitch::Pitch() {
    }

    void applyEffect(AudioPlayBuffer<>& buffer, HANDLE h) override {
        soundtouch_setPitch(h,pitch);
    }

private:
    float pitch;

};

#endif 

