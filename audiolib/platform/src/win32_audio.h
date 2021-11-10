//
// Created by Jonathan on 08.11.2021.
//

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
//TODO: Include Stubs

#pragma comment(lib, "runtimeobject.lib")

#include <sal.h>
#include "types.h"
#include<mfidl.h>
#include<mfreadwrite.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <xaudio2.h>
#include <xtr1common>
#include <type_traits>
#include "../../include/vorbisfile.h"

namespace PlatformWin32
{
    class ProcPtr {
    public:
        explicit ProcPtr(FARPROC ptr) : _ptr(ptr) {}
        template <typename T, typename = std::enable_if_t<std::is_function_v<T>>>
        operator T* () const {
            return reinterpret_cast<T *>(_ptr);
        }

    private:
        FARPROC _ptr;
    };

    class DllHelper {
    public:
        explicit DllHelper(_In_z_ const wchar_t* libPath) : _module(LoadLibraryW(libPath)) {}
        ~ DllHelper() { FreeLibrary(_module);}

        ProcPtr operator[] (_In_z_ const char* proc_name) const {
            return ProcPtr(GetProcAddress(_module, proc_name));
        }
        static HMODULE _parent_module;

    private:
        HMODULE _module;

    };

    struct AudioDevice {
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


#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL)  \
    { (punk)->Release(); (punk) = NULL; }

    class AudioDeviceNotifiactionHandler : public IMMNotificationClient {
        LONG _cRef;
        IMMDeviceEnumerator *_enumerator;

    public:
        AudioDeviceNotifiactionHandler() : _cRef(1), _enumerator(nullptr) {}
        ~AudioDeviceNotifiactionHandler() { SAFE_RELEASE(_enumerator)}

        ULONG STDMETHODCALLTYPE AddRef() override {
            return InterlockedIncrement(&_cRef);
        }
        ULONG STDMETHODCALLTYPE Release() override {
            ULONG ulRef = InterlockedDecrement(&_cRef);
            if(!ulRef) {
                delete this;
            }
            return ulRef;
        }
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvInterface) override {
            if(IID_IUnknown == riid) {
                AddRef();
                *ppvInterface = (IUnknown*)this;
            } else if(__uuidof(IMMNotificationClient) == riid) {
                AddRef();
                *ppvInterface = (IMMNotificationClient*)this;
            } else {
                *ppvInterface = nullptr;
                return E_NOINTERFACE;
            }
            return S_OK;
        }
        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceID) override;
        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged();
    };

     u32 winFileHandleToCFileHandle(_Inout_ HANDLE* winFileHandle,
                                    _Out_ FILE** cFileHandle);


    /// Retrieves all audio output devices currently available
    /// \param devices OUT: Pointer to all available devices, free with freeAudioDeviceList
    /// \param numDevices OUT: Pointer to the number of devices found
    /// \return Non-Zero on success
    u32 getAvailableAudioOutputDevices(
            _Out_ AudioDeviceList* devices,
            _Out_ u32* numDevices
    );

    u32 getDefaultAudioOutputDevice(_Out_ AudioDevice* device);

    /// Frees an audio device
    void freeAudioDevice(_In_ AudioDevice* device);


    struct AudioPlaybackContext {
        IXAudio2* xaudio;
        IXAudio2MasteringVoice* master;
    };

    struct PCMAudioBufferInfo {
        WAVEFORMATEX waveformat;
        u8* rawDataBuffer;
        size_t bufferSize;
    };

    struct AudioHandle {
        IXAudio2SourceVoice* source;
        PCMAudioBufferInfo audioInfo;
        bool loop;
    };

    u32 setupAudioPlayback(
            bool debug,
            _In_opt_ AudioDevice*           device,
            _Out_    AudioPlaybackContext*  context
            );

    u32 setMasterVolume(
            _Inout_ AudioPlaybackContext*   context,
                    float                   volume);

    u32 submitSoundBuffer(
            _Inout_ AudioPlaybackContext*   context,
            _In_    PCMAudioBufferInfo*     buffer,
            _Out_   AudioHandle*            handle);

    inline
    u32 playAudioBuffer(
            _In_    AudioPlaybackContext*   context,
            _In_    AudioHandle*            handle,
                    bool                    loop) {

        if(handle->source == nullptr) {return 0;}
        HRESULT result = handle->source->Start();
        if(FAILED(result)) {return 0;}
        return 1;
    }

    struct VorbisDecoderFileApi {
        DllHelper _dll{L"vorbisfile.dll"};
        decltype(ov_open_callbacks)* ov_open_callbacks = _dll["ov_open_callbacks"];
        decltype(ov_info)* ov_info = _dll["ov_info"];
        decltype(ov_pcm_total)* ov_pcm_total = _dll["ov_pcm_total"];
        decltype(ov_read)* ov_read = _dll["ov_read"];
    };

    u32 decodeVorbisFile(_In_ VorbisDecoderFileApi *api,
                          _In_z_ const wchar_t* filePath,
                          _Out_ PCMAudioBufferInfo* buffer);

    struct MediaFoundationAudioDecoder {
        IMFSourceReader* reader;
        IMFMediaType* mediaType;
        WAVEFORMATEX* wf;
    };

    inline u32 mediaFoundationSetup() {
        if(FAILED(MFStartup(MF_VERSION))) {return 0;}
        return 1;
    }

    u32 mediaFoundationOpenEncodedAudioFile(_In_z_ const wchar_t* path,
                                            _Out_ MediaFoundationAudioDecoder* decoder);

    u32 mediaFoundationGetAudioDuration(_In_ MediaFoundationAudioDecoder* decoder,
                                        _Out_ u64* durationMS);
    u32 mediaFoundationDecodeSample(_In_ MediaFoundationAudioDecoder* decoder,
                                    _Out_writes_bytes_all_(dataWritten) u8* dest,
                                    size_t destBufferSize,
                                    u32 maxAudioData,
                                    u32 offset,
                                    _Out_ u32* dataWritten,
                                    bool* eof);


    u32 mediaFoundationDecodeFile(const wchar_t* path,
                                  _Out_ PCMAudioBufferInfo* bufferOut);
}


