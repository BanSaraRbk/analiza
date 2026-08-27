#include "TTimeStamp.h"
#include "TTree.h"
#include "TFile.h"
#include "TRandom.h"

void Read() {
   Long64_t TimeStamp1,TimeStamp2;
   Double_t x1,x2;
   TFile *f1 = new TFile("prodtime1.root");
   TTree *T1 = (TTree*)f1->Get("T1");
   T1->SetBranchAddress("TimeStamp",&TimeStamp1);
   T1->SetBranchAddress("x",&x1);
   TFile *f2 = new TFile("prodtime2.root");
   TTree *T2 = (TTree*)f2->Get("T2");
   T2->SetBranchAddress("TimeStamp",&TimeStamp2);
   T2->SetBranchAddress("x",&x2);
   Long64_t n1 = T1->GetEntries();
   Long64_t n2 = T2->GetEntries();
   //allocate space to store all entries of TimeStamp in T2
   Long64_t *ts2  = new Long64_t[n2];
   Long64_t *its2 = new Long64_t[n2];
   TBranch *b2_ts = T2->GetBranch("TimeStamp");
   for (Long64_t i=0;i<n2;i++) {
      b2_ts->GetEntry(i);
      ts2[i] = TimeStamp2;
   }
   //compute sorted index for array ts2
   TMath::Sort(n2,ts2,its2,kFALSE);
   //sort array ts2 in case it is not already sorted
   Bool_t mustSort = kFALSE;
   for (Long64_t i=0;i<n2;i++) {
      if (its2[i] != i) {mustSort = kTRUE; break;}
   }
   if (mustSort) {
      printf("Sorting array ts2\n");
      Long64_t *tsort2 = new Long64_t[n2];
      for (Long64_t i=0;i<n2;i++) tsort2[i] = ts2[its2[i]];
      for (Long64_t i=0;i<n2;i++) ts2[i]    = tsort2[i];
   }
   //loop on all entries of T1
   for (Long64_t i=0;i<n1;i++) {
      T1->GetEntry(i);
      //search the closest entry to TimeStamp1 in array ts2
      Long64_t entry2 = TMath::BinarySearch(n2,ts2,TimeStamp1);
      if (entry2 < 0) entry2 = 0;
      T2->GetEntry(entry2);
      if (i <20) printf("i=%lld, TimeStamp1=%llu,entry2=%lld, TimeStamp2=%llu, x1=%g, x2=%g\n",
         i,TimeStamp1,entry2,TimeStamp2, x1,x2);
   }
}

void Write() {
   //create 2 Trees
   //Each tree contains a TimeStamp
   //The 2nd Tree contains a subset of the 1st Tree
   Long64_t TimeStamp;
   Double_t x;
   TFile *f1 = new TFile("prodtime1.root","recreate");
   TTree *T1 = new TTree("T1","Tree stamp 1");
   T1->Branch("TimeStamp",&TimeStamp,"TimeStamp/L");
   T1->Branch("x",&x,"x/D");
   TFile *f2 = new TFile("prodtime2.root","recreate");
   TTree *T2 = new TTree("T2","Tree stamp 2");
   T2->Branch("TimeStamp",&TimeStamp,"TimeStamp/L");
   T2->Branch("x",&x,"x/D");
   TTimeStamp t;
   TRandom r;
   
   for (Int_t i=0;i<1000000;i++) {
      t.Set();
      x = r.Gaus(0,1);
      Long64_t tsec = t.GetSec();
      Long64_t tnanosec = t.GetNanoSec();
      TimeStamp = (tsec <<30) + (tnanosec>>1);
      T1->Fill();
      if (r.Rndm() > 0.4) T2->Fill();
   }
   T1->AutoSave();
   T2->AutoSave();
   T2->Print();
   delete f1;
   delete f2;
}
   
void prodtime() {
   Write();
   Read();
}      
   
