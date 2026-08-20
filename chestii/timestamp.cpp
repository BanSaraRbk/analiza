#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>

void timestamp()
{
    TFile *file_root = TFile::Open("20260820_countrate1_timestamp.root");
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
    Long64_t prev_timestamp = 0;

    for (Long64_t i = 0; i < nEntries && i < kEntries; ++i)
    {
        tree->GetEntry(i);
        Int_t current_channel = channel;
        Long64_t current_ts = timestamp;

        tree->GetEntry(i + 1);
        Long64_t next_ts = timestamp;

        if (i > 0 && current_channel == 0)
        {
            Long64_t delta_prev = current_ts - prev_timestamp;
            if (delta_prev > 0)
            {
                double freq = 1.0 / (delta_prev * 8e-9);

                std::cout << "Entry: " << i
                          << " | Current TS: " << current_ts
                          << " | Prev TS: " << prev_timestamp
                          << " | Frequency (Hz): " << freq << std::endl;

                hist1->Fill(freq);
            }
        }

        Long64_t delta_next = next_ts - current_ts;
        // std::cout << "Entry: " << i
        //           << " | Current TS: " << current_ts
        //           << " | Next TS: " << next_ts
        //           << " | Delta: " << delta_next << std::endl;

        hist2->Fill(delta_next);

        prev_timestamp = current_ts;
    }
    TCanvas *c = new TCanvas("c", "Rate and Delta Analysis", 1200, 600);
    c->Divide(2, 1);

    c->cd(1);
    gPad->SetLogy();
    hist1->SetTitle("Calculated Rate;Rate [Hz];Counts");
    hist1->SetLineColor(kBlue + 1);
    hist1->Draw();

    c->cd(2);
    gPad->SetLogy();
    hist2->SetTitle("Time Difference (#Delta t);Ticks (8 ns / tick);Counts");
    hist2->SetLineColor(kRed + 1);
    hist2->Draw();

    c->Update();
}