float* showLeveling() {
	static float vol1 = 1.0f;
	static float vol2 = 1.0f;
	static int counter = 0;
	
	ImGui::SetNextWindowPos(ImVec2(756, 663));
	ImGui::SetNextWindowSize(ImVec2(522, 100));
	ImGui::Begin("Leveling");
	
	ImGui::SliderFloat("Level###Level1", &vol1, 0.3f, 3.0f);
	ImGui::SameLine();
	if (ImGui::Button("Mute###mute1"))
	vol1 = 0;
	
	ImGui::SliderFloat("Level###Level2", &vol2, 0.3f, 3.0f);
	ImGui::SameLine();
	if (ImGui::Button("Mute###mute2"))
	vol2 = 0;
	
	ImGui::End();
	float volumes[] = { vol1,vol2 };
	return volumes;
}