//
// Created by Jonathan on 30.12.2021.
//

#ifndef AUDIOLIB_EXAMPLES1_GUI_FILE_H
#define AUDIOLIB_EXAMPLES1_GUI_FILE_H

#include "imgui/imgui.h"
namespace gui {

    class FileWindow {
    public:
        FileWindow(ImVec2 windowPosition = ImVec2(0,0), ImVec2 windowSize = ImVec2(198, 88), _In_z_ wchar_t* windowName = "Import your audio files")
        : _windowName(windowName), _windowPosition(windowPosition), _windowSize(windowSize);


    private:
        ImVec2 _windowPosition;
        ImVec2 _windowSize;

        wchar_t* _windowName;

    };
}


#endif //AUDIOLIB_EXAMPLES1_GUI_FILE_H
