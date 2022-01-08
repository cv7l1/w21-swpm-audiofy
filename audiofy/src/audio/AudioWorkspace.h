//
// Created by Jonathan on 04.01.2022.
//

#ifndef AUDIOFY_AUDIOWORKSPACE_H
#define AUDIOFY_AUDIOWORKSPACE_H

#include<list>
#include<map>
#include "al_player.h"
#include "al_file.h"

#include "../win32/ay_fileManager.h"
#include<optional>
#include<string>
#include <al_file.h>

class AudioFile {
public:
    AudioFile(IAudioFile* audioFile, FileItem& file, std::string name) :  _name(name), _file(file), audioInfo(audioFile) {}
    const std::string& getProjectName() {
        return _name;
    }
    FileItem& getFile() {
        return _file;
    }
    IAudioFile* audioInfo = nullptr;

private:
    std::string _name;
    FileItem& _file;
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
    AudioFile* file = nullptr;
    AudioPlayBuffer<> buffer;
};
class AudioContext {
public:
    AudioContext();

    AudioPlayer* _player;
    AudioDecoder* _decoder;
};

class AudioWorkspace {
    std::list<AudioTrack> audioTracks = std::list<AudioTrack>();

};


#endif //AUDIOFY_AUDIOWORKSPACE_H
