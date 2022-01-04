#include "../../win32/ay_fileManager.h"

class ImportWindow {
public:
    static void show();
private:
    static std::optional<FileItem> selectedFile;
    static void onImportButtonPressed();
};
std::optional<FileItem> ImportWindow::selectedFile = std::nullopt;

void ImportWindow::show() {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(198, 88), ImGuiCond_FirstUseEver);

    ImGui::Begin("Import", 0, 0);
    if (ImGui::Button("Öffnen")) {
        onImportButtonPressed();
    }

    if(selectedFile.has_value()) {
        ImGui::Text("Dateiname: %ls", selectedFile.value().getDisplayName());
        ImGui::Text("Dateityp: %ls", selectedFile.value().getFileTypeDescription());
        ImGui::Text("Größe: %fmb", selectedFile.value().getFileSize());
        if(ImGui::Button("Zu Projekt hinzufügen")) {

        }
        if(ImGui::Button("Abbrechen")) {
            selectedFile = std::nullopt;
        }
    }

    ImGui::End();
}

HRESULT onFileAccept(FileItem file) {
    return S_OK;
}

void ImportWindow::onImportButtonPressed() {
    OpenFileItemDialog dialog(onFileAccept);
    dialog.show();
    ImportWindow::selectedFile = dialog.getResult();
}

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