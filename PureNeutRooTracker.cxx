#include "PureNeutRooTracker.hxx"

NRooTrackerVtx::NRooTrackerVtx(){
  StdHepPdg = 0;
  StdHepStatus = 0;
  StdHepFd = 0;
  StdHepLd = 0;
  StdHepFm = 0;
  StdHepLm = 0;
  NEipvc = 0;
  NEiorgvc = 0;
  NEiflgvc = 0;
  NEicrnvc = 0;
  NEiflgvert = 0;
  NEabspvert = 0;
  NEabstpvert = 0;
  NEipvert = 0;
  NEiverti = 0;
  NEivertf = 0;
  NFiflag = 0;
  NFx = 0;
  NFy = 0;
  NFz = 0;
  NFpx = 0;
  NFpy = 0;
  NFpz = 0;
  NFe = 0;
  NFfirststep = 0;
  NFecms2 = 0;
}
NRooTrackerVtx::~NRooTrackerVtx(){
  if(StdHepPdg) { delete StdHepPdg; }
  if(StdHepStatus) { delete StdHepStatus; }
  if(StdHepFd) { delete StdHepFd; }
  if(StdHepLd) { delete StdHepLd; }
  if(StdHepFm) { delete StdHepFm; }
  if(StdHepLm) { delete StdHepLm; }
  if(NEipvc) { delete NEipvc; }
  if(NEiorgvc) { delete NEiorgvc; }
  if(NEiflgvc) { delete NEiflgvc; }
  if(NEicrnvc) { delete NEicrnvc; }
  if(NEiflgvert) { delete NEiflgvert; }
  if(NEabspvert) { delete NEabspvert; }
  if(NEabstpvert) { delete NEabstpvert; }
  if(NEipvert) { delete NEipvert; }
  if(NEiverti) { delete NEiverti; }
  if(NEivertf) { delete NEivertf; }
  if(NFiflag) { delete NFiflag; }
  if(NFx) { delete NFx; }
  if(NFy) { delete NFy; }
  if(NFz) { delete NFz; }
  if(NFpx) { delete NFpx; }
  if(NFpy) { delete NFpy; }
  if(NFpz) { delete NFpz; }
  if(NFe) { delete NFe; }
  if(NFfirststep) { delete NFfirststep; }
  if(NFecms2) { delete NFecms2; }
}

void NRooTrackerVtx::Reset(){
  EvtCode = "";
  EvtNum = 0;
  EvtXSec = 0;
  EvtDXSec = 0;
  EvtWght = 0;
  EvtProb = 0;
  ClearArray(EvtVtx);
  StdHepN = 0;

  if(StdHepPdg) { delete StdHepPdg;}
  StdHepPdg = 0;
  if(StdHepStatus) { delete StdHepStatus;}
  StdHepStatus = 0;
  ClearArray2D(StdHepX4);
  ClearArray2D(StdHepP4);
  ClearArray2D(StdHepPolz);
  if(StdHepFd) { delete StdHepFd; }
  StdHepFd = 0;
  if(StdHepLd) { delete StdHepLd; }
  StdHepLd = 0;
  if(StdHepFm) { delete StdHepFm; }
  StdHepFm = 0;
  if(StdHepLm) { delete StdHepLm; }
  StdHepLm = 0;

  NEnvc = 0;
  if(NEipvc){ delete NEipvc; }
  NEipvc = 0;
  ClearArray2D(NEpvc);
  if(NEiorgvc){ delete NEiorgvc; }
  NEiorgvc = 0;

  if(NEiflgvc){ delete NEiflgvc; }
  NEiflgvc = 0;
  if(NEicrnvc){ delete NEicrnvc; }
  NEicrnvc = 0;

  NEcrsx = 0;
  NEcrsy = 0;
  NEcrsz = 0;
  NEcrsphi = 0;

  NEnvert = 0;
  ClearArray2D(NEposvert);
  if(NEiflgvert) { delete NEiflgvert; }
  NEiflgvert = 0;

  NEnvcvert = 0;
  ClearArray2D(NEdirvert);
  if(NEabspvert) { delete NEabspvert; }
  NEabspvert = 0;
  if(NEabstpvert) { delete NEabstpvert; }
  NEabstpvert = 0;
  if(NEipvert) { delete NEipvert; }
  NEipvert = 0;
  if(NEiverti) { delete NEiverti; }
  NEiverti = 0;
  if(NEivertf) { delete NEivertf; }
  NEivertf = 0;

  NFnvert = 0;
  if(NFiflag) { delete NFiflag; }
  NFiflag = 0;
  if(NFx) { delete NFx; }
  NFx = 0;
  if(NFy) { delete NFy; }
  NFy = 0;
  if(NFz) { delete NFz; }
  NFz = 0;
  if(NFpx) { delete NFpx; }
  NFpx = 0;
  if(NFpy) { delete NFpy; }
  NFpy = 0;
  if(NFpz) { delete NFpz; }
  NFpz = 0;
  if(NFe) { delete NFe; }
  NFe = 0;
  if(NFfirststep) { delete NFfirststep; }
  NFfirststep = 0;

  NFnstep = 0;

  if(NFecms2) { delete NFecms2; }
  NFecms2 = 0;

  GeneratorName = "NEUT";
}

ClassImp(NRooTrackerVtx);
