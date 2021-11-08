//
// Created by Jonathan on 08.11.2021.
//

#ifndef AUDIOFY_LIB_WIN32_AUDIO_H
#define AUDIOFY_LIB_WIN32_AUDIO_H
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include "types.h"
#include <mfidl.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <xaudio2.h>
namespace PlatformWin32
{
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

        ULONG STDMETHODCALLTYPE AddRef() {
            return InterlockedIncrement(&_cRef);
        }
        ULONG STDMETHODCALLTYPE Release() {
            ULONG ulRef = InterlockedDecrement(&_cRef);
            if(!ulRef) {
                delete this;
            }
            return ulRef;
        }
        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvInterface) {
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
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged();
    };

    /// Retrieves all audio output devices currently available
    /// \param devices OUT: Pointer to all available devices, free with freeAudioDeviceList
    /// \param numDevices OUT: Pointer to the number of devices found
    /// \return Non-Zero on success
    u32 getAvailableAudioOutputDevices(_Out_ AudioDeviceList* devices,
    _Out_ u32* numDevices);

    u32 getDefaultAudioOutputDevice(_Out_ AudioDevice* device);

    /// Frees an audio device
    void freeAudioDevice(_In_ AudioDevice* device);

    void freeAudioDeviceList(_In_ AudioDeviceList* list) {
        for(int i = 0; i<list->deviceCount; ++i) {
            freeAudioDevice(&list->devices[i]);
        }
        delete[] list->devices;
        list->deviceCount = 0;
        list->devices = nullptr;
    }

}


#endif //AUDIOFY_LIB_WIN32_AUDIO_H
