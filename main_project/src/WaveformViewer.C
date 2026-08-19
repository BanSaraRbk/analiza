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

        if (ch >= 0 && ch < kNumChannels && eventReader->analog_probe1 != nullptr)
        {
            for (size_t sample = 0; sample < eventReader->analog_probe1->size(); ++sample)
            {
                hWaveforms[ch]->Fill(sample, (*eventReader->analog_probe1)[sample]);
            }
        }
    }

    Draw();
}

void WaveformViewer::Draw()
{
    int nCols = std::ceil(std::sqrt(kNumChannels));
    int nRows = std::ceil(static_cast<double>(kNumChannels) / nCols);

    TCanvas *c3 = new TCanvas("c3", "Waveforms Canvas", 1200, 800);
    c3->Divide(nCols, nRows);

    for (int ch = 0; ch < kNumChannels; ++ch)
    {
        c3->cd(ch + 1);
        hWaveforms[ch]->Draw("COLZ");
    }
}
