


void chkDaqEfficiency()
{
  TFile *fin = gFile;
  if (!fin || !fin->IsOpen()) {
    std::cout << "No ROOT file is currently open." << std::endl;
    std::cout << "Example: TFile::Open(\"data.root\"); ana30(\"Trace_Tree\");" << std::endl;
    return;
  }

  TTree *tree = (TTree*)fin->Get("tr");
  if (!tree) {
    std::cout << "Tree 'tr' not found in this file" << std::endl;
    return;
  }
  
  UChar_t module;
  UChar_t channel;
  Double_t timestampNs;

  tree->SetBranchAddress("Module",&module);
  tree->SetBranchAddress("Channel",&channel);
  tree->SetBranchAddress("TimestampNs",&timestampNs);

  ULong64_t nEntries = tree->GetEntries();
  std::cout << "Total Events: " << nEntries << std::endl;

  // For Analysis
  Double_t timestampInit = 0;
  Double_t timestampEnd = 0;
  Double_t timestampPre = 0;
  Int_t    counter_frequency = 0;
  Double_t frequencyAccum = 0;
  Double_t frequencyAve = 0;

  TH1D* hFrequency = new TH1D("h1","frequency",500,0,100);
  
  for (int i = 0; i < nEntries; i++)
  {
    if (i % 100000 == 0){
      std::cout << "Processing Entry" << i << "/" << nEntries << std::endl;
    }
    
    tree->GetEntry(i);
    if (i == 0) timestampInit = timestampNs;
    if (i == (nEntries -1)) timestampEnd = timestampNs;

    Double_t timestampTmp = timestampNs;
    Double_t timestampDiff = timestampTmp - timestampPre;

    if (timestampDiff != 0) {
      counter_frequency++;
      Double_t frequency = 1 / timestampDiff * 1e6; // kcps
      frequencyAccum += frequency;
      hFrequency->Fill(frequency);
      //      std::cout << "Frequency: " << frequency << std::endl;
    }
    timestampPre = timestampNs;
   
  }
  frequencyAve = frequencyAccum / counter_frequency;
  Double_t runTime = timestampEnd - timestampInit; // ns
  runTime /= 1e9;                                  // s
  std::cout << "DAQ acquisition time: " << runTime << " s" << std::endl;
  std::cout << "Trigger Rate: " << frequencyAve << " kcps" << std::endl;
  hFrequency->Draw();
}
