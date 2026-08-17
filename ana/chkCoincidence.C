#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <vector>
#include <iostream>
#include <cmath>

const double COINC_WINDOW = 100.0; // ns
// const Double_t kClkToNs = 8.0;

struct Hit
{
    int channel;
    double time;
};

void chkCoincidence()
{
    TFile *f = new TFile("../root/run10008.root");
    TTree *tree = (TTree *)f->Get("tr");

    UChar_t module;
    UChar_t channel;
    Double_t timestamp;
    UShort_t energy;
    ULong64_t userInfo0;

    tree->SetBranchAddress("Module", &module);
    tree->SetBranchAddress("Channel", &channel);
    tree->SetBranchAddress("TimestampNs", &timestamp);
    tree->SetBranchAddress("Energy", &energy);
    tree->SetBranchAddress("UserInfo0", &userInfo0);

    double board0_hits;
    double board1_hits;
    double board0_hits_0;

    std::vector<double> timestamp_test;

    std::vector<double> event_no;
    TCanvas *c1 = new TCanvas("c1", "Delta t histogram", 800, 600);

    // 🔹 Draw histogram

    Long64_t nentries = tree->GetEntries();

    // 🔹 Δt histogram
    TH1D *hdt = new TH1D("hdt",
                         "Delta t (B0 - B1);dt [ns];Counts",
                         1000, -500, 500);
    double time0, time1;
    const Long64_t window = 100; // ns

    for (Long64_t i = 0; i < tree->GetEntries(); i++)
    {
        tree->GetEntry(i);

        if (module == 0 && channel == 0)
        {
            Long64_t t0 = timestamp;

            for (Long64_t j = 0; j < tree->GetEntries(); j++)
            {
                tree->GetEntry(j);

                if (module == 1 && channel == 0)
                {
                    Long64_t dt = (Long64_t)timestamp - (Long64_t)t0;

                    if (TMath::Abs(dt) < window)
                    {
                        std::cout << "Coincidence Δt = " << dt << std::endl;
                    }
                }
            }
        }

        // if (module == 0 && channel == 1)
        // {
        //     board0_hits_0 = timestamp;
        // }

        // if (module == 1 && channel == 0)
        // {
        //     board1_hits = timestamp;
        // }

        // std::cout << board0_hits << "   " << board0_hits_0 << "  " << board1_hits << std::endl;
    }
    TGraph *plot_timestamp = new TGraph(event_no.size(), event_no.data(), timestamp_test.data());

    // 4. Draw the plot at the very end
    plot_timestamp->SetMarkerStyle(20);
    plot_timestamp->GetXaxis()->SetTitle("Id_number");
    plot_timestamp->GetYaxis()->SetTitle("Timestamp");

    plot_timestamp->Draw("AP*");
}
// int coinc_count = 0;

// for (auto &h0 : board0_hits)
// {
//     for (auto &h2 : board2_hits)
//     {
//         double dt = h0.time - h2.time;

//         hdt->Fill(dt);
//         if (std::fabs(dt) < COINC_WINDOW)
//         {
//             coinc_count++;

//             if (coinc_count == 100)
//             {
//                 std::cout << "Coincidence: "
//                           << "B0 Ch " << h0.channel
//                           << " <-> B2 Ch " << h2.channel
//                           << " | dt = " << dt << " ns\n";
//             }
//         }
//     }
// }

// std::cout << "\nTotal coincidences: " << coinc_count << std::endl;

// // 🔹 Draw histogram
// TCanvas *c1 = new TCanvas("c1", "Delta t histogram", 800, 600);
// hdt->Draw();

// c1->SetLogy(); // optional but useful

// // 🔹 Optional save
// TFile *out = new TFile("dt_hist.root", "RECREATE");
// hdt->Write();
// out->Close();
// }