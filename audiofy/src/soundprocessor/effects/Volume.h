#ifndef AUDIOFY_VOLUMEEFFECT_H
#define AUDIOFY_VOLUMEEFFECT_H

#include "Effect.h"

class Volume : public Effect {

public:
    Volume::Volume(int v) : volume(v) {
    }

    ~Volume::Volume() {
    }

    void applyEffect(AudioPlayBuffer<>& buffer,HANDLE h) override {
        for (auto& sample : buffer.getRawData()) {
            sample += vol;
        }     
    }

private:
    int volume;
};

#endif 