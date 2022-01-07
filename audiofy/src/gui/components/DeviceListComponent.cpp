//
// Created by Jonathan on 07.01.2022.
//

#include <comutil.h>
#include "DeviceListComponent.h"
#include "imgui.h"
DeviceListComponent::DeviceListComponent(AudioContext* context, AudioDeviceManager *deviceManager) : _deviceManager(deviceManager), _audioContext(context){
    currentDeviceList = _deviceManager->getAudioDeviceList(AudioDeviceRole::Playback);
    currentDefaultDevice = _deviceManager->getDefaultDevice(AudioDeviceRole::Playback);
}
void DeviceListComponent::Show() {
    if(ImGui::Begin("Audio Devices")) {
        if(currentDefaultDevice != nullptr) {
            auto name = currentDefaultDevice->getDeviceName();
            if(name.has_value()) {
                ImGui::Text( "Current default device: %ls", name.value());
            } else {
                ImGui::Text( "Current default device has no name");
            }
        }
        if(ImGui::BeginListBox("")) {
            int index = 0;
            for(auto device : currentDeviceList) {
                auto name = device.getDeviceName();
                if(name.has_value()) {
                    if(ImGui::Selectable(bstr_t(device.getDeviceName().value()))) {
                        selectedDevice = index;
                    }
                } else {
                    if(ImGui::Selectable("No name")) {
                        selectedDevice = index;
                    }
                }
                index++;
            }
            ImGui::EndListBox();
            ImGui::SameLine();
            if(ImGui::Button("Update")) {
                currentDeviceList = _deviceManager->getAudioDeviceList(AudioDeviceRole::Playback);
                currentDefaultDevice = _deviceManager->getDefaultDevice(AudioDeviceRole::Playback);
            }
            if(ImGui::Button("Use")) {
                _audioContext->_player->setDevice(&currentDeviceList[selectedDevice]);
            }

            ImGui::SameLine();

            if(selectedDevice != -1) {
                ImGui::Text("%ls", currentDeviceList[selectedDevice].getDeviceName().value());
            }

        }
        ImGui::End();
    }
}
