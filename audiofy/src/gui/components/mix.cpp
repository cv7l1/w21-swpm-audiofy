//
// Created by Jonathan on 05.01.2022.
//
#include "mix.h"
#include<format>
#include "samplerate.h"
#include "../GuiMain.h"
#include "EffectWindow.h"

#include "controlElements.h"
void showMixer() {
    ImGui::SetNextWindowPos(ImVec2(904, 0));
    ImGui::SetNextWindowSize(ImVec2(363, 401));
    ImGui::Begin("MIX");

    ImGui::End();

}
MixerComponent::MixerComponent(AudioContext* context) : _context(context), sequencer(context) {
    for (auto& item : _context->manager->getItems()) {
        sequencer._tracks.emplace_back();
    }
     
    sequencer.frameMin = 0;
    sequencer.frameMax = 1000;
}
void MixerComponent::Show() {
    bool ret;
    if (!visible) { return; }
    if(ret = ImGui::Begin("Mixer", &visible, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse)) {
        static int selectedEntry = -1;
        static int firstFrame = 0;
        static bool expanded = false;
        ImGui::Button("Add");
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_DD", 0)) {
                if (payload->Data != nullptr) {
                    int index = *(const int*)payload->Data;
                    sequencer.Add(index);
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remix")) {
            auto& buffer = _context->_mixer->getOutputBuffer();
            if (buffer.getCurrentBufferSize() > 0) {
				_context->_mixer->remixAll();
                auto& buffer = _context->_mixer->getOutputBuffer();
				_context->_player->playAudioBuffer(buffer, false);
            }
        }

        Sequencer(&sequencer, &_context->currentPositionSec,
                               &expanded, &selectedEntry,
                               &firstFrame,
                               ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD |
                               ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | (!_context->isPlaying ? ImSequencer::SEQUENCER_OPTIONS::SEQUENCER_CHANGE_FRAME : 0)
                               );
        ImGui::End();
    }
}

void MixerComponent::SetPosition(u32 positionSec) {
    auto seekPos = positionSec * _context->_player->getAudioFormat().sampleRate;
    _context->currentPositionSec = positionSec;
    _context->_player->seekToSample(seekPos);
    counter = 0;
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
    return _context->manager->getItems().size();
}

const char *AudioSequencer::GetItemTypeName(int i) const {
    auto item = _context->manager->getItems()[i];
    if (item->getProjectName().empty()) { return "No name"; }

    auto name = _context->manager->getItems()[i]->getProjectName().c_str();
    return name;
}

const char *AudioSequencer::GetItemLabel(int i) const {
    if(_tracks[i]->file == nullptr) {
        return "NULL";
    }
    if (_tracks[i]->file->getProjectName().empty()) { return "No name"; }
    return _tracks[i]->file->getProjectName().c_str();
}


void AudioSequencer::Get(int index, int **start, int **end, int *type, unsigned int *color) {
    AudioTrack* item = _tracks[index];
    if(color) {
        *color = 0xFFAA8080;
    }
    if(start) {
        *start = &item->positionStart;
    }
    if(end) {
        *end = &item->positionEnd;
    }
    if(type) {
        *type = item->trackCount;
    }
}


void bufferTrackResample(AudioTrack* track) {
    u32 targetSampleRate = 44100;
    al_ErrorInfo("Resampling");

    auto sampleCount = track->file->audioInfo->getSampleCount();
    u32 frameCount = sampleCount * 2;
    u32 oldFrameCount = track->file->audioInfo->getSampleCount();
    u32 newFrameCount = track->file->audioInfo->getLengthSeconds() * targetSampleRate;

    auto messageString = std::format("old sample rate: {} target sample rate: {}", track->file->audioInfo->getSampleRate(), targetSampleRate);
    al_ErrorInfo(messageString.c_str());

    auto floatBuffer = static_cast<float*>(malloc(sizeof(float) * frameCount * 2));
    auto tempResBuffer = static_cast<float*>(malloc(newFrameCount * sizeof(float) * 2));

    al_ErrorInfo("Converting target to float buffer");
    src_short_to_float_array(track->buffer.getRawData().data(), floatBuffer, frameCount);
    
    SRC_DATA srcData = { 0 };

    srcData.data_in = floatBuffer;
    srcData.input_frames = oldFrameCount;
    srcData.output_frames = newFrameCount;

    srcData.data_out = tempResBuffer;
    srcData.src_ratio = (float)targetSampleRate / (float)track->buffer.getAudioFormat().sampleRate;

    al_ErrorInfo("Resample");
    auto result = src_simple(&srcData, SRC_LINEAR, 2);
    al_ErrorInfo("Resample done");

    if (result != 0) {
        auto error = src_strerror(result);
        al_ErrorCritical(error);
        throw std::exception();
    }
    al_ErrorInfo("Convert back to i16");
    track->buffer.getRawData().resize(srcData.output_frames_gen * 2);
    src_float_to_short_array(tempResBuffer, track->buffer.getRawData().data(), srcData.output_frames_gen * 2);

    al_ErrorInfo("Convert done");
    AudioFormatInfo info = track->buffer.getAudioFormat();
    info.sampleRate = targetSampleRate;
       
    auto stringMessage = std::format("new buffer size: {}", srcData.output_frames_gen * 2);

    track->buffer.setAudioFormat(info);

    al_ErrorInfo("Resampling done");
   
    free(floatBuffer);
    free(tempResBuffer);
}

void AudioSequencer::Add(int i) {
    _context->_player->pause();
    auto item = _context->manager->getItems()[i];
    Add(item);
}
void AudioSequencer::Add(AudioFile* item) {
    auto track = new AudioTrack(item);

    track->positionStart = 0;
    track->positionEnd = (int)item->audioInfo->getLengthSeconds();
    
    if (track->buffer.getCurrentBufferSize() != 0) {
        track->buffer.getRawData().clear();
    }

	try {
	    _context->_decoder->decodeAudioFile(track->file->audioInfo, track->buffer);
		track->effectProcessor = new SoundProcessor(&track->buffer);
	} catch(std::exception& e) {
		return;
	}

	if (track->buffer.getAudioFormat().sampleRate != 44100) {
		bufferTrackResample(track);
	}

    track->trackCount = _tracks.size();
    _tracks.emplace_back(track);
    _context->_mixer->mixTrack(track);
    
    _context->_player->playAudioBuffer(_context->_mixer->getOutputBuffer(), false);
}

void AudioSequencer::Del(int i) {
    _tracks[i]->file->audioInfo = _context->_decoder->loadAudioFile(_tracks[i]->file->getFile().getFullFilePath());

    _context->_mixer->remove(_tracks[i]);
    _tracks.erase(_tracks.begin() + i);
    
    _context->_mixer->remixAll();
    _context->_player->playAudioBuffer(_context->_mixer->getOutputBuffer(), false);
}

void AudioSequencer::Duplicate(int i) {
    AudioTrack* track = _tracks[i];
    _tracks.emplace_back(track);
    _context->_mixer->mixTrack(track);
    
    _context->_player->playAudioBuffer(_context->_mixer->getOutputBuffer(), false);
}

void AudioSequencer::Copy() {
    SequenceInterface::Copy();
}

void AudioSequencer::Paste() {
    SequenceInterface::Paste();
}

size_t AudioSequencer::GetCustomHeight(int i) {
    return 0;
}

void AudioSequencer::DoubleClick(int i) {
    GuiMain::AddComponent(new EffectWindow(_tracks[i]));
}

void AudioSequencer::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect) {
    /*
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
    */
}

void AudioSequencer::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) {
}
