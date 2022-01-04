//
// Created by Jonathan on 04.12.2021.
//

#ifndef AUDIOLIB_EXAMPLES1_EXAMPLE_DEVICES_H
#define AUDIOLIB_EXAMPLES1_EXAMPLE_DEVICES_H

#include "al_device.h"

/*
 * If we want to check if the state of specific devices has changed, we can register an
 * observer that inherits from the IAAudioDeviceNotificationObserver interface
 */

class DeviceListener : public IAudioDeviceNotificationObserver {

public:
    HANDLE deviceRemoveHandle = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    void OnDefaultDeviceChanged(AudioDeviceRole role, const wchar_t *id) override;

    void OnDeviceAdded(AudioDevice device) override;

    void OnDeviceStateChanged(const wchar_t* id, u32 state) override;

    void OnDeviceRemoved() override;
};

#endif //AUDIOLIB_EXAMPLES1_EXAMPLE_DEVICES_H
