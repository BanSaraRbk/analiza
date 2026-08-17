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
constexpr int kTargetChannel = 0;

// trapezoidal filter: accept vector input and preserve existing algorithm
std::vector<int> trapezoidal(const std::vector<int> &wave, int L, int G)
{
    std::vector<int> ret;
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
void NormalizeSignal(std::vector<int> &wave)
{
    int baseline = 0;
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

void ana10_waves_plotsoft(const char *treeName = "events")
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

    if (!tree->GetBranch("waveform") || !tree->GetBranch("energy") || !tree->GetBranch("user_info_0") ||
        !tree->GetBranch("user_info_1") || !tree->GetBranch("channel"))
    {
        std::cout << "Required branches waveform/energy/user_info_0/user_info_1/channel are missing." << std::endl;
        tree->Print();
        return;
    }

    std::cout << "Using file: " << f->GetName() << std::endl;
    std::cout << "Using tree: " << tree->GetName() << " (entries=" << tree->GetEntries() << ")" << std::endl;

    TTreeReader reader(tree);
    TTreeReaderValue<std::vector<int>> waveform(reader, "waveform");
    TTreeReaderValue<int> adcHard(reader, "energy");
    TTreeReaderValue<Long64_t> amaxHard(reader, "user_info_0");
    TTreeReaderValue<Long64_t> userInfo1(reader, "user_info_1");
    TTreeReaderValue<int> channel(reader, "channel");

    // TGraph *gr = new TGraph(samples.size(), samples.data(), waves.data());
    // gr->SetTitle("Filtered Waveform;Sample;Amplitude");
    // gr->SetLineColor(kBlue);
    TH2D *hSoftware = new TH2D(
        "hSoftware", "Amax software;ADC_channel;A_{max}^{software}",
        700, 0, 18000, 700, 0, 18000);
    TH2D *hHardware = new TH2D(
        "hHardware", "Amax hardware;ADC_channel;A_{max}^{hardware}",
        700, 0, 18000, 700, 0, 18000);
    TH1D *hAmaxGosa = new TH1D(
        "hAmaxGosa",
        "Relative difference between FPGA and offline A_{max};(A_{max}^{off}-A_{max}^{FPGA}) / A_{max}^{off};Entries",
        400, -0.9, 0.2);
    const Long64_t nEntries = reader.GetEntries();
    Long64_t index = 0;

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

        if (ch != kTargetChannel)
            continue;

        if ((*waveform).empty())
        {
            continue;
        }

        std::vector<int> waveform_tmp = *waveform;

        // down sampling
        std::vector<int> waveform_ds;
        waveform_ds.reserve((waveform_tmp.size() + 3) / 4);
        for (size_t i = 0; i < waveform_tmp.size(); i += 4)
        {
            waveform_ds.push_back(waveform_tmp[i]);
        }
        waveform_tmp.swap(waveform_ds);

        NormalizeSignal(waveform_tmp);

        std::vector<int> trapezoidal_result = trapezoidal(waveform_tmp, 32, 0);
        std::vector<int> x(waveform_tmp.size());
        for (size_t i = 0; i < x.size(); i++)
        {
            x[i] = i;
        }

        //    gr_trap = new TGraph(waveform_tmp.size(), x.data(), waveform_tmp.data());
        // Find max and min of trapezoidal_result
        int trap_max = *std::max_element(trapezoidal_result.begin(), trapezoidal_result.end());
        int trap_min = *std::min_element(trapezoidal_result.begin(), trapezoidal_result.end());
        int amax = trap_max - trap_min;

        // if (amax <= 2000)
        // {
        //     continue; // Skip if Amax is not positive
        // }

        hSoftware->Fill(static_cast<double>(*adcHard), amax);

        hAmaxGosa->Fill((amax - static_cast<double>(*adcHard)) / (amax));
    }

    for (size_t i = 0; i < nEntries; ++i)
    {
        reader.SetEntry(i);
        const int ch = *channel;
        if (ch < 0 || ch >= kNumChannels)
        {
            continue;
        }

        if (ch != kTargetChannel)
            continue;

        hHardware->Fill(static_cast<double>(*adcHard), static_cast<double>(*amaxHard));
    }
    gStyle->SetOptStat(00000000);
    TCanvas *c1 = new TCanvas("c1", "Amax Hard vs Amax Soft", 800, 600);
    // Hide statistics box when drawing
    // hAmaxHardVsAmaxSoft->SetStats(false);
    hSoftware->Draw("COL");
    // Save as PDF
    c1->SaveAs("AmaxHardVsAmaxSoft.pdf");

    TCanvas *c2 = new TCanvas("c2", "Amax Difference", 800, 600);
    // c2->SetLogy();
    // hAmaxDiff->SetStats(false);
    hHardware->Draw();
    // Save as PDF
    c2->SaveAs("AmaxDifference.pdf");

    TCanvas *c3 = new TCanvas("c3", "Amax Relative Difference", 800, 600);
    // hAmaxGosa->SetStats(false);
    hAmaxGosa->Draw();
    // Save as PDF
    c3->SaveAs("AmaxRelativeDifference.pdf");

    // c2->Update();
}
