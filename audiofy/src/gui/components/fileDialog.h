#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H
#include "../../win32/ay_fileManager.h"
#include "IComponent.h"
#include<comdef.h>
#include "../GuiMain.h"
#include "../../Application.h"
#include "../../../../audiolib/platform/src/al_file.h"
#include "imgui.h"
class FileInfoWindow : public IComponent {
public:
    explicit FileInfoWindow(FileItem item) ;
    void Show() override;
private:
    FileItem _item;
    std::optional<IAudioFile*> audioFile = std::nullopt;
    bool _open = true;
};


class ProjectAddWindow : public IComponent{
public:
    explicit ProjectAddWindow(FileItem item) : _item(item) {}
    void Show() override;
private:
    FileItem _item;
    bool _close = false;
};

class ImportWindow : public IComponent {

public:
    ImportWindow() = default;
    void Show() override;

private:
    static std::optional<FileItem> selectedFile;
    static void onImportButtonPressed();
};
/*
void showExport() {
    static float f = 0.0f;
    static int counter = 0;

    //ImGui::SetNextWindowPos(ImVec2(0, 88));
    //ImGui::SetNextWindowSize(ImVec2(198, 111));

    bool active;
    ImGui::Begin("Export your audio file", &active, ImGuiWindowFlags_AlwaysAutoResize);


    ImGui::SliderFloat("Bitrate", &f, 0.5f, 1.0f);

    static char name[128] = "";
    ImGui::Text("Filename: "); ImGui::SameLine(); ImGui::InputText("", name, IM_ARRAYSIZE(name));
    if (ImGui::Button("Export"))
        counter++;

    ImGui::End();
}
 */
#endif