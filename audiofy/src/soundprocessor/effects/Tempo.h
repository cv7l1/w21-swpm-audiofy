#ifndef AUDIOFY_PITCHEFFECT_H
#define AUDIOFY_PITCHEFFECT_H

#include "Effect.h"
#include "SoundTouchDLL.h"

class Tempo : public Effect {

public:
    Tempo::Tempo(float t) : tempo(t) {
    }

    ~Tempo::Tempo() {
    }

    void applyEffect(AudioPlayBuffer<>& buffer, HANDLE h) override {
        soundtouch_setTempo(h, tempo);
    }

private:
    float tempo;

};

#endif 

#pragma once
