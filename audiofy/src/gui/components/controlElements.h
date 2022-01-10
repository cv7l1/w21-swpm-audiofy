#pragma once

#include "../../audio/AudioContext.h"
#include "IComponent.h"
#include "imgui.h"
#include "mix.h"

class ControlElements : public IComponent {
public:
	ControlElements(AudioContext* context, MixerComponent* mixer) : _context(context), _mixer(mixer) {}
	void Show() override;

private:
	bool _visible = true;
	AudioContext* _context;
	MixerComponent* _mixer;
};