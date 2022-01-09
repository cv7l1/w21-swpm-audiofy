//
// Created by Jonathan on 04.01.2022.
//

#include "AudioWorkspace.h"
AudioTrack::AudioTrack()
{
}

AudioTrack::AudioTrack(AudioFile* file) : file(file) {
}


const std::string& AudioFile::getProjectName() {
    return _name;
}

FileItem& AudioFile::getFile() {
    return _file;
}
