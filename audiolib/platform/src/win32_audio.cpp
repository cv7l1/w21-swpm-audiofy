#include <Windows.h>
#include "win32_audio.h"
#include "memUtil.h"
#include <io.h>
#include <propvarutil.h>
#include <Audioclient.h>
#include<mmreg.h>
namespace PlatformWin32 {

#define min(a, b) (((a) < (b)) ? (a) : (b))

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

    void DebugNotifyErrorLoud(const wchar_t* message) {
        MessageBoxW(nullptr, message, nullptr, MB_HELP | MB_OK);
    }

    void DebugHaltAndCatchFire() {
        PostMessageW(nullptr, WM_CLOSE, 0, 0);
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
        result = endpoint->GetState(&state);
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
        IMMDevice* endpoint;
        HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(devEnum.GetAddressOf()));


        if(FAILED(result)) {return AUDIOLIB_ENDPOINT_RETRIEVAL;}

        result = devEnum->GetDefaultAudioEndpoint(static_cast<EDataFlow>(role), eMultimedia,
                                                  &endpoint);
        if(FAILED(result)) {
            if(result == E_POINTER || result == E_INVALIDARG) {return AUDIOLIB_ENDPOINT_INFO_ERROR;}
            else if(result == E_OUTOFMEMORY) {return AUDIOLIB_OUTOFMEMORY;}
            else if(result == E_NOTFOUND) {return AUDIOLIB_ENDPOINT_NODEVICE;}
        }

        AudiolibError aResult = getDescFromEndpoint(endpoint,&defaultDevice);
        device->endpoint = endpoint;
        device->description = defaultDevice.description;
        device->deviceID = defaultDevice.deviceID;

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
        wf.wFormatTag = (info->optTag > 0) ? info->optTag : WAVE_FORMAT_PCM;
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
                          _In_opt_ WAVEFORMATEXTENSIBLE *wfopt,
                          _Out_ AudioHandle *handle,
                          bool loop) {

        if(context == nullptr || buffer == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        IXAudio2SourceVoice* source = nullptr;
        WAVEFORMATEX wf {0};
        wf = audioInfoToWF(&buffer->audioInfo);

        HRESULT result = context->xaudio->CreateSourceVoice(&source, wfopt? reinterpret_cast<WAVEFORMATEX *>(wfopt) : &wf,
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
                                                     reinterpret_cast<u32*>(&waveSize),
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
        MediaFoundationAudioDecoder decoder {nullptr};
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

    //The DebugHaltAndCatchFire() calls may or may not actually do anthing here. windows is weird.
    DWORD WINAPI vorbisStreamThread(LPVOID lParam) {
        auto* sound = static_cast<VorbisAudioStreamContext*>(lParam);
        if(!sound) {DebugHaltAndCatchFire();}
        u8* buffers = static_cast<u8 *>(malloc(
                sound->individualStreamingBufferSize * sound->streamingBufferCount));

        if(buffers == nullptr) {DebugHaltAndCatchFire();}
        auto result = sound->source->Start();

        u32 currentDiskReadBufffer = 0;
        u32 currentPosition = 0;

        bool eof = false;
        int currentSection;
        while(!eof) {
            if(SUCCEEDED(result)) {
                u64 bytesRead = 0;

                u64 ret = sound->vorbisContext->ov_read(sound->file, reinterpret_cast<char *>(buffers + (sound->individualStreamingBufferSize * currentDiskReadBufffer)),
                                                            sound->individualStreamingBufferSize,
                                                            0, 2, 1 ,
                                                            &currentSection);
                if(ret == 0) {
                    eof = true;
                } else {
                    bytesRead += ret;
                    currentPosition += bytesRead;
                }

                XAUDIO2_VOICE_STATE state;

                while(sound->source->GetState(&state), state.BuffersQueued >= sound->streamingBufferCount - 1) {
                    WaitForSingleObject(sound->streamingContext->hBufferEndEvent, INFINITE);
                }

                XAUDIO2_BUFFER buf = {0};
                buf.AudioBytes = bytesRead;
                buf.pAudioData = reinterpret_cast<const BYTE *>(buffers + (sound->individualStreamingBufferSize * currentDiskReadBufffer));
                if(eof) {
                    if(sound->loop) {
                        if(sound->vorbisContext->ov_raw_seek(sound->file,0) == 0) {
                            eof = false;
                            continue;
                        } else {
                            break;
                        }
                    }
                    else {
                        buf.Flags = XAUDIO2_END_OF_STREAM;
                        if(bytesRead <= 0) {break;}
                    }
                }
                sound->source->SubmitSourceBuffer(&buf);
                currentDiskReadBufffer++;
                currentDiskReadBufffer = (currentDiskReadBufffer % sound->streamingBufferCount);
            }
        }

        XAUDIO2_VOICE_STATE state {0};
        while(sound->source->GetState(&state), state.BuffersQueued > 0) {
            WaitForSingleObjectEx(sound->streamingContext->hBufferEndEvent, INFINITE, TRUE);
        }
        //TODO: Free the stuff
        free(buffers);

        //BAD!
        delete sound;
        return 0;
    }

    AudiolibError streamVorbisFileFromDisk(AudioPlaybackContext* player, VorbisDecoderFileApi* api,
                                               _In_z_ const wchar_t* filePath) {
        auto vorbisFile = new OggVorbis_File;

        HANDLE winFileHandle = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           nullptr, OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL, nullptr);

        if(winFileHandle == INVALID_HANDLE_VALUE) {return AUDIOLIB_IO_ERROR;}
        FILE* cFile;
        auto result = winFileHandleToCFileHandle(&winFileHandle, &cFile);
        if(result != AUDIOLIB_OK || cFile == nullptr) {
            CloseHandle(winFileHandle);
            return result;
        }

        int oggResult = api->ov_open_callbacks(cFile, vorbisFile, nullptr, 0, OV_CALLBACKS_NOCLOSE);

        if( oggResult < 0) {
            fclose(cFile);
            return AUDIOLIB_OGG_FAILURE;
        }

        IXAudio2SourceVoice* source;
        auto context = new StreamingVoiceContext();

        auto format = new AudioFormatInfo;

        vorbis_info* oggInfo = api->ov_info(vorbisFile, -1);
        format->numberOfChannels = oggInfo->channels;
        format->bitsPerSample = 16;
        format->sampleRate = oggInfo->rate;

        auto wf = audioInfoToWF(format);

        auto winResult = player->xaudio->CreateSourceVoice(&source, &wf, 0, XAUDIO2_MAX_FREQ_RATIO, context, nullptr,
                                                           nullptr);
        if(FAILED(winResult)) {return AUDIOLIB_XAUDIO_GENERIC_ERROR;}
        auto* streamContext = new VorbisAudioStreamContext(
                    source,
                    context,
                    player,
                    api,
                    vorbisFile,
                    5,
                    4096 * 5,
                    true
                );

        DWORD threadID;
        auto streamThead = CreateThread(nullptr, 0, vorbisStreamThread, streamContext, 0, &threadID);
        return AUDIOLIB_OK;
    }

    DWORD WINAPI wmfStreamThread(LPVOID lParam) {
        auto* sound = static_cast<WMFReaderStreamContext*>(lParam);
        if(!sound) {DebugHaltAndCatchFire();}
        u32 bytesPerSecond = MFGetAttributeUINT32(sound->decoder->mediaType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND,0);
        u32 blockSize = MFGetAttributeUINT32(sound->decoder->mediaType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);

        u64 initialIndividualBufferSize = 500000;

        u8* buffers = static_cast<u8*>(malloc(initialIndividualBufferSize * sound->streamingBufferCount));

        if(buffers == nullptr) {DebugHaltAndCatchFire();}
        auto result = sound->source->Start();
        if(FAILED(result)) {DebugHaltAndCatchFire();}

        u32 currentDiskReadBuffer = 0;
        u32 currentPosition = 0;
        bool eof = false;
        u64 individualBufferSize = initialIndividualBufferSize;
        IMFMediaBuffer* buffer = nullptr;
        IMFSample* sample = nullptr;

        while(!eof) {
            DWORD flags;

            HRESULT result = sound->decoder->reader->ReadSample(
                        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                        0,
                        nullptr,
                        &flags,
                        nullptr,
                        &sample
            );
            if(flags & MF_SOURCE_READERF_ENDOFSTREAM) {eof = true;}

            if(eof) {
                if(sound->loop) {
                    PROPVARIANT var {0};
                    InitPropVariantFromInt64(0, &var);
                    sound->decoder->reader->SetCurrentPosition(GUID_NULL, var);
                    PropVariantClear(&var);
                    eof=false;
                    continue;
                } else {
                    break;
                }
            }
            XAUDIO2_VOICE_STATE state;

            if(FAILED(result) || sample == nullptr) {DebugHaltAndCatchFire();}
            result = sample->ConvertToContiguousBuffer(&buffer);
            if(FAILED(result) || sample == nullptr) {
                sample->Release();
                DebugHaltAndCatchFire();
            }
            u8* audioData = nullptr;
            DWORD bufferReadCount;


            result = buffer->Lock(&audioData, nullptr, &bufferReadCount);
            if(FAILED(result)) {DebugHaltAndCatchFire();}
            XAUDIO2_BUFFER buf = {0};

            while(sound->source->GetState(&state), state.BuffersQueued >= sound->streamingBufferCount) {
                WaitForSingleObject(sound->streamContext->hBufferEndEvent, INFINITE);
            }

            buf.AudioBytes = bufferReadCount;
            memcpy(buffers + (currentDiskReadBuffer * individualBufferSize), audioData, bufferReadCount);
            buf.pAudioData = reinterpret_cast<const BYTE *>(buffers + individualBufferSize * currentDiskReadBuffer);

            result = buffer->Unlock();


            sound->source->SubmitSourceBuffer(&buf);

            currentDiskReadBuffer++;
            currentDiskReadBuffer = (currentDiskReadBuffer % sound->streamingBufferCount);

            if(FAILED(result)) {return AUDIOLIB_MF_GENERIC;}
            sample->Release();
            sample = nullptr;
            buffer->Release();
            buffer = nullptr;
        }
        if(sample) {sample->Release();}
        if(buffer) {
            buffer->Unlock();
            buffer->Release();
        }


        XAUDIO2_VOICE_STATE state;
        while(sound->source->GetState(&state), state.BuffersQueued > 0) {
            WaitForSingleObjectEx(sound->streamContext->hBufferEndEvent, INFINITE, TRUE);
        }
        //TODO: Free the stuff
        free(buffers);

        return 0;
    }

    AudiolibError streamWMFFileFromDisk(AudioPlaybackContext* player, const wchar_t* filePath) {
        auto* decoder = new MediaFoundationAudioDecoder;
        auto result = mediaFoundationOpenEncodedAudioFile(filePath, decoder);
        if(result != AUDIOLIB_OK) {delete decoder; return result;}
        IXAudio2SourceVoice* source;
        auto context = new StreamingVoiceContext();
        auto winResult = player->xaudio->CreateSourceVoice(&source, decoder->wf, 0, XAUDIO2_MAX_FREQ_RATIO, context, nullptr, nullptr);
        if(FAILED(winResult)) {
            delete decoder;
            source->DestroyVoice();
            return AUDIOLIB_XAUDIO_GENERIC_ERROR;
        }
        auto* streamContext = new WMFReaderStreamContext {
            source,
            context,
            player,
            decoder,
            2000,
            3,
            true
        };
        DWORD threadID;
        auto streamThread = CreateThread(nullptr, 0, wmfStreamThread, streamContext, 0, &threadID);
        return AUDIOLIB_OK;
    }

    AudiolibError startRecordingFromEndpopint(AudioDevice *device, u8** pcmOut, u32* pcmBufferSize, WAVEFORMATEXTENSIBLE** wfOut) {
        HRESULT result;
        if(device == nullptr) {return AUDIOLIB_INVALID_PARAMETER;}
        WAVEFORMATEXTENSIBLE* wf;
        IAudioClient* captureClient;

        AudioFormatInfo info {0};
        info.bitsPerSample = 16;
        info.sampleRate = 48000;
        info.numberOfChannels = 1;
        WAVEFORMATEX desiredWF = audioInfoToWF(&info);

        result = device->endpoint->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, reinterpret_cast<void **>(&captureClient));
        if(FAILED(result) || captureClient == nullptr) {return AUDIOLIB_ERROR_UNKNOWN;}

        result = captureClient->GetMixFormat(reinterpret_cast<WAVEFORMATEX **>(&wf));

        if(wf == nullptr) {return AUDIOLIB_ERROR_UNKNOWN;}
        const MFTIME ONE_SECOND = 10000000;
        const LONG   ONE_MSEC = 1000;

        const u64 bufferLength = ONE_SECOND * 3;

        /* For testing purposes, we'll allocate a buffer of 3 secs,
         * I really don't know what the max buffer size should be here at this point,
         * we might increase or decrease this in the future, given how possible it is to
         * change this buffer size while already recording...
        */

        result = captureClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                0,
                bufferLength,
                0,
                reinterpret_cast<WAVEFORMATEX*>(wf),
                nullptr
                );

        if(FAILED(result)) {return AUDIOLIB_ERROR_UNKNOWN;}
        //wf = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(&desiredWF);
        u32 bufferSampleCount;
        result = captureClient->GetBufferSize(&bufferSampleCount);
        if(FAILED(result) || bufferSampleCount == 0) {return AUDIOLIB_ERROR_UNKNOWN;}

        IAudioCaptureClient* recoder = nullptr;
        result = captureClient->GetService(__uuidof(IAudioCaptureClient),
                                           (void**)&recoder);

        if(FAILED(result) || recoder == nullptr) {return AUDIOLIB_ERROR_UNKNOWN;}

        u64 actualDuration = ONE_SECOND * bufferSampleCount / wf->Format.nSamplesPerSec;
        if(FAILED(result)) {return AUDIOLIB_ERROR_UNKNOWN;}
        u32 packetLen;
        bool eof = false;
        result = captureClient->Start();

        Sleep(actualDuration/(ONE_MSEC * 10));
        result = recoder->GetNextPacketSize(&packetLen);
        if(FAILED(result)) {return AUDIOLIB_ERROR_UNKNOWN;}
        u8* data = nullptr;
        u32 numFramesAvailable = 0;
        DWORD flags;
        u64 bufferSize = actualDuration/(ONE_SECOND) * wf->Format.nAvgBytesPerSec;

        u8* _buffer = static_cast<u8 *>(calloc(bufferSize * 10, 1));
        u64 offset = 0;
        float counter = 0;

        bool done = false;
        while (!done) {
			while(packetLen != 0) {
				result = recoder->GetBuffer(&data, &numFramesAvailable, &flags, nullptr, nullptr);
				if(FAILED(result)) {break;}
				if (numFramesAvailable == 0) {
					break;
				}
				
				if(offset + (numFramesAvailable * wf->Format.wBitsPerSample * wf->Format.nChannels) >= bufferSize) {
					break;
				}
				if(flags & AUDCLNT_BUFFERFLAGS_SILENT) {
					memset(_buffer + offset, 0, numFramesAvailable* wf->Format.wBitsPerSample * wf->Format.nChannels);
				}
				else if(data) {
					memcpy(_buffer + offset, data, numFramesAvailable * wf->Format.wBitsPerSample * wf->Format.nChannels);
				}
				offset += numFramesAvailable * wf->Format.wBitsPerSample * wf->Format.nChannels;
				result = recoder->ReleaseBuffer(numFramesAvailable);

				recoder->GetNextPacketSize(&packetLen);
                counter += 1;
                if (counter >= 1000) {
                    OutputDebugStringW(L"Done");
                    done = true;
                }
			}
            counter++;
            if (counter >= 100) {
                break;
            }
        }
        OutputDebugStringW(L"Done2");
        captureClient->Stop();
        captureClient->Release();

        *pcmOut= _buffer;
        *pcmBufferSize = offset;
        wf->Format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        *wfOut = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(wf);

        return AUDIOLIB_OK;
    }
}