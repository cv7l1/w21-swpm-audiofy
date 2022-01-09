//
// Created by Jonathan on 05.01.2022.
//
#include "mix.h"
#include<format>
#include "samplerate.h"
#include "../GuiMain.h"
#include "controlElements.h"
void showMixer() {
    ImGui::SetNextWindowPos(ImVec2(904, 0));
    ImGui::SetNextWindowSize(ImVec2(363, 401));
    ImGui::Begin("MIX");

    ImGui::End();

}
MixerComponent::MixerComponent(AudioContext* context) : _context(context), sequencer(context) {
    AudioTrack* track = new AudioTrack;
    track->positionStart = 0;
    track->positionEnd = 10;


    sequencer._tracks.emplace_back(track);
    for (auto& item : ProjectFiles::getItems()) {
        sequencer._tracks.emplace_back();
    }
     
    sequencer.frameMin = 0;
    sequencer.frameMax = 1000;
    _context->addMetronomeListener([this](u64 ticks) {
        if (this->isPlaying) {
            al_ErrorInfo("Moin");
            this->currentPositionSec++;
        }
        });
    GuiMain::AddComponent(new ControlElements(_context, this));
}
void MixerComponent::Show() {
    if(ImGui::Begin("Mixer"), &visible, ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysAutoResize) {
        static int selectedEntry = -1;
        static int firstFrame = 0;
        static bool expanded = false;

        if (ImGui::Button("Remix")) {
            auto& buffer = _context->_mixer->getOutputBuffer();
            if (buffer.getCurrentBufferSize() > 0) {
				_context->_mixer->remixAll();
                auto& buffer = _context->_mixer->getOutputBuffer();
				_context->_player->playAudioBuffer(buffer, false);
            }
        }

        Sequencer(&sequencer, &currentPositionSec,
                               &expanded, &selectedEntry,
                               &firstFrame,
                               ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD |
                               ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | (!isPlaying ? ImSequencer::SEQUENCER_OPTIONS::SEQUENCER_CHANGE_FRAME : 0)
                               );

        /*
        if(ImGui::Button("Play")) {
            isPlaying = true;
            auto seekPos = currentPositionSec * _context->_player->getAudioFormat().sampleRate;

            _context->_player->seekToSample(seekPos);
            counter = 0;
            _context->_player->play();

        }
        */
        ImGui::End();
    }
}

void MixerComponent::SetPosition(u32 positionSec) {
    auto seekPos = positionSec * _context->_player->getAudioFormat().sampleRate;
    currentPositionSec = positionSec;
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
    return ProjectFiles::getItems().size();
}

const char *AudioSequencer::GetItemTypeName(int i) const {
    return ProjectFiles::getItems()[i].getProjectName().c_str();
}

const char *AudioSequencer::GetItemLabel(int i) const {
    if(_tracks[i]->file == nullptr) {
        return "NULL";
    }
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
    auto item = &ProjectFiles::getItems()[i];

    auto track = new AudioTrack(item);

    track->positionStart = 0;
    track->positionEnd = (int)item->audioInfo->getLengthSeconds();

    if(track->buffer.getCurrentBufferSize() == 0) {
        try {
            _context->_decoder->decodeAudioFile(track->file->audioInfo, track->buffer);

        } catch(std::exception& e) {
            return;
        }
    }

	if (track->buffer.getAudioFormat().sampleRate != 44100) {
		bufferTrackResample(track);
	}

    track->trackCount = _tracks.size();
    _tracks.emplace_back(track);
    _context->_mixer->mixTrack(track);
    
    _context->_player->playAudioBuffer(_context->_mixer->getOutputBuffer());
}

void AudioSequencer::Del(int i) {
    _tracks.erase(_tracks.begin() + i);
    _context->_mixer->remixAll();
    _context->_player->playAudioBuffer(_context->_mixer->getOutputBuffer());
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
    return 30;
}

void AudioSequencer::DoubleClick(int i) {
    SequenceInterface::DoubleClick(i);
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
