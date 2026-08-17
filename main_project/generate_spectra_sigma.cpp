#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <TH1F.h>
#include <TCanvas.h>
#include <TApplication.h>

void generate_spectra_sigma()
{
    std::ofstream file("output_SIGMA.csv");
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        return;
    }

    const int kTotalEntries = 16384;
    const int kInterval = 500;
    const double kSigma = 4.0; // Requested sigma = 4
    const double kTwoSigmaSq = 2.0 * kSigma * kSigma;
    const int kWindowWidth = static_cast<int>(4.0 * kSigma); // Compute within +/- 4*sigma

    TCanvas *c1 = new TCanvas("c1", "5-Peak Pattern with Sigma=4", 1200, 600);
    c1->SetGrid();

    TH1F *h1 = new TH1F("h1", "5-Peak Pattern (#sigma = 4);Sample Index;Amplitude",
                        kTotalEntries, 0, kTotalEntries);

    // Buffer to hold continuous spectrum values
    std::vector<double> spectrum(kTotalEntries, 0.0);

    double pulse_value = 1000.0;
    const double kStep = 100.0;

    // 1. Generate Gaussian peaks at every interval
    for (int center = 0; center < kTotalEntries; center += kInterval)
    {
        // Increase amplitude every 5 peaks
        if (center > 0 && (center % (kInterval * 1) == 0))
        {
            pulse_value += kStep;
        }

        // Apply Gaussian profile around the peak center
        int start = std::max(0, center - kWindowWidth);
        int end = std::min(kTotalEntries - 1, center + kWindowWidth);

        for (int i = start; i <= end; ++i)
        {
            double diff = i - center;
            double gaussian_val = pulse_value * std::exp(-(diff * diff) / kTwoSigmaSq);
            spectrum[i] += gaussian_val;
        }
    }

    // 2. Write to CSV and fill the ROOT histogram
    for (int i = 0; i < kTotalEntries; ++i)
    {
        file << spectrum[i] << "\n";
        h1->SetBinContent(i + 1, spectrum[i]);
    }

    file.close();

    // 3. Style and draw
    h1->SetLineColor(kBlue + 1);
    h1->SetLineWidth(2);
    h1->SetStats(0);
    h1->Draw("HIST");

    c1->SaveAs("histogram.png");

    std::cout << "Successfully generated 'output.csv' and saved 'histogram.png' with sigma = "
              << kSigma << "." << std::endl;
}