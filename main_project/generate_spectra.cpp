#include <iostream>
#include <fstream>
#include <TH1F.h>
#include <TCanvas.h>
#include <TApplication.h>
void generate_spectra()
{

    std::ofstream file("output_same_amplitude.csv");
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file for writing." << std::endl;
    }

    const int kTotalEntries = 16384;
    const int kInterval = 500;

    TCanvas *c1 = new TCanvas("c1", "5-Peak Pattern", 1200, 600);
    c1->SetGrid();

    TH1F *h1 = new TH1F("h1", "5-Peak Pattern;Sample Index;Amplitude", kTotalEntries, 0, kTotalEntries);

    int pulse_value = 65535;
    const int kStep = 100;

    for (int i = 0; i < kTotalEntries; ++i)
    {
        int current_val = 0;

        if (i % kInterval == 0)
        {
            current_val = pulse_value;
        }
        // if (i % (kInterval * 5) == 0)
        // {
        //     pulse_value += kStep;
        // }

        file << current_val << "\n";

        h1->SetBinContent(i + 1, current_val);
    }

    file.close();

    h1->SetLineColor(kBlue + 1);
    h1->SetLineWidth(2);
    h1->SetStats(0); // Hide stats box
    h1->Draw("HIST");

    c1->SaveAs("histogram.png");

    std::cout << "Successfully generated 'output.csv' and saved 'histogram.png'." << std::endl;
}