#include <sstream>
#include <iostream>

#include "TChain.h"
#include "TTree.h"
#include "TFile.h"

#include "neutvect.h"
#include "neutvtx.h"

#include "CLITools.hxx"
#include "PureGenUtils.hxx"

#include "PureNeutRooTracker.hxx"

namespace {

std::ostream& operator<<(std::ostream& o, const TLorentzVector& tlv){
  return o << "[" << tlv.X() << ", " << tlv.Y() << ", " << tlv.Z() << ", "
    << tlv.T() <<  "]";
}

std::string outfname;
std::string inpfdescript;
bool ObjectOutput = false;
bool useSimpleTree = false;
bool OutputInGev = false;
bool SaveIsBound = false;
int verbosity = 0;
long MaxEntries = 0;
}

int NeutToRooTracker(const char* InputFileDescriptor){

//Input stuff
  TChain* NeutTree = new TChain("neuttree");

  int nFiles = NeutTree->Add(InputFileDescriptor);

  if(!nFiles){
    std::cout << "\"" << InputFileDescriptor << "\" matched 0 input files."
      << std::endl;
    return 2;
  }

  NeutVect *vector = new NeutVect();
  NeutTree->SetBranchAddress("vectorbranch",&vector);
  NeutVtx *vertex = new NeutVtx();
  NeutTree->SetBranchAddress("vertexbranch",&vertex);

  long NEntries = NeutTree->GetEntries();
  long FilledEntries = 0;

  if(!NEntries){
    std::cout << "Failed to find any entries (" << NEntries
      << ")." << std::endl;
    return 4;
  }
  std::cout << "Reading " << nFiles << " input files with "
    << NEntries << " entries." << std::endl;

//Output stuff
  TFile* outFile = new TFile(outfname.c_str(),"CREATE");

  if(!outFile->IsOpen()){
    std::cerr << "[ERROR]: Couldn't open output file: " << outfname
    << std::endl;
    return 8;
  } else {
    std::cout << "Created output file: " << outFile->GetName() << std::endl;
  }

  TTree* rooTrackerTree = new TTree("nRooTracker","Pure NEUT RooTracker");
  NRooTrackerVtx* outRooTracker = new NRooTrackerVtx();
  if(ObjectOutput){
    rooTrackerTree->Branch("nRooTracker", &outRooTracker);
  } else {
    outRooTracker->AddBranches(rooTrackerTree, useSimpleTree, SaveIsBound);
  }

  long long doEntries = (MaxEntries==-1) ?
    NEntries : (std::min(MaxEntries, NEntries));

  for(long entryNum = 0; entryNum < doEntries; ++entryNum){

    if( entryNum && (!(entryNum%10000)) ){
      std::cout << "Read " << entryNum << " entries." << std::endl;
    }

    if(!entryNum){ // cout for my sanity
      std::cout << "Reading first entry... " << std::flush;
    }
    NeutTree->GetEntry(entryNum);
    if(!entryNum){ // cout for my sanity
      std::cout << "Read first entry!" << std::endl;
    }

    //**************************************************
    //Event Level
    std::stringstream ss("");
    ss << vector->Mode;
    outRooTracker->EvtCode->SetString(ss.str().c_str());
    outRooTracker->EvtNum = vector->EventNo;
    outRooTracker->EvtXSec = vector->Totcrs;

    outRooTracker->NEcrsx = vector->Crsx;
    outRooTracker->NEcrsy = vector->Crsy;
    outRooTracker->NEcrsz = vector->Crsz;
    outRooTracker->NEcrsphi = vector->Crsphi;

    (void)outRooTracker->EvtVtx[0];
    (void)outRooTracker->EvtVtx[0];
    (void)outRooTracker->EvtVtx[0];
    (void)outRooTracker->EvtVtx[0];

    //**************************************************
    //StdHepN Particles

    if(verbosity>3){
      std::cout << "**"
"******************************************************************************"
    << std::endl;
    vector->Dump();
      std::cout << "**"
"******************************************************************************"
    << std::endl;
    }

    if(verbosity > 1){
      std::cout << "Vector #:" << entryNum << std::endl;
      std::cout << "\tNPrimary: " << vector->Npart() << std::endl;
      std::cout << "\tNPrimary: " << vector->Nprimary() << std::endl;
      std::cout << "\tEvtCode: " << outRooTracker->EvtCode->String() << std::endl;
      std::cout << "\tEvtXSec: " << outRooTracker->EvtXSec << std::endl;
      std::cout << "\tNEcrsx: " << outRooTracker->NEcrsx << std::endl;
      std::cout << "\tNEcrsy: " << outRooTracker->NEcrsy << std::endl;
      std::cout << "\tNEcrsz: " << outRooTracker->NEcrsz << std::endl;
      std::cout << "\tNEcrsphi: " << outRooTracker->NEcrsphi << std::endl;
      std::cout << "\tNOutgoing Particles: " << vector->Npart() << std::endl;
      std::cout << std::endl;
    }

    //Fill the particle info
    outRooTracker->StdHepN = vector->Npart()+1;//Need to include nuclear target.
    outRooTracker->IsBound = vector->Ibound;

    for(int partNum = 0, saveInd = 0; partNum < vector->Npart();
      ++partNum, ++saveInd){
      const NeutPart& part = (*vector->PartInfo(partNum));

      if(partNum == 1){
        // As in TNeutOutput, to emulate neutgeom
        // StdHepX[1] is the target
        outRooTracker->StdHepPdg[saveInd] =
          PGUtils::MakeNuclearPDG(vector->TargetZ, vector->TargetA);
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] = vector->TargetA;
        if(verbosity > 1){
          std::cout << "TARGET"
            << "\n\tA: " << vector->TargetA
            << "\n\tZ: " << vector->TargetZ
            << "\n\tPDG: " << outRooTracker->StdHepPdg[saveInd]
            << "\n\tStatus: " << outRooTracker->StdHepStatus[saveInd]
            << "\n\tHEPP4: " << PGUtils::PrintArray(
              outRooTracker->StdHepP4[saveInd])
            << "\n\tIsBoundTarget: " << outRooTracker->IsBound
            << std::endl;
        }

        //Now save the struck nucleon properties
        ++saveInd;
      }

      outRooTracker->StdHepPdg[saveInd] = part.fPID;

      switch(part.fStatus){
        case -1:{
          outRooTracker->StdHepStatus[saveInd] = 0;
          break;
        }
        case 0:{
          outRooTracker->StdHepStatus[saveInd] = 1;
          break;
        }
        case 2:{
          outRooTracker->StdHepStatus[saveInd] = 1;
          break;
        }
        default:{
          if(verbosity > 1){
            std::cout << "--Found other neutcode: " << part.fStatus << std::endl;
            outRooTracker->StdHepStatus[saveInd] =
              (part.fStatus==1)?-1:part.fStatus;
          }
        }
      }
      if(partNum == 1 && part.fStatus == -1){
        outRooTracker->StdHepStatus[saveInd] = 11; //To sync with GENIE code for
        //Struck Nucleon.
      }

      if(OutputInGev){
        static constexpr float MeVToGeV = 1.0/1000;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPx] = part.fP.X()*MeVToGeV;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPy] = part.fP.Y()*MeVToGeV;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPz] = part.fP.Z()*MeVToGeV;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] = part.fP.E()*MeVToGeV;
      } else {
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPx] = part.fP.X();
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPy] = part.fP.Y();
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPz] = part.fP.Z();
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] = part.fP.E();
      }

      if(verbosity > 1){
        std::cout << ((saveInd>1)?"Particle:":"Incoming Neutrino:")
          << "\n\tStdHEPPDG: " << outRooTracker->StdHepPdg[saveInd]
          << "\n\tStdHEPStatus: " << outRooTracker->StdHepStatus[saveInd]
          << "\n\tStdHEPP4: " << PGUtils::PrintArray(
            outRooTracker->StdHepP4[saveInd])
          << std::endl;
      }

      //Not implemented in NEUT
      (void)outRooTracker->StdHepX4[saveInd][kNStdHepIdxX];
      (void)outRooTracker->StdHepX4[saveInd][kNStdHepIdxY];
      (void)outRooTracker->StdHepX4[saveInd][kNStdHepIdxZ];
      (void)outRooTracker->StdHepX4[saveInd][kNStdHepIdxT];
      (void)outRooTracker->StdHepPolz[saveInd][kNStdHepIdxPx];
      (void)outRooTracker->StdHepPolz[saveInd][kNStdHepIdxPy];
      (void)outRooTracker->StdHepPolz[saveInd][kNStdHepIdxPz];
      (void)outRooTracker->StdHepPolz[saveInd][kNStdHepIdxE];
    }

    //**************************************************
    //NEUT VCWork Particles
    //Not yet implemented.
    outRooTracker->NEnvc = 1;
    for(int i = 0; i < outRooTracker->NEnvc; ++i){
      (void)outRooTracker->NEpvc[i];
      (void)outRooTracker->NEiorgvc[i];
      (void)outRooTracker->NEiflgvc[i];
      (void)outRooTracker->NEicrnvc[i];
    }

    //**************************************************
    //NEUT Pion FSI interaction history
    //Not yet implemented
    outRooTracker->NEnvert = 1;

    for(int i = 0; i < outRooTracker->NEnvert; ++i){
      (void)outRooTracker->NEposvert[i];
      (void)outRooTracker->NEiflgvert[i];
    }

    outRooTracker->NEnvcvert = 1;
    for(int i = 0; i < outRooTracker->NEnvcvert; ++i){
      (void)outRooTracker->NEdirvert[i];
      (void)outRooTracker->NEabspvert[i];
      (void)outRooTracker->NEabstpvert[i];
      (void)outRooTracker->NEipvert[i];
      (void)outRooTracker->NEiverti[i];
      (void)outRooTracker->NEivertf[i];
    }

    //**************************************************
    //NEUT Nucleon FSI interaction history
    //Not yet implemented
    outRooTracker->NFnvert = 1;
    (void)outRooTracker->NFiflag[0];
    (void)outRooTracker->NFx[0];
    (void)outRooTracker->NFy[0];
    (void)outRooTracker->NFz[0];
    (void)outRooTracker->NFpx[0];
    (void)outRooTracker->NFpy[0];
    (void)outRooTracker->NFpz[0];
    (void)outRooTracker->NFe[0];
    (void)outRooTracker->NFfirststep[0];

    outRooTracker->NFnstep = 1;
    (void)outRooTracker->NFecms2[0];

    rooTrackerTree->Fill();
    FilledEntries++;
    if(verbosity > 1){
      std::cout << "*****************Filled*****************\n" << std::endl;
    }
    outRooTracker->Reset();
  }
  std::cout << "Wrote " << FilledEntries << " events to disk." << std::endl;
  rooTrackerTree->Write();
  outFile->Close();
  return 0;
}

namespace {

void SetOpts(){
  CLIArgs::AddOpt("-h","--help", false,
    [&] (std::string const &opt) -> bool {
      CLIArgs::SayRunLike();
      exit(0);
    });

  CLIArgs::AddOpt("-i", "--input-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Reading from file descriptor : " << opt << std::endl;
      inpfdescript = opt;
      return true;
    }, true,[](){},"<TChain::Add descriptor>");

  CLIArgs::AddOpt("-o", "--output-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Writing to File: " << opt << std::endl;
      outfname = opt;
      return true;
    }, false,
    [&](){outfname = "vector.ntrac.root";},
    "<File Name>{default=vector.ntrac.root}");

  CLIArgs::AddOpt("-n", "--nentries", true,
    [&] (std::string const &opt) -> bool {
      long vbhold;
      if(PGUtils::str2int(vbhold,opt.c_str()) == PGUtils::STRINT_SUCCESS){
        if(vbhold != -1){
          std::cout << "\t--Looking at, at most, " << vbhold << " entries."
            << std::endl;
        }
        MaxEntries = vbhold;
        return true;
      }
      return false;
    }, false,
    [&](){MaxEntries = -1;}, "<-1>: Read all {default=-1}");

  CLIArgs::AddOpt("-v", "--verbosity", true,
    [&] (std::string const &opt) -> bool {
      int vbhold;
      if(PGUtils::str2int(vbhold,opt.c_str()) == PGUtils::STRINT_SUCCESS){
        std::cout << "\t--Verbosity: " << vbhold << std::endl;
        verbosity = vbhold;
        return true;
      }
      return false;
    }, false,
    [&](){verbosity = 0;}, "<0-4>{default=0}");

  CLIArgs::AddOpt("-s", "--simple-tree", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Using simple tree." << std::endl;
      useSimpleTree = true;
      return true;
    }, false,
    [&](){useSimpleTree = false;}, "Only output StdHep.");

  CLIArgs::AddOpt("-G", "--GeV-mode", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Outputting in GeV." << std::endl;
      OutputInGev = true;
      return true;
    }, false,
    [&](){OutputInGev = false;}, "Use GeV rather than MeV.");

  CLIArgs::AddOpt("-O", "--objectify-output", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Using simple tree." << std::endl;
      ObjectOutput = true;
      return true;
    }, false,
    [&](){ObjectOutput = false;}, "Output object tree.");

  CLIArgs::AddOpt("-b", "--save-isbound", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Adding IsBound branch to output tree." << std::endl;
      SaveIsBound = true;
      return true;
    }, false,
    [&](){SaveIsBound = false;}, "Output IsBound Branch");
}
}

int main(int argc, char const * argv[]){
  SetOpts();

  CLIArgs::AddArguments(argc,argv);
  if(!CLIArgs::HandleArgs()){
    CLIArgs::SayRunLike();
    return 1;
  }

  int rtncode = 0;
  if((rtncode = NeutToRooTracker(inpfdescript.c_str()))){
    CLIArgs::SayRunLike();
  }
  return rtncode;
}
