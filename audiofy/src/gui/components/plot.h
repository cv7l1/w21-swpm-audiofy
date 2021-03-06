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
    std::vector<float*> additionalTracks;

    u32 mixBufferCount = 0;

    u32 max = 30000;
    u32 start;
    u32 end;
    bool visible = true;
    
};



void showPlot(const float data[], int arraysize);