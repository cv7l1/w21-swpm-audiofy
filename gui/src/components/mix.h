typedef struct file
{
    int length;
    char* name;
    int id;   //id=0 is used to ignore following files
} audiofile;

char* fileDetailString(audiofile f) {
    return(f.name);
}

void showMixer(audiofile* importedfiles, int length) {
    ImGui::SetNextWindowPos(ImVec2(904, 0));
    ImGui::SetNextWindowSize(ImVec2(363, 401));
    ImGui::Begin("MIX");

    ImGui::Text("Sequencer (Drag&Drop)");
    ImGui::Indent();
    int move_from = -1, move_to = -1;
    for (int n = 0; n < length; n++)
    {
        if (importedfiles[n].id != 0) {
            ImGui::Selectable(fileDetailString(importedfiles[n]));
        }
        else {
            ImGui::Unindent();
            ImGui::Selectable("Ignored:");
            ImGui::Indent();
        }

        ImGuiDragDropFlags src_flags = 0;
        src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep item displayed while hovering
        src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers; // Keep changes in local space
        if (ImGui::BeginDragDropSource(src_flags))//Object is being dropped
        {
            if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                ImGui::Text("Moving \"%s\"", importedfiles[n].name);
            ImGui::SetDragDropPayload("DND_NAME", &n, sizeof(int));
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            ImGuiDragDropFlags target_flags = 0;
            target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;   // Move while still hovering (no additional action needed)
            target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // hide yellow rectangle
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_NAME", target_flags))
            {
                move_from = *(const int*)payload->Data;
                move_to = n;
            }
            ImGui::EndDragDropTarget();
        }
    }
    
    if (move_from != -1 && move_to != -1)
    {
        // Reorder items
        int copy_dst = (move_from < move_to) ? move_from : move_to + 1;
        int copy_src = (move_from < move_to) ? move_from + 1 : move_to;
        int copy_count = (move_from < move_to) ? move_to - move_from : move_from - move_to;
        const audiofile tmp = importedfiles[move_from];
        memmove(&importedfiles[copy_dst], &importedfiles[copy_src], (size_t)copy_count * sizeof(audiofile));
        importedfiles[move_to] = tmp;
        ImGui::SetDragDropPayload("DND_NAME", &move_to, sizeof(int)); //make sure to update immediately
    }
    ImGui::Unindent();

    static audiofile current_item = {-1,"Not Selected",-1};
    ImGui::Text("\nDuplicate/Remove file:");
    if (ImGui::BeginCombo("##combo", current_item.name)) // currentitem = preselected item
    {
        for (int n = 0; n < length; n++)
        {
            if (importedfiles[n].id != 0) {
                bool is_selected = (current_item.id == importedfiles[n].id);
                if (ImGui::Selectable(importedfiles[n].name, is_selected))
                    current_item = importedfiles[n]; // store selected item
                if (is_selected)
                    ImGui::SetItemDefaultFocus();   // initial focus
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Duplicate"))
    {
        if (current_item.id != -1) {
            //DUPLICATE
        }
        current_item = { -1,"Not Selected",-1 };
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove"))
    {
        if (current_item.id !=-1) {
            //REMOVE
        }
        current_item = { -1,"Not Selected",-1 };
    }


    ImGui::End();
}

