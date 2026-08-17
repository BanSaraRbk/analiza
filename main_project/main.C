#include <iostream>
#include <TApplication.h>
#include <TSpectrum.h>
#include "include/tr.h"
#include "include/WaveformViewer.h"
#include "include/linearity.h"
#include <TGraph.h>
#include <TF1.h>
#include <numeric>
int main(int argc, char **argv)
{
    TApplication app("app", &argc, argv);

    const char *filename = (argc > 1) ? argv[1] : "data/20260817_test_multiple_sigma4_countsmulte.root";

    std::cout << "--- Initializare analiza pentru: " << filename << " ---" << std::endl;

    tr t(filename);

    if (!t.fChain)
    {
        std::cerr << "Eroare: Tree-ul nu a putut fi incarcat!" << std::endl;
        return 1;
    }

    // std::cout << "Alege numarul de canale pentru vizualizare (0-1): ";
    // int kNumChannels;
    // std::cin >> kNumChannels;
    // std::cout << "Alege numarul modulului pentru analiza: ";
    // int kTargetModule;
    // std::cin >> kTargetModule;
    LinearityAnalyzer linearityAnalyzer;
    TH1F *h_refference = linearityAnalyzer.ReadRefferenceSpectrum("output_SIGMA.csv");
    TH1F *h_energy = linearityAnalyzer.Process(&t);
    // TH1F *h_energy = linearityAnalyzer.ReadRefferenceSpectrum("output_SIGMA.csv");

    std::vector<std::pair<int, int>> Reff_Spectrum;
    std::vector<std::pair<int, int>> energy_Spectrum;

    Reff_Spectrum = linearityAnalyzer.FindPeaks(h_refference);
    energy_Spectrum = linearityAnalyzer.FindPeaks(h_energy);

    // for (size_t i = 0; i < 32 + 1; ++i)
    // {
    //     std::cout << "REFERENCE SPECTRUM PEAKS: " << std::endl;
    //     std::cout << "Peak " << i + 1 << ": X = " << Reff_Spectrum[i].first
    //               << ", Amplitude = " << Reff_Spectrum[i].second << std::endl;
    // }

    // for (size_t j = 0; j < 32 + 1; ++j)
    // {
    //     std::cout << "ENERGY SPECTRUM PEAKS: " << std::endl;
    //     std::cout << "Peak " << j + 1 << ": X = " << energy_Spectrum[j].first
    //               << ", Amplitude = " << energy_Spectrum[j].second << std::endl;
    // }
    // calculR2 coeficient de determinare
    double y_real = 0.0;

    double y_compute = 0.0;

    double y_mean = 0.0;
    double sum_y = 0.0;
    double x_real;
    double SS_res = 0.0;
    double SS_tot = 0.0;
    double R = 0.0;
    std::pair<std::vector<double>, std::vector<double>> fitted_results = linearityAnalyzer.FitAllPeaks(h_energy, energy_Spectrum);
    std::vector<double> fitted_means = fitted_results.first;
    std::vector<double> energy_resolution = fitted_results.second;

    std::pair<std::vector<double>, std::vector<double>> reff_results = linearityAnalyzer.FitAllPeaks(h_refference, Reff_Spectrum);
    std::vector<double> reff_means = reff_results.first;
    std::vector<double> reff_resolution = reff_results.second;

    TGraph *gr = new TGraph();
    gr->SetTitle("Linearity;Reference Amplitude;Fitted Mean");
    gr->SetMarkerStyle(20);

    for (size_t i = 0; i < Reff_Spectrum.size() && i < fitted_means.size(); ++i)
    {
        double x = reff_means[i]; // Reference amplitude
        double y = fitted_means[i];
        sum_y = sum_y + fitted_means[i];

        std::cout << x << std::endl;
        // std::cout << "Reference amplitude for peak " << i + 1 << ": " << x << std::endl;
        gr->SetPoint(i, x, y);
    }
    TCanvas *c_lin = new TCanvas("c_lin", "Linearity Fit", 800, 600);
    gr->Draw("APL");
    gr->Fit("pol1");
    TF1 *myfit = gr->GetFunction("pol1");
    double b = myfit->GetParameter(0); // Offset
    double a = myfit->GetParameter(1); // Panta dreptei (a)
                                       // ax+b
    std::cout << b << "  " << a << std::endl;
    y_mean = sum_y / Reff_Spectrum.size();
    for (size_t j = 0; j < Reff_Spectrum.size() && j < fitted_means.size(); ++j)
    {
        y_real = fitted_means[j];
        x_real = reff_means[j];

        double y_compute = a * x_real + b;

        // std::cout << y_compute << std::endl;

        SS_res = SS_res + std::pow(y_real - y_compute, 2);
        SS_tot = SS_tot + std::pow(y_real - y_mean, 2);
    }
    // calcul r^2

    double R2 = (SS_tot != 0.0) ? (1.0 - (SS_res / SS_tot)) : 0.0;

    // // // 4. Afisarea rezultatelor
    std::cout << "\n========== REZULTATE FIT & STATISTICA ==========\n";
    //  std::cout << "Numar de puncte (N): " << N << "\n";
    std::cout << "Parametru p0 (Offset): " << b << "\n";
    std::cout << "Parametru p1 (Panta) : " << a << "\n";
    std::cout << "Media y (y_mean)     : " << y_mean << "\n";
    std::cout << "SS_res               : " << SS_res << "\n";
    std::cout << "SS_tot               : " << SS_tot << "\n";
    std::cout << "R^2 (Determinare)    : " << R2 << "\n";
    std::cout << "================================================\n";

    // std::cout << "Value of the fit is: " << gr->GetFunction("pol1")->GetParameter(0) << " + " << gr->GetFunction("pol1")->GetParameter(1) << " * x" << std::endl;

    // linearityAnalyzer.ProcessEnergy_RootFile();

    TCanvas *c_comp = new TCanvas("c_comparison", "Energy resolution difference", 800, 600);
    TGraph *gr_resolution = new TGraph();
    gr_resolution->SetTitle("Energy Resolution Difference;Reference Energy Resolution;Fitted Energy Resolution");
    gr_resolution->SetMarkerStyle(21);

    for (size_t i = 0; i < reff_resolution.size() && i < energy_resolution.size(); ++i)
    {
        double x = reff_resolution[i]; // Reference energy resolution
        double y = energy_resolution[i];

        // std::cout << "Fitted energy resolution for peak " << i + 1 << ": " << y << std::endl;
        // std::cout << "Reference energy resolution for peak " << i + 1 << ": " << x << std::endl;
        // std::cout << "Difference " << i + 1 << ": " << x - y << std::endl;
        gr_resolution->SetPoint(i, i + 1, x - y);
    }

    gr_resolution->Draw("APL");
    // gr_resolution->GetXaxis()->SetRangeUser();
    gr_resolution->GetYaxis()->SetRangeUser(-0.1, 0.1);

    linearityAnalyzer.Draw(h_refference);
    linearityAnalyzer.Draw(h_energy);

    std::cout << "Analiza finalizata cu succes." << std::endl;

    app.Run();

    return 0;
}
//  g++ -o main  main.C src/tr.C src/WaveformViewer.C src/linearity.C `root-config --cflags` `root-config --libs`-lSpectrum