#include <Windows.h>
#include "win32_audio.h"
#include "memUtil.h"
#include <io.h>

namespace PlatformWin32 {
    u32 winFileHandleToCFileHandle(HANDLE *winFileHandle, FILE **cFileHandle) {
         if(!winFileHandle) {return 0;}
         int cDesc = _open_osfhandle(reinterpret_cast<intptr_t>(*winFileHandle), 0);
         if(cDesc == -1) {
             return 0;
         }
         *cFileHandle = _fdopen(cDesc, "r+b");
         return 1;
    }

    constexpr u32 INVALID_PROP_DESCRIPTOR = 2;
    u32 getDescFromEndpoint(_In_ IMMDevice* endpoint,
                            _Out_ AudioDevice* _device) {
        using Microsoft::WRL::ComPtr;
        HRESULT result;
        LPWSTR id = nullptr;
        result = endpoint->GetId(&id);
        if(FAILED(result) || id == nullptr) {return 0;}
        _device->deviceID = id;

        ComPtr<IPropertyStore> props;
        result = endpoint->OpenPropertyStore(STGM_READ, props.GetAddressOf());
        if(FAILED(result) || props.GetAddressOf() == nullptr) {return 0;}
        PROPVARIANT var;
        PropVariantInit(&var);
        result = props->GetValue(PKEY_Device_FriendlyName, &var);
        if(FAILED(result)) {return 0;}

        _device->endpoint = endpoint;
        _device->var = var;

        if(var.vt != VT_LPWSTR) {
            _device->description = nullptr;
            return INVALID_PROP_DESCRIPTOR;
        }

        _device->description = var.pwszVal;
        DWORD state;
        result = endpoint->GetState(&state);
        if(FAILED(result)) {return 0;}
        _device->currentState = state;

        return 1;
    }

    u32 getAvailableAudioOutputDevices(_Out_ AudioDeviceList* _devices,
                                       _Out_ u32* numDevices) {
        using Microsoft::WRL::ComPtr;
        ComPtr<IMMDeviceEnumerator> devEnum;
        ComPtr<IMMDeviceCollection> devices;
        AudioDevice* deviceList = nullptr;
        u32 deviceCount = 0;

        HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(devEnum.GetAddressOf()));
        if(FAILED(result)) {
            goto exit;
        }

        result = devEnum->EnumAudioEndpoints(eRender,
                                             DEVICE_STATE_ACTIVE,
                                             &devices);
        if(FAILED(result)) {goto exit;}

        result = devices->GetCount(&deviceCount);
        if(FAILED(result) || deviceCount == 0) {goto exit;}

        *numDevices = deviceCount;
        deviceList = new AudioDevice[deviceCount];

        for(u32 i = 0; i < deviceCount; ++i) {
            ComPtr<IMMDevice> endpoint;
            result = devices->Item(i, endpoint.GetAddressOf());
            if(FAILED(result) || endpoint.GetAddressOf() == nullptr) {goto exit;}

            auto res = getDescFromEndpoint(endpoint.Get(), &deviceList[i]);
            if(!res) {goto exit;}
            if(res == INVALID_PROP_DESCRIPTOR) {continue;}
        }

        _devices->deviceCount = deviceCount;
        _devices->devices = deviceList;

        return 1;
        exit:
        errno = result;
        return 0;
    }

    u32 getDefaultAudioOutputDevice(_Out_ AudioDevice* device) {
        using Microsoft::WRL::ComPtr;
        AudioDevice defaultDevice {0};

        ComPtr<IMMDeviceEnumerator> devEnum;
        HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(devEnum.GetAddressOf()));

        ComPtr<IMMDevice> endpoint;

        if(FAILED(result)) {
            goto exit;
        }

        result = devEnum->GetDefaultAudioEndpoint(eRender, eMultimedia,
                                                  endpoint.GetAddressOf());
        if(FAILED(result)) {
            goto exit;
        }
        if(!getDescFromEndpoint(endpoint.Get(),&defaultDevice)) {goto exit;}
        return 1;
        exit:
        {
            errno = result;
            return 0;
        };

    }
    void freeAudioDevice(_In_ AudioDevice *device);

    void freeAudioDevice(AudioDevice *device) {
        CoTaskMemFree(device->deviceID);
        device->deviceID = nullptr;
        PropVariantClear(&device->var);
        device->description = nullptr;
        device->endpoint->Release();
        device->endpoint = nullptr;
    }

    HRESULT AudioDeviceNotifiactionHandler::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceID) {
        //TODO: Handle this
        return S_OK;
    }

    HRESULT AudioDeviceNotifiactionHandler::OnDeviceAdded(LPCWSTR pwstrDeviceId) {
        //TODO: Handle this
        return S_OK;
    }

    HRESULT AudioDeviceNotifiactionHandler::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
        //TODO: Handle this
        return S_OK;
    }

    HRESULT AudioDeviceNotifiactionHandler::OnPropertyValueChanged() {
        //TODO: Handle this
        return S_OK;
    }

    void freeAudioDeviceList(_In_ AudioDeviceList* list) {
        for(int i = 0; i<list->deviceCount; ++i) {
            freeAudioDevice(&list->devices[i]);
        }
        delete[] list->devices;
        list->deviceCount = 0;
        list->devices = nullptr;
    }

    u32 setupAudioPlayback(bool debug,
                           _In_opt_ AudioDevice *device,
                           _Out_ AudioPlaybackContext *context) {
        IXAudio2* xAudioContext = nullptr;
        IXAudio2MasteringVoice* masterV = nullptr;

        u32 flags = debug? XAUDIO2_DEBUG_ENGINE : 0;
        HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if(FAILED(result)) {goto exitFailure;}

        result = XAudio2Create(&xAudioContext, flags, XAUDIO2_USE_DEFAULT_PROCESSOR);
        if(FAILED(result) || xAudioContext == nullptr) {goto exitFailure;}

        if(debug) {
            XAUDIO2_DEBUG_CONFIGURATION config {0};
            config.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
            config.BreakMask = XAUDIO2_LOG_ERRORS;
            config.LogFileline = true;
            xAudioContext->SetDebugConfiguration(&config);
        }

        result = xAudioContext->CreateMasteringVoice(&masterV,
                                                     XAUDIO2_DEFAULT_CHANNELS,
                                                     XAUDIO2_DEFAULT_SAMPLERATE,
                                                     0,
                                                     (device != nullptr) ? device->deviceID : nullptr,
                                                     nullptr,
                                                     AudioCategory_Media);
        if(FAILED(result) || masterV == nullptr) {goto exitFailure;}
        context->master = masterV;
        context->xaudio = xAudioContext;

        return 1;

        exitFailure: {
            if(masterV) {masterV->DestroyVoice();}
            if(xAudioContext) {xAudioContext->Release();}
            return 0;
        };
    }

    u32 setMasterVolume(AudioPlaybackContext *context, const float volume) {
        if(volume > XAUDIO2_MAX_VOLUME_LEVEL || volume < -XAUDIO2_MAX_VOLUME_LEVEL) {return 0;}

        HRESULT result = context->master->SetVolume(volume);
        if(FAILED(result)) {return 0;}
        return 1;
    }

    u32 submitSoundBuffer(_Inout_ AudioPlaybackContext *context,
                          _In_ PCMAudioBufferInfo *buffer,
                          _Out_ AudioHandle *handle) {

        IXAudio2SourceVoice* source = nullptr;
        HRESULT result = context->xaudio->CreateSourceVoice(&source, &buffer->waveformat,
                                                            0,
                                                            XAUDIO2_DEFAULT_FREQ_RATIO,
                                                            nullptr,
                                                            nullptr,
                                                            nullptr);
        if(FAILED(result) || source == nullptr) {return 0;}

        XAUDIO2_BUFFER xaudio2Buffer{};
        xaudio2Buffer.AudioBytes = buffer->bufferSize;
        xaudio2Buffer.pAudioData = buffer->rawDataBuffer;
        xaudio2Buffer.Flags = XAUDIO2_END_OF_STREAM;
        xaudio2Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        //TODO: Handle loop
        AudioHandle _handle {0};
        _handle.source = source;
        _handle.audioInfo = *buffer;
        _handle.loop = false;

        result = source->SubmitSourceBuffer(&xaudio2Buffer);
        if(FAILED(result)) {
            source->DestroyVoice();
            _handle.source = nullptr;

            return 0;
        }
        *handle = _handle;
        return 1;
    }

    u32 decodeVorbisFile(_In_ VorbisDecoderFileApi *api,
                          _In_z_ const wchar_t *filePath,
                          _Out_ PCMAudioBufferInfo* buffer) {


        OggVorbis_File vorbisFile;
        HANDLE winFileHandle = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL, nullptr);
        if(winFileHandle == INVALID_HANDLE_VALUE) {return 0;}
        FILE* cFile;
        auto result = winFileHandleToCFileHandle(&winFileHandle, &cFile);
        if(!result) { CloseHandle(winFileHandle);}
        result = api->ov_open_callbacks(cFile, &vorbisFile, NULL, 0, OV_CALLBACKS_NOCLOSE);
        if(result < 0) {return 0;}

        vorbis_info* info = api->ov_info(&vorbisFile, -1);
        u64 pcmSize = api->ov_pcm_total(&vorbisFile, -1);

        size_t blockSize = 4096;
        bool eof = false;
        i16 *bufferRawData = static_cast<i16 *>(malloc(pcmSize * sizeof(i16) * 2));
        int currentSection;
        u64 bytesRead = 0;

        i8* dest = reinterpret_cast<i8 *>(bufferRawData);
        while(!eof) {
            u64 ret = api->ov_read(&vorbisFile, reinterpret_cast<char*>(dest),
                                   blockSize, 0, 2, 1, &currentSection);
            if(ret == 0) {
                eof = true;
            } else {
                bytesRead += ret;
                dest += ret;
            }
        }
        WAVEFORMATEX wf {0};
        wf.wFormatTag = WAVE_FORMAT_PCM;
        wf.nChannels = info->channels;
        wf.nSamplesPerSec = info->rate;
        wf.wBitsPerSample = 16;
        wf.nBlockAlign = (wf.nChannels * wf.wBitsPerSample) / 8;
        wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
        wf.cbSize = 0;

        buffer->waveformat = wf;
        buffer->rawDataBuffer = reinterpret_cast<u8 *>(bufferRawData);
        buffer->bufferSize = pcmSize * sizeof(i16) * 2;
        return 1;
    }
}