#ifndef LINEARITY_ANALYZER_H
#define LINEARITY_ANALYZER_H

#include <vector>
#include <TH2D.h>
#include <TCanvas.h>
#include <TSpectrum.h>
#include "tr.h"

class tr;

class LinearityAnalyzer
{
public:
    LinearityAnalyzer();
    ~LinearityAnalyzer();

    TH1F *Process(tr *eventReader, int No_Channels);
    TH1F *ReadRefferenceSpectrum(const std::string &filename);
    // spectrum de la signal generator .csv file
    std::vector<std::pair<int, int>> FindPeaks(TH1F *h);
    int ProcessEnergy_RootFile(); // return the energy fitted peaks with corresponding mean values
    void compute_R_2(int N, double a, double b, std::vector<double> fitted_means, std::vector<double> reff_means);

    void Draw(TH1F *h, int No_channels); // Draw the histogram with peaks marked
    std::pair<std::vector<double>, std::vector<double>> FitAllPeaks(TH1F *h, const std::vector<std::pair<int, int>> &peaks);
    void process_linearity(const std::string &csv_file, tr *t, int N_channels);

private:
    TH1F *h1 = nullptr; // Histogram for reference spectrum
    TH1F *h2 = nullptr;
    std::vector<double> referenceAmplitudes, referenceXPositions;
    std::vector<std::pair<double, double>> Reff_Spectrum;

    // Store amplitudes and positions of peaks in the reference spectrum
};
#endif