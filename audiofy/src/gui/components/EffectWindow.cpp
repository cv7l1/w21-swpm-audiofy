#include "EffectWindow.h"
#include "../../soundprocessor/effects/Effect.h"
#include "../../soundprocessor/effects/Pitch.h"
#include "../../soundprocessor/effects/Overmodulation.h"
#include "../../soundprocessor/effects/Tempo.h"
#include "../../soundprocessor/effects/Volume.h"

void EffectWindow::Show()
{
	if (!visible) { return; }
	if (ImGui::Begin("Effects", &visible, 0)) {
		const char* availableEffects[] = { "Pitch", "Overmodulation", "Volume", "Tempo" };
		ImGui::Text(_track->file->getProjectName().c_str());
		static int selected = 0;
		ImGui::BeginChild("left", ImVec2(150, 0), true); {
			for (int i = 0; i < IM_ARRAYSIZE(availableEffects); ++i) {
				if (ImGui::Selectable(availableEffects[i], selected == i)) {
					selected = i;
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::BeginChild("details", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
		ImGui::Text(availableEffects[selected]);
		ImGui::Separator();

		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
			if (ImGui::BeginTabItem("Description")) {
				if (selected == 0) {
					ImGui::TextWrapped("Set a pitch without changing the speed");
				}
				if (selected == 1) {
					ImGui::TextWrapped("Change the current volume with a total disregard for doing damage to your ears");
				}
				if (selected == 2) {
					ImGui::TextWrapped("Change the mixing volume");
				}
				if (selected == 3) {
					ImGui::TextWrapped("Change the tempo (DONT USE!!!)");
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Settings")) {
				if (selected == 0) {
					static float pitch = 0.0f;
					ImGui::SliderFloat("Pitch", &pitch, -12.0, 12.0);

					if (ImGui::Button("Save & Apply")) {
						_track->effectProcessor->addEffect(new Pitch(pitch));
					}
				}
				if (selected == 1) {
					static int vol = 0;
					ImGui::SliderInt("Volume", &vol, 0, 20);

					if (ImGui::Button("Save & Apply")) {
						_track->effectProcessor->addEffect(new Overmodulation(vol));
					}
				}
				if (selected == 2) {
					static float vol = 0;
					ImGui::SliderFloat("Volume", &vol, 0, 1);

					if (ImGui::Button("Save & Apply")) {
						_track->mixVol = vol;
					}
				}
				if (selected == 3) {
					static float tempo = 1.0f;
					ImGui::SliderFloat("Tempo", &tempo, 0.0, 1.0);

					if (ImGui::Button("Save & Apply")) {
						_track->effectProcessor->addEffect(new Tempo(tempo));
					}
				}
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
		ImGui::EndChild();
		ImGui::EndGroup();
		ImGui::End();
	}
}
