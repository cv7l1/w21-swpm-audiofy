//
// Created by Jonathan on 27.11.2021.
//

#include "al_player.h"
#include "al_debug.h"
#include <comdef.h>
#include <format>
#include <string>

template<typename T> XAUDIO2_BUFFER AudioPlayBuffer<typename T>::toXAudioBuffer() {
    XAUDIO2_BUFFER buffer = {0};
    buffer.pAudioData = reinterpret_cast<const BYTE *>(bufferData.data());
    buffer.AudioBytes = bufferData.size() * sizeof(T) ;
    if(!playFullBuffer) {
        buffer.PlayBegin = playCursorStartIndex;
        buffer.PlayLength = playLength;
        if(loop) {
            buffer.LoopBegin =  playCursorStartIndex;
            buffer.LoopLength =  playLength;
        }
    }

    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : loopCount;
    return buffer;
}


AudioPlayer::AudioPlayer(bool debug, std::shared_ptr<AudioDevice> device, AudioFormatInfo<> info) :
        _currentDevice(std::move(device)), currentAudioFormat(info), _debug(debug) {

    initEngine();
}
void AudioPlayer::attach(IAudioPlayerObserver *observer) {_observer.push_back(observer);}
void AudioPlayer::detach(IAudioPlayerObserver *observer) {_observer.remove(observer);}

void AudioPlayer::setMasterVolume(const float volume) {
    if(volume > XAUDIO2_MAX_VOLUME_LEVEL || volume < -XAUDIO2_MAX_VOLUME_LEVEL) {return;}
    throwIfFailed(_master->SetVolume(volume));
}

float AudioPlayer::getCurrentVolume() {
    float vol = 0;
    _master->GetVolume(&vol);
    return vol;
}

void AudioPlayer::stopLoop() {
    al_ErrorInfo("Stop loop");
    frontVoice->ExitLoop();
}

bool AudioPlayer::isPlaying() {
    XAUDIO2_VOICE_STATE currentState;
    frontVoice->GetState(&currentState, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    return (currentState.BuffersQueued > 0);
}

void AudioPlayer::play() {
    al_ErrorInfo("Start playing from front buffer");
    throwIfFailed(frontVoice->Start());
    paused = false;
}

void AudioPlayer::pause() {
    al_ErrorInfo("Stop plaing form front buffer");
    throwIfFailed(frontVoice->Stop());
    paused = true;
}


void AudioPlayer::flipBufferData(_In_ AudioPlayBuffer<>& buffer) {
    al_ErrorInfo("Flipping to front audio buffer");
    al_ErrorInfo(std::format("Frontbuffer: SampleRate: {}", frontAudioBuffer->getAudioFormat().sampleRate).c_str());
    al_ErrorInfo(std::format("Backbuffer: SampleRate: {}", buffer.getAudioFormat().sampleRate).c_str());
    bool wasPlaying = isPlaying();
    if(wasPlaying) {
        al_ErrorInfo("Buffer is currently playing. Stopping first...");
        pause();
        al_ErrorInfo("Flush existing buffer queue");
        throwIfFailed(frontVoice->FlushSourceBuffers());
    }
    auto msg = std::format("Copying: FrontBufferSize: {}, BackBufferSize: {}", frontAudioBuffer->getRawData().size(), buffer.getRawData().size());
    al_ErrorInfo(msg.c_str());
    frontAudioBuffer->getRawData().resize(buffer.getRawData().size());
    frontAudioBuffer->getRawData() = buffer.getRawData();
    al_ErrorInfo("Finished flipping buffers");
}

void AudioPlayer::submitBuffer() {
    al_ErrorInfo("Resubmit front buffer");
    auto xBuffer = frontAudioBuffer->toXAudioBuffer();
    if(isPlaying()) {
        al_ErrorInfo("Restarting front buffer playback");
        frontVoice->Stop();
        throwIfFailed(frontVoice->FlushSourceBuffers());
    }
    al_ErrorInfo("Wait for bufferEndEvent...");
    WaitForSingleObjectEx(bufferEndEvent, 100, TRUE);
    al_ErrorInfo("Done!");

    throwIfFailed(frontVoice->SetSourceSampleRate(frontAudioBuffer->getAudioFormat().sampleRate));
    al_ErrorInfo("Submit buffer to front voice");
    throwIfFailed(frontVoice->SubmitSourceBuffer(&xBuffer));
    al_ErrorInfo("Wé're good!");
}

void AudioPlayer::playAudioBuffer(AudioPlayBuffer<>& buffer, bool start) {
    al_ErrorInfo("Play buffer request...");
    flipBufferData(buffer);

    frontAudioBuffer->setLoop(buffer.isLoop());
    frontAudioBuffer->setPlayFullBuffer(buffer.isPlayFullBuffer());
    frontAudioBuffer->setLoopCount(buffer.getLoopCount());
    frontAudioBuffer->setPlayLength(buffer.getPlayLength());
    frontAudioBuffer->setAudioFormat(buffer.getAudioFormat());
    frontAudioBuffer->setPlayCursor(buffer.getPlayCursor());

    submitBuffer();
    al_ErrorInfo("Play buffer request done");
    if(start){play();}
}

void AudioPlayer::initEngine() {
    if(_context || _master) {
        al_ErrorInfo("XAudio2 Engine already exists: Release");
        _master->DestroyVoice();
        _context->Release();
    }
    al_ErrorInfo("Create XAudio2 Engine...");
    if(_debug) {al_ErrorInfo("...with Debug capabilites");}

    u32 flags = _debug? XAUDIO2_DEBUG_ENGINE : 0;
    al_ErrorInfo("Init COM\n");
    throwIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

    throwIfFailed(XAudio2Create(_context.GetAddressOf(), flags, XAUDIO2_USE_DEFAULT_PROCESSOR));
    al_ErrorInfo("XAudio2 Context created");
    al_ErrorInfo("Register XAudio2 Callbacks");
    throwIfFailed(_context->RegisterForCallbacks(this));

    if(_debug) {
        XAUDIO2_DEBUG_CONFIGURATION config {0};
        config.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
        config.BreakMask = XAUDIO2_LOG_ERRORS;
        config.LogFileline = true;
        _context->SetDebugConfiguration(&config);
    }

    al_ErrorInfo("Create Mastering Voice");
    WarnOnNull(_currentDevice, "No device: Use default");
    if(_currentDevice) {
        al_ErrorInfo("Using device: ");
        al_ErrorInfo(_bstr_t(_currentDevice->getDeviceName().value()));
    }
    throwIfFailed(_context->CreateMasteringVoice(&_master,
                                                 XAUDIO2_DEFAULT_CHANNELS,
                                                 44100,
                                                 0,
                                                 (_currentDevice != nullptr) ? _currentDevice->getDeviceID() : nullptr,
                                                 nullptr,
                                                 AudioCategory_Media));
    if(_master == nullptr) {throw Win32Exception(GetLastError());}

    auto wf = currentAudioFormat.toWaveFormat();
    al_ErrorInfo("Create source Voice");
    throwIfFailed(_context->CreateSourceVoice(&frontVoice,
                                              &wf,
                                              0,
                                              XAUDIO2_DEFAULT_FREQ_RATIO,
                                              this, nullptr, nullptr));
    if(frontVoice == nullptr) {throw Win32Exception(GetLastError());}
}

void AudioPlayer::seekToSample(const u64 sampleIndex) {
    bool wasPlaying = isPlaying();
    if(!sampleIndex) {
        frontAudioBuffer->setPlayCursor(0);
        frontAudioBuffer->setPlayLength(0);
    } else {
        frontAudioBuffer->setPlayFullBuffer(false);
        frontAudioBuffer->setPlayCursor(sampleIndex);
        frontAudioBuffer->setPlayLength((frontAudioBuffer->getRawData().size() / frontAudioBuffer->getAudioFormat().numberOfChannels)- sampleIndex);
    }
    this->submitBuffer();
    if(wasPlaying) {
        play();
    }
}

void AudioPlayer::OnStreamEnd() {
    for(auto& observer : _observer) {
        observer->OnStreamEnd();
    }
    al_ErrorInfo("Stream end");
}

void AudioPlayer::OnBufferEnd(void *pBufferContext) {
    for(auto& observer : _observer) {
        observer->OnBufferPlayEnd();
    }
    SetEvent(bufferEndEvent);
    al_ErrorInfo("Buffer end");
}

void AudioPlayer::OnBufferStart(void *pBufferContext) {
    for(auto& observer : _observer) {
        observer->OnBufferPlayStart();
    }
    al_ErrorInfo("Buffer start");
    ResetEvent(bufferEndEvent);
}

void AudioPlayer::OnVoiceError(void *pBufferContext, HRESULT Error) {
    for(auto& observer : _observer) {
        observer->OnError();
    }
    al_ErrorWarn("Voice error");
}

void AudioPlayer::OnLoopEnd(void *pBufferContext) {
    for(auto& observer : _observer) {
        observer->OnLoopEnd();
    }
    al_ErrorInfo("Loop end");
}
