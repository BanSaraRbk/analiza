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

    std::vector<TH1F *> h_freq(total_channels, nullptr);
    std::vector<TH1F *> h_ch_dif(total_channels, nullptr);

    for (int mod = 0; mod < N_MODULES; ++mod)
    {
        for (int ch = 0; ch < CHANNELS_PER_MODULE; ++ch)
        {
            int global_ch = mod * CHANNELS_PER_MODULE + ch;

            TString name_freq = Form("h_freq_m%d_ch%d", mod, ch);
            TString title_freq = Form("Mod %d Ch %d #Delta t;#Delta t [ns];Counts", mod, ch);
            h_freq[global_ch] = new TH1F(name_freq, title_freq, 10000, 900000, 1100000);

            TString name_diff = Form("h_ch_dif_m%d_ch%d", mod, ch);
            TString title_diff = Form("Mod %d Ch %d Difference;#Delta TS wrt Mod0-Ch0 [ns];Counts", mod, ch);
            h_ch_dif[global_ch] = new TH1F(name_diff, title_diff, 2000, -1000, 1000);
        }
    }

    std::vector<Long64_t> ref_timestamps;

    for (Long64_t i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        if (module == 1 && channel == 0)
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

        if (prev_timestamp[current_channel] > 0)
        {
            double another_difference = prev_timestamp[current_channel] - current_ts;
            std::cout << "Entry: " << i
                      << " | Channel: " << current_channel
                      << " | Current TS: " << current_ts
                      << " | Prev TS: " << prev_timestamp[current_channel]
                      << " | Frequency (Hz): " << another_difference << std::endl;
        }
        prev_timestamp[current_channel] = current_ts;
    }
}