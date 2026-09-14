#include "../include/WaveformViewer.h"
#include "../include/tr.h"

#include <iostream>
#include <cmath>
#include <TString.h>

WaveformViewer::WaveformViewer(int numChannels, int targetModule)
    : kNumChannels(numChannels), kTargetModule(targetModule)
{
    InitHistograms();
}

WaveformViewer::~WaveformViewer()
{
    for (auto hist : hWaveforms)
    {
        if (hist)
            delete hist;
    }
}

void WaveformViewer::InitHistograms()
{
    hWaveforms.resize(kNumChannels);
    for (int ch = 0; ch < kNumChannels; ++ch)
    {
        hWaveforms[ch] = new TH2D(
            Form("h_wave_ch%d", ch),
            Form("Waveform Channel %d;Sample;Amplitude", ch),
            400, 0, 3000,
            400, 7000, 20000);
    }
}

void WaveformViewer::Process(tr *eventReader)
{
    if (!eventReader || !eventReader->fChain)
        return;

    Long64_t nentries = eventReader->fChain->GetEntries();

    for (Long64_t jentry = 0; jentry < nentries; ++jentry)
    {
        eventReader->fChain->GetEntry(jentry);

        const int ch = static_cast<int>(eventReader->channel);
        const int module = static_cast<int>(eventReader->module);

        if (module >= 0 && module != kTargetModule)
            continue;

        if (ch >= 0 && ch < kNumChannels && eventReader->analog_probe1 != nullptr)
        {
            for (size_t sample = 0; sample < eventReader->analog_probe1->size(); ++sample)
            {
                // std::cout << "Sample: " << sample << ", Value: " << (*eventReader->analog_probe1)[sample] << std::endl;
                hWaveforms[module][ch].Fill(sample, (*eventReader->analog_probe1)[sample]);
            }
        }
    }

    Draw();
}

void WaveformViewer::Draw()
{
    int nCols = std::ceil(std::sqrt(kNumChannels));
    int nRows = std::ceil(static_cast<double>(kNumChannels) / nCols);

    for (int module = 0; module < kTargetModule; ++module)
    {
        TString cName = Form("c_module_%d", module);
        TString cTitle = Form("Waveforms Module %d", module);
        TCanvas *c = new TCanvas(cName, cTitle, 1200, 800);
        c->Divide(nCols, nRows);

        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            c->cd(ch + 1);
            hWaveforms[module][ch].Draw("COLZ");
        }
        c->Update();
    }
}
