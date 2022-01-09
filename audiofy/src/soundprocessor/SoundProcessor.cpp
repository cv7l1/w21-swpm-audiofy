//
// Created by Clemens on 07.01.2022.
//

#include "SoundProcessor.h"

SoundProcessor::SoundProcessor(AudioPlayBuffer<>* buffer)
: buffer(buffer) , soundtouchHandle(soundtouch_createInstance()) { 
    soundtouch_setChannels(soundtouchHandle,2);
    soundtouch_setSampleRate(soundtouchHandle, buffer->getAudioFormat().sampleRate);
    soundtouch_putSamples_i16(soundtouchHandle,buffer->getRawData().data(),buffer->getRawData().size());
}

SoundProcessor::~SoundProcessor(){
    soundtouch_clear(soundtouchHandle);
    soundtouch_destroyInstance(soundtouchHandle);
}

void SoundProcessor::addEffect(Effect* e) {
    effects.push_back(e);
}

void SoundProcessor::removeEffect(Effect* e) {
    //effects.erase(effects.size()-1);
}

void SoundProcessor::build() {
    for(auto &effect : effects) {
        effect->applyEffect(*this->buffer, soundtouchHandle);
    }

}

AudioPlayBuffer<> SoundProcessor::getBuffer() {
    return *this->buffer;
}

