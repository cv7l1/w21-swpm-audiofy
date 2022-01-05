//
// Created by Jonathan on 05.01.2022.
//

#ifndef AUDIOFY_APPLICATION_H
#define AUDIOFY_APPLICATION_H
#include <list>
#include <al_player.h>
#include <al_file.h>
#include "win32/ay_fileManager.h"
class Application {
public:
    static AudioPlayer player;
    static AudioDecoder decoder;
};

class ProjectFiles {
public:
    static void AddFile(FileItem item) {
        files.push_back(item);
    }
    static std::vector<FileItem>& getItems() {
        return files;
    };

private:
    static std::vector<FileItem> files;
};

#endif //AUDIOFY_APPLICATION_H

