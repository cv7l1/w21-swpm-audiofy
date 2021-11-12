#include <Windows.h>
#include "win32_audio.h"
#include "memUtil.h"
#include <io.h>

namespace PlatformWin32 {
    /*! \brief Utility function to retrieve a FILE* from a HANDLE
     \param [in] winFileHandle Pointer to a HANDLE created using OpenFile
     \param [out] cFileHandle Resulting File Pointer
     \return Error code
     */
    AudiolibError winFileHandleToCFileHandle(_Inout_ _Notnull_ HANDLE *winFileHandle,
                                             _Outptr_ FILE **cFileHandle) {
         if(!winFileHandle) {return AUDIOLIB_INVALID_PARAMETER;}
         int cDesc = _open_osfhandle(reinterpret_cast<intptr_t>(*winFileHandle), 0);
         if(cDesc == -1) {return AUDIOLIB_IO_ERROR;}
         *cFileHandle = _fdopen(cDesc, "r+b");
         if(*cFileHandle == nullptr) {return AUDIOLIB_IO_ERROR;}

         return AUDIOLIB_OK;
    }

    /*!
     *
     * @brief Retrieve information about an audio device, including the GUID
     * @param endpoint [In] Audio Endpoint
     * @param _device [Out] Structure containing information about the audio endpoint
     * @return Error code
     */
    AudiolibError getDescFromEndpoint(_In_ IMMDevice* endpoint,
                                      _Out_ AudioDevice* _device) {
        using Microsoft::WRL::ComPtr;

        if(endpoint == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}

        u32 result = S_OK;
        wchar_t* id = nullptr;
        result = endpoint->GetId(&id);

        if(FAILED(result) || id == nullptr) {return AUDIOLIB_ENDPOINT_INFO_ERROR;}
        _device->deviceID = id;

        ComPtr<IPropertyStore> props;
        result = endpoint->OpenPropertyStore(STGM_READ, props.GetAddressOf());
        if(FAILED(result) || props.GetAddressOf() == nullptr) {return AUDIOLIB_ENDPOINT_INFO_ERROR;}

        PROPVARIANT var = {0};
        PropVariantInit(&var);
        result = props->GetValue(PKEY_Device_FriendlyName, &var);
        if(FAILED(result)) {return AUDIOLIB_ENDPOINT_INFO_ERROR;}

        _device->endpoint = endpoint;
        _device->var = var;

        if(var.vt != VT_LPWSTR) {
            _device->description = nullptr;
            return AUDIOLIB_ENDPOINT_INVALID_PROP_DESC;
        }

        _device->description = var.pwszVal;
        DWORD state = 0;
        result = endpoint->GetState(0);
        if(FAILED(result)) {return AUDIOLIB_ENDPOINT_INFO_ERROR;}
        _device->currentState = state;

        return AUDIOLIB_OK;
    }

    AudiolibError getAvailableAudioDevices(_Out_ AudioDeviceList* _devices,
                                                 AudiolibDeviceRole role,
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

        if(FAILED(result)) {return AUDIOLIB_ENDPOINT_RETRIEVAL;}

        result = devEnum->EnumAudioEndpoints(static_cast<EDataFlow>(role),
                                             DEVICE_STATE_ACTIVE,
                                             &devices);
        if(FAILED(result)) {return AUDIOLIB_ENDPOINT_RETRIEVAL;}

        result = devices->GetCount(&deviceCount);
        if(FAILED(result) || deviceCount == 0) {return AUDIOLIB_ENDPOINT_NODEVICE;}

        *numDevices = deviceCount;
        deviceList = new AudioDevice[deviceCount];

        for(u32 i = 0; i < deviceCount; ++i) {
            ComPtr<IMMDevice> endpoint;
            result = devices->Item(i, endpoint.GetAddressOf());
            if(FAILED(result) || endpoint.GetAddressOf() == nullptr) {
                delete[] deviceList;
                return AUDIOLIB_ENDPOINT_RETRIEVAL;
            }

            auto res = getDescFromEndpoint(endpoint.Get(), &deviceList[i]);
            if(res == AUDIOLIB_ENDPOINT_INVALID_PROP_DESC) {continue;}
            if(res != AUDIOLIB_OK) {return res;}
        }

        _devices->deviceCount = deviceCount;
        _devices->devices = deviceList;

        return AUDIOLIB_OK;
    }

    AudiolibError getDefaultAudioOutputDevice(_Out_ AudioDevice* device, AudiolibDeviceRole role) {
        using Microsoft::WRL::ComPtr;
        AudioDevice defaultDevice {0};

        ComPtr<IMMDeviceEnumerator> devEnum;
        ComPtr<IMMDevice> endpoint;
        HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(devEnum.GetAddressOf()));


        if(FAILED(result)) {return AUDIOLIB_ENDPOINT_RETRIEVAL;}

        result = devEnum->GetDefaultAudioEndpoint(static_cast<EDataFlow>(role), eMultimedia,
                                                  endpoint.GetAddressOf());
        if(FAILED(result)) {
            if(result == E_POINTER || result == E_INVALIDARG) {return AUDIOLIB_ENDPOINT_INFO_ERROR;}
            else if(result == E_OUTOFMEMORY) {return AUDIOLIB_OUTOFMEMORY;}
            else if(result == E_NOTFOUND) {return AUDIOLIB_ENDPOINT_NODEVICE;}
        }
        AudiolibError aResult = getDescFromEndpoint(endpoint.Get(),&defaultDevice);
        if(aResult != AUDIOLIB_OK) {return aResult;}
        return AUDIOLIB_OK;

    }
    void freeAudioDevice(_Frees_ptr_opt_ AudioDevice *device) {
        if(device == nullptr) {
            return;
        }

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

    void freeAudioDeviceList(_Frees_ptr_opt_ AudioDeviceList* list) {
        if(list == nullptr) {
            return;
        }
        for(int i = 0; i<list->deviceCount; ++i) {
            freeAudioDevice(&list->devices[i]);
        }
        delete[] list->devices;
        list->deviceCount = 0;
        list->devices = nullptr;
    }

    AudiolibError setupAudioPlayback(bool debug,
                           _In_opt_ AudioDevice *device,
                           _Out_ AudioPlaybackContext *context) {

        IXAudio2* xAudioContext = nullptr;
        IXAudio2MasteringVoice* masterV = nullptr;

        u32 flags = debug? XAUDIO2_DEBUG_ENGINE : 0;
        HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if(FAILED(result)) {return AUDIOLIB_COINIT_FAILURE;}

        result = XAudio2Create(&xAudioContext, flags, XAUDIO2_USE_DEFAULT_PROCESSOR);
        if(FAILED(result) || xAudioContext == nullptr) {return AUDIOLIB_XAUDIO_CONTEXT_CREATION;}

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

        if(FAILED(result) || masterV == nullptr) {return AUDIOLIB_XAUDIO_VOICE_CREATION;}
        context->master = masterV;
        context->xaudio = xAudioContext;

        return AUDIOLIB_OK;
    }

    AudiolibError setMasterVolume(AudioPlaybackContext *context, const float volume) {
        if(context == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        if(volume > XAUDIO2_MAX_VOLUME_LEVEL || volume < -XAUDIO2_MAX_VOLUME_LEVEL) {return AUDIOLIB_INVALID_PARAMETER;}

        HRESULT result = context->master->SetVolume(volume);
        if(FAILED(result)) {return AUDIOLIB_XAUDIO_GENERIC_ERROR;}
        return AUDIOLIB_OK;
    }

    WAVEFORMATEX audioInfoToWF(_In_ AudioFormatInfo* info) {
        WAVEFORMATEX wf {0};
        wf.wFormatTag = WAVE_FORMAT_PCM;
        wf.nSamplesPerSec = info->sampleRate;
        wf.wBitsPerSample = info->bitsPerSample;
        wf.nChannels = info->numberOfChannels;
        wf.nBlockAlign = (wf.nChannels * wf.wBitsPerSample) / 8;
        wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
        wf.cbSize = 0;
        return wf;
    }

    AudiolibError submitSoundBuffer(_Inout_ AudioPlaybackContext *context,
                          _In_ PCMAudioBufferInfo *buffer,
                          _Out_ AudioHandle *handle,
                          bool loop) {

        if(context == nullptr || buffer == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        IXAudio2SourceVoice* source = nullptr;
        WAVEFORMATEX wf = audioInfoToWF(&buffer->audioInfo);

        HRESULT result = context->xaudio->CreateSourceVoice(&source, &wf,
                                                            0,
                                                            XAUDIO2_MAX_FREQ_RATIO,
                                                            nullptr,
                                                            nullptr,
                                                            nullptr);
        if(FAILED(result) || source == nullptr) {return AUDIOLIB_XAUDIO_SOURCE_CREATION;}

        XAUDIO2_BUFFER xaudio2Buffer{};
        xaudio2Buffer.AudioBytes = buffer->bufferSize;
        xaudio2Buffer.pAudioData = buffer->rawDataBuffer;
        xaudio2Buffer.Flags = XAUDIO2_END_OF_STREAM;
        xaudio2Buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

        AudioHandle _handle {0};
        _handle.source = source;
        _handle.audioInfo = *buffer;
        _handle.loop = loop;

        result = source->SubmitSourceBuffer(&xaudio2Buffer);
        if(FAILED(result)) {
            source->DestroyVoice();
            _handle.source = nullptr;
            return AUDIOLIB_XAUDIO_SUBMIT_BUFFER;
        }

        *handle = _handle;
        return AUDIOLIB_OK;
    }

    AudiolibError playAudioBuffer(AudioPlaybackContext *context, AudioHandle *handle, bool loop) {
        if(context == nullptr || handle == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        if(handle->source == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        HRESULT result = handle->source->Start();
        if(FAILED(result)) {return AUDIOLIB_XAUDIO_GENERIC_ERROR;}
        return AUDIOLIB_OK;
    }

    AudiolibError decodeVorbisFile(_In_ VorbisDecoderFileApi *api,
                          _In_z_ const wchar_t *filePath,
                          _Out_ PCMAudioBufferInfo* buffer) {


        if(api == nullptr || filePath == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        OggVorbis_File vorbisFile;
        HANDLE winFileHandle = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL, nullptr);

        if(winFileHandle == INVALID_HANDLE_VALUE) {return AUDIOLIB_IO_ERROR;}
        FILE* cFile;
        auto result = winFileHandleToCFileHandle(&winFileHandle, &cFile);
        if(result != AUDIOLIB_OK || cFile == nullptr) {
            CloseHandle(winFileHandle);
            return result;
        }

        int oggResult = api->ov_open_callbacks(cFile, &vorbisFile, nullptr, 0, OV_CALLBACKS_NOCLOSE);
        if(oggResult < 0) {
            fclose(cFile);
            return AUDIOLIB_OGG_FAILURE;
        }

        vorbis_info* info = api->ov_info(&vorbisFile, -1);
        if(info == nullptr) {
            fclose(cFile);
            return AUDIOLIB_OGG_FAILURE;
        }

        i64 pcmSize = api->ov_pcm_total(&vorbisFile, -1);
        if(pcmSize <= 0) {
            fclose(cFile);
            return AUDIOLIB_OGG_FAILURE;
        }

        size_t blockSize = 4096;
        bool eof = false;
        i16 *bufferRawData = static_cast<i16 *>(malloc(pcmSize * sizeof(i16) * 2));

        if(bufferRawData == nullptr) {
            fclose(cFile);
            return AUDIOLIB_OUTOFMEMORY;
        }

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

        AudioFormatInfo audioInfo{0};
        audioInfo.sampleRate = info->rate;
        audioInfo.bitsPerSample = 16;
        audioInfo.numberOfChannels = info->channels;

        buffer->audioInfo = audioInfo;
        buffer->rawDataBuffer = reinterpret_cast<u8 *>(bufferRawData);
        buffer->bufferSize = pcmSize * sizeof(i16) * 2;
        return AUDIOLIB_OK;
    }

    AudiolibError mediaFoundationOpenEncodedAudioFile(_In_z_ const wchar_t *path,
                                            _Out_ MediaFoundationAudioDecoder *decoder) {

        if(path == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}

        IMFSourceReader* reader = nullptr;
        HRESULT result = MFCreateSourceReaderFromURL(path, nullptr, &reader);
        if(FAILED(result) || reader == nullptr) {
            OutputDebugStringW(L"Unable to open audio file from URL");
            return AUDIOLIB_MF_OPENURL;
        }
        IMFMediaType* uncompressedAudioType = nullptr;
        IMFMediaType* partialType = nullptr;
        result = reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
        if(FAILED(result)) {
            OutputDebugStringW(L"Unable to select stream");
            SAFE_RELEASE(reader);
            return AUDIOLIB_MF_GENERIC;
        }
        result = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
        if(FAILED(result)) {
            OutputDebugStringW(L"Unable to select stream");
            SAFE_RELEASE(reader);
            return AUDIOLIB_MF_GENERIC;
        }
        result = MFCreateMediaType(&partialType);
        if(FAILED(result)) {
            OutputDebugStringW(L"Unable to create media type");
            SAFE_RELEASE(reader);
            return AUDIOLIB_MF_GENERIC;
        }

        //NOTE: I'll add error handling proper later. Usually nothing bad should happen here...
        MFCreateMediaType(&partialType);
        partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                                    nullptr, partialType);
        reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &uncompressedAudioType);
        reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
        WAVEFORMATEX* waveFormat;
        size_t waveSize = 0;
        result = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType,
                                                     reinterpret_cast<WAVEFORMATEX **>(&waveFormat),
                                                     &waveSize,
                                                     MFWaveFormatExConvertFlag_Normal);
        if(FAILED(result) || waveFormat == nullptr) {
            OutputDebugStringW(L"Unable to retrieve wave format form media type\n");
            return AUDIOLIB_MF_WF;
        }
        decoder->reader = reader;
        decoder->mediaType = uncompressedAudioType;
        decoder->wf = reinterpret_cast<WAVEFORMATEX*>(waveFormat);

        return AUDIOLIB_OK;
    }

    AudiolibError mediaFoundationGetAudioDuration(MediaFoundationAudioDecoder *decoder, u64 *durationMS) {
        if(decoder == nullptr || durationMS == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}

        PROPVARIANT prop;
        HRESULT result = decoder->reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop);
        if(FAILED(result)) {return AUDIOLIB_MF_GENERIC;}

        const MFTIME ONE_SECOND = 10000000;
        const LONG   ONE_MSEC = 1000;

        u64 duration = prop.uhVal.QuadPart / (ONE_SECOND / ONE_MSEC);
        *durationMS = duration;
        return AUDIOLIB_OK;
    }

    AudiolibError
    mediaFoundationDecodeSample(_In_ MediaFoundationAudioDecoder *decoder,
                                _Out_writes_bytes_all_(dataWritten) _Notnull_ u8 *dest,
                                size_t destBufferSize,
                                u32 maxAudioData,
                                u32 offset,
                                _Out_ u32 *dataWritten,
                                _Out_ bool *eof) {

        IMFMediaBuffer* buffer = nullptr;
        IMFSample* sample = nullptr;

        DWORD flags;
        HRESULT result = decoder->reader->ReadSample(
                MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                0,
                nullptr,
                &flags,
                nullptr,
                &sample
                );

        if(FAILED(result) || sample == nullptr) {
            OutputDebugStringW(L"MF: Unable to read sample\n");
            return AUDIOLIB_MF_GENERIC;
        }

        result = sample->ConvertToContiguousBuffer(&buffer);
        if(FAILED(result) || sample == nullptr) {
            OutputDebugStringW(L"Unable to get sample buffer from reader \n");
            sample->Release();
            return AUDIOLIB_MF_GENERIC;
        }

        DWORD bufferReadCount;
        u8* audioData = nullptr;

        result = buffer->Lock(&audioData, nullptr, &bufferReadCount);
        if(FAILED(result) || audioData == nullptr) {
            OutputDebugStringW(L"Unable to get audio data from buffer\n");
            sample->Release();
            buffer->Release();
            return AUDIOLIB_MF_GENERIC;
        }

        bool endOfStream = flags & MF_SOURCE_READERF_ENDOFSTREAM;
        if(bufferReadCount >= maxAudioData || (bufferReadCount + offset) >= destBufferSize) {
            buffer->Unlock();

            sample->Release();
            buffer->Release();
            *eof = true;
            return AUDIOLIB_MF_READMAX;
        }

        memcpy(dest + offset , audioData, bufferReadCount);
        result = buffer->Unlock();
        if(FAILED(result)) {return AUDIOLIB_MF_GENERIC;}
        sample->Release();
        buffer->Release();
        *dataWritten = bufferReadCount;
        *eof = endOfStream;

        return AUDIOLIB_OK;
    }

    AudiolibError mediaFoundationDecodeFile(const wchar_t *path, PCMAudioBufferInfo *bufferOut) {
        AudiolibError aResult;
        MediaFoundationAudioDecoder decoder {0};
        aResult = mediaFoundationOpenEncodedAudioFile(path, &decoder);
        if(aResult != AUDIOLIB_OK) {return aResult;}

        bool _eof = false;
        u64 duration = 0;

        aResult = mediaFoundationGetAudioDuration(&decoder, &duration);
        if(aResult != AUDIOLIB_OK) {return aResult;}
        size_t bufferSize = decoder.wf->nAvgBytesPerSec * (u64)(duration / 1000);

        i16* buffer = static_cast<i16 *>(malloc(bufferSize));
        u64 offset = 0;
        u32 dataWritten = 0;
        while(!_eof) {
            aResult = mediaFoundationDecodeSample(&decoder, reinterpret_cast<u8 *>(buffer), bufferSize + 1, bufferSize, offset, &dataWritten, &_eof);
            offset += dataWritten;
            if(aResult == AUDIOLIB_MF_READMAX) {
                break;
            }
            if(aResult != AUDIOLIB_OK) {
                return aResult;
            }
        }
        bufferOut->bufferSize = bufferSize;
        bufferOut->rawDataBuffer = reinterpret_cast<u8 *>(buffer);
        AudioFormatInfo info {0};
        info.bitsPerSample = decoder.wf->wBitsPerSample;
        info.sampleRate = decoder.wf->nSamplesPerSec;
        info.numberOfChannels= decoder.wf->nChannels;

        bufferOut->audioInfo = info;
        return AUDIOLIB_OK;
    }
}