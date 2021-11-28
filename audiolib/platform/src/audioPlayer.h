//
// Created by Jonathan on 27.11.2021.
//

#ifndef AUDIOFY_LIB_AUDIOPLAYER_H
#define AUDIOFY_LIB_AUDIOPLAYER_H

#include "al_error.h"
#include "audioDevice.h"

template<typename T = i16> struct AudioFormatInfo {
    static_assert(std::is_integral<T>::value, "AudioFormatInfo currently only suppors integral pcm data types");

    AudioFormatInfo(u32 numChannels = 2, u32 samplesPerSecond = 44100) : numberOfChannels(numChannels), sampleRate(samplesPerSecond){};

    static AudioFormatInfo<T> PCMDefault() {return AudioFormatInfo();}
    static AudioFormatInfo<T> PCMDefaultMono() {return AudioFormatInfo(1);}

    u32 numberOfChannels;
    u32 sampleRate;     //Samples per second
    size_t bitsPerSample = sizeof(T) * 8;

    u64 getSampleSize() const {return (bitsPerSample / 8) * numberOfChannels;}
    u64 getSampleCount(const u64 seconds) const {return getSampleSize() * seconds * sampleRate;}
    WAVEFORMATEX toWaveFormat() const;

};

template<typename T>
WAVEFORMATEX AudioFormatInfo<T>::toWaveFormat() const {
    WAVEFORMATEX wf;
    wf.wBitsPerSample = bitsPerSample;
    wf.nChannels = numberOfChannels;
    wf.nSamplesPerSec = sampleRate;
    wf.nBlockAlign = (wf.nChannels * wf.wBitsPerSample) / 8;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    wf.cbSize = 0;
    wf.wFormatTag = WAVE_FORMAT_PCM;
    return wf;
}

template<class T = i16> class AudioPlayBuffer {
public:
    AudioPlayBuffer(AudioFormatInfo<T> info, u64 initialSize, bool playEntireBuffer = true) : audioFormat(info), bufferSize(initialSize), playFullBuffer(playEntireBuffer){
        bufferData.reserve(initialSize);
    }

    AudioPlayBuffer() : audioFormat(AudioFormatInfo<T>::PCMDefault()), bufferSize(0){
        bufferSize = audioFormat.getSampleSize();
        bufferData.reserve(bufferSize);
    }
    XAUDIO2_BUFFER toXAudioBuffer();
    u64 getCurrentBufferSize() {return bufferData.size();}

    std::vector<T>& getRawData() {return bufferData;}
    AudioFormatInfo<T> getAudioFormat() {return audioFormat;}

private:
    u64 bufferSize;
    std::vector<T> bufferData;

    u64 playCursorStartIndex = 0;
    u64 playLength = bufferSize;
    u32 loopCount = 0;
    bool loop = false;
public:
    void setPlayCursor(const u64 cursorIndex) {playCursorStartIndex = cursorIndex;}

    u64 getPlayLength() const {
        return playLength;
    }

    void setPlayLength(u64 playLength) {
        AudioPlayBuffer::playLength = playLength;
    }

    u32 getLoopCount() const {
        return loopCount;
    }

    void setLoopCount(u32 loopCount) {
        AudioPlayBuffer::loopCount = loopCount;
    }

    bool isLoop() const {
        return loop;
    }

    void setLoop(bool _loop) {
        loop = _loop;
    }

    void setAudioFormat(const AudioFormatInfo<T> &audioFormat) {
        AudioPlayBuffer::audioFormat = audioFormat;
    }

    bool isPlayFullBuffer() const {
        return playFullBuffer;
    }

    void setPlayFullBuffer(bool playFullBuffer) {
        AudioPlayBuffer::playFullBuffer = playFullBuffer;
    }

private:
    AudioFormatInfo<T> audioFormat;
    bool playFullBuffer = true;
};


class AudioMixer {

};

class AudioPlayer : IXAudio2EngineCallback {
public:
    explicit AudioPlayer(bool debug = false,
                         _In_opt_ std::shared_ptr<AudioDevice> device = nullptr,
                         _In_opt_ AudioFormatInfo<> info = AudioFormatInfo<>::PCMDefault());

    void playAudioBuffer(_In_ AudioPlayBuffer<>& buffer);

    void setMasterVolume(float volume);
    float getCurrentVolume();
    bool isPlaying();
    void setErrorCallback(std::function<void()> callback) {onErrorCallback = std::move(callback);}

    void play();
    void pause();

    STDMETHODIMP_(void) OnCriticalError(HRESULT error) override {
        onErrorCallback();
        initEngine();
    };
    STDMETHODIMP_(void) OnProcessingPassEnd() override {}
    STDMETHODIMP_(void) OnProcessingPassStart() override {}

private:

private:
    void initEngine();

    void flipBufferData(AudioPlayBuffer<>& buffer);
    void submitBuffer();
    std::function<void()> onErrorCallback = []() { printf("CRITICAL ERROR: RESTARTING\n");};

    bool paused = true;
    bool _debug;
    Microsoft::WRL::ComPtr<IXAudio2> _context = nullptr;
    IXAudio2MasteringVoice* _master = nullptr;

    IXAudio2SourceVoice* frontVoice = nullptr;

     std::unique_ptr<AudioPlayBuffer<>> frontAudioBuffer = std::make_unique<AudioPlayBuffer<>>();

    std::shared_ptr<AudioDevice> _currentDevice = nullptr;
    AudioFormatInfo<> currentAudioFormat;
};

#endif //AUDIOFY_LIB_AUDIOPLAYER_H
