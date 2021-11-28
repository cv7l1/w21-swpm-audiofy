//
// Created by Jonathan on 09.11.2021.
//

#include "audioDevice.h"
#include "audioPlayer.h"
#include <cmath>

#define M_PI 3.14159265358979323846

i16 sineWave(double time, double freq, double amplitude) {
    i16 result;
    double tpc = 44100 / freq;
    double cycles = time / tpc;
    double rad = M_PI * 2 * cycles;
    i16 _amp = 32767 * amplitude;
    result = _amp  * sin(rad);
    return result;
}
std::vector<i16> createSineWaveBuffer(size_t len) {
    std::vector<i16> vec;

    for(int i = 0; i<len - 1; i+=2) {
        //This should give us a middle C4 (for reference: https://www.youtube.com/watch?v=t8ZE1Mlkg2s)
        i16 currentSample = sineWave(i, 261.6, 0.9);
        vec.push_back(currentSample);
        vec.push_back(currentSample);
    }
    return vec;
}

void sineWavePlaybackExample() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    size_t bufferLen = 44100 * 2 * 8;
    auto sine = createSineWaveBuffer(bufferLen);

    auto dev = AudioDevice::getDefaultDevice(AudioDeviceRole::Playback);
    auto devPtr = std::make_shared<AudioDevice>(dev.value());
    AudioPlayer player(false, devPtr, AudioFormatInfo<i16>::PCMDefault());
    player.setErrorCallback([]() { printf("Critial error, we have to restart \n");});

    AudioPlayBuffer audioBuffer(AudioFormatInfo<i16>::PCMDefault(), bufferLen);

    audioBuffer.setLoop(true);
    audioBuffer.setPlayFullBuffer(true);

    audioBuffer.getRawData() = sine;

    player.playAudioBuffer(audioBuffer);

    for(;;) {
        Sleep(100);
    }
}
int WINAPI WinMain(
        _In_ HINSTANCE  hinstance,
        _In_ HINSTANCE  hPrevInstance,
        _In_ LPSTR      lpCmdLine,
        _In_ int        nShowCmd
        ) {

    OutputDebugStringW(L"Moin!\n");
    //Still kinda broken but who cares
    sineWavePlaybackExample();
    //opusPlaybackExample();
    //wmfPlaybackExample();
    //oggStreamPlaybackExample();

}