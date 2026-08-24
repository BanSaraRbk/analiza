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
        TString title2 = Form("Channel %d  difference (ch_0 reff);#Delta t_{0} - #Delta t [Ticks];Counts", j);
        h_ch_dif[j] = new TH1F(name2, title2, 100, -10, 100);
        h_ch_dif[j]->SetDirectory(nullptr);
    }
}

void timestamp::ProcessTree(tr *eventReader)
{
    std::cout << "Process timestamp from reader" << std::endl;
    Long64_t reference = -1;
    std::vector<Long64_t> prev_timestamp(fTotalChannels);
    std::vector<double> sums(fTotalChannels, 0.0);
    std::vector<double> results(fTotalChannels, 0.0);
    results.clear();
    std::vector<int> counter(fTotalChannels, 0.0);
    int entriesss = 1000;
    for (int i = 0; i < eventReader->fChain->GetEntries() && i < entriesss; i++)
    {
        eventReader->fChain->GetEntry(i);
        Int_t current_channel = eventReader->channel;
        Long64_t current_ts = eventReader->timestamp;

        for (int j = 0; j < fTotalChannels; j++)
        { // channel 0 reference

            if (i > 0 && current_channel == j)
            {

                if (j == 0)
                {

                    reference = current_ts;
                    // std::cout << "Refereinta e " << reference << " Canal " << j << std::endl;
                }

                if (reference > 0)
                {
                    double ch_difference = current_ts - reference;
                    sums[j] += ch_difference;
                    counter[j]++;
                    // std::cout << "Entry: " << i
                    //           << " | Canal: " << current_channel
                    //           << " | Current TS: " << current_ts
                    //           << " | Ref TS (Ch0): " << reference
                    //           << " | Diferenta fata de Ch0: " << ch_difference << std::endl;
                    h_ch_dif[j]->Fill(ch_difference);
                }
                Long64_t delta_prev = current_ts - prev_timestamp[j];

                double freq = 1.0 / (delta_prev * 8e-9);

                // std::cout << "Entry: " << i
                //           << " | Channel: " << current_channel
                //           << " | Current TS: " << current_ts
                //           << " | Prev TS: " << prev_timestamp[j]
                //           << " | Frequency (Hz): " << freq << std::endl;

                h_freq[j]->Fill(freq);

                prev_timestamp[j] = current_ts;
            }
        }
    }
    for (int i = 0; i < fTotalChannels; i++)
    {
        double tmp = static_cast<double>(sums[i]) / counter[i];

        results.push_back(tmp);
        fGraph->SetPoint(fGraph->GetN(), i, tmp);
        // std::cout << sums[i] << " / " << counter[i] << " = " << tmp << "\n";
    }
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
