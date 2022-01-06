//
// Created by Jonathan on 05.01.2022.
//
#include "mix.h"
void showMixer() {
    ImGui::SetNextWindowPos(ImVec2(904, 0));
    ImGui::SetNextWindowSize(ImVec2(363, 401));
    ImGui::Begin("MIX");

    ImGui::End();

}

void Mixer::Show() {
    if(ImGui::Begin("Mixer")) {
        if (ImGui::CollapsingHeader("Sequencer"))
        {
            static int selectedEntry = -1;
            static int firstFrame = 0;
            static bool expanded = false;
            static int currentFrame = 100;

            Sequencer(&sequencer, &currentFrame,
                                   &expanded, &selectedEntry,
                                   &firstFrame,
                                   ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD |
                                   ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE |
                                   ImSequencer::SEQUENCER_CHANGE_FRAME);
        }
        ImGui::End();
    }
}

int AudioSequencer::GetFrameMin() const {
    return frameMin;
}

int AudioSequencer::GetFrameMax() const {
    return frameMax;
}

int AudioSequencer::GetItemCount() const {
    return (int)_tracks.size();
}



int AudioSequencer::GetItemTypeCount() const {
    return 2;
}

const char *AudioSequencer::GetItemTypeName(int i) const {
    return "AudioTrack";
}

const char *AudioSequencer::GetItemLabel(int i) const {
    if(_tracks[i].file == nullptr) {
        return "NULL";
    }
    return _tracks[i].file->getProjectName().c_str();
}


void AudioSequencer::Get(int index, int **start, int **end, int *type, unsigned int *color) {
    AudioTrack item = _tracks[index];
    if(color) {
        *color = 0xFFAA8080;
    }
    if(start) {
        *start = &item.positionStart;
    }
    if(end) {
        *end = &item.positionEnd;
    }
    if(type) {
        *type = 1;
    }
}


void AudioSequencer::Add(int i) {
    _tracks.emplace_back();
}

void AudioSequencer::Del(int i) {
    _tracks.erase(_tracks.begin() + i);
}

void AudioSequencer::Duplicate(int i) {
    _tracks.push_back(_tracks[i]);
}

void AudioSequencer::Copy() {
    SequenceInterface::Copy();
}

void AudioSequencer::Paste() {
    SequenceInterface::Paste();
}

size_t AudioSequencer::GetCustomHeight(int i) {
    return 300;
}

void AudioSequencer::DoubleClick(int i) {
    SequenceInterface::DoubleClick(i);
}

void AudioSequencer::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect) {
    static const char* labels[] = { "Translation", "Rotation" , "Scale" };

    rampEdit.mMax = ImVec2(float(frameMax), 1.f);
    rampEdit.mMin = ImVec2(float(frameMin), 0.f);
    draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
    for (int i = 0; i < 3; i++)
    {
        ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
        ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
        draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
        if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
            rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
    }
    draw_list->PopClipRect();

    ImGui::SetCursorScreenPos(rc.Min);
}

void AudioSequencer::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) {
    draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
    draw_list->PopClipRect();
}
