#ifndef FILE_DIALOG_H
#define FILE_DIALOG_H
#include "../../win32/ay_fileManager.h"
#include "IComponent.h"
#include<comdef.h>
#include "../GuiMain.h"
#include "../../Application.h"
#include "../../../../audiolib/platform/src/al_file.h"
#include "imgui.h"
#include "../../src/audio/AudioContext.h"
class FileInfoWindow : public IComponent {
public:
    explicit FileInfoWindow(AudioContext* context, FileItem item);


    void Show() override;
private:
    AudioContext* _context;
    FileItem _item;
    std::optional<IAudioFile*> audioFile = std::nullopt;
    bool _open = true;
};


class ProjectAddWindow : public IComponent{
public:
    explicit ProjectAddWindow(AudioContext* context, FileItem item) : _item(item), _context(context) {}
    void Show() override;
private:
    AudioContext* _context;
    FileItem _item;
    bool _close = false;
};

class ImportWindow : public IComponent {

public:
    ImportWindow(AudioContext* context) : _context(context) {}
    void Show() override;

private:
    static std::optional<FileItem> selectedFile;
    static void onImportButtonPressed();
    AudioContext* _context;
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