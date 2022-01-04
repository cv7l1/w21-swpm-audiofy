//
// Created by Jonathan on 08.11.2021.
//

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN

class VorbisDecoderFileApi;

#endif
//TODO: Include Stubs

#pragma comment(lib, "runtimeobject.lib")

#include <sal.h>
#include "types.h"
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <xaudio2.h>
#include <utility>
#include <xtr1common>
#include <type_traits>
#include "../../include/vorbisfile.h"
#include <optional>
#include <vector>
#include <memory>
#include <functional>

namespace PlatformWin32
{
    enum AudiolibError {
        AUDIOLIB_OK,
        AUDIOLIB_COINIT_FAILURE,
        AUDIOLIB_ERROR_UNKNOWN,
        AUDIOLIB_XAUDIO_CONTEXT_CREATION,
        AUDIOLIB_XAUDIO_VOICE_CREATION,
        AUDIOLIB_XAUDIO_SOURCE_CREATION,
        AUDIOLIB_XAUDIO_SUBMIT_BUFFER,
        AUDIOLIB_XAUDIO_GENERIC_ERROR,
        AUDIOLIB_OGG_FAILURE,
        AUDIOLIB_INVALID_PARAMETER,
        AUDIOLIB_OUTOFMEMORY,
        AUDIOLIB_IO_ERROR,
        AUDIOLIB_ENDPOINT_RETRIEVAL,
        AUDIOLIB_ENDPOINT_NODEVICE,
        AUDIOLIB_ENDPOINT_INFO_ERROR,
        AUDIOLIB_ENDPOINT_INVALID_PROP_DESC,
        AUDIOLIB_MF_OPENURL,
        AUDIOLIB_MF_GENERIC,
        AUDIOLIB_MF_WF,
        AUDIOLIB_MF_READMAX,
    };


    enum CIntferfaceAudiolibDeviceRole {
        AUDIOLIB_ROLE_PLAYBACK = eRender,
        AUDIOLIB_ROLE_RECORDING = eCapture,
        AUDIOLIB_ROLE_ALL = eAll,
    };



    struct CInterfaceAudioDevice {
        wchar_t* deviceID;
        wchar_t* description;
        tagPROPVARIANT var;

        IMMDevice* endpoint;
        DWORD currentState;
    };

    struct AudioDeviceList {
        u32 deviceCount;
        AudioDevice* devices;
    };



     AudiolibError winFileHandleToCFileHandle(
             _Inout_ _Notnull_  HANDLE* winFileHandle,
             _Out_              FILE** cFileHandle);

    enum DecoderType {
        DECODER_PCM,
        DECODER_WMF,
        DECODER_OGG
    };

    struct AudioPlaybackContext {
        IXAudio2* xaudio;
        IXAudio2MasteringVoice* master;
    };

    struct VorbisDecoderFileApi {
        DllHelper _dll{L"vorbisfile.dll"};
        decltype(ov_open_callbacks)* ov_open_callbacks = _dll["ov_open_callbacks"];
        decltype(ov_info)* ov_info = _dll["ov_info"];
        decltype(ov_pcm_total)* ov_pcm_total = _dll["ov_pcm_total"];
        decltype(ov_read)* ov_read = _dll["ov_read"];
        decltype(ov_pcm_seek)* ov_pcm_seek = _dll["ov_read"];
        decltype(ov_raw_seek)* ov_raw_seek = _dll["ov_raw_seek"];
    };

    struct StreamingVoiceContext : public IXAudio2VoiceCallback{
        HANDLE hBufferEndEvent;
        StreamingVoiceContext() : hBufferEndEvent(CreateEvent(nullptr, false, false, nullptr)) {}

        STDMETHODIMP_(void) OnBufferEnd(void* pBufferContext) override {SetEvent(hBufferEndEvent);}

        STDMETHODIMP_(void) OnVoiceProcessingPassEnd() override {}

        STDMETHODIMP_(void)OnBufferStart(void* pBufferContext) override {}

        STDMETHODIMP_(void)OnVoiceError(void* pBufferContext, HRESULT Error) override {}

        STDMETHODIMP_(void)OnStreamEnd() override {}

        STDMETHODIMP_(void)OnLoopEnd(void *pBufferContext) override {}

        STDMETHODIMP_(void)OnVoiceProcessingPassStart(UINT32 BytesRequired) override {};

    };
    struct VorbisAudioStreamContext {
        VorbisAudioStreamContext(
                IXAudio2SourceVoice* source,
                StreamingVoiceContext *streamContext,
                AudioPlaybackContext* xcontext,
                PlatformWin32::VorbisDecoderFileApi* fileapi,
                OggVorbis_File* file,

                u32 streamingBufferCount,
                size_t individualStreamingBufferSize,
                bool loop
                )
        : source(source), streamingContext(streamContext),
          XAudioContext(xcontext), vorbisContext(fileapi),
          file(file), streamingBufferCount(streamingBufferCount),
          individualStreamingBufferSize(individualStreamingBufferSize),
          loop(loop) {}

        IXAudio2SourceVoice* source;
        StreamingVoiceContext* streamingContext;
        AudioPlaybackContext* XAudioContext;
        AudioFormatInfo decodedAudioFormat = {0};
        VorbisDecoderFileApi* vorbisContext;
        OggVorbis_File* file;

        u32 streamingBufferCount;
        size_t individualStreamingBufferSize;
        bool loop;
    };

    struct AudioStreamContext {
        IXAudio2SourceVoice* source;
        WAVEFORMATEX decodedMediaTypeWF;
        DecoderType decoderType;
        union FileContext {
        } fileContext;
    };

    struct PCMAudioBufferInfo {
        AudioFormatInfo audioInfo;
        u8* rawDataBuffer;
        size_t bufferSize;
    };

    struct AudioHandle {
        IXAudio2SourceVoice* source;
        PCMAudioBufferInfo audioInfo;
        bool loop;
    };


    AudiolibError setupAudioPlayback(
            bool debug,
            _In_opt_ AudioDevice*           device,
            _Out_    AudioPlaybackContext*  context
            );

    AudiolibError setMasterVolume(
            _Inout_ AudioPlaybackContext*   context,
                    float                   volume);

    AudiolibError submitSoundBuffer(
            _Inout_ AudioPlaybackContext*   context,
            _In_    PCMAudioBufferInfo*     buffer,
            _Out_   AudioHandle*            handle,
                    bool                    loop);

    AudiolibError playAudioBuffer(
            _In_ _Notnull_   AudioPlaybackContext*   context,
            _In_ _Notnull_   AudioHandle*            handle,
                             bool                    loop);


    AudiolibError decodeVorbisFile( _In_ _Notnull_ VorbisDecoderFileApi *api,
                                    _In_z_ _Notnull_ const wchar_t* filePath,
                                    _Out_ PCMAudioBufferInfo* buffer);

    struct MediaFoundationAudioDecoder {
        IMFSourceReader* reader;
        IMFMediaType* mediaType;
        WAVEFORMATEX* wf;
    };

    struct WMFReaderStreamContext {
        IXAudio2SourceVoice* source;
        StreamingVoiceContext* streamContext;
        AudioPlaybackContext* xcontext;
        MediaFoundationAudioDecoder* decoder;
        size_t individualStreamingBufferSize;
        u32 streamingBufferCount;
        bool loop;

    };

    inline AudiolibError mediaFoundationSetup() {
        if(FAILED(MFStartup(MF_VERSION))) {return AUDIOLIB_MF_GENERIC;}
        return AUDIOLIB_OK;
    }

    AudiolibError mediaFoundationOpenEncodedAudioFile(_In_z_ const wchar_t* path,
                                            _Out_ MediaFoundationAudioDecoder* decoder);

    AudiolibError mediaFoundationGetAudioDuration(_In_ MediaFoundationAudioDecoder* decoder,
                                        _Out_ u64* durationMS);
    AudiolibError mediaFoundationDecodeSample(_In_ MediaFoundationAudioDecoder* decoder,
                                    _Out_writes_bytes_all_(dataWritten) u8* dest,
                                    size_t destBufferSize,
                                    u32 maxAudioData,
                                    u32 offset,
                                    _Out_ u32* dataWritten,
                                    bool* eof);


    AudiolibError mediaFoundationDecodeFile(const wchar_t* path,
                                  _Out_ PCMAudioBufferInfo* bufferOut);

    AudiolibError streamVorbisFileFromDisk(AudioPlaybackContext* player,
                                           VorbisDecoderFileApi* api,
                                           const wchar_t* filePath);

    AudiolibError streamWMFFileFromDisk(AudioPlaybackContext* player, const wchar_t* filePath);
}


