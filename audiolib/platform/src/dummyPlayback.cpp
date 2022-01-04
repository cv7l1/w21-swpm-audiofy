//
// Created by Jonathan on 09.11.2021.
//

#include "al_device.h"
#include "al_player.h"
#include "al_file.h"
#include "al_debug.h"

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

    AudioDeviceManager manager;
    auto dev = manager.getDefaultDevice(AudioDeviceRole::Playback);

    if(dev == nullptr) {
        printf("Unable to find device\n");
        return;
    }

    AudioPlayer player(false, dev, AudioFormatInfo<i16>::PCMDefault());
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
void opusPlay() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    AudioDeviceManager manager;
    auto dev = manager.getDefaultDevice(AudioDeviceRole::Playback);

    AudioPlayer player(false, dev, AudioFormatInfo<i16>::PCMDefault());

    {
        AudioDecoder decoder;
        auto file = decoder.loadAudioFile(L"allTheTime.ogg");

        AudioPlayBuffer<i16> buffer;
        player.setErrorCallback([]() { printf("Critial error, we have to restart \n");});

        decoder.decodeAudioFile(file, buffer);

        buffer.setLoop(true);
        buffer.setPlayFullBuffer(true);

        player.playAudioBuffer(buffer, false);

    }
    player.play();

    for(;;) {

        Sleep(100);
    }
}
void opusInfo() {
    VorbisAudio audio;
    auto file = audio.openFile(L"allTheTime.ogg");

    auto seconds = file->getLengthSeconds();
    auto sampleCount = file->getSampleCount();
}
void wmfPlay() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    AudioDeviceManager manager;

    auto dev = manager.getDefaultDevice(AudioDeviceRole::Playback);

    AudioPlayer player(true,dev, AudioFormatInfo<i16>::PCMDefault());
    player.setErrorCallback([]() {printf("Critical error, restarting\n");});

    {
        AudioDecoder decoder;
        auto file = decoder.loadAudioFile(L"b.mp3");

        AudioPlayBuffer<i16> buffer;
        decoder.decodeAudioFile(file, buffer);

        buffer.setLoop(true);
        buffer.setPlayFullBuffer(true);

        player.playAudioBuffer(buffer, false);
    }

    player.seekToSample(44100 * 42);

    player.play();
    for(;;) {
        Sleep(100);
    }
}

void wavWrite() {
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    AudioDecoder decoder;
    auto file = decoder.loadAudioFile(L"b.mp3");
    auto file2 = decoder.loadAudioFile(L"allTheTime.ogg");

    AudioPlayBuffer<i16> bufferAudio1;
    decoder.decodeAudioFile(file, bufferAudio1);

    AudioPlayBuffer<i16> bufferAudio2;
    decoder.decodeAudioFile(file2, bufferAudio2);

    auto newAudioFormat = AudioFormatInfo<>::PCMDefault();

    AudioPlayBuffer<i16> mixBuffer;
    mixBuffer.getRawData().resize(bufferAudio2.getRawData().size());
    mixBuffer.setAudioFormat(bufferAudio2.getAudioFormat());

    for(int i = 0; i<mixBuffer.getRawData().size(); ++i) {
        if(i < bufferAudio1.getRawData().size()) {
            mixBuffer.getRawData()[i] = (bufferAudio1.getRawData()[i] / 4 + bufferAudio2.getRawData()[i]) / 2;
        } else {
            mixBuffer.getRawData()[i] = bufferAudio2.getRawData()[i];
        }
    }

    DebugWaveFileWriter::writeBufferToWaveFile(L"test.wav", mixBuffer);
}
int WINAPI WinMain(
        _In_ HINSTANCE  hinstance,
        _In_ HINSTANCE  hPrevInstance,
        _In_ LPSTR      lpCmdLine,
        _In_ int        nShowCmd
        ) {

    Console::setup();
    al_ErrorInfo("Let's go");
    OutputDebugStringW(L"Moin!\n");
    //Still kinda broken but who cares
    //sineWavePlaybackExample();
    //opusPlay();
    wavWrite();
    //wmfPlay();
    //opusPlaybackExample();
    //wmfPlaybackExample();
    //oggStreamPlaybackExample();
}