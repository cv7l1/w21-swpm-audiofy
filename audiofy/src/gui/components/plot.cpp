//
// Created by Jonathan on 05.01.2022.
//

#include <al_player.h>
#include <imgui.h>
#include <implot.h>
#include "plot.h"
#include "IComponent.h"
#include <array>
#include <format>
void showPlot(const float data[], int arraysize) {
    static bool fixedSize = false;
    static float dSize = arraysize;
    static float start = 0.0f;
    static float end = arraysize;

    if (!fixedSize) {
        if (end <= start) {
            start = 0;
            end = arraysize;
        }
    }
    else {
        if(start+dSize > arraysize){
            start = 0;
        }
        end = start + dSize;
    }

    const int arrSize = static_cast<int>(end - start);
    float* cutData = new float[arrSize];
    for (int i = static_cast<int>(start); i < static_cast<int>(end) - 1; i++)
    {
        cutData[static_cast<int>(i - start)] = data[i];
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(905, 664));
    ImGui::Begin("Plot");
    ImGui::PlotConfig conf;
    conf.values.ys = cutData;
    conf.values.count = static_cast<int>(end-start);
    conf.scale.min = 10;
    conf.scale.max = 20;
    conf.tooltip.show = true;
    conf.tooltip.format = "x=%.2f, y=%.2f";
    conf.grid_x.show = true;
    conf.grid_y.show = true;
    conf.frame_size = ImVec2(905, 464);
    conf.line_thickness = 2.f;

    ImGui::Plot("plot", conf);

    ImGui::SliderFloat("Start", &start, 0.0f, (float)arraysize);
    ImGui::SliderFloat("End", &end, 0.0f, (float)arraysize);


    ImGui::Checkbox("Fixed Size :", &fixedSize);
    ImGui::SameLine();
    ImGui::SliderFloat("(From Start)###FixedSize", &dSize, 0.0f, (float)arraysize);

    ImGui::End();
}

void WaveformPlot::Show() {
    static float counter;
    counter += 50;
    u32 bufferIndex = 0;

    if(ImGui::Begin("WaveformPlot")) {
        for(auto& buffer : _buffer) {
            for(int i = 0; i<max; i++) {
                float sample = (float)buffer.getRawData()[i + 1  + counter] / 32768;
                yData[bufferIndex][i] = sample;
            }
            bufferIndex++;
        }
        if(ImPlot::BeginPlot("WaveformPlot")) {
            u32 dataIndex = 0;
            for(auto& data : yData) {
                //auto name = std::format("AudioData{}", dataIndex);
                auto name = std::string("Test");
                ImPlot::PlotLine<float>(name.c_str(), xData, data, max);
                dataIndex++;
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
    }
}

