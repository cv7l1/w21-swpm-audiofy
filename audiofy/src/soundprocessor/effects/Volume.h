#ifndef AUDIOFY_VOLUMEEFFECT_H
#define AUDIOFY_VOLUMEEFFECT_H

#include "Effect.h"

class Volume : public Effect {

public:
    Volume(int v) : Effect::Effect(), volume(v) {
    }

    ~Volume() {
    }

    void applyEffect(AudioPlayBuffer<>& buffer,HANDLE h) override {
        for (auto& sample : buffer.getRawData()) {
            sample += volume;
        }     
    }

private:
    int volume;
};

#endif 