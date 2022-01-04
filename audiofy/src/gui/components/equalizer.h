float* showEqualizer() {
	static float f = 1.0f;
	static int counter = 0;
	static float multipliers[10]{1,1,1,1,1,1,1,1,1,1};

	ImGui::SetNextWindowPos(ImVec2(904, 401));
	ImGui::SetNextWindowSize(ImVec2(363, 263));
	ImGui::Begin("Equalizer");

	ImGui::SliderFloat("32", &multipliers[0], 0.3f, 3.0f);
	ImGui::SliderFloat("64", &multipliers[1], 0.3f, 3.0f);
	ImGui::SliderFloat("128", &multipliers[2], 0.3f, 3.0f);
	ImGui::SliderFloat("256", &multipliers[3], 0.3f, 3.0f);
	ImGui::SliderFloat("512", &multipliers[4], 0.3f, 3.0f);
	ImGui::SliderFloat("1k", &multipliers[5], 0.3f, 3.0f);
	ImGui::SliderFloat("2k", &multipliers[6], 0.3f, 3.0f);
	ImGui::SliderFloat("4k", &multipliers[7], 0.3f, 3.0f);
	ImGui::SliderFloat("8k", &multipliers[8], 0.3f, 3.0f);
	ImGui::SliderFloat("16k", &multipliers[9], 0.3f, 3.0f);

	ImGui::End();
	return multipliers;
}