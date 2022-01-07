//
// Created by Jonathan on 04.01.2022.
//

#ifndef AUDIOFY_AUDIOWORKSPACE_H
#define AUDIOFY_AUDIOWORKSPACE_H

#include<list>
#include<map>
#include "al_player.h"
#include "../win32/ay_fileManager.h"
#include<optional>
#include<string>

class AudioFile {
public:
    AudioFile(FileItem& file, std::string name) :  _name(name), _file(file){}
    const std::string& getProjectName() {
        return _name;
    }
    FileItem& getFile() {
        return _file;
    }

private:
    std::string _name;
    FileItem& _file;
    AudioPlayBuffer<i16>* pcmBuffer = nullptr;
};
class AudioTrack {
public:
    AudioTrack() {};
    AudioTrack(AudioFile* file) : file(file) {
    }
    int positionStart = 0;
    int positionEnd= 0;

    u32 start = 0;
    u32 end = 0;
    int trackCount = 0;
    AudioFile*  file = nullptr;

};


class AudioWorkspace {
    std::list<AudioTrack> audioTracks = std::list<AudioTrack>();

};


#endif //AUDIOFY_AUDIOWORKSPACE_H
