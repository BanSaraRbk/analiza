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
#include <vector>

extern int kNumChannels;
extern int kTargetModule;
class tr
{
public:
   TTree *fChain;  ///<!pointer to the analyzed TTree or TChain
   Int_t fCurrent; ///<!current Tree number in a TChain

   // Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   UChar_t module;
   UChar_t channel;
   UShort_t energy;
   UShort_t energy_short;
   ULong64_t timestamp;
   ULong64_t flags;
   ULong64_t user_info[4];
   Bool_t has_waveform;
   UChar_t time_resolution;
   UShort_t trigger_threshold;
   Double_t ns_per_sample;
   std::vector<short> *analog_probe1;
   std::vector<short> *analog_probe2;
   std::vector<short> *analog_probe3;
   std::vector<short> *digital_probe1;
   std::vector<short> *digital_probe2;
   std::vector<short> *digital_probe3;
   std::vector<short> *digital_probe4;
   std::vector<short> *digital_probe5;
   std::vector<short> *digital_probe6;
   std::vector<short> *digital_probe7;
   std::vector<short> *digital_probe8;
   std::vector<short> *digital_probe9;
   std::vector<short> *digital_probe10;
   std::vector<short> *digital_probe11;
   std::vector<short> *digital_probe12;
   std::vector<short> *digital_probe13;
   std::vector<short> *digital_probe14;
   std::vector<short> *digital_probe15;
   std::vector<short> *digital_probe16;
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
   TBranch *b_timestamp;               ///<!
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
};

#endif
