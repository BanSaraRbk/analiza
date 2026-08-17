#include "../include/linearity.h"
#include "../include/tr.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <TString.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TSpectrum.h>
#include <TVirtualFitter.h>
#include <TF1.h>
LinearityAnalyzer::LinearityAnalyzer()
{
}

LinearityAnalyzer::~LinearityAnalyzer()
{
}

TH1F *LinearityAnalyzer::ReadRefferenceSpectrum(const std::string &filename)
{
    static int histCount = 0; // Increments every time the function is called
    histCount++;
    std::cout << "Reading reference spectrum from: " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open reference spectrum file: " << filename << std::endl;
        return nullptr;
    }

    const int kTotalEntries = 16384;

    h1 = new TH1F("hRefSpectrum", "Reference Spectrum;Sample Index;Amplitude",
                  kTotalEntries, 0, kTotalEntries);

    double value = 0.0;
    int sample_index = 0;
    // h1->Scale(1.0 / h1->GetEntries()); // Normalize the histogram

    while (file >> value && sample_index < kTotalEntries)
    {
        h1->SetBinContent(sample_index + 1, value);
        sample_index++;

        if (value != 0)
        {

            //   std::cout << "Sample Index: " << sample_index << ", Value: " << value << std::endl;
            referenceAmplitudes.push_back(value);
        }
    }
    file.close();
    std::cout << "Loaded " << sample_index << " samples into reference spectrum histogram." << std::endl;

    return h1;
}

TH1F *LinearityAnalyzer::Process(tr *eventReader)
{
    // Implement the processing logic for the eventReader
    // This function will analyze the data and fill the necessary histograms
    std::cout << "Processing data from event reader..." << std::endl;
    // Example: Fill h1 with processed data
    // h1->Fill(...);
    const int nBins = 8192;
    const double minEnergy = 0.0;
    const double maxEnergy = 32768.0;

    h2 = new TH1F("mySpectrum", "Energy Spectrum;Energy [channels];Counts",
                  nBins, minEnergy, maxEnergy);

    // h2->Scale(1.0 / h2->GetEntries()); // Normalize the histogram

    for (int i = 0; i < eventReader->fChain->GetEntries(); i++)
    {
        eventReader->fChain->GetEntry(i);
        h2->Fill(eventReader->energy);

        // Process each entry and fill h1 accordingly
        // Example: h1->Fill(eventReader->someValue);
    }

    Draw(h2);
    return h2;
}

std::vector<std::pair<int, int>> LinearityAnalyzer::FindPeaks(TH1F *h)
{
    if (!h)
    {
        std::cerr << "Error: Spectrum histogram is not initialized." << std::endl;
        return {};
    }

    std::cout << "Finding peaks in spectrum: " << h->GetName() << std::endl;

    TSpectrum s;
    int nPeaks = s.Search(h, 1, "", 0.1);
    std::cout << "Found " << nPeaks << " peaks." << std::endl;

    std::vector<std::pair<int, int>> detectedPeaks;
    detectedPeaks.reserve(nPeaks);

    for (int i = 0; i < nPeaks; i++)
    {
        int peakX = s.GetPositionX()[i];
        int peakBin = h->GetXaxis()->FindBin(peakX);
        int peakAmplitude = h->GetBinContent(peakBin);

        detectedPeaks.emplace_back(peakX, peakAmplitude);
    }

    std::sort(detectedPeaks.begin(), detectedPeaks.end(), [](const auto &left, const auto &right)
              { return left.first < right.first; });

    return detectedPeaks;
}
std::pair<std::vector<double>, std::vector<double>> LinearityAnalyzer::FitAllPeaks(TH1F *h, const std::vector<std::pair<int, int>> &peaks)
{
    std::vector<double> peak_means;
    std::vector<double> energy_resolution;
    for (size_t i = 0; i < 32; ++i)
    {
        int xPeak = peaks[i].first;
        int yPeak = peaks[i].second;

        double delta = 30.0;
        double xMin = xPeak - delta;
        double xMax = xPeak + delta;

        TString funcName = Form("gaus_%zu", i);
        TF1 *fit = new TF1(funcName, "gaus", xMin, xMax);

        fit->SetParameters(yPeak, xPeak, 1.0);

        h->Fit(fit, "R+Q");

        double mean_value = fit->GetParameter(1);
        peak_means.push_back(mean_value);

        energy_resolution.push_back((fit->GetParameter(2) * 2.355) / mean_value); // FWHM = 2.355 * Sigma

        // std::cout << "Peak " << i + 1 << " -> Mean: " << fit->GetParameter(1)
        //           << ", Sigma: " << fit->GetParameter(2)

        //           << std::endl;
    }

    return std::make_pair(peak_means, energy_resolution);
}
void LinearityAnalyzer::Draw(TH1F *h)
{
    TCanvas *c = new TCanvas(Form("c_%s", h->GetName()), h->GetTitle(), 800, 600);

    h->SetLineWidth(2);

    h->Draw("HIST");

    TSpectrum *s = new TSpectrum();
    s->Search(h, 1, "", 0.05);
}