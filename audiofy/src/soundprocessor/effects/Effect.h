//
// Created by Clemens on 07.01.2022.
//

#ifndef AUDIOFY_EFFECT_H
#define AUDIOFY_EFFECT_H

#include "al_player.h"

class Effect {
public:
    virtual void applyEffect(AudioPlayBuffer<>& buffer) = 0;
};

#endif //AUDIOFY_EFFECT_H
