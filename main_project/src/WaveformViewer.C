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
    for (int module = 0; module < kTargetModule; ++module)
    {
        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            if (hWaveforms[module][ch])
                delete hWaveforms[module][ch];
        }
    }
}

void WaveformViewer::InitHistograms()
{
    hWaveforms.resize(kTargetModule);
    for (int module = 0; module < kTargetModule; ++module)
    {
        hWaveforms[module].resize(kNumChannels);
        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            hWaveforms[module][ch] = new TH2D(
                Form("h_wave_mod%d_ch%d", module, ch),
                Form("Waveform Channel %d;Sample;Amplitude", ch),
                400, 0, 500,
                400, 7000, 20000);
        }
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

        if (ch >= 0 && ch < kNumChannels && module < kTargetModule && eventReader->analog_probe1 != nullptr)
        {
            for (size_t sample = 0; sample < eventReader->analog_probe1->size(); ++sample)
            {
                hWaveforms[module][ch]->Fill(sample, (*eventReader->analog_probe1)[sample]);
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
        // Unique name ("c_mod0", "c_mod1", ...) and title per canvas
        TCanvas *c = new TCanvas(
            Form("c_mod%d", module),
            Form("Waveforms - Module %d", module),
            1200, 800);
        c->Divide(nCols, nRows);

        for (int ch = 0; ch < kNumChannels; ++ch)
        {
            c->cd(ch + 1);
            hWaveforms[module][ch]->Draw("COLZ");
        }
    }
}