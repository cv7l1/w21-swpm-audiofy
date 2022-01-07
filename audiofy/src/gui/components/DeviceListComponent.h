//
// Created by Jonathan on 07.01.2022.
//

#ifndef AUDIOFY_DEVICELISTCOMPONENT_H
#define AUDIOFY_DEVICELISTCOMPONENT_H


#include <al_device.h>
#include "IComponent.h"
#include "../../audio/AudioWorkspace.h"

class DeviceListComponent : public IComponent{
public:
    DeviceListComponent(AudioContext* context, AudioDeviceManager* deviceManager);
    void Show() override;
private:
    AudioContext* _audioContext;
    AudioDeviceManager* _deviceManager;
    std::vector<AudioDevice> currentDeviceList = std::vector<AudioDevice>();
    AudioDevice* currentDefaultDevice;
    int selectedDevice = -1;
};


#endif //AUDIOFY_DEVICELISTCOMPONENT_H
