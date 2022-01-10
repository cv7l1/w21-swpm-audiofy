#pragma once

#include "imgui.h"
#include "IComponent.h"
#include "../../audio/AudioWorkspace.h"

class EffectWindow : public IComponent {
public:
	EffectWindow(AudioTrack* track) : _track(track) {};
	void Show() override;
private:
	AudioTrack* _track;
	bool visible = true;
};
