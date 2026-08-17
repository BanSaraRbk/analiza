#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>

void intelegere_bini()
{
    TFile *file_root = TFile::Open("20260814_test_multiple_peaks_4sigma.root");
    if (!file_root || file_root->IsZombie())
        return;

    TTree *tree = (TTree *)file_root->Get("events");
    if (!tree)
    {
        file_root->Close();
        return;
    }

    Int_t energy = 0;
    tree->SetBranchAddress("energy", &energy);

    TH1F *hist1 = new TH1F("hist1", "CSV Reference", 10000, 0, 16384);
    TH1F *hist2 = new TH1F("hist2", "ROOT Tree Energy", 5000, 0, 32768);

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; ++i)
    {
        tree->GetEntry(i);
        // hist2->SetBinContent(i + 1, energy);

        hist2->Fill(energy);
    }
    std::ifstream csv_file("../main_project/output_SIGMA.csv");
    double value = 0.0;
    int sample_index = 0;
    const int kTotalEntries = 16384;
    std::vector<double> referenceAmplitudes;

    while (csv_file >> value && sample_index < kTotalEntries)
    {
        hist1->SetBinContent(sample_index + 1, value);
        sample_index++;

        if (value != 0)
        {

            //   std::cout << "Sample Index: " << sample_index << ", Value: " << value << std::endl;
            referenceAmplitudes.push_back(value);
        }
    }

    csv_file.close();
    std::cout << "Sampleeee" << sample_index << std::endl;
    TCanvas *c = new TCanvas("c", "Canvas", 900, 600);
    c->Divide(2, 1);

    c->cd(1);
    hist1->SetLineColor(kBlue);
    // hist1->GetXaxis()->SetRangeUser(780, 860);
    hist1->Draw("hist");

    c->cd(2);
    hist2->SetLineColor(kRed);
    // hist2->GetXaxis()->SetRangeUser(900, 13170);

    hist2->Draw("hist");
}