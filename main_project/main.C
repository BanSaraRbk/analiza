#include <iostream>
#include <TApplication.h>
#include <TSpectrum.h>
#include "include/tr.h"
#include "include/WaveformViewer.h"
#include "include/linearity.h"
#include "include/timestamp.h"
#include <TGraph.h>
#include <TF1.h>
#include <numeric>
#include <fstream>
#include <cstdio>
#include <TError.h>

int main(int argc, char **argv)
{
    TApplication app("app", &argc, argv);
    // std::freopen("output.txt", "w", stdout);
    // std::freopen("output.txt", "a", stderr);
    gErrorIgnoreLevel = kWarning;

    const char *root_filename = (argc > 1) ? argv[1] : "data/20260820_countrate1_timestamp.root";
    std::string csv_file = "output_same_amplitude.csv";

    tr t(root_filename);
    if (!t.fChain)
    {
        std::cerr << "Error: Tree could not be loaded!" << std::endl;
        return 1;
    }
    int N_channels;
    std::cout << "Cate canale doriti??" << std::endl;
    std::cin >> N_channels;
    //  int N_channels = 2;
    // LinearityAnalyzer analyzer;

    // analyzer.process_linearity(csv_file, &t, N_channels);

    // std::fflush(stdout);
    // std::fflush(stderr);
    // WaveformViewer wave(N_channels);
    // wave.Process(&t);
    // wave.InitHistograms();
    // wave.Draw();

    timestamp timestamp(N_channels);
    timestamp.Initialize_Hist();
    timestamp.ProcessTree(&t);
    timestamp.DrawHistograms();

    app.Run();
    return 0;
}

//  g++ -o main  main.C src/tr.C src/WaveformViewer.C src/linearity.C `root-config --cflags` `root-config --libs`-lSpectrum
// g++ -o main  main.C src/tr.C src/WaveformViewer.C src/linearity.C `root-config --cflags` `root-config --libs` -lSpectrum
// ./main 2>&1 | tee file.log

// git add . ':!*.zip' ':!**/*.zip' ':!*.root' ':!**/*.root' ':!data/**' ':!**/data/**'