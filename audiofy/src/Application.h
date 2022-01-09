//
// Created by Jonathan on 05.01.2022.
//

#ifndef AUDIOFY_APPLICATION_H
#define AUDIOFY_APPLICATION_H
#include <list>
#include <al_player.h>
#include <al_file.h>
#include "win32/ay_fileManager.h"
#include "audio/AudioWorkspace.h"
#include "imgui.h"
#include "audio/AudioContext.h"
struct GUIWin32Context {
    HWND hwnd;
    WNDCLASSEX windowClass;
    ImVec4 clear_color;
};

int setup(GUIWin32Context* context);
bool SanityCheck(bool debug = false);
void setupGUI(AudioContext& context);

class Application {
public:
};

#endif //AUDIOFY_APPLICATION_H

