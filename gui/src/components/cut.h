void showCut() {
    static float f = 1.0f;
    static int counter = 0;

    ImGui::SetNextWindowPos(ImVec2(395, 663));
    ImGui::SetNextWindowSize(ImVec2(363, 100));
    ImGui::Begin("Selection");

    static char name[128] = "";
    ImGui::Text("Start: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
    static char name2[128] = "";
    ImGui::Text("Stop: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name2));
    if (ImGui::Button("Cut"))
        counter++;

    ImGui::End();
}