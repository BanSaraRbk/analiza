#ifndef timestamp_H
#define timestamp_H

#include <vector>
#include <TH2D.h>
#include <TCanvas.h>
#include <TGraph.h>

class tr;

class timestamp
{
public:
    timestamp(int total_channels, int total_modules);
    ~timestamp();

    void Initialize_Hist();
    void ProcessTree(tr *eventReader);
    void DrawHistograms();

private:
    std::vector<TH1F *> h_freq;
    std::vector<TH1F *> h_ch_dif;
    TGraph *fGraph;
    int fTotalChannels;
    int fTotalModules;
};
#endif