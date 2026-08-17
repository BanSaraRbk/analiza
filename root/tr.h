//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Thu Aug 13 10:24:39 2026 by ROOT version 6.40.02
// from TTree tr/DELILA events
// found on file: run20037.root
//////////////////////////////////////////////////////////

#ifndef tr_h
#define tr_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class tr
{
public:
   TTree *fChain;  ///<!pointer to the analyzed TTree or TChain
   Int_t fCurrent; ///<!current Tree number in a TChain

   // Fixed size dimensions of array or collections stored in the TTree if any.
   int kNumChannels;
   int kTargetModule;
   // Declaration of leaf types
   UChar_t module;
   UChar_t channel;
   UShort_t energy;
   UShort_t energy_short;
   Double_t timestamp_ns;
   ULong64_t flags;
   ULong64_t user_info[4];
   Bool_t has_waveform;
   UChar_t time_resolution;
   UShort_t trigger_threshold;
   Double_t ns_per_sample;
   vector<short> *analog_probe1;
   vector<short> *analog_probe2;
   vector<short> *analog_probe3;
   vector<short> *digital_probe1;
   vector<short> *digital_probe2;
   vector<short> *digital_probe3;
   vector<short> *digital_probe4;
   vector<short> *digital_probe5;
   vector<short> *digital_probe6;
   vector<short> *digital_probe7;
   vector<short> *digital_probe8;
   vector<short> *digital_probe9;
   vector<short> *digital_probe10;
   vector<short> *digital_probe11;
   vector<short> *digital_probe12;
   vector<short> *digital_probe13;
   vector<short> *digital_probe14;
   vector<short> *digital_probe15;
   vector<short> *digital_probe16;
   Bool_t analog_probe1_is_signed;
   Bool_t analog_probe2_is_signed;
   Bool_t analog_probe3_is_signed;
   UChar_t analog_probe_type[3];
   UChar_t digital_probe_type[16];

   // List of branches
   TBranch *b_module;                  ///<!
   TBranch *b_channel;                 ///<!
   TBranch *b_energy;                  ///<!
   TBranch *b_energy_short;            ///<!
   TBranch *b_timestamp_ns;            ///<!
   TBranch *b_flags;                   ///<!
   TBranch *b_user_info;               ///<!
   TBranch *b_has_waveform;            ///<!
   TBranch *b_time_resolution;         ///<!
   TBranch *b_trigger_threshold;       ///<!
   TBranch *b_ns_per_sample;           ///<!
   TBranch *b_analog_probe1;           ///<!
   TBranch *b_analog_probe2;           ///<!
   TBranch *b_analog_probe3;           ///<!
   TBranch *b_digital_probe1;          ///<!
   TBranch *b_digital_probe2;          ///<!
   TBranch *b_digital_probe3;          ///<!
   TBranch *b_digital_probe4;          ///<!
   TBranch *b_digital_probe5;          ///<!
   TBranch *b_digital_probe6;          ///<!
   TBranch *b_digital_probe7;          ///<!
   TBranch *b_digital_probe8;          ///<!
   TBranch *b_digital_probe9;          ///<!
   TBranch *b_digital_probe10;         ///<!
   TBranch *b_digital_probe11;         ///<!
   TBranch *b_digital_probe12;         ///<!
   TBranch *b_digital_probe13;         ///<!
   TBranch *b_digital_probe14;         ///<!
   TBranch *b_digital_probe15;         ///<!
   TBranch *b_digital_probe16;         ///<!
   TBranch *b_analog_probe1_is_signed; ///<!
   TBranch *b_analog_probe2_is_signed; ///<!
   TBranch *b_analog_probe3_is_signed; ///<!
   TBranch *b_analog_probe_type;       ///<!
   TBranch *b_digital_probe_type;      ///<!

   tr(const char *file = "run20037.root");
   virtual ~tr();
   virtual Int_t Cut(Long64_t entry);
   virtual Int_t GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void Init(TTree *tree);
   virtual void Loop();
   virtual bool Notify();
   virtual void Show(Long64_t entry = -1);
   virtual void wave();
   virtual void getUserInput();
};

#endif

#ifdef tr_cxx
tr::tr(const char *file) : fChain(0)
{
   // if parameter tree is not specified (or zero), connect the file
   // used to generate this class and read the Tree.
   TTree *tree = nullptr;
   std::cout << "Using file: " << file << std::endl;
   if (tree == 0)
   {
      TFile *f = (TFile *)gROOT->GetListOfFiles()->FindObject(file);
      if (!f || !f->IsOpen())
      {
         f = new TFile(file);
      }

      f->GetObject("tr", tree);
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
   // Read contents of entry.
   if (!fChain)
      return 0;
   return fChain->GetEntry(entry);
}
Long64_t tr::LoadTree(Long64_t entry)
{
   // Set the environment to read one entry
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
void tr::getUserInput()
{
   std::cout << "Insert the desired number of channels to view (1-16): ";
   std::cin >> kNumChannels;
   std::cout << "Insert the board number to analyze : ";
   std::cin >> kTargetModule;
}
void tr::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.

   // Set object pointer
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
   // Set branch addresses and branch pointers
   if (!tree)
      return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("module", &module, &b_module);
   fChain->SetBranchAddress("channel", &channel, &b_channel);
   fChain->SetBranchAddress("energy", &energy, &b_energy);
   fChain->SetBranchAddress("energy_short", &energy_short, &b_energy_short);
   fChain->SetBranchAddress("timestamp_ns", &timestamp_ns, &b_timestamp_ns);
   fChain->SetBranchAddress("flags", &flags, &b_flags);
   fChain->SetBranchAddress("user_info", user_info, &b_user_info);
   fChain->SetBranchAddress("has_waveform", &has_waveform, &b_has_waveform);
   fChain->SetBranchAddress("time_resolution", &time_resolution, &b_time_resolution);
   fChain->SetBranchAddress("trigger_threshold", &trigger_threshold, &b_trigger_threshold);
   fChain->SetBranchAddress("ns_per_sample", &ns_per_sample, &b_ns_per_sample);
   fChain->SetBranchAddress("analog_probe1", &analog_probe1, &b_analog_probe1);
   fChain->SetBranchAddress("analog_probe2", &analog_probe2, &b_analog_probe2);
   fChain->SetBranchAddress("analog_probe3", &analog_probe3, &b_analog_probe3);
   fChain->SetBranchAddress("digital_probe1", &digital_probe1, &b_digital_probe1);
   fChain->SetBranchAddress("digital_probe2", &digital_probe2, &b_digital_probe2);
   fChain->SetBranchAddress("digital_probe3", &digital_probe3, &b_digital_probe3);
   fChain->SetBranchAddress("digital_probe4", &digital_probe4, &b_digital_probe4);
   fChain->SetBranchAddress("digital_probe5", &digital_probe5, &b_digital_probe5);
   fChain->SetBranchAddress("digital_probe6", &digital_probe6, &b_digital_probe6);
   fChain->SetBranchAddress("digital_probe7", &digital_probe7, &b_digital_probe7);
   fChain->SetBranchAddress("digital_probe8", &digital_probe8, &b_digital_probe8);
   fChain->SetBranchAddress("digital_probe9", &digital_probe9, &b_digital_probe9);
   fChain->SetBranchAddress("digital_probe10", &digital_probe10, &b_digital_probe10);
   fChain->SetBranchAddress("digital_probe11", &digital_probe11, &b_digital_probe11);
   fChain->SetBranchAddress("digital_probe12", &digital_probe12, &b_digital_probe12);
   fChain->SetBranchAddress("digital_probe13", &digital_probe13, &b_digital_probe13);
   fChain->SetBranchAddress("digital_probe14", &digital_probe14, &b_digital_probe14);
   fChain->SetBranchAddress("digital_probe15", &digital_probe15, &b_digital_probe15);
   fChain->SetBranchAddress("digital_probe16", &digital_probe16, &b_digital_probe16);
   fChain->SetBranchAddress("analog_probe1_is_signed", &analog_probe1_is_signed, &b_analog_probe1_is_signed);
   fChain->SetBranchAddress("analog_probe2_is_signed", &analog_probe2_is_signed, &b_analog_probe2_is_signed);
   fChain->SetBranchAddress("analog_probe3_is_signed", &analog_probe3_is_signed, &b_analog_probe3_is_signed);
   fChain->SetBranchAddress("analog_probe_type", analog_probe_type, &b_analog_probe_type);
   fChain->SetBranchAddress("digital_probe_type", digital_probe_type, &b_digital_probe_type);
   Notify();
}

bool tr::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be for a new TTree in a TChain. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}
void tr::wave()
{
   // std::cout << "Insert the desired number of channels to view (1-16): ";
   // int kNumChannels;
   // std::cin >> kNumChannels;
   // std::cout << "Insert the board number to analyze : ";
   // int kTargetModule;
   // std::cin >> kTargetModule;
   // Create histograms for each channel
   TCanvas *canvas = new TCanvas("canvas", "Histograms", 800, 600);
   std::vector<TH2D *> h(kNumChannels);

   for (int ch = 0; ch < kNumChannels; ch++)
   {
      h[ch] = new TH2D(Form("h%d", ch),
                       "Waveform;Sample;Amplitude",
                       400, 0, 500,
                       400, 7000, 20000);
   }
   for (Long64_t jentry = 0; jentry < fChain->GetEntries(); ++jentry)
   {
      fChain->GetEntry(jentry);
      const int ch = static_cast<int>(channel);
      if (ch < kNumChannels)
      {
         for (size_t sample = 0; sample < analog_probe1->size(); ++sample)
         {
            h[ch]->Fill(sample, (*analog_probe1)[sample]);
         }
      }
   }
   int nCols = std::ceil(std::sqrt(kNumChannels));
   int nRows = std::ceil(static_cast<double>(kNumChannels) / nCols);

   TCanvas *c3 = new TCanvas("c3", "Relative Difference", 1200, 800);
   c3->Divide(nCols, nRows);

   for (int ch = 0; ch < kNumChannels; ++ch)
   {
      c3->cd(ch + 1);
      h[ch]->Draw("COLZ");
   }
}

void tr::Show(Long64_t entry)
{
   // Print contents of entry.
   // If entry is not specified, print current entry
   if (!fChain)
      return;
   fChain->Show(entry);
}
Int_t tr::Cut(Long64_t entry)
{
   // This function may be called from Loop.
   // returns  1 if entry is accepted.
   // returns -1 otherwise.
   return 1;
}
#endif // #ifdef tr_cxx
