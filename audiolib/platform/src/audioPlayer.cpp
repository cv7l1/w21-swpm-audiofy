//
// Created by Jonathan on 27.11.2021.
//

#include "audioPlayer.h"

template<> XAUDIO2_BUFFER AudioPlayBuffer<>::toXAudioBuffer() {
    XAUDIO2_BUFFER buffer = {0};
    buffer.pAudioData = reinterpret_cast<const BYTE *>(bufferData.data());
    buffer.AudioBytes = bufferData.size();
    buffer.PlayBegin = playFullBuffer? 0 : playCursorStartIndex;
    buffer.PlayLength = playFullBuffer? 0 : playLength;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : loopCount;
    return buffer;
}

AudioPlayer::AudioPlayer(bool debug, std::shared_ptr<AudioDevice> device, AudioFormatInfo<> info) :
        _currentDevice(std::move(device)), currentAudioFormat(info), _debug(debug) {

    initEngine();
}
void AudioPlayer::setMasterVolume(const float volume) {
    if(volume > XAUDIO2_MAX_VOLUME_LEVEL || volume < -XAUDIO2_MAX_VOLUME_LEVEL) {return;}
    throwIfFailed(_master->SetVolume(volume));
}

float AudioPlayer::getCurrentVolume() {
    float vol = 0;
    _master->GetVolume(&vol);
    return vol;
}

bool AudioPlayer::isPlaying() {
    XAUDIO2_VOICE_STATE currentState;
    frontVoice->GetState(&currentState, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    return (currentState.BuffersQueued > 0 && !paused);
}

void AudioPlayer::play() {
    throwIfFailed(frontVoice->Start());
    paused = false;
}

void AudioPlayer::pause() {
    throwIfFailed(frontVoice->Stop());
    paused = true;
}

void AudioPlayer::flipBufferData(_In_ AudioPlayBuffer<>& buffer) {
    bool wasPlaying = isPlaying();
    if(wasPlaying) {
        pause();
        throwIfFailed(frontVoice->FlushSourceBuffers());
    }
    frontAudioBuffer->getRawData().resize(buffer.getRawData().size());
    frontAudioBuffer->getRawData() = buffer.getRawData();

}

void AudioPlayer::submitBuffer() {
    auto xBuffer = frontAudioBuffer->toXAudioBuffer();

    if(isPlaying()) {
        frontVoice->Stop();
        throwIfFailed(frontVoice->FlushSourceBuffers());
    }
    throwIfFailed(frontVoice->SubmitSourceBuffer(&xBuffer));
}

void AudioPlayer::playAudioBuffer(AudioPlayBuffer<>& buffer) {
    flipBufferData(buffer);

    frontAudioBuffer->setLoop(buffer.isLoop());
    frontAudioBuffer->setPlayFullBuffer(buffer.isPlayFullBuffer());
    frontAudioBuffer->setLoopCount(buffer.getLoopCount());
    frontAudioBuffer->setPlayLength(buffer.getPlayLength());

    submitBuffer();
    play();
}

void AudioPlayer::initEngine() {
    if(_context || _master) {
        _master->DestroyVoice();
        _context->Release();
    }

    u32 flags = _debug? XAUDIO2_DEBUG_ENGINE : 0;
    throwIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

    throwIfFailed(XAudio2Create(_context.GetAddressOf(), flags, XAUDIO2_USE_DEFAULT_PROCESSOR));

    if(_debug) {
        XAUDIO2_DEBUG_CONFIGURATION config {0};
        config.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
        config.BreakMask = XAUDIO2_LOG_ERRORS;
        config.LogFileline = true;
        _context->SetDebugConfiguration(&config);
    }

    throwIfFailed(_context->CreateMasteringVoice(&_master,
                                                 XAUDIO2_DEFAULT_CHANNELS,
                                                 XAUDIO2_DEFAULT_SAMPLERATE,
                                                 0,
                                                 (_currentDevice != nullptr) ? _currentDevice->getDeviceID() : nullptr,
                                                 nullptr,
                                                 AudioCategory_Media));
    if(_master == nullptr) {throw Win32Exception(GetLastError());}

    auto wf = currentAudioFormat.toWaveFormat();
    throwIfFailed(_context->CreateSourceVoice(&frontVoice,
                                              &wf,
                                              0,
                                              XAUDIO2_DEFAULT_FREQ_RATIO,
                                              nullptr, nullptr, nullptr));
    if(frontVoice == nullptr) {throw Win32Exception(GetLastError());}
}

