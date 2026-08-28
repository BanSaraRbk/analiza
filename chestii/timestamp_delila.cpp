#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TString.h>
#include <TPad.h>

void timestamp_delila()
{
    TFile *file_root = TFile::Open("run20056.root");
    if (!file_root || file_root->IsZombie())
    {
        std::cerr << "Error: Could not open file run20042.root" << std::endl;
        return;
    }

    TTree *tree = dynamic_cast<TTree *>(file_root->Get("tr"));
    if (!tree)
    {
        std::cerr << "Error: Tree 'tr' not found in file." << std::endl;
        file_root->Close();
        return;
    }

    UShort_t energy = 0;
    Double_t timestamp = 0.0;
    UChar_t channel = 0;
    UChar_t module = 0;

    tree->SetBranchAddress("energy", &energy);
    tree->SetBranchAddress("timestamp_ns", &timestamp);
    tree->SetBranchAddress("channel", &channel);
    tree->SetBranchAddress("module", &module);

    Long64_t nEntries = tree->GetEntries();

    const Int_t CHANNELS_PER_MODULE = 14;
    const Int_t N_MODULES = 2;
    const Int_t total_channels = N_MODULES * CHANNELS_PER_MODULE;

    TH1F *h_freq[N_MODULES][total_channels];
    TH1F *h_ch_dif[N_MODULES][total_channels];
    double peak = 1.0000e6;
    double window = 500.0;
    for (int mod = 0; mod < N_MODULES; mod++)
    {
        for (int ch = 0; ch < CHANNELS_PER_MODULE; ++ch)
        {
            h_freq[mod][ch] = new TH1F(
                Form("h_freq_m%d_ch%d", mod, ch),
                Form("Module %d Channel %d;#Delta t;Counts", mod, ch),
                200, 1000000.0, 1000040.0);

            h_ch_dif[mod][ch] = new TH1F(
                Form("h_ch_dif_m%d_ch%d", mod, ch),
                Form("Module %d Channel %d;Timestamp difference;Counts",
                     mod, ch),
                2000, 0, 200);
        }
    }
    std::vector<Long64_t> ref_timestamps;

    for (Long64_t i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        if (module == 0 && channel == 0)
        {
            ref_timestamps.push_back(timestamp);
        }
    }
    const Long64_t WINDOW = 100; // ns
    std::vector<Long64_t> prev_timestamp(total_channels);

    for (Long64_t i = 0; i < nEntries && i < 100; ++i)
    {
        tree->GetEntry(i);
        Int_t current_channel = channel;
        Long64_t current_ts = timestamp;
        Int_t current_module = module;

        Long64_t closest_ref = -1;

        auto it = std::lower_bound(ref_timestamps.begin(), ref_timestamps.end(), current_ts - WINDOW);
        if (it != ref_timestamps.end() && *it <= current_ts + WINDOW)
        {
            closest_ref = *it;
        }

        double ch_difference = (closest_ref != -1) ? (current_ts - closest_ref) : 0;
        h_ch_dif[current_module][current_channel]->Fill(ch_difference);

        if (prev_timestamp[current_channel] > 0)
        {
            double another_difference = prev_timestamp[current_channel] - current_ts;
            h_freq[current_module][current_channel]->Fill(std::abs(another_difference));
            std::cout << "Entry: " << i
                      << " | Channel: " << current_channel
                      << " | Current TS: " << current_ts
                      << "Mod" << current_module
                      << " | Prev TS: " << prev_timestamp[current_channel]
                      << "Ch difference" << ch_difference
                      << " | Frequency (Hz): " << std::abs(another_difference) << std::endl;
        }
        prev_timestamp[current_channel] = current_ts;
    }

    int nCols = std::ceil(std::sqrt(CHANNELS_PER_MODULE));
    int nRows = std::ceil(static_cast<double>(CHANNELS_PER_MODULE) / nCols);

    TCanvas *test = new TCanvas("test", "Channel test", 1200, 600);

    test->Divide(2, 1);
    test->cd(1);

    h_freq[0][0]->Draw();

    test->cd(2);
    // h_ch_dif[0][0]->Draw();

    h_ch_dif[1][2]->GetXaxis()->SetRangeUser(30, 50);
    h_ch_dif[1][2]->Draw();

    // TCanvas *c = new TCanvas("c", "Channel Frequencies", 1200, 600);
    // c->Divide(nCols, nRows);

    // TCanvas *c1 = new TCanvas("c1", "Channel Frequencies", 1200, 600);
    // c1->Divide(nCols, nRows);

    // for (int j = 0; j < CHANNELS_PER_MODULE; j++)
    // {
    //     c->cd(j + 1);
    //     gPad->SetLogy();
    //     h_freq[0][j]->Draw();

    //     c1->cd(j + 1);
    //     gPad->SetLogy();
    //     h_ch_dif[0][j]->Draw();
    // }
    // c->Update();
    // c1->Update();
    // TCanvas *c2 = new TCanvas("c2", "Channel Differences", 1200, 600);
    // c2->Divide(nCols, nRows);

    // TCanvas *c3 = new TCanvas("c3", "Channel Differences", 1200, 600);
    // c3->Divide(nCols, nRows);

    // for (int j = 0; j < CHANNELS_PER_MODULE; j++)
    // {
    //     c2->cd(j + 1);
    //     gPad->SetLogy();
    //     h_ch_dif[1][j]->Draw();
    //     c3->cd(j + 1);
    //     gPad->SetLogy();
    //     h_freq[1][j]->Draw();
    // }
    // c2->Update();
    // c3->Update();
    // c->Update();

    // TCanvas *c2 = new TCanvas("c2", "Channel Differences", 1200, 600);
    // c2->Divide(total_channels, 1);

    // for (int j = 0; j < total_channels; j++)
    // {
    //     c2->cd(j + 1);
    //     gPad->SetLogy();
    //     h_ch_dif[j]->Draw();
    // }
}
