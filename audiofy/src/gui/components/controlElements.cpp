#include "controlElements.h"

void ControlElements::Show()
{
	ImGui::SetNextWindowSize(ImVec2(350, 90));
	if (ImGui::Begin("Control Elements")) {

		if (ImGui::Button("Play")) {
			_mixer->SetPosition(_mixer->currentPositionSec);
			_context->_player->play();
			_mixer->isPlaying = true;
		} 
		ImGui::SameLine();
		if (ImGui::Button("Pause")) {
			_context->_player->pause();
			_mixer->isPlaying = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			_mixer->SetPosition(0);
			_context->_player->pause();
			_mixer->isPlaying = false;
		}
		ImGui::End();
	}
}
