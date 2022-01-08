//
// Created by Jonathan on 04.01.2022.
//

#include "projectFileListComponent.h"
#include "../../Application.h"
#include "imgui.h"
#include "../../win32/ay_fileManager.h"
#include "../GuiMain.h"
#include "fileDialog.h"
#include <string>
#include <comdef.h>

void ProjectFileListComponent::Show() {
    if(ImGui::Begin("Projekt")) {
        std::vector<AudioFile>& itemList = ProjectFiles::getItems();
        if(ImGui::BeginListBox("")) {
            int index = 0;

            for(auto item : itemList) {
                if(ImGui::Selectable(item.getProjectName().c_str())) {
                    selectedItem = index;
                }
                if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover)) {
                    ImGui::Text("Moving file");
                    ImGui::SetDragDropPayload("FILE_DD", &selectedItem, sizeof(int));
                    ImGui::EndDragDropSource();
                }

                index++;
            }
            ImGui::EndListBox();
        }

        if(selectedItem != -1) {
            ImGui::Text("Dateiname: %ls",itemList[selectedItem].getFile().getDisplayName());
            ImGui::Text("Dateityp: %ls", itemList[selectedItem].getFile().getFileTypeDescription());
            ImGui::Text("Größe: %fmb", itemList[selectedItem].getFile().getFileSize());

            if(ImGui::Button("Mehr Informationen")) {
                GuiMain::AddComponent(new FileInfoWindow(_context, itemList[selectedItem].getFile()));
            }
        }

        ImGui::End();
    }

}
