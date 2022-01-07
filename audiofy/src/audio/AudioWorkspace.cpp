//
// Created by Jonathan on 04.01.2022.
//

#include "AudioWorkspace.h"

AudioContext::AudioContext() {
    try {
         _player = new AudioPlayer(false, nullptr, AudioFormatInfo<>::PCMDefault());
    } catch(std::exception& e) {
        MessageBoxW(nullptr, L"Unable to set up", nullptr, MB_OK);
        throw;
    }
   _decoder = new AudioDecoder();
}
