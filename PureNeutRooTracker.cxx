#include <sstream>

#include "PureNeutRooTracker.hxx"

namespace {
std::string Int2Str(Int_t i){
  std::stringstream ss("");
  ss << i;
  return ss.str();
}
}

NRooTrackerVtx::NRooTrackerVtx(){
  EvtCode = new TString("");
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
  if(GeneratorName){ delete GeneratorName; }
}

void NRooTrackerVtx::Reset(){

  (*EvtCode) = "";
  EvtNum = 0;
  EvtXSec = 0;
  EvtDXSec = 0;
  EvtWght = 0;
  EvtProb = 0;
  ClearArray(EvtVtx);

  StdHepN = 0;

  ClearPointer(StdHepPdg,kNStdHepNPmax);
  ClearPointer(StdHepStatus,kNStdHepNPmax);
  ClearArray2D(StdHepX4);
  ClearArray2D(StdHepP4);
  ClearArray2D(StdHepPolz);
  ClearPointer(StdHepFd,kNStdHepNPmax);
  ClearPointer(StdHepLd,kNStdHepNPmax);
  ClearPointer(StdHepFm,kNStdHepNPmax);
  ClearPointer(StdHepLm,kNStdHepNPmax);

  NEnvc = 0;

  ClearPointer(NEipvc,kNEmaxvc);
  ClearArray2D(NEpvc);
  ClearPointer(NEiorgvc,kNEmaxvc);
  ClearPointer(NEiflgvc,kNEmaxvc);
  ClearPointer(NEicrnvc,kNEmaxvc);

  NEcrsx = 0;
  NEcrsy = 0;
  NEcrsz = 0;
  NEcrsphi = 0;

  NEnvert = 0;

  ClearArray2D(NEposvert);
  ClearPointer(NEiflgvert,kNEmaxvert);

  NEnvcvert = 0;
  ClearArray2D(NEdirvert);
  ClearPointer(NEabspvert,kNEmaxvertp);
  ClearPointer(NEabstpvert,kNEmaxvertp);
  ClearPointer(NEipvert,kNEmaxvertp);
  ClearPointer(NEiverti,kNEmaxvertp);
  ClearPointer(NEivertf,kNEmaxvertp);

  NFnvert = 0;
  ClearPointer(NFiflag,kNFMaxNucleonVert);
  ClearPointer(NFx,kNFMaxNucleonVert);
  ClearPointer(NFy,kNFMaxNucleonVert);
  ClearPointer(NFz,kNFMaxNucleonVert);
  ClearPointer(NFpx,kNFMaxNucleonVert);
  ClearPointer(NFpy,kNFMaxNucleonVert);
  ClearPointer(NFpz,kNFMaxNucleonVert);
  ClearPointer(NFe,kNFMaxNucleonVert);
  ClearPointer(NFfirststep,kNFMaxNucleonVert);

  NFnstep = 0;

  ClearPointer(NFecms2,kNFMaxNucleonSteps);

  (*GeneratorName) = "NEUT";
}

void NRooTrackerVtx::AddBranches(TTree* &tree){
  std::string NStdHepNPmaxstr = Int2Str(kNStdHepNPmax);
  std::string NEmaxvcstr = Int2Str(kNEmaxvc);
  std::string NEmaxvertstr = Int2Str(kNEmaxvert);
  std::string NEmaxvertpstr = Int2Str(kNEmaxvertp);
  std::string NFMaxNucleonVert = Int2Str(kNFMaxNucleonVert);
  std::string NFMaxNucleonSteps = Int2Str(kNFMaxNucleonSteps);

  tree->Branch("EvtCode", &EvtCode);
  tree->Branch("EvtNum", &EvtNum,"EvtNum/I");
  tree->Branch("EvtXSec", &EvtXSec,"EvtXSec/D");
  tree->Branch("EvtDXSec", &EvtDXSec,"EvtDXSec/D");
  tree->Branch("EvtWght", &EvtWght,"EvtWght/D");
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
  tree->Branch("GeneratorName", &GeneratorName);

}
ClassImp(NRooTrackerVtx);
