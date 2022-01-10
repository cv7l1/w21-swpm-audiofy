//
// Created by Clemens on 07.01.2022.
//
#ifndef AUDIOFY_EFFECT_H
#define AUDIOFY_EFFECT_H

#include "al_player.h"
#include "SoundTouchDLL.h"

/*
* Base Class for all Effects
* 
* All Effects used by the SoundProcessor has to inherit this Class and override applyEffect()
*/
class Effect {
public:
    /*
    * Applies the Effect to the Buffer and may use the SoundTouch handle
    */
    virtual void applyEffect(AudioPlayBuffer<>* buffer, HANDLE h) = 0;
    
    /*
    * Returns the Name of the Effect
    */
    virtual const char* getEffectName() = 0;
};
#endif //AUDIOFY_EFFECT_H