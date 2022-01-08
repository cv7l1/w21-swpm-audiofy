//
// Created by Jonathan on 05.01.2022.
//

#include "fileDialog.h"
std::optional<FileItem> ImportWindow::selectedFile = std::nullopt;

HRESULT onFileAccept(FileItem file) {
    return S_OK;
}

FileInfoWindow::FileInfoWindow(AudioContext* context, FileItem item) : _context(context), _item(item) {
    try {
        audioFile = context->_decoder->loadAudioFile(item.getFullFilePath());
    } catch(std::exception& e) {
        audioFile = std::nullopt;
    }
}

void FileInfoWindow::Show() {
    if(!_open){return;}

    ImGui::Begin("Info", &_open);
    ImGui::Text("Dateiname: %ls", _item.getDisplayName());
    ImGui::Text("Dateityp: %ls", _item.getFileTypeDescription());
    ImGui::Text("Größe: %fmb", _item.getFileSize());
    if(audioFile.has_value()) {
        ImGui::Text("Anzahl Samples: %llu", audioFile.value()->getSampleCount());
        ImGui::Text("Spielzeit Sek: %f", audioFile.value()->getLengthSeconds());
        ImGui::Text("Samples pro Sekunde: %u", audioFile.value()->getSampleRate());
    } else {
        ImGui::Text("Keine Audiodatei");
    }
    ImGui::End();
}
void ImportWindow::Show() {
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
            GuiMain::AddComponent(new ProjectAddWindow(_context, selectedFile.value()));
            selectedFile = std::nullopt;
        }

        if(ImGui::Button("Abbrechen")) {
            selectedFile = std::nullopt;
        }
    }

    ImGui::End();
}

void ImportWindow::onImportButtonPressed() {
    OpenFileItemDialog dialog(onFileAccept);
    dialog.show();
    ImportWindow::selectedFile = dialog.getResult();
}

void ProjectAddWindow::Show() {
    char buf[50];
    _bstr_t name(_item.getDisplayName());
    sprintf(buf, "%s", name.operator const char *());

    if(_close) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(198, 88), ImGuiCond_FirstUseEver);
    ImGui::Begin("Zu Projekt hinzufügen", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::InputText("Name", buf, IM_ARRAYSIZE(buf));

    if(ImGui::Button("Abbrechen")) {
        _close = true;
    }
    if(ImGui::Button("Hinzufügen")) {
        ProjectFiles::AddFile(AudioFile(_context->_decoder->loadAudioFile(_item.getFullFilePath()), _item,  std::string(buf)));
        _close = true;
    }
    ImGui::End();
}
