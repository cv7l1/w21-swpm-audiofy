//
// Created by Jonathan on 27.11.2021.
//

#ifndef AUDIOFY_LIB_AL_DEVICE_H
#define AUDIOFY_LIB_AL_DEVICE_H

#include "win32_framework.h"
#include "al_util.h"

enum AudioDeviceRole {
    Playback = eRender, // Everything that can be used to render sound (Speakers and so on)
    Capture = eCapture, // Everything that can be used to record sound, may be loopback
    All = eAll, //All Roles
    Undefined // Cur

};
class AudioDevice {
public:
    bool operator==(const AudioDevice &rhs) const;

    bool operator!=(const AudioDevice &rhs) const;

    /// Retrieve a human readable name of the device
    ///\return The name, may be empty
    std::optional<wchar_t*> getDeviceName() {return _description;}

    /// Retrieve the current state of the device
    /// TODO: Document what each state means
    /// \return The current state
    unsigned long getCurrentDeviceState() {getDeviceState(); return _currentState;}

    /// Retrieve the id, can be used to actually identify the device
    /// \return ID, null terminated
    const wchar_t* getDeviceID() {return _id;}

    AudioDevice() = default;

    explicit AudioDevice(_In_ IMMDevice* dev) : _endpoint(dev) {getIDFromEndpoint(); getDescriptionFromEndpoint(); getDeviceState();}
    explicit AudioDevice(_In_z_ const wchar_t* id) : _id(id) {getEndpointFromID(); getDescriptionFromEndpoint(); getDeviceState();}

    //~AudioDevice();

private:
    void getDeviceState();
    void getDescriptionFromEndpoint();
    void getIDFromEndpoint();
    void getEndpointFromID();

    const wchar_t* _id = nullptr;
    std::optional<wchar_t*> _description = std::nullopt;
    tagPROPVARIANT _var {0};

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> devEnum;
    Microsoft::WRL::ComPtr<IMMDevice> _endpoint;
    unsigned long _currentState = 0;
};


class IAudioDeviceNotificationObserver {
public:
    virtual void OnDefaultDeviceChanged(AudioDeviceRole role, const wchar_t* id) = 0;
    virtual void OnDeviceAdded(AudioDevice device) = 0;
    virtual void OnDeviceStateChanged(const WCHAR *deviceID, u32 state) = 0;
    virtual void OnDeviceRemoved() = 0;
};

class AudioDeviceManager : public IMMNotificationClient {
    LONG _cRef;
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> _enumerator;
    std::list<IAudioDeviceNotificationObserver*> _observers;

    void setup();
public:
    AudioDeviceManager();

    void Attach(_In_ IAudioDeviceNotificationObserver* observer);
    void Detach(_In_ IAudioDeviceNotificationObserver* observer);

    /// Retrieve the default Audio Endpoint for a specific role
    /// @param [In] role Role
    /// \return The Device, null if no device could be found for the role
    AudioDevice* getDefaultDevice(AudioDeviceRole role);

    /// Retrieve all currently plugged in Audio Endpoints for a specific role
    /// \param [In] role Role
    /// \return Vector containing all the devices that were found
    std::vector<AudioDevice> getAudioDeviceList(AudioDeviceRole role);



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

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR id) override;
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override;
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) override {return S_OK;}
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;
};


#endif //AUDIOFY_LIB_AL_DEVICE_H
