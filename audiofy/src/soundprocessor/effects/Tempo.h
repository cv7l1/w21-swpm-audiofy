#ifndef AUDIOFY_TEMPOEFFECT_H
#define AUDIOFY_TEMPOEFFECT_H

#include "Effect.h"
#include "SoundTouchDLL.h"

class Tempo : public Effect {
public:
    Tempo(float t) : Effect::Effect(), tempo(t) {
    }

    ~Tempo() {
    }

    void applyEffect(AudioPlayBuffer<>* buffer, HANDLE h) override {
        soundtouch_setTempo(h, tempo);
    }
    const char* getEffectName() override {
        return "Tempo";
    }

private:
    float tempo;
};
#endif // AUDIOFY_TEMPOEFFECT_H
