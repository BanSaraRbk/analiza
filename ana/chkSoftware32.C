#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TIterator.h"
#include "TKey.h"
#include "TROOT.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

// #include "../common/TWaveformAnalysis.hpp"

// Amax culc like FPGA
#include <cmath>

constexpr int kNumChannels = 16;

// trapezoidal filter: accept vector input and preserve existing algorithm
std::vector<short> trapezoidal(const std::vector<short> &wave, int L, int G)
{
    std::vector<short> ret;
    int length = static_cast<int>(wave.size());
    for (int j = 0, n = length - (2 * L + G); j < n; j++)
    {
        int tmp = 0;
        for (int k = 0; k < L; k++)
        {
            tmp += static_cast<int>(wave[j + k] * (-1.0 / static_cast<double>(L)));
        }
        for (int k = 0; k < G; k++)
        {
            tmp += static_cast<int>(wave[j + k + G] * 0);
        }
        for (int k = 0; k < L; k++)
        {
            tmp += static_cast<int>(wave[j + k + G + L] * (1.0 / static_cast<double>(L)));
        }
        ret.push_back(tmp);
    }
    return ret;
}

// Normalize waveform so that the maximum value is scaled to 2^14
void NormalizeSignal(std::vector<short> &wave)
{
    short baseline = 0;
    for (size_t i = 0; i < 10; ++i)
    {
        baseline += wave[i] / 10;
    }

    for (int i = 0; i < wave.size(); i++)
    {
        wave[i] = fabs(wave[i] - baseline + 30);
    }

    if (wave.empty())
        return;
    int max_value = *std::max_element(wave.begin(), wave.end());

    // std::cout << max_value << std::endl;
    if (max_value == 0)
        return;
    for (size_t i = 0; i < wave.size(); ++i)
    {
        // v *= std::pow(2, 13);
        int v = (wave[i] * 8192) / max_value; // 2^13 = 819
        wave[i] = v;
    }
}

void chkSoftware32(const char *treeName = "tr", int kTargetModule = 0)
{
    TFile *f = gFile;

    if (!f || !f->IsOpen())
    {
        TIter nextFile(gROOT->GetListOfFiles());
        f = dynamic_cast<TFile *>(nextFile());
    }

    if (!f || !f->IsOpen())
    {
        std::cout << "No ROOT file is currently open." << std::endl;
        std::cout << "Example: TFile::Open(\"data.root\"); ana6(\"events\");" << std::endl;
        return;
    }

    TTree *tree = nullptr;
    f->GetObject(treeName, tree);
    if (!tree)
    {
        std::cout << "TTree \"" << treeName << "\" not found in file: " << f->GetName() << std::endl;
        std::cout << "Available TTrees:" << std::endl;
        TIter nextKey(f->GetListOfKeys());
        while (TKey *key = static_cast<TKey *>(nextKey()))
        {
            TString className = key->GetClassName();
            if (className == "TTree")
            {
                std::cout << "  - " << key->GetName() << std::endl;
            }
        }
        return;
    }

    std::cout << "Using file: " << f->GetName() << std::endl;
    std::cout << "Using tree: " << tree->GetName() << " (entries=" << tree->GetEntries() << ")" << std::endl;

    TTreeReader reader(tree);
    TTreeReaderValue<std::vector<short>> waveform(reader, "AnalogProbe1");
    TTreeReaderValue<unsigned short> adcHard(reader, "Energy");
    TTreeReaderValue<unsigned long long> amaxHard(reader, "UserInfo0");
    TTreeReaderValue<unsigned long long> userInfo1(reader, "UserInfo1");
    TTreeReaderValue<unsigned char> channel(reader, "Channel");
    TTreeReaderValue<unsigned char> module(reader, "Module");

    // TGraph *gr = new TGraph(samples.size(), samples.data(), waves.data());
    // gr->SetTitle("Filtered Waveform;Sample;Amplitude");
    // gr->SetLineColor(kBlue);
    std::vector<TH2D *> hSoftware(kNumChannels);
    std::vector<TH2D *> hHardware(kNumChannels);
    std::vector<TH1D *> hAmaxGosa(kNumChannels);
    for (int ch = 0; ch < kNumChannels; ++ch)
    {

        hSoftware[ch] = new TH2D(
            Form("hSoftware_ch%d", ch),
            Form("Software A_{max} - Channel %d;ADC_channel;A_{max}^{software}", ch),
            700, 0, 6000,
            700, 6000, 7500);

        hHardware[ch] = new TH2D(
            Form("hHardware_ch%d", ch),
            Form("Hardware A_{max} - Channel %d;ADC_channel;A_{max}^{hardware}", ch),
            700, 0, 6000,
            700, 6000, 7500);

        hAmaxGosa[ch] = new TH1D(
            Form("hAmaxGosa_ch%d", ch),
            Form("Relative difference - Channel %d;(A_{max}^{off}-A_{max}^{FPGA}) / A_{max}^{off};Entries", ch),
            400, -0.9, 0.2);
    }
    const Long64_t nEntries = reader.GetEntries();
    Long64_t index = 0;
    int iteration = 0;

    while (reader.Next())
    {
        ++index;

        if (nEntries >= 10 && index % (nEntries / 10) == 0)
        {
            std::cout << "Processing entry " << index << " / " << nEntries << " (" << (index * 100.0 / nEntries) << "%)" << std::endl;
        }

        const int ch = *channel;
        if (ch < 0 || ch >= kNumChannels)
        {
            continue;
        }

        const int mod = *module;
        if (mod != kTargetModule)
            continue;

        // if (ch != kTargetChannel)
        //     continue;

        const unsigned short adc_value = *adcHard;
        const unsigned long long amax_value = *amaxHard;
        for (iteration = 0; iteration < kNumChannels; ++iteration)
        {

            if (ch == iteration)
            {
                hHardware[ch]->Fill(static_cast<double>(adc_value), static_cast<double>(amax_value));
                // std::cout << "Channel: " << ch << ", Module: " << mod << ", ADC: " << adc_value << ", Amax: " << amax_value << std::endl;

                if ((*waveform).empty())
                {
                    continue;
                }
                std::vector<short> waveform_tmp = *waveform;

                // down sampling
                std::vector<short> waveform_ds;
                waveform_ds.reserve((waveform_tmp.size() + 3) / 4);
                for (size_t i = 0; i < waveform_tmp.size(); i += 4)
                {
                    waveform_ds.push_back(waveform_tmp[i]);
                }
                waveform_tmp.swap(waveform_ds);

                NormalizeSignal(waveform_tmp);

                std::vector<short> trapezoidal_result = trapezoidal(waveform_tmp, 32, 0);
                std::vector<short> x(waveform_tmp.size());
                for (size_t i = 0; i < x.size(); i++)
                {
                    x[i] = i;
                }

                // gr_trap = new TGraph(waveform_tmp.size(), x.data(), waveform_tmp.data());
                // Find max and min of trapezoidal_result
                short trap_max = *std::max_element(trapezoidal_result.begin(), trapezoidal_result.end());
                short trap_min = *std::min_element(trapezoidal_result.begin(), trapezoidal_result.end());
                short amax = trap_max - trap_min;

                // if (amax <= 2000)
                // {
                //     continue; // Skip if Amax is not positive
                // }

                hSoftware[ch]->Fill(static_cast<double>(adc_value), amax);
                // hAmaxDiff->Fill(amax - static_cast<double>(amax_value));
                hAmaxGosa[ch]->Fill((amax - static_cast<double>(amax_value)) / (amax));
            }
        }
    }
    gStyle->SetOptStat(00000000);

    int nCols = std::ceil(std::sqrt(kNumChannels));
    int nRows = std::ceil(static_cast<double>(kNumChannels) / nCols);

    TCanvas *c1 = new TCanvas("c1", "Software", 1200, 800);
    c1->Divide(nCols, nRows);

    for (int ch = 0; ch < kNumChannels; ++ch)
    {
        c1->cd(ch + 1);
        hSoftware[ch]->Draw("COLZ");
    }
    TCanvas *c2 = new TCanvas("c2", "Hardware", 1200, 800);
    c2->Divide(nCols, nRows);

    for (int ch = 0; ch < kNumChannels; ++ch)
    {
        c2->cd(ch + 1);
        hHardware[ch]->Draw("COLZ");
    }
    TCanvas *c3 = new TCanvas("c3", "Relative Difference", 1200, 800);
    c3->Divide(nCols, nRows);

    for (int ch = 0; ch < kNumChannels; ++ch)
    {
        c3->cd(ch + 1);
        hAmaxGosa[ch]->Draw();
    }
    // c2->Update();
}
