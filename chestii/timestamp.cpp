#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>

void timestamp()
{
    TFile *file_root = TFile::Open("20260713_master_16channels_1.root");
    if (!file_root || file_root->IsZombie())
        return;

    TTree *tree = (TTree *)file_root->Get("events");
    if (!tree)
    {
        file_root->Close();
        return;
    }
    Int_t kEntries = 100;
    Int_t energy = 0;
    Long64_t timestamp;
    Int_t channel;
    tree->SetBranchAddress("energy", &energy);
    tree->SetBranchAddress("timestamp", &timestamp);
    tree->SetBranchAddress("channel", &channel);

    TH1F *hist1 = new TH1F("hist1", "CSV Reference", 10000, 900, 1100);
    TH1F *hist2 = new TH1F("hist2", "ROOT Tree Energy", 100, -10, 100);

    Long64_t nEntries = tree->GetEntries();

    Int_t total_channels = 5;
    std::vector<Long64_t> prev_timestamp(total_channels);
    std::vector<TH1F *> h_freq(total_channels);
    std::vector<TH1F *> h_ch_dif(total_channels);

    for (int j = 0; j < total_channels; ++j)
    {
        TString name = Form("h_freq_ch%d", j);
        TString title = Form("Channel %d Frequency;Frequency [Hz];Counts", j);
        h_freq[j] = new TH1F(name, title, 10000, 900, 1100);

        TString name2 = Form("h_ch_dif_ch%d", j);
        TString title2 = Form("Channel %d Difference;Ch0_refference [Hz];Counts", j);
        h_ch_dif[j] = new TH1F(name2, title2, 100, -10, 100);
    }
    Long64_t reference = -1;
    for (Long64_t i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        Int_t current_channel = channel;
        Long64_t current_ts = timestamp;

        // std::cout << "Channel number" << current_channel << std::endl;

        // std::cout << "Total chanels" << total_channels << std::endl;
        for (int j = 0; j < total_channels; j++)
        { // channel 0 reference

            if (i > 0 && current_channel == j)
            {
                if (j == 0)
                {

                    reference = current_ts;
                    std::cout << "Refereinta e " << reference << " Canal " << j << std::endl;
                }

                Long64_t delta_prev = current_ts - prev_timestamp[j];
                double ch_difference = current_ts - reference;
                std::cout << "Entry: " << i
                          << " | Canal: " << current_channel
                          << " | Current TS: " << current_ts
                          << " | Ref TS (Ch0): " << reference
                          << " | Diferenta fata de Ch0: " << ch_difference << std::endl;

                double freq = 1.0 / (delta_prev * 8e-9);

                // std::cout << "Entry: " << i
                //           << " | Channel: " << current_channel
                //           << " | Current TS: " << current_ts
                //           << " | Prev TS: " << prev_timestamp[j]
                //           << " | Frequency (Hz): " << freq << std::endl;

                h_freq[j]->Fill(freq);
                h_ch_dif[j]->Fill(ch_difference);

                prev_timestamp[j] = current_ts;
            }
        }
    }
    TCanvas *c = new TCanvas("c", "Channel Frequencies", 1200, 600);
    c->Divide(total_channels, 1);

    for (int j = 0; j < total_channels; j++)
    {
        c->cd(j + 1);
        gPad->SetLogy();
        h_freq[j]->Draw();
    }
    c->Update();

    TCanvas *c2 = new TCanvas("c2", "Channel Differences", 1200, 600);
    c2->Divide(total_channels, 1);

    for (int j = 0; j < total_channels; j++)
    {
        c2->cd(j + 1);
        gPad->SetLogy();
        h_ch_dif[j]->Draw();
    }

    c2->Update();
    // c->cd(2);
    // gPad->SetLogy();
    // hist2->SetTitle("Time Difference (#Delta t);Ticks (8 ns / tick);Counts");
    // hist2->SetLineColor(kRed + 1);
    // hist2->Draw();

    // c->Update();
}