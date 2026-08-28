#include "../include/linearity.h"
#include "../include/tr.h"

#include "../include/timestamp.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <TString.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TSpectrum.h>
#include <TVirtualFitter.h>
#include <TF1.h>
#include <TGraph.h>
#include <TF1.h>

timestamp::timestamp(int total_channels)
    : fTotalChannels(total_channels)
{
    Initialize_Hist();
    fGraph = new TGraph();
    fGraph->SetName("gr_channels");
}

timestamp::~timestamp()
{
    for (auto h : h_freq)
        delete h;
    for (auto h : h_ch_dif)
        delete h;
    if (fGraph)
    {
        delete fGraph;
        fGraph = nullptr;
    }
}
void timestamp::Initialize_Hist()
{
    h_freq.resize(fTotalChannels, nullptr);
    h_ch_dif.resize(fTotalChannels, nullptr);

    for (int j = 0; j < fTotalChannels; ++j)
    {
        TString name = Form("h_freq_ch%d", j);
        TString title = Form("Channel %d Freq ;Frequency [Hz];Counts", j);
        h_freq[j] = new TH1F(name, title, 256, 900, 1100);
        h_freq[j]->SetDirectory(nullptr);

        TString name2 = Form("h_ch_dif_ch%d", j);
        TString title2 = Form("Channel %d  difference (ch_0 reff);t_{0} -  t[clks];Counts", j);
        h_ch_dif[j] = new TH1F(name2, title2, 100, -10, 100);
        h_ch_dif[j]->SetDirectory(nullptr);
    }
}

void timestamp::ProcessTree(tr *eventReader)
{
    std::cout << "Process timestamp from reader" << std::endl;
    // Safety check 1: null pointer or empty chain
    if (!eventReader || !eventReader->fChain)
    {
        std::cerr << "[Error] Null pointer provided for eventReader or chain!\n";
        return;
    }

    Long64_t nEntries = eventReader->fChain->GetEntries();
    if (nEntries <= 0 || fTotalChannels <= 0)
    {
        std::cerr << "[Warning] No entries or invalid channel count.\n";
        return;
    }
    Long64_t reference = -1;
    std::vector<Long64_t> prev_timestamp(fTotalChannels);
    const Long64_t WINDOW = 100; // ns
    std::vector<std::vector<double>> sums(2, std::vector<double>(fTotalChannels, 0.0));
    std::vector<double> results(fTotalChannels, 0.0);
    results.clear();

    std::vector<std::vector<double>> mean(2, std::vector<double>(fTotalChannels, 0.0));
    std::vector<std::vector<int>> counter(2, std::vector<int>(fTotalChannels, 0)); // int entriesss = 1000;
    std::vector<Long64_t> ref_timestamps;

    for (Long64_t i = 0; i < eventReader->fChain->GetEntries() && i < 500; ++i)
    {
        eventReader->fChain->GetEntry(i);
        if (eventReader->module == 0 && eventReader->channel == 0)
        {
            ref_timestamps.push_back(eventReader->timestamp);
            // std::cout << eventReader->timestamp << std::endl;
        }
    }
    for (int i = 0; i < eventReader->fChain->GetEntries() && i < 500; i++)
    {
        eventReader->fChain->GetEntry(i);
        Int_t current_channel = eventReader->channel;
        Long64_t current_ts = eventReader->timestamp;
        Int_t current_module = eventReader->module;

        Long64_t closest_ref = -1;
        if (current_module < 0 || current_module >= static_cast<int>(sums.size()) ||
            current_channel < 0 || current_channel >= static_cast<int>(sums[current_module].size()))
        {
            // Ignores channels that exceed your allocated buffer size
            continue;
        }
        auto it = std::lower_bound(ref_timestamps.begin(), ref_timestamps.end(), current_ts - WINDOW);
        if (it != ref_timestamps.end() && *it <= current_ts + WINDOW)
        {
            closest_ref = *it;
        }
        if (closest_ref == -1)
        {
            continue; // Moves straight to the next event/entry in the loop
        }

        double ch_difference = current_ts - closest_ref;
        if (closest_ref != -1)
        {
            sums.at(current_module).at(current_channel) += ch_difference;
            counter.at(current_module).at(current_channel)++;

            std::cout << "[Hit Match] Entry: " << i
                      << " | Mod: " << current_module
                      << " | Ch: " << current_channel
                      << " | TS: " << current_ts
                      << " | Ref TS: " << closest_ref
                      << " | Diff: " << ch_difference
                      << " | Running Sum: " << sums.at(current_module).at(current_channel)
                      << " | Running Count: " << counter.at(current_module).at(current_channel)
                      << '\n';
        }
        else
        {
            // Optional: Print when a hit fails to find a coincidence reference
            std::cout << "[No Match] Mod: " << current_module << " | Ch: " << current_channel << " | TS: " << current_ts << '\n';
        }
        // std::cout << "Entry: " << i
        //           << " | Canal: " << current_channel
        //           << " | Current TS: " << current_ts
        //           << " | Current Module: " << current_module
        //           << " | Ref TS (Ch0): " << closest_ref
        //           << " | Diferenta fata de Ch0: " << ch_difference << std::endl;
        // //  h_ch_dif[j]->Fill(ch_difference);

        Long64_t delta_prev = current_ts - prev_timestamp[current_channel];

        // std::cout << "Entry: " << i
        //           << " | Channel: " << current_channel
        //           << " | Current TS: " << current_ts
        //           << " | Prev TS: " << prev_timestamp[j]
        //           << " | Frequency (Hz): " << freq << std::endl;

        // h_freq[j]->Fill(freq);

        prev_timestamp[current_channel] = current_ts;

        //   mean[current_module][current_channel] = sums[current_module][current_channel] / counter[current_module][current_channel];
    }
    for (int mod = 0; mod < 2; mod++)
    {
        for (int i = 0; i < fTotalChannels; i++)
        {
            mean[mod][i] = sums[mod][i] / counter[mod][i];
            std::cout << "[Module " << mod << ", Channel " << i << "] "
                      << "Sum: " << sums[mod][i] << " / "
                      << "Count: " << counter[1][i] << " = "
                      << "Mean: " << mean[mod][i] << '\n';
        }
    }
    TCanvas *c1 = new TCanvas("c_means", "Mean vs Channel", 1200, 500);
    c1->Divide(2, 1);

    Color_t colors[2] = {kAzure + 2, kOrange + 7};

    for (int mod = 0; mod < 2; mod++)
    {
        c1->cd(mod + 1);
        gPad->SetGrid();

        TGraph *gr = new TGraph(fTotalChannels);
        gr->SetName(Form("gr_mean_mod%d", mod));
        gr->SetTitle(Form("Module %d Mean vs Channel;Channel ID;Mean Value", mod));

        for (int i = 0; i < fTotalChannels; i++)
        {
            gr->SetPoint(i, i, mean[mod][i]);
        }

        gr->SetMarkerStyle(20);
        gr->SetMarkerSize(0.9);
        gr->SetMarkerColor(colors[mod]);
        gr->SetLineColor(colors[mod]);
        gr->SetLineWidth(2);

        gr->Draw("APL");
    }

    c1->Update();
}

void timestamp::DrawHistograms()
{
    TCanvas *c1 = new TCanvas("c_freq", "Frequencies", 1200, 600);

    int nCols = std::ceil(std::sqrt(fTotalChannels));
    int nRows = std::ceil(static_cast<double>(fTotalChannels) / nCols);
    c1->Divide(nCols, nRows);
    for (int j = 0; j < fTotalChannels; ++j)
    {
        c1->cd(j + 1);
        gPad->SetLogy();
        if (h_freq[j])
            h_freq[j]->Draw();
    }
    c1->Update();

    TCanvas *c2 = new TCanvas("c_diff", "Differences to Ch0", 1200, 600);
    c2->Divide(nCols, nRows);
    for (int j = 0; j < fTotalChannels; ++j)
    {
        c2->cd(j + 1);
        gPad->SetLogy();
        if (h_ch_dif[j])
            h_ch_dif[j]->Draw();
    }
    c2->Update();
    TCanvas *c3 = new TCanvas("c3", "Analysis Results", 800, 600);
    fGraph->SetMarkerStyle(20);
    fGraph->SetMarkerSize(1.0);
    fGraph->SetTitle("Difference");
    fGraph->GetXaxis()->SetTitle("Difference from #Delta t_{0}");
    fGraph->GetYaxis()->SetTitle("Average Value");

    fGraph->SetMarkerColor(kBlue + 1);
    fGraph->Draw("APL");
}