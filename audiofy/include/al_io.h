//
// Created by Jonathan on 09.12.2021.
//

#ifndef AUDIOFY_LIB_AL_IO_H
#define AUDIOFY_LIB_AL_IO_H
#include<Windows.h>
#include<string>

namespace al_io {
    class OpenFileDialog {
    public:
        void showDialog();

    private:
        std::wstring startPath;
    };
    class FileManager {
    public:
        static wchar_t* getCurrentExecutablePath();
        static void setCurrentWorkingDirectory(const wchar_t* path);_

    };
}

#endif //AUDIOFY_LIB_AL_IO_H
