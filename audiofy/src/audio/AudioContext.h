#pragma once
#include "al_player.h"
#include "al_file.h"
#include "AudioMixer.h"
struct AudioContext {
    AudioContext();

    void addMetronomeListener(std::function<void(u64)> callback) {
        callbacks.push_back(callback);
    }

    void onMetronomeTick() {
        secMetronomeTicks++;
        for (auto& c : callbacks) {
            c(secMetronomeTicks);
        }
        if (isPlaying) {
            currentPositionSec++;
        }
    }
    AudioPlayer* _player;
    AudioDecoder* _decoder;
    ay_AudioMixer* _mixer;
    ProjectFileManager* manager;
    std::vector<std::function<void(u64)>> callbacks;
    u64 secMetronomeTicks = 0;
    int currentPositionSec = 0;
    bool isPlaying = false;
};
