#include <sstream>

#include "LUtils/Utils.hxx"

#include "PureNeutRooTracker.hxx"

char const * NEUTStatusCodes[] = {
  "DETERMINED LATER PROCEDURE",
  "DECAY TO OTHER PARTICLE",
  "ESCAPE FROM DETECTOR",
  "ABSORPTION",
  "CHARGE EXCHANGE",
  "STOP AND NOT CONSIDER IN M.C.",
  "E.M. SHOWER",
  "HADRON PRODUCTION",
  "QUASI-ELASTIC SCATTER",
  "FORWARD (ELASTIC-LIKE) SCATTER"
};

NRooTrackerVtx::NRooTrackerVtx(){
  EvtCode = new TObjString("");
  StdHepPdg = new Int_t[kNStdHepNPmax];
  StdHepStatus = new Int_t[kNStdHepNPmax];
  StdHepFd = new Int_t[kNStdHepNPmax];
  StdHepLd = new Int_t[kNStdHepNPmax];
  StdHepFm = new Int_t[kNStdHepNPmax];
  StdHepLm = new Int_t[kNStdHepNPmax];

  NEipvc = new Int_t[kNEmaxvc];
  NEiorgvc = new Int_t[kNEmaxvc];
  NEiflgvc = new Int_t[kNEmaxvc];
  NEicrnvc = new Int_t[kNEmaxvc];

  NEiflgvert = new Int_t[kNEmaxvert];

  NEabspvert = new Float_t[kNEmaxvertp];
  NEabstpvert = new Float_t[kNEmaxvertp];
  NEipvert = new Int_t[kNEmaxvertp];
  NEiverti = new Int_t[kNEmaxvertp];
  NEivertf = new Int_t[kNEmaxvertp];
#ifdef HAVE_NUCLEON_FSI_TRACKING
  NFiflag = new Int_t[kNFMaxNucleonVert];
  NFx = new Float_t[kNFMaxNucleonVert];
  NFy = new Float_t[kNFMaxNucleonVert];
  NFz = new Float_t[kNFMaxNucleonVert];
  NFpx = new Float_t[kNFMaxNucleonVert];
  NFpy = new Float_t[kNFMaxNucleonVert];
  NFpz = new Float_t[kNFMaxNucleonVert];
  NFe = new Float_t[kNFMaxNucleonVert];
  NFfirststep = new Int_t[kNFMaxNucleonVert];

  NFecms2 = new Float_t[kNFMaxNucleonSteps];
  NFProb = new Float_t[kNFMaxNucleonSteps];
#endif
  GeneratorName = new TString("NEUT");
}

NRooTrackerVtx::~NRooTrackerVtx(){
  if(EvtCode){ delete EvtCode; }
  if(StdHepPdg) { delete [] StdHepPdg; }
  if(StdHepStatus) { delete [] StdHepStatus; }
  if(StdHepFd) { delete [] StdHepFd; }
  if(StdHepLd) { delete [] StdHepLd; }
  if(StdHepFm) { delete [] StdHepFm; }
  if(StdHepLm) { delete [] StdHepLm; }
  if(NEipvc) { delete [] NEipvc; }
  if(NEiorgvc) { delete [] NEiorgvc; }
  if(NEiflgvc) { delete [] NEiflgvc; }
  if(NEicrnvc) { delete [] NEicrnvc; }
  if(NEiflgvert) { delete [] NEiflgvert; }
  if(NEabspvert) { delete [] NEabspvert; }
  if(NEabstpvert) { delete [] NEabstpvert; }
  if(NEipvert) { delete [] NEipvert; }
  if(NEiverti) { delete [] NEiverti; }
  if(NEivertf) { delete [] NEivertf; }
#ifdef HAVE_NUCLEON_FSI_TRACKING
  if(NFiflag) { delete [] NFiflag; }
  if(NFx) { delete [] NFx; }
  if(NFy) { delete [] NFy; }
  if(NFz) { delete [] NFz; }
  if(NFpx) { delete [] NFpx; }
  if(NFpy) { delete [] NFpy; }
  if(NFpz) { delete [] NFpz; }
  if(NFe) { delete [] NFe; }
  if(NFfirststep) { delete [] NFfirststep; }
  if(NFecms2) { delete [] NFecms2; }
  if(NFProb) { delete [] NFProb; }
#endif
  if(GeneratorName){ delete GeneratorName; }
}

void NRooTrackerVtx::Reset(){

  (*EvtCode) = "";
  EvtNum = 0;
  EvtXSec = 0;
  EvtDXSec = 0;
  EvtWght = 0;
  EvtHistWght = 0;
  NEntriesInFile = 0;
  EvtProb = 0;
  Utils::ClearArray(EvtVtx);

  StdHepN = 0;

  Utils::ClearPointer(StdHepPdg,kNStdHepNPmax);
  Utils::ClearPointer(StdHepStatus,kNStdHepNPmax);
  Utils::ClearArray2D(StdHepX4);
  Utils::ClearArray2D(StdHepP4);
  Utils::ClearArray2D(StdHepPolz);
  Utils::ClearPointer(StdHepFd,kNStdHepNPmax);
  Utils::ClearPointer(StdHepLd,kNStdHepNPmax);
  Utils::ClearPointer(StdHepFm,kNStdHepNPmax);
  Utils::ClearPointer(StdHepLm,kNStdHepNPmax);

  IsBound = 0;

  NEnvc = 0;

  Utils::ClearPointer(NEipvc,kNEmaxvc);
  Utils::ClearArray2D(NEpvc);
  Utils::ClearPointer(NEiorgvc,kNEmaxvc);
  Utils::ClearPointer(NEiflgvc,kNEmaxvc);
  Utils::ClearPointer(NEicrnvc,kNEmaxvc);

  NEcrsx = 0;
  NEcrsy = 0;
  NEcrsz = 0;
  NEcrsphi = 0;

  NEnvert = 0;

  Utils::ClearArray2D(NEposvert);
  Utils::ClearPointer(NEiflgvert,kNEmaxvert);

  NEnvcvert = 0;
  Utils::ClearArray2D(NEdirvert);
  Utils::ClearPointer(NEabspvert,kNEmaxvertp);
  Utils::ClearPointer(NEabstpvert,kNEmaxvertp);
  Utils::ClearPointer(NEipvert,kNEmaxvertp);
  Utils::ClearPointer(NEiverti,kNEmaxvertp);
  Utils::ClearPointer(NEivertf,kNEmaxvertp);
#ifdef HAVE_NUCLEON_FSI_TRACKING
  NFnvert = 0;
  Utils::ClearPointer(NFiflag,kNFMaxNucleonVert);
  Utils::ClearPointer(NFx,kNFMaxNucleonVert);
  Utils::ClearPointer(NFy,kNFMaxNucleonVert);
  Utils::ClearPointer(NFz,kNFMaxNucleonVert);
  Utils::ClearPointer(NFpx,kNFMaxNucleonVert);
  Utils::ClearPointer(NFpy,kNFMaxNucleonVert);
  Utils::ClearPointer(NFpz,kNFMaxNucleonVert);
  Utils::ClearPointer(NFe,kNFMaxNucleonVert);
  Utils::ClearPointer(NFfirststep,kNFMaxNucleonVert);

  NFnstep = 0;

  Utils::ClearPointer(NFecms2,kNFMaxNucleonSteps);
  Utils::ClearPointer(NFProb,kNFMaxNucleonSteps);
#endif
  (*GeneratorName) = "NEUT";
}

void NRooTrackerVtx::AddBranches(TTree* &tree,
  bool SaveIsBound,
  bool SaveStruckNucleonPDG){

  std::string NStdHepNPmaxstr = Utils::int2str(kNStdHepNPmax);
  std::string NEmaxvcstr = Utils::int2str(kNEmaxvc);
  std::string NEmaxvertstr = Utils::int2str(kNEmaxvert);
  std::string NEmaxvertpstr = Utils::int2str(kNEmaxvertp);

#ifdef HAVE_NUCLEON_FSI_TRACKING
  std::string NFMaxNucleonVert = Utils::int2str(kNFMaxNucleonVert);
  std::string NFMaxNucleonSteps = Utils::int2str(kNFMaxNucleonSteps);
#endif

  tree->Branch("EvtCode", &EvtCode);
  tree->Branch("EvtNum", &EvtNum,"EvtNum/I");
  tree->Branch("EvtXSec", &EvtXSec,"EvtXSec/D");
  tree->Branch("EvtDXSec", &EvtDXSec,"EvtDXSec/D");
  tree->Branch("EvtWght", &EvtWght,"EvtWght/D");
  tree->Branch("EvtHistWght", &EvtHistWght,"EvtHistWght/D");
  tree->Branch("NEntriesInFile", &NEntriesInFile, "NEntriesInFile/D");
  tree->Branch("EvtProb", &EvtProb,"EvtProb/D");
  tree->Branch("EvtVtx", EvtVtx,"EvtVtx[4]/D");
  tree->Branch("StdHepN", &StdHepN,"StdHepN/I");
  tree->Branch("StdHepPdg", StdHepPdg,"StdHepPdg[StdHepN]/I");
  tree->Branch("StdHepStatus", StdHepStatus,"StdHepStatus[StdHepN]/I");
  tree->Branch("StdHepX4", StdHepX4,
    ("StdHepX4["+NStdHepNPmaxstr+"][4]/D").c_str());
  tree->Branch("StdHepP4", StdHepP4,
    ("StdHepP4["+NStdHepNPmaxstr+"][4]/D").c_str());
  tree->Branch("StdHepPolz", StdHepPolz,
    ("StdHepPolz["+NStdHepNPmaxstr+"][3]/D").c_str());
  tree->Branch("StdHepFd", StdHepFd,"StdHepFd[StdHepN]/I");
  tree->Branch("StdHepLd", StdHepLd,"StdHepLd[StdHepN]/I");
  tree->Branch("StdHepFm", StdHepFm,"StdHepFm[StdHepN]/I");
  tree->Branch("StdHepLm", StdHepLm,"StdHepLm[StdHepN]/I");

  if(SaveIsBound){ tree->Branch("IsBound", &IsBound, "IsBound/I"); }
  if(SaveStruckNucleonPDG){ tree->Branch("StruckNucleonPDG", &StruckNucleonPDG, "StruckNucleonPDG/I"); }

  tree->Branch("NEnvc", &NEnvc,"NEnvc/I");
  tree->Branch("NEipvc", NEipvc,"NEipvc[NEnvc]/I");
  tree->Branch("NEpvc", NEpvc,("NEpvc["+NEmaxvcstr+"][3]/F").c_str());
  tree->Branch("NEiorgvc", NEiorgvc,"NEiorgvc[NEnvc]/I");
  tree->Branch("NEiflgvc", NEiflgvc,"NEiflgvc[NEnvc]/I");
  tree->Branch("NEicrnvc", NEicrnvc,"NEicrnvc[NEnvc]/I");
  tree->Branch("NEcrsx", &NEcrsx,"NEcrsx/F");
  tree->Branch("NEcrsy", &NEcrsy,"NEcrsy/F");
  tree->Branch("NEcrsz", &NEcrsz,"NEcrsz/F");
  tree->Branch("NEcrsphi", &NEcrsphi,"NEcrsphi/F");
  tree->Branch("NEnvert", &NEnvert,"NEnvert/I");
  tree->Branch("NEposvert", NEposvert,
    ("NEposvert["+NEmaxvertstr+"][3]/F").c_str());
  tree->Branch("NEiflgvert", NEiflgvert,"NEiflgvert[NEnvert]/I");
  tree->Branch("NEnvcvert", &NEnvcvert,"NEnvcvert");
  tree->Branch("NEdirvert", NEdirvert,
    ("NEdirvert["+NEmaxvertpstr+"][3]/F").c_str());
  tree->Branch("NEabspvert", NEabspvert,"NEabspvert[NEnvcvert]/F");
  tree->Branch("NEabstpvert", NEabstpvert,"NEabstpvert[NEnvcvert]/F");
  tree->Branch("NEipvert", NEipvert,"NEipvert[NEnvcvert]/I");
  tree->Branch("NEiverti", NEiverti,"NEiverti[NEnvcvert]/I");
  tree->Branch("NEivertf", NEivertf,"NEivertf[NEnvcvert]/I");
#ifdef HAVE_NUCLEON_FSI_TRACKING
  tree->Branch("NFnvert", &NFnvert,"NFnvert/I");
  tree->Branch("NFiflag", NFiflag,"NFiflag[NFnvert]/I");
  tree->Branch("NFx", NFx,"NFx[NFnvert]/F");
  tree->Branch("NFy", NFy,"NFy[NFnvert]/F");
  tree->Branch("NFz", NFz,"NFz[NFnvert]/F");
  tree->Branch("NFpx", NFpx,"NFpx[NFnvert]/F");
  tree->Branch("NFpy", NFpy,"NFpy[NFnvert]/F");
  tree->Branch("NFpz", NFpz,"NFpz[NFnvert]/F");
  tree->Branch("NFe", NFe,"NFe[NFnvert]/F");
  tree->Branch("NFfirststep", NFfirststep,"NFfirststep[NFnvert]/I");
  tree->Branch("NFnstep", &NFnstep,"NFnstep/I");
  tree->Branch("NFecms2", NFecms2,"NFecms2[NFnstep]/F");
  tree->Branch("NFProb",NFProb,"NFProb[NFnstep]/F");
#endif
  tree->Branch("GeneratorName", &GeneratorName);
}

NRooTrackerVtxB::NRooTrackerVtxB(){
  EvtCode = new TObjString("");
  StdHepPdg = new Int_t[kNStdHepNPmax];
  StdHepStatus = new Int_t[kNStdHepNPmax];

}

NRooTrackerVtxB::~NRooTrackerVtxB(){
  if(EvtCode){ delete EvtCode; }
  if(StdHepPdg) { delete [] StdHepPdg; }
  if(StdHepStatus) { delete [] StdHepStatus; }

}

void NRooTrackerVtxB::Reset(){

  (*EvtCode) = "";
  EvtNum = 0;

  StdHepN = 0;

  Utils::ClearPointer(StdHepPdg,kNStdHepNPmax);
  Utils::ClearPointer(StdHepStatus,kNStdHepNPmax);
  Utils::ClearArray2D(StdHepP4);

  IsBound = 0;
  StruckNucleonPDG = 0;

}

void NRooTrackerVtxB::AddBranches(TTree* &tree,
  bool SaveIsBound,
  bool SaveStruckNucleonPDG){

  std::string NStdHepNPmaxstr = Utils::int2str(kNStdHepNPmax);

  tree->Branch("EvtCode", &EvtCode);
  tree->Branch("EvtNum", &EvtNum,"EvtNum/I");
  tree->Branch("StdHepN", &StdHepN,"StdHepN/I");
  tree->Branch("StdHepPdg", StdHepPdg,"StdHepPdg[StdHepN]/I");
  tree->Branch("StdHepStatus", StdHepStatus,"StdHepStatus[StdHepN]/I");
  tree->Branch("StdHepP4", StdHepP4,
    ("StdHepP4["+NStdHepNPmaxstr+"][4]/D").c_str());

  if(SaveIsBound){ tree->Branch("IsBound", &IsBound, "IsBound/I"); }
  if(SaveStruckNucleonPDG){ tree->Branch("StruckNucleonPDG", &StruckNucleonPDG, "StruckNucleonPDG/I"); }

}

ClassImp(NRooTrackerVtxB);
ClassImp(NRooTrackerVtx);
