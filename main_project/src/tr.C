#define tr_cxx
#include "../include/tr.h"

// Standard C++ includes
#include <iostream>
#include <cmath>
#include <vector>

// ROOT includes
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TTree.h>
#include <TROOT.h>

tr::tr(const char *file) : fChain(0)
{
   std::cout << "Using file: " << file << std::endl;

   TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(file);
   if (!f || !f->IsOpen())
   {
      f = TFile::Open(file, "READ");
   }

   if (!f || f->IsZombie())
   {
      std::cerr << "[Error] Could not open file: " << file << std::endl;
      return;
   }

   TTree *tree = nullptr;
   f->GetObject("events", tree);

   if (!tree)
   {
      std::cerr << "[Error] Tree 'events' not found in file: " << file << std::endl;
      return;
   }

   Init(tree);
}

tr::~tr()
{
   if (!fChain)
      return;
   delete fChain->GetCurrentFile();
}

Int_t tr::GetEntry(Long64_t entry)
{
   if (!fChain)
      return 0;
   return fChain->GetEntry(entry);
}

Long64_t tr::LoadTree(Long64_t entry)
{
   if (!fChain)
      return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0)
      return centry;
   if (fChain->GetTreeNumber() != fCurrent)
   {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void tr::Init(TTree *tree)
{
   // Set object pointers to null before binding branches
   analog_probe1 = 0;
   analog_probe2 = 0;
   analog_probe3 = 0;
   digital_probe1 = 0;
   digital_probe2 = 0;
   digital_probe3 = 0;
   digital_probe4 = 0;
   digital_probe5 = 0;
   digital_probe6 = 0;
   digital_probe7 = 0;
   digital_probe8 = 0;
   digital_probe9 = 0;
   digital_probe10 = 0;
   digital_probe11 = 0;
   digital_probe12 = 0;
   digital_probe13 = 0;
   digital_probe14 = 0;
   digital_probe15 = 0;
   digital_probe16 = 0;

   if (!tree)
      return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("module", &module, &b_module);
   fChain->SetBranchAddress("channel", &channel, &b_channel);
   fChain->SetBranchAddress("energy", &energy, &b_energy);
   fChain->SetBranchAddress("waveform", &analog_probe1, &b_analog_probe1);

   // fChain->SetBranchAddress("energy_short", &energy_short, &b_energy_short);
   fChain->SetBranchAddress("timestamp", &timestamp, &b_timestamp);
   // fChain->SetBranchAddress("flags", &flags, &b_flags);
   // fChain->SetBranchAddress("user_info", user_info, &b_user_info);
   // fChain->SetBranchAddress("has_waveform", &has_waveform, &b_has_waveform);
   // fChain->SetBranchAddress("time_resolution", &time_resolution, &b_time_resolution);
   // fChain->SetBranchAddress("trigger_threshold", &trigger_threshold, &b_trigger_threshold);
   // fChain->SetBranchAddress("ns_per_sample", &ns_per_sample, &b_ns_per_sample);
   // fChain->SetBranchAddress("analog_probe1", &analog_probe1, &b_analog_probe1);
   // fChain->SetBranchAddress("analog_probe2", &analog_probe2, &b_analog_probe2);
   // fChain->SetBranchAddress("analog_probe3", &analog_probe3, &b_analog_probe3);
   // fChain->SetBranchAddress("digital_probe1", &digital_probe1, &b_digital_probe1);
   // fChain->SetBranchAddress("digital_probe2", &digital_probe2, &b_digital_probe2);
   // fChain->SetBranchAddress("digital_probe3", &digital_probe3, &b_digital_probe3);
   // fChain->SetBranchAddress("digital_probe4", &digital_probe4, &b_digital_probe4);
   // fChain->SetBranchAddress("digital_probe5", &digital_probe5, &b_digital_probe5);
   // fChain->SetBranchAddress("digital_probe6", &digital_probe6, &b_digital_probe6);
   // fChain->SetBranchAddress("digital_probe7", &digital_probe7, &b_digital_probe7);
   // fChain->SetBranchAddress("digital_probe8", &digital_probe8, &b_digital_probe8);
   // fChain->SetBranchAddress("digital_probe9", &digital_probe9, &b_digital_probe9);
   // fChain->SetBranchAddress("digital_probe10", &digital_probe10, &b_digital_probe10);
   // fChain->SetBranchAddress("digital_probe11", &digital_probe11, &b_digital_probe11);
   // fChain->SetBranchAddress("digital_probe12", &digital_probe12, &b_digital_probe12);
   // fChain->SetBranchAddress("digital_probe13", &digital_probe13, &b_digital_probe13);
   // fChain->SetBranchAddress("digital_probe14", &digital_probe14, &b_digital_probe14);
   // fChain->SetBranchAddress("digital_probe15", &digital_probe15, &b_digital_probe15);
   // fChain->SetBranchAddress("digital_probe16", &digital_probe16, &b_digital_probe16);
   // fChain->SetBranchAddress("analog_probe1_is_signed", &analog_probe1_is_signed, &b_analog_probe1_is_signed);
   // fChain->SetBranchAddress("analog_probe2_is_signed", &analog_probe2_is_signed, &b_analog_probe2_is_signed);
   // fChain->SetBranchAddress("analog_probe3_is_signed", &analog_probe3_is_signed, &b_analog_probe3_is_signed);
   // fChain->SetBranchAddress("analog_probe_type", analog_probe_type, &b_analog_probe_type);
   // fChain->SetBranchAddress("digital_probe_type", digital_probe_type, &b_digital_probe_type);
   Notify();
}

bool tr::Notify()
{
   return true;
}

void tr::Show(Long64_t entry)
{
   if (!fChain)
      return;
   fChain->Show(entry);
}

Int_t tr::Cut(Long64_t entry)
{
   return 1;
}

void tr::Loop()
{
   if (fChain == 0)
      return;

   Long64_t nentries = fChain->GetEntriesFast();
   Long64_t nbytes = 0, nb = 0;

   for (Long64_t jentry = 0; jentry < nentries; jentry++)
   {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0)
         break;
      nb = fChain->GetEntry(jentry);
      nbytes += nb;
      std::cout << "Entry: " << ientry << std::endl;
   }
}