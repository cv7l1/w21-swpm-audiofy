#include <math.h>
#include <array>
#include <imgui_plot.h>
#include "IComponent.h"

class WaveformPlot: public IComponent{
public:
    WaveformPlot(AudioPlayBuffer<i16>& buffer) : _buffer(buffer) {
        max = 10000;
        xData = static_cast<float *>(malloc(max * sizeof(float)));
        for(int i = 0; i<max; ++i) {
            xData[i] = i;
        }
        yData = static_cast<float *>(malloc(max * sizeof(float)));

        /*
        for(int i = 0; i<buffer.getRawData().size(); i+=2) {
            float sample = (float)buffer.getRawData()[i] / 32768;
            if(sample > 0) {
            }
            if(sample < 0) {
                sum += -sample;
            } else {
                sum += sample;
            }
            yData.push_back(sample);
        }

        xData.resize(_buffer.getRawData().size());

        for(int i = 0; i<yData.size(); i++) {
            xData[i] = i;
        }
    */
    }
    void Show() override;
private:
    float* xData;
    float* yData;
    u32 max;
    AudioPlayBuffer<i16>& _buffer;
};



void showPlot(const float data[], int arraysize);