//
// Created by Clemens on 07.01.2022.
//

#ifndef AUDIOFY_OVERMODULATIONEFFECT_H
#define AUDIOFY_OVERMODULATIONEFFECT_H

#include "Effect.h"

class Overmodulation : public Effect {
public:
    Overmodulation(int vol) : vol(vol) {};

    void applyEffect(AudioPlayBuffer<>* buffer,HANDLE h) override {
        for (auto sample : buffer->getRawData()) {
            sample *= 10;
        }
    }
    const char* getEffectName() override {
        return "Overmodulation";
    }
private:
    int vol;
};

#endif //AUDIOFY_OVERMODULATIONEFFECT_H
