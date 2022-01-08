void showToolBar(ImVec4* colors) {
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Files"))
        {
            if (ImGui::MenuItem("Import"))
            {
                //Use filedialog to import files
            }
            else if (ImGui::MenuItem("Export"))
            {
                //Use filedialog to export file
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Reset"))
        {
            if (ImGui::MenuItem("All"))
            {
                //Reset all values
            }
            else if (ImGui::MenuItem("Equalizer"))
            {
                //Reset Equalizer
            }
            else if (ImGui::MenuItem("Leveling"))
            {
                //Reset Leveling
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Color Schemes"))
        {
            if (ImGui::MenuItem("White"))
            {
                colors[ImGuiCol_WindowBg] = ImVec4(0.39f, 0.00f, 0.63f, 0.11f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.39f, 0.00f, 0.63f, 0.11f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.39f, 0.00f, 0.63f, 0.11f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.39f, 0.00f, 0.63f, 0.11f);
                colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            }
            else if (ImGui::MenuItem("Black"))
            {
                colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
                colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            else if (ImGui::MenuItem("Blue"))
            {
                colors[ImGuiCol_WindowBg] = ImVec4(0.050f, 0.0f, 0.521f, 1.0f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.050f, 0.0f, 0.521f, 1.0f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.050f, 0.0f, 0.521f, 1.0f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.050f, 0.0f, 0.521f, 1.0f);
                colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}