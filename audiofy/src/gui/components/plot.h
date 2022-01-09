#include <math.h>
#include <array>
#include <imgui_plot.h>
#include "IComponent.h"
#include "../../audio/AudioContext.h"

class WaveformPlot: public IComponent{
public:
    WaveformPlot(AudioContext* context) : _context(context) {

        xData = static_cast<float *>(malloc(max * sizeof(float)));

        for(int i = 0; i<max; ++i) {
            xData[i] = i;
        }
        yData = static_cast<float*>(malloc(max * sizeof(float)));
    }

    void Show() override;
private:
    AudioContext* _context;
    float* xData = nullptr;
    float* yData = nullptr;

    u32 max = 1000;
    u32 start;
    u32 end;
    std::vector<AudioPlayBuffer<i16>> _buffer = std::vector<AudioPlayBuffer<i16>>();
    bool visible = true;
};



void showPlot(const float data[], int arraysize);