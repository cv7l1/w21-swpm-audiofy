#pragma once
#include "IComponent.h"
#include <imgui.h>
#include "../GuiMain.h"
#include "fileDialog.h"
#include "projectFileListComponent.h"
#include "DeviceListComponent.h"
#include "mix.h"
#include "../../win32/ay_fileManager.h"
class Toolbar : public IComponent {
public:
    Toolbar(AudioContext* context, AudioDeviceManager* deviceManager) : _context(context), _deviceManager(deviceManager) {}
    void Show() override;
private:
    AudioContext* _context;
    AudioDeviceManager* _deviceManager;
};