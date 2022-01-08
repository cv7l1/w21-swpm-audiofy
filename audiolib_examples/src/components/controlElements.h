float* showControl() {
    static float playbackSpeed = 1.0f;
    int command = 0;
    ImGui::SetNextWindowPos(ImVec2(0, 663));
    ImGui::SetNextWindowSize(ImVec2(398, 98));
    ImGui::Begin("Control Elements");

    ImGui::SliderFloat("Playback Speed", &playbackSpeed, 0.3f, 3.0f);

    if (ImGui::Button("Play"))
        command = 1;
    ImGui::SameLine();
    if (ImGui::Button("Pause"))
        command = 2;
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
        command = 3;
    ImGui::SameLine();
    if (ImGui::Button("Record"))
        command = 4;
    ImGui::SameLine();
    if (ImGui::Button("Export"))
        command = 5;
    
    float controlValues[] = {command, playbackSpeed};
    ImGui::End();
    return controlValues;
}