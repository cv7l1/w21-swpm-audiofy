//
// Created by Jonathan on 27.11.2021.
//

#include "audioDevice.h"
#include "al_error.h"

void AudioDevice::getDescriptionFromEndpoint() {
    using Microsoft::WRL::ComPtr;

    if(_endpoint == nullptr) {
        throw std::exception("No endpoint available to get description from");
    }

    throwIfFailed(_endpoint->GetId(&_id));
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
    throwIfFailed(_endpoint->GetState(&state));
    _currentState = state;

}

std::optional<AudioDevice> AudioDevice::getDefaultDevice(const AudioDeviceRole role) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IMMDeviceEnumerator> devEnum;
    IMMDevice* dev;
    throwIfFailed(CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(devEnum.GetAddressOf())));

    HRESULT result;
    result = devEnum->GetDefaultAudioEndpoint(static_cast<EDataFlow>(role),
                                              eMultimedia,
                                              &dev);
    if(FAILED(result)) {
        if(result == E_NOTFOUND) {return {};}
        else {throw Win32Exception(result);}
    }
    return AudioDevice(dev);
}

std::vector<AudioDevice> AudioDevice::getAudioDeviceList(const AudioDeviceRole role) {

    using Microsoft::WRL::ComPtr;

    ComPtr<IMMDeviceEnumerator> devEnum;
    ComPtr<IMMDeviceCollection> devices;
    u32 deviceCount = 0;

    throwIfFailed(CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARGS(devEnum.GetAddressOf())));

    throwIfFailed(devEnum->EnumAudioEndpoints(static_cast<EDataFlow>(role),
                                              DEVICE_STATE_ACTIVE,
                                              &devices));


    throwIfFailed(devices->GetCount(&deviceCount));
    if(deviceCount <= 0) {return {};}

    auto vec = std::vector<AudioDevice>();
    vec.reserve(deviceCount);

    for(u32 i = 0; i < deviceCount; ++i) {
        IMMDevice* endpoint;
        throwIfFailed(devices->Item(i, &endpoint));
        vec.push_back(AudioDevice(endpoint));
    }
    return vec;
}

AudioDevice::~AudioDevice() {
    /*
    CoTaskMemFree(_id.get());
    _id = nullptr;
    PropVariantClear(&_var);
     */
}
