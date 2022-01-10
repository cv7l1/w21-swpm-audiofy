//
// Created by Clemens on 07.01.2022.
//
#ifndef AUDIOFY_DEVELOP_SOUNDPROCESSOR_H
#define AUDIOFY_DEVELOP_SOUNDPROCESSOR_H

#include "effects/Effect.h"

class SoundProcessor {
public:
    /*
    * Creates a new SoundProcessor with a Pointer to the AudioBuffer
    */
    SoundProcessor(AudioPlayBuffer<>* buffer);

    /*
    * Deletes the SoundProcessor and destroys the SoundTouch instance
    */
    ~SoundProcessor();

    /*
    * adds an Effect to the List of applying Effects
    *
    * This can be undone by calling removeEffect()
    */
    void addEffect(Effect* e);

    /*
    * removes an Effect from the List of applying Effects
    */
    void removeEffect(Effect* e);

    /*
    * Builds the AudioBuffer by applying the Effect.apply() method to the AudioBuffer
    *
    * This can not be undone
    */
    void build();

    /*
    * Returns the SoundTouch Handle used by the SoundProcessor
    */
    HANDLE getHandle() { return soundtouchHandle; }

    /*
    * Returns the AudioBuffer used by the SoundProcessor
    */
    AudioPlayBuffer<>* getBuffer();

    void reset();

private:
    std::vector<Effect*> effects = std::vector<Effect*>();

    AudioPlayBuffer<>* buffer;

    HANDLE soundtouchHandle;
};
#endif //AUDIOFY_DEVELOP_SOUNDPROCESSOR_H