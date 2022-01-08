//
// Created by Clemens on 07.01.2022.
//

#ifndef AUDIOFY_EFFECT_H
#define AUDIOFY_EFFECT_H

#include "al_player.h"
#include "SoundTouchDLL.h"

class Effect {
public:
    virtual void applyEffect(AudioPlayBuffer<>& buffer,HANDLE h) = 0;
};

#endif //AUDIOFY_EFFECT_H
