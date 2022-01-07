//
// Created by Clemens on 07.01.2022.
//

#ifndef AUDIOFY_OVERMODULATIONEFFECT_H
#define AUDIOFY_OVERMODULATIONEFFECT_H

#include "Effect.h"

class Overmodulation : public Effect {
    void applyEffect(AudioPlayBuffer<>& buffer) override {
        for (auto& sample : buffer.getRawData()) {
            sample *= 10;
        }
    }
};

#endif //AUDIOFY_OVERMODULATIONEFFECT_H
