void showImport() {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(198, 88));
    ImGui::Begin("Import your audio files");

    ImGui::SliderFloat("Bitrate", &f, 0.0f, 1.0f);

    if (ImGui::Button("Import"))
    counter++;
    ImGui::SameLine();
    ImGui::Text("Files Imported = %d", counter);

    ImGui::End();
}
void showExport() {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::SetNextWindowPos(ImVec2(0, 88));
    ImGui::SetNextWindowSize(ImVec2(198, 111));
    ImGui::Begin("Export your audio file");


    ImGui::SliderFloat("Bitrate", &f, 0.5f, 1.0f);

    static char name[128] = "";
    ImGui::Text("Filename: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
    if (ImGui::Button("Export"))
        counter++;

    ImGui::End();
}