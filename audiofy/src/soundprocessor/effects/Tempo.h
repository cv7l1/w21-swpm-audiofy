#ifndef AUDIOFY_PITCHEFFECT_H
#define AUDIOFY_PITCHEFFECT_H

#include "Effect.h"
#include "SoundTouchDLL.h"

class Tempo : public Effect {

public:
    Tempo(float t) : Effect::Effect(), tempo(t) {
    }

    ~Tempo() {
    }

    void applyEffect(AudioPlayBuffer<>& buffer, HANDLE h) override {
        soundtouch_setTempo(h, tempo);
    }

private:
    float tempo;

};

#endif 

#pragma once
