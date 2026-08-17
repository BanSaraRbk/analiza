#ifndef WAVEFORM_VIEWER_H
#define WAVEFORM_VIEWER_H

#include <vector>
#include <TH2D.h>
#include <TCanvas.h>

class tr;

class WaveformViewer
{
public:
    WaveformViewer(int kNumChannels = 2, int kTargetModule = 0);
    ~WaveformViewer();

    void Process(tr *eventReader);
    void Draw();

private:
    std::vector<TH2D *> hWaveforms;
    int kNumChannels;
    int kTargetModule;

    void InitHistograms();
};

#endif