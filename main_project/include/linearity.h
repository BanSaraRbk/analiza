#ifndef LINEARITY_ANALYZER_H
#define LINEARITY_ANALYZER_H

#include <vector>
#include <TH2D.h>
#include <TCanvas.h>
#include <TSpectrum.h>

class tr;

class LinearityAnalyzer
{
public:
    LinearityAnalyzer();
    ~LinearityAnalyzer();

    TH1F *Process(tr *eventReader);
    TH1F *ReadRefferenceSpectrum(const std::string &filename);
    // spectrum de la signal generator .csv file
    std::vector<std::pair<int, int>> FindPeaks(TH1F *h); // return the reference oeaks with corresponding amplitudes
    int ProcessEnergy_RootFile();                        // return the energy fitted peaks with corresponding mean values

    void Draw(TH1F *h); // Draw the histogram with peaks marked
    std::pair<std::vector<double>, std::vector<double>> FitAllPeaks(TH1F *h, const std::vector<std::pair<int, int>> &peaks);

private:
    TH1F *h1 = nullptr; // Histogram for reference spectrum
    TH1F *h2 = nullptr;
    std::vector<double> referenceAmplitudes, referenceXPositions;
    std::vector<std::pair<double, double>> Reff_Spectrum; // Store amplitudes and positions of peaks in the reference spectrum
};
#endif