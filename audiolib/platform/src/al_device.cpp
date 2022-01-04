//
// Created by Jonathan on 27.11.2021.
//

#include "al_device.h"

#include <memory>
#include "al_error.h"

AudioDeviceManager::AudioDeviceManager() : _cRef(1), _enumerator(nullptr){
    setup();
}

void AudioDeviceManager::Attach(_In_ IAudioDeviceNotificationObserver* observer) {
    _observers.push_back(observer);
}

HRESULT AudioDeviceManager::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceID) {
    for(auto &observer : _observers) {
        observer->OnDefaultDeviceChanged(static_cast<AudioDeviceRole>(role), pwstrDeviceID);
    }
    return S_OK;
}

HRESULT AudioDeviceManager::OnDeviceAdded(LPCWSTR pwstrDeviceId) {
    for(auto &observer : _observers) {
        observer->OnDeviceAdded(AudioDevice(pwstrDeviceId));
    }
    return S_OK;
}

HRESULT AudioDeviceManager::OnDeviceRemoved(LPCWSTR pwstrDeviceId) {
    for(auto &observer : _observers) {
        observer->OnDeviceRemoved();
    }
    return S_OK;
}

HRESULT AudioDeviceManager::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
    for(auto &observer : _observers) {
        observer->OnDeviceStateChanged(pwstrDeviceId, dwNewState);
    }
    return S_OK;
}

void AudioDeviceManager::Detach(_In_ IAudioDeviceNotificationObserver* observer) {
    _observers.remove(observer);
}

void AudioDeviceManager::setup() {
    throwIfFailed(CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(_enumerator.GetAddressOf())));
    _enumerator->RegisterEndpointNotificationCallback(this);
}

void AudioDevice::getDescriptionFromEndpoint() {
    using Microsoft::WRL::ComPtr;

    ComPtr<IPropertyStore> props;
    throwIfFailed(_endpoint->OpenPropertyStore(STGM_READ, props.GetAddressOf()));

    PROPVARIANT var = {0};
    PropVariantInit(&var);
    throwIfFailed(props->GetValue(PKEY_Device_FriendlyName, &var));

    _var = var;

    if(var.vt != VT_LPWSTR || var.pwszVal == nullptr) {
        return;
    }
    _description = var.pwszVal;
    DWORD state = 0;

    _currentState = state;

}

void AudioDevice::getIDFromEndpoint() {
    if(_endpoint == nullptr) {
        throw std::exception("No endpoint available to get id from");
    }
    throwIfFailed(_endpoint->GetId(const_cast<LPWSTR *>(&_id)));

}

void AudioDevice::getDeviceState() {
    if(_endpoint == nullptr) {
        throw std::exception("No endpoint available to get state from");
    }
    throwIfFailed(_endpoint->GetState(&_currentState));
}

void AudioDevice::getEndpointFromID() {
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> devEnum;
    throwIfFailed(CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(devEnum.GetAddressOf())));

    throwIfFailed(devEnum->GetDevice(_id, _endpoint.GetAddressOf()));
}

bool AudioDevice::operator==(const AudioDevice &rhs) const {
    return(wcscmp(_id, rhs._id) == 0);
}

bool AudioDevice::operator!=(const AudioDevice &rhs) const {
    return !(rhs == *this);
}

std::shared_ptr<AudioDevice> AudioDeviceManager::getDefaultDevice(const AudioDeviceRole role) {
    using Microsoft::WRL::ComPtr;

    IMMDevice* dev;

    HRESULT result;
    result = _enumerator->GetDefaultAudioEndpoint(static_cast<EDataFlow>(role),
                                              eMultimedia,
                                              &dev);
    if(FAILED(result)) {
        if(result == E_NOTFOUND) {return nullptr;}
        else {throw Win32Exception(result);}
    }
    return std::make_shared<AudioDevice>(dev);
}

std::vector<AudioDevice> AudioDeviceManager::getAudioDeviceList(const AudioDeviceRole role) {

    using Microsoft::WRL::ComPtr;
    ComPtr<IMMDeviceCollection> devices;
    u32 deviceCount = 0;

    throwIfFailed(_enumerator->EnumAudioEndpoints(static_cast<EDataFlow>(role),
                                              DEVICE_STATE_ACTIVE,
                                              &devices));

    throwIfFailed(devices->GetCount(&deviceCount));
    if(deviceCount <= 0) {return {};}

    auto vec = std::vector<AudioDevice>();
    vec.reserve(deviceCount);

    for(u32 i = 0; i < deviceCount; ++i) {
        IMMDevice* endpoint;
        throwIfFailed(devices->Item(i, &endpoint));
        vec.emplace_back(endpoint);
    }
    return vec;
}


/*
AudioDevice::~AudioDevice() {
    CoTaskMemFree(_id);
    _id = nullptr;
    PropVariantClear(&_var);
}
 */




