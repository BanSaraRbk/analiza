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

    const char *root_filename = (argc > 1) ? argv[1] : "data/20260818_test_multiple_sigma4_mpr_2channels.root";
    std::string csv_file = "output_SIGMA.csv";

    tr t(root_filename);
    if (!t.fChain)
    {
        std::cerr << "Error: Tree could not be loaded!" << std::endl;
        return 1;
    }

    int N_channels = 2; // or read from argv/cin
    LinearityAnalyzer analyzer;
    analyzer.process_linearity(csv_file, &t, N_channels);

    app.Run();
    return 0;
}
//  g++ -o main  main.C src/tr.C src/WaveformViewer.C src/linearity.C `root-config --cflags` `root-config --libs`-lSpectrum
// g++ -o main  main.C src/tr.C src/WaveformViewer.C src/linearity.C `root-config --cflags` `root-config --libs` -lSpectrum