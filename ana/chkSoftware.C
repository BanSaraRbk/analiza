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
        wave[i] = fabs(wave[i] - baseline + 10);
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

void chkSoftware(const char *treeName = "tr")
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

    // if (!tree->GetBranch("waveform") || !tree->GetBranch("energy") || !tree->GetBranch("user_info_0") ||
    //     !tree->GetBranch("user_info_1") || !tree->GetBranch("channel"))
    // {
    //     std::cout << "Required branches waveform/energy/user_info_0/user_info_1/channel are missing." << std::endl;
    //     tree->Print();
    //     return;
    // }

    std::cout << "Using file: " << f->GetName() << std::endl;
    std::cout << "Using tree: " << tree->GetName() << " (entries=" << tree->GetEntries() << ")" << std::endl;

    TTreeReader reader(tree);
    TTreeReaderValue<std::vector<short>> waveform(reader, "AnalogProbe1");
    TTreeReaderValue<unsigned short> adcHard(reader, "Energy");
    TTreeReaderValue<unsigned long long> amaxHard(reader, "UserInfo0");
    TTreeReaderValue<unsigned long long> userInfo1(reader, "UserInfo1");
    TTreeReaderValue<unsigned char> channel(reader, "Channel");

    // TGraph *gr = new TGraph(samples.size(), samples.data(), waves.data());
    // gr->SetTitle("Filtered Waveform;Sample;Amplitude");
    // gr->SetLineColor(kBlue);
    TH2D *hAmax_Software = new TH2D(
        "hAmax_Software", "A_{max} Software;ADC channels;A_{max}^{soft}",
        800, 0, 18000, 4800, 0, 18000);
    //  TGraph *gr_trap = nullptr;

    TH2D *hAmax_Hardware = new TH2D(
        "hAmax_Hardware", "A_{max} Hardware;ADC channels;A_{max}^{hard}",
        800, 0, 18000, 4800, 0, 18000);
    TH1D *hAmaxDiff = new TH1D(
        "hAmaxDiff", "A_{max} Difference (Soft - Hard);A_{max}^{off} - A_{max}^{FPGA};Entries",
        400, -24000, 24000);
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

        hAmax_Software->Fill(static_cast<double>(*adcHard), amax);
        hAmax_Hardware->Fill(static_cast<double>(*adcHard), static_cast<double>(*amaxHard));
        // hAmaxDiff->Fill(amax - static_cast<double>(*amaxHard));
        hAmaxGosa->Fill((amax - static_cast<double>(*amaxHard)) / (amax));
    }
    gStyle->SetOptStat(00000000);
    TCanvas *c1 = new TCanvas("c1", "Amax Soft", 800, 600);
    // Hide statistics box when drawing
    // hAmaxHardVsAmaxSoft->SetStats(false);
    hAmax_Software->Draw("COL");
    // Save as PDF
    c1->SaveAs("AmaxHardVsAmaxSoft.pdf");

    TCanvas *c2 = new TCanvas("c2", "Amax Hardware", 800, 600);

    // hAmaxDiff->SetStats(false);
    hAmax_Hardware->Draw();
    // Save as PDF
    c2->SaveAs("AmaxDifference.pdf");

    TCanvas *c3 = new TCanvas("c3", "Amax Relative Difference", 800, 600);
    // hAmaxGosa->SetStats(false);
    hAmaxGosa->Draw();
    // Save as PDF
    c3->SaveAs("AmaxRelativeDifference.pdf");

    // c2->Update();
}
