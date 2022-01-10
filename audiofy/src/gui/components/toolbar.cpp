#include "toolbar.h"
#include "al_file.h"
#include "plot.h"
HRESULT onFileExport(FileItem item) {
    return S_OK;
}

void Toolbar::Show()
{
    static bool showAbout = false;
    static bool showExportFinished = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Import file"))
            {
                GuiMain::AddComponent(new ImportWindow(_context));
            }
            else if (ImGui::MenuItem("Export"))
            {
                Win32FileItemDialog dialog(onFileExport, DialogType::ayFile_Save);
                dialog.show();
                auto file = dialog.getResult();
                if (file.has_value()) {
                    auto path = file.value().getFullFilePath();
                    auto& buffer = _context->_mixer->getOutputBuffer();

                    if (buffer.getCurrentBufferSize() > 0) {
                        DebugWaveFileWriter::writeBufferToWaveFile(path, buffer);
                    }
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Project")) {
            if (ImGui::MenuItem("Project files")) {
                GuiMain::AddComponent(new ProjectFileListComponent(_context));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Devices")) {
            if (ImGui::MenuItem("Show devices")) {
                GuiMain::AddComponent(new DeviceListComponent( _context, _deviceManager));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Sequencer")) {
                GuiMain::AddComponent(new MixerComponent(_context));
            }
            if (ImGui::MenuItem("Waveform")) {
                GuiMain::AddComponent(new WaveformPlot(_context));
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

        /*
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
        */

        if (ImGui::BeginMenu("Other"))
        {
            if (ImGui::MenuItem("ABOUT US")) {
                showAbout = true;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        if (showAbout) {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(300, 300));
            ImGui::Begin("About us");
            ImGui::Text("\n\n\n\n\n\t\t    --Presented by-- \n\t\t\tJonathan Hauter\n\t\t\t Vivien Traue\n\t\t    Dennis Gorpinic\n\t\t   Clemens Vogtländer\n\t\t\t Marc Leenders\n\t\t\tMario Blomenkamp\n\t\t\t Mathis Grabert\n\n\n\n\n\n");
            if (ImGui::Button("CLOSE")) {
                showAbout = false;
            }
            ImGui::End();
        }
    }
}

