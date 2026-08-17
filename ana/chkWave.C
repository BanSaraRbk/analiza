
#include <iostream>
#include <vector>
#include <numeric> // Necesar pentru std::iota
// constexpr int kNumChannels = 16;
// constexpr int kTargetChannel = 0;
void chkWave()
{
    std::cout << "Insert the desired number of channels to view (1-16): ";
    int kNumChannels;
    std::cin >> kNumChannels;
    std::cout << "Insert the board number to analyze : ";
    int kTargetModule;
    std::cin >> kTargetModule;
    TFile *fin = gFile;
    if (!fin || !fin->IsOpen())
    {
        std::cout << "No ROOT file is currently open." << std::endl;
        std::cout << "Example: TFile::Open(\"data.root\"); ana30(\"Trace_Tree\");" << std::endl;
        return;
    }

    TTree *tree = (TTree *)fin->Get("tr");
    if (!tree)
    {
        std::cout << "Tree 'tr' not found in this file" << std::endl;
        return;
    }
    unsigned char module, channel;
    Double_t timestampNs, baseline = 0.0;
    vector<short> *waveform = nullptr;

    tree->SetBranchAddress("module", &module);
    tree->SetBranchAddress("channel", &channel);
    tree->SetBranchAddress("analog_probe1", &waveform);
    Long64_t nEntries = tree->GetEntries();
    std::cout << "Total Events: " << nEntries << std::endl;
    // TMultiGraph *mg = new TMultiGraph();
    std::vector<TH2D *> h(kNumChannels);

    for (int ch = 0; ch < kNumChannels; ch++)
    {
        h[ch] = new TH2D(Form("h%d", ch),
                         "Waveform;Sample;Amplitude",
                         400, 0, 500,
                         400, 7000, 20000);
    }

    for (Long64_t entry = 0; entry < 1000 && entry < nEntries; ++entry)
    {
        tree->GetEntry(entry);

        if (!waveform || waveform->empty())
            continue;

        const int mod = static_cast<int>(module);
        ;
        if (mod != kTargetModule)
            continue;

        const int ch = static_cast<int>(channel);
        if (ch < kNumChannels)
        {
            for (size_t sample = 0; sample < waveform->size(); ++sample)
            {
                h[ch]->Fill(sample, (*waveform)[sample]);
            }
        }
    }
    int nCols = std::ceil(std::sqrt(kNumChannels));
    int nRows = std::ceil(static_cast<double>(kNumChannels) / nCols);

    TCanvas *c3 = new TCanvas("c3", "Relative Difference", 1200, 800);
    c3->Divide(nCols, nRows);

    for (int ch = 0; ch < kNumChannels; ++ch)
    {
        c3->cd(ch + 1);
        h[ch]->Draw("COLZ");
    }
}