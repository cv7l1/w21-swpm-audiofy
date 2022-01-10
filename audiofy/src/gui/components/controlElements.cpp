#include "controlElements.h"

void ControlElements::Show()
{
	if (ImGui::Begin("Control Elements", &_visible, ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::Button("Play")) {
			_mixer->SetPosition(_context->currentPositionSec);
			_context->_player->play();
			_context->isPlaying = true;
		} 
		ImGui::SameLine();
		if (ImGui::Button("Pause")) {
			_context->_player->pause();
			_context->isPlaying = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			_mixer->SetPosition(0);
			_context->_player->pause();
			_context->isPlaying = false;
		}
		static float vol = 1;
		if (ImGui::SliderFloat("Master Volume", &vol, 0.0, 1.0)) {
			_context->_player->setMasterVolume(vol);
		}
	}
	ImGui::End();
}
