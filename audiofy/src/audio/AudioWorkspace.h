//
// Created by Jonathan on 04.01.2022.
//

#pragma once

#include<list>
#include<map>
#include "al_player.h"
#include "al_file.h"

#include "../win32/ay_fileManager.h"
#include<optional>
#include<string>
#include "../soundprocessor/SoundProcessor.h"

class AudioFile {
public:
    AudioFile(IAudioFile* audioFile, FileItem& file, std::string name) :  _name(name), _file(file), audioInfo(audioFile) {}
    const std::string& getProjectName();
    FileItem& getFile();
    IAudioFile* audioInfo = nullptr;

private:
    std::string _name;
    FileItem& _file;
};

class AudioTrack {
public:
    AudioTrack();
    AudioTrack(AudioFile* file);
    AudioPlayBuffer<>& getBuffer() { return buffer; }
    int positionStart = 0;
    int positionEnd= 0;

    u32 start = 0;
    u32 end = 0;
    int trackCount = 0;
    float mixVol = 1;

    AudioFile* file = nullptr;
    AudioPlayBuffer<> buffer;
    SoundProcessor* effectProcessor;
};

class ProjectFileManager {
public:
	void AddFile(AudioFile* file) {
		audioFiles.push_back(file);
	 }
    const std::vector<AudioFile*>& getItems() {
        return audioFiles;
     }
              
private:
    std::vector<AudioFile*> audioFiles;
};

