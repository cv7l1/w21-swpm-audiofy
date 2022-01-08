void showToolBar() {
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

        ImGui::EndMainMenuBar();
    }
}