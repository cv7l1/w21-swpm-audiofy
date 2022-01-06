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
class Application {
public:
    static AudioPlayer player;
    static AudioDecoder decoder;
};

class ProjectFiles {
public:
    static void AddFile(AudioFile item) {
        files.push_back(item);
    }

    static std::vector<AudioFile>& getItems() {
        return files;
    };

private:
    static std::vector<AudioFile> files;
};

#endif //AUDIOFY_APPLICATION_H

