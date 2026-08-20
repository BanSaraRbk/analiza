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
#include <TGraph.h>
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

TH1F *LinearityAnalyzer::Process(tr *eventReader, int No_Channels)
{
    // Implement the processing logic for the eventReader
    // This function will analyze the data and fill the necessary histograms
    std::cout << "Processing data from event reader..." << std::endl;
    // Example: Fill h1 with processed data
    // h1->Fill(...);
    const int nBins = 8000;
    const double minEnergy = 0.0;
    const double maxEnergy = 32768.0;
    TString h_name = Form("h_spectrum_ch%d", No_Channels);
    TString h_title = Form("Energy Spectrum - Channel %d;ADC Channels;Counts", No_Channels);

    h2 = new TH1F(h_name, h_title,
                  nBins, minEnergy, maxEnergy);

    // h2->Scale(1.0 / h2->GetEntries()); // Normalize the histogram

    for (int i = 0; i < eventReader->fChain->GetEntries(); i++)
    {
        eventReader->fChain->GetEntry(i);

        if (eventReader->channel == No_Channels)
        {
            h2->Fill(eventReader->energy);
        }

        // Process each entry and fill h1 accordingly
        // Example: h1->Fill(eventReader->someValue);
    }

    Draw(h2, No_Channels);
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
    int nPeaks = s.Search(h, 4, "", 0.1);
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

        double delta = 70.0;
        double xMin = xPeak - delta;
        double xMax = xPeak + delta;

        TString funcName = Form("gaus_%zu", i);
        TF1 *fit = new TF1(funcName, "gaus", xMin, xMax);

        fit->SetParameters(yPeak, xPeak, 1.0);

        h->Fit(fit, "R+Q");

        double mean_value = fit->GetParameter(1);
        peak_means.push_back(mean_value);

        energy_resolution.push_back(((fit->GetParameter(2) * 2.355) / mean_value)); // FWHM = 2.355 * Sigma

        std::cout << "Peak " << i + 1 << " -> Mean: " << fit->GetParameter(1)
                  << ", Sigma: " << fit->GetParameter(2)

                  << std::endl;
    }

    return std::make_pair(peak_means, energy_resolution);
}
void LinearityAnalyzer::Draw(TH1F *h, int No_channels)

{

    if (!h)
        return;

    TString canvas_name = Form("c_ch%d_%s", No_channels, h->GetName());
    TString canvas_title = Form("Channel %d - %s", No_channels, h->GetTitle());

    TCanvas *c = new TCanvas(canvas_name, canvas_title, 800, 600);
    c->cd();

    h->SetLineWidth(2);
    h->Draw("HIST");

    TSpectrum *s = new TSpectrum();
    s->Search(h, 1, "", 0.05);
}

void LinearityAnalyzer::compute_R_2(int N, double a, double b, std::vector<double> fitted_means, std::vector<double> reff_means)
{
    std::cout << "Valoarea lui N este " << N << std::endl;

    double sum_y;
    for (size_t i = 0; i < N; i++)
    {
        sum_y += fitted_means[i];
    }
    double y_mean = sum_y / N;
    double SS_res = 0.0;
    double SS_tot = 0.0;

    for (size_t j = 0; j < N; ++j)
    {
        double y_real = fitted_means[j];
        double x_real = reff_means[j];

        double y_compute = a * x_real + b;

        // std::cout << y_compute << std::endl;

        SS_res = SS_res + std::pow(y_real - y_compute, 2);
        SS_tot = SS_tot + std::pow(y_real - y_mean, 2);
    }

    double R2 = (SS_tot != 0.0) ? (1.0 - (SS_res / SS_tot)) : 0.0;
    std::cout << "\n========== REZULTATE FIT & STATISTICA ==========\n";
    //  std::cout << "Numar de puncte (N): " << N << "\n";
    std::cout << "Parametru p0 (Offset): " << b << "\n";
    std::cout << "Parametru p1 (Panta) : " << a << "\n";
    std::cout << "Media y (y_mean)     : " << y_mean << "\n";
    std::cout << "SS_res               : " << SS_res << "\n";
    std::cout << "SS_tot               : " << SS_tot << "\n";
    std::cout << "R^2 (Determinare)    : " << R2 << "\n";
    std::cout << "================================================\n";
}

// void LinearityAnalyzer::ShowGraph(std::vector<double> x, std::vector<double> y)
// {
//     int N = std::min(x.size(), y.zie());

//     TCanvas *c = new TCanvas(Form("c_%s", h->GetName()), h->GetTitle(), 800, 600);

//     TGraph *gr = new TGraph();

//     for(size_t i = 0;i<N;i++)

//     {
//         double x_axis=x[i];
//         double y_axis=y[i];

//         gr->SetPoint(i,i+1)

//     }
// }
void LinearityAnalyzer::process_linearity(const std::string &csv_file, tr *t, int N_channels)
{

    LinearityAnalyzer linearityAnalyzer;
    TH1F *h_refference = linearityAnalyzer.ReadRefferenceSpectrum(csv_file);

    auto Reff_Spectrum = linearityAnalyzer.FindPeaks(h_refference);

    std::vector<int> peak_position;
    std::vector<int> peak_amplitude;

    peak_position.reserve(Reff_Spectrum.size());
    peak_amplitude.reserve(Reff_Spectrum.size());

    for (const auto &[pos, amp] : Reff_Spectrum)
    {
        peak_position.push_back(pos);
        peak_amplitude.push_back(amp);
    }

    std::pair<std::vector<double>, std::vector<double>> reff_results = linearityAnalyzer.FitAllPeaks(h_refference, Reff_Spectrum);
    std::vector<double> reff_means = reff_results.first;
    std::vector<double> reff_resolution = reff_results.second;

    std::vector<TH1F *> h_energy(N_channels);
    std::vector<std::vector<std::pair<int, int>>> energy_Spectrum(N_channels);
    std::vector<std::vector<double>> fitted_means(N_channels);
    std::vector<std::vector<double>> energy_resolution(N_channels);
    std::vector<TGraph *> gr(N_channels);
    std::vector<TGraph *> gr_resolution(N_channels);

    TCanvas *c_lin = new TCanvas("c_lin", "Linearity_fit", 800, 600);

    TCanvas *c_resolution = new TCanvas("c_resolution", "Energy Resolution", 800, 600);

    int nCols = std::ceil(std::sqrt(N_channels));
    int nRows = std::ceil(static_cast<double>(N_channels) / nCols);
    c_lin->Divide(nCols, nRows);
    c_resolution->Divide(nCols, nRows);

    std::vector<double> a(N_channels, 0.0);
    std::vector<double> b(N_channels, 0.0);
    // std::vector<double>

    for (size_t i = 0; i < N_channels; i++)
    {
        h_energy[i] = linearityAnalyzer.Process(t, i);
        energy_Spectrum[i] = linearityAnalyzer.FindPeaks(h_energy[i]);
        auto fitted_results = linearityAnalyzer.FitAllPeaks(h_energy[i], energy_Spectrum[i]);

        fitted_means[i] = fitted_results.first;
        energy_resolution[i] = fitted_results.second;

        gr[i] = new TGraph();
        gr[i]->SetName(Form("gr_ch_%zu", i));
        gr[i]->SetTitle(Form("Channel %zu Linearity;Reference Amplitude;Fitted means", i));

        gr_resolution[i] = new TGraph();
        gr_resolution[i]->SetName(Form("gr_ch_%zu", i));
        gr_resolution[i]->SetTitle(Form("Channel %zu Energy_resolution;No of peaks;Difference", i));

        size_t N = std::min(Reff_Spectrum.size(), fitted_means[i].size());

        for (size_t j = 0; j < N; ++j)
        {
            double x = reff_means[j];
            double y = fitted_means[i][j];
            gr[i]->SetPoint(j, x, y);
        }

        gr[i]->Fit("pol1", "Q");
        TF1 *myfit = gr[i]->GetFunction("pol1");

        if (myfit)
        {
            b[i] = myfit->GetParameter(0); // Offset (b)
            a[i] = myfit->GetParameter(1); // Slope (a)

            linearityAnalyzer.compute_R_2(N, a[i], b[i], fitted_means[i], reff_means);
        }

        for (size_t k = 0; k < N; ++k)
        {
            double x = reff_resolution[k]; // Reference energy resolution
            double y = energy_resolution[i][k];

            std::cout << "Channel [" << i << "] | Peak [" << k << "] | "
                      << "Energy Resolution: " << y << " | "
                      << "Reference: " << x << std::endl;
            // std::cout << "Fitted energy resolution for peak " << i + 1 << ": " << y << std::endl;
            // std::cout << "Reference energy resolution for peak " << i + 1 << ": " << x << std::endl;
            // std::cout << "Difference " << i + 1 << ": " << x - y << std::endl;
            gr_resolution[i]->SetPoint(k, k + 1, x - y);
        }

        c_lin->cd(i + 1);
        gr[i]->SetMarkerStyle(20);
        gr[i]->Draw("APL");

        // Draw to c_resolution pad
        c_resolution->cd(i + 1);
        gr_resolution[i]->SetMarkerStyle(20);
        gr_resolution[i]->Draw("APL");
        gr_resolution[i]->GetYaxis()->SetRangeUser(-0.1, 0.1);

        // // // 4. Afisarea rezultatelor
    }
    c_lin->Update();
    c_resolution->Update();

    std::cout
        << "Successfully processed " << N_channels << " channels." << std::endl;
}