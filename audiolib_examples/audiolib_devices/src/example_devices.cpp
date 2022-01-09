//
// Created by Jonathan on 04.12.2021.
//

#include "example_devices.h"
#include <Windows.h>
#include "al_device.h"
#include "al_debug.h"
#include <string>
#include <comutil.h>
#include <format>

#define WIN32_LEAN_AND_MEAN

int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_ HINSTANCE hPrevInstance,
                   _In_ PSTR lpCmdLine,
                   _In_ INT nChmdShow) {

    /*This will initialize some specific windows components and has to be called before using any audiolib functions.
     * For release, this will be handled by the library.
    */
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    /*
     * This will initialize the console logger. Only for debug purposes
     */

    Console::setup();

    /*This will initialize the audio device manager, through which we can recieve information
     * about all recognized audio devices.
     */
    AudioDeviceManager audioDeviceManager;

    /*
     * This will return the audio device set as 'default' for a specific role.
     */
    auto defaultDevice = audioDeviceManager.getDefaultDevice(AudioDeviceRole::Playback);

    /* Note that getDefaultDevice() may return a nullptr in which case no default device could be found
     * This usually means, that there are no device available for the given role.
     * This is not an error and does not throw an exception. This case should be handled by the user
     */

    if(!defaultDevice) {
        al_ErrorWarn("There is no usable speaker currently plugged in");
    }

    /*
     * The default device may have been unplugged or disabled in some other way. This is something
     * that the user of the library should always watch out for and handle
     */
    u32 deviceState = defaultDevice->getCurrentDeviceState();
    if(deviceState == DEVICE_STATE_ACTIVE) {
        al_ErrorInfo("Device ready and working! :)");
    } else {
        al_ErrorWarn("Device is not plugged in");
    }

    /*
     * Note that there may be cases where a device doesn't have an actual, human friendly name.
     * If so, getDeviceName() will return no value, this is not an error and won't throw an exception
     */
    auto deviceName = _bstr_t(defaultDevice.get()->getDeviceName().value());
    auto deviceID = _bstr_t(defaultDevice.get()->getDeviceID());
    //auto notification = std::format("Default Device name: {}, id: {}", deviceName, deviceID);
    //al_ErrorInfo(notification.c_str());

    /*
     * We can also print a list of all devices
     */
    for(auto & dev : audioDeviceManager.getAudioDeviceList(AudioDeviceRole::Playback)) {
        if(dev.getDeviceName().has_value()) {
            //al_ErrorInfo(std::format("{}",bstr_t(dev.getDeviceName().value())).c_str());
        }
    }

    /*
     * Attach our device listener to the manager
     */
    auto listener = new DeviceListener;
    audioDeviceManager.Attach(listener);

    /*
     * Wait until a device has been removed, using our custom DeviceListener class
     */
    WaitForSingleObjectEx(listener->deviceRemoveHandle, INFINITE, FALSE);


    return S_OK;
}


void DeviceListener::OnDeviceAdded(AudioDevice device) {
    al_ErrorInfo("New device found");
    //al_ErrorInfo(std::format("name: {}", bstr_t(device.getDeviceName().value())).c_str());

}

void DeviceListener::OnDeviceStateChanged(const wchar_t* id, u32 state) {
    //TODO: Handle
}

void DeviceListener::OnDeviceRemoved() {
    SetEvent(deviceRemoveHandle);
}

void DeviceListener::OnDefaultDeviceChanged(AudioDeviceRole role, const wchar_t *id) {

}
