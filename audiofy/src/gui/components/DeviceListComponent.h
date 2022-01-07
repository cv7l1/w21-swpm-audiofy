//
// Created by Jonathan on 07.01.2022.
//

#ifndef AUDIOFY_DEVICELISTCOMPONENT_H
#define AUDIOFY_DEVICELISTCOMPONENT_H


#include <al_device.h>
#include "IComponent.h"

class DeviceListComponent : public IComponent{
public:
    DeviceListComponent(AudioDeviceManager* deviceManager);
    void Show() override;
private:
    AudioDeviceManager* _deviceManager;
    std::vector<AudioDevice> currentDeviceList = std::vector<AudioDevice>();
    AudioDevice* currentDefaultDevice;
    int selectedDevice = -1;
};


#endif //AUDIOFY_DEVICELISTCOMPONENT_H
