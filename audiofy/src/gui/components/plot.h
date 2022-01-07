#include <math.h>
#include <array>
#include <imgui_plot.h>
#include "IComponent.h"

class WaveformPlot: public IComponent{
public:
    explicit WaveformPlot(AudioPlayBuffer<i16>& buffer) {

        max = 10000;
        _buffer.push_back(buffer);
        xData = static_cast<float *>(malloc(max * sizeof(float)));
        for(int i = 0; i<max; ++i) {
            xData[i] = i;
        }

        yData.push_back(static_cast<float *>(malloc(max * sizeof(float))));
    }
    void AddBuffer(AudioPlayBuffer<i16>& buffer) {
        _buffer.push_back(buffer);
        yData.push_back(static_cast<float *>(malloc(max * sizeof(float))));
    }

    void Show() override;
private:
    float* xData;
    std::vector<float*> yData;
    u32 max;

    u32 start;
    u32 end;
    std::vector<AudioPlayBuffer<i16>> _buffer = std::vector<AudioPlayBuffer<i16>>();
};



void showPlot(const float data[], int arraysize);