//
// Created by Jonathan on 27.11.2021.
//

#ifndef AUDIOFY_LIB_AUDIODEVICE_H
#define AUDIOFY_LIB_AUDIODEVICE_H

#include "win32_framework.h"

enum AudioDeviceRole {
    Playback = eRender,
    Capture = eCapture,
    All = eAll,
};

class AudioDevice {
public:
    static std::optional<AudioDevice> getDefaultDevice(AudioDeviceRole role);
    static std::vector<AudioDevice> getAudioDeviceList(AudioDeviceRole role);

    std::optional<wchar_t*> getDeviceName() {return _description;}
    const wchar_t* getDeviceID() {return _id;}

    AudioDevice() = default;

    ~AudioDevice();

private:
    void getDescriptionFromEndpoint();
    explicit AudioDevice(_In_ IMMDevice* dev) : _endpoint(dev) {getDescriptionFromEndpoint();};

    wchar_t* _id = nullptr;
    std::optional<wchar_t*> _description = std::nullopt;
    tagPROPVARIANT _var {0};

    Microsoft::WRL::ComPtr<IMMDevice> _endpoint;
    u32 _currentState = 0;
};

#endif //AUDIOFY_LIB_AUDIODEVICE_H
