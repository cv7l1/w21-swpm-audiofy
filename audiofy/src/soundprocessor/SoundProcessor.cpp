//
// Created by Clemens on 07.01.2022.
//

#include "SoundProcessor.h"

SoundProcessor::SoundProcessor(AudioPlayBuffer<>* buffer)
: buffer(buffer) , h(soundtouch_createInstance()) { }

SoundProcessor::~SoundProcessor(){

}

void SoundProcessor::addEffect(Effect* e) {
    effects.push_back(e);
}

void SoundProcessor::removeEffect(Effect* e) {
    //effects.erase(effects.size()-1);
}

void SoundProcessor::build() {
    for(auto &effect : effects) {
        effect->applyEffect(*this->buffer);
    }

}

AudioPlayBuffer<> SoundProcessor::getBuffer() {
    return *this->buffer;
}

