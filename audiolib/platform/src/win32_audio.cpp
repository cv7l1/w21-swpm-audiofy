#include <Windows.h>
#include "win32_audio.h"
#include "memUtil.h"
#include <io.h>
#include <propvarutil.h>

#include <utility>
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

    template<> WAVEFORMATEX AudioFormatInfo<>::toWaveFormat() const {
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

}

