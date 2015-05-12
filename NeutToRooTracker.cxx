#include <sstream>
#include <iostream>

#include "TChain.h"
#include "TTree.h"
#include "TFile.h"

#include "neutvect.h"
#include "neutvtx.h"

#include "PureNeutRooTracker.hxx"

namespace {
inline long MakeNuclearPDG(int Z, int A){
  // 100%03d%03d0 % Z, A
  return 1000000000L + Z*10000L + A*10L;
}

std::ostream& operator<<(std::ostream& o, const TLorentzVector& tlv){
  return o << "[" << tlv.X() << ", " << tlv.Y() << ", " << tlv.Z() << ", "
    << tlv.T() <<  "]";
}

std::string outfname;
bool ObjectOutput = false;
int verbosity = 0;

template<typename T, size_t N>
std::string SayArray(const T (&arr)[N]){
  std::stringstream ss("");
  ss << "[";
  for(size_t i = 0; i < N; ++i){
    ss << arr[i] << ((i==N-1)?"":", ");
  }
  ss << "]";
  return ss.str();
}
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

  std::cout << "Reading " << nFiles << " input files with "
    << NEntries << " entries." << std::endl;

//Output stuff
  TFile* outFile = new TFile(outfname.c_str(),"CREATE");

  if(!outFile->IsOpen()){
    std::cerr << "[ERROR]: Couldn't open output file: " << outfname
    << std::endl;
    return 4;
  } else {
    std::cout << "Created output file: " << outFile->GetName() << std::endl;
  }

  TTree* rooTrackerTree = new TTree("nRooTracker","Pure NEUT RooTracker");
  NRooTrackerVtx* outRooTracker = new NRooTrackerVtx();
  if(ObjectOutput){
    rooTrackerTree->Branch("nRooTracker", &outRooTracker);
  } else {
    outRooTracker->AddBranches(rooTrackerTree);
  }

  for(long entryNum = 0; entryNum < NEntries; ++entryNum){

    if( entryNum && (!(entryNum%5000)) ){
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
    (*outRooTracker->EvtCode) = ss.str();
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
    //Work out how many good particles there are
    int NGoodStatus = 0;
    for(int partNum = 0; partNum < vector->Npart(); ++partNum){
      const NeutPart& part = (*vector->PartInfo(partNum));
      if( !partNum ||
            (part.fIsAlive &&
              ( (part.fStatus == -1)|| // Initial State
                (part.fStatus == 0) || // Final State (Trackable)
                (part.fStatus == 2)    // Escape from detector (Trackable)
              )
            ) ){
        NGoodStatus++;
      }
    }

    if(verbosity > 1){
      std::cout << "Vector #:" << entryNum << std::endl;
      std::cout << "\tEvtCode: " << outRooTracker->EvtCode << std::endl;
      std::cout << "\tEvtXSec: " << outRooTracker->EvtXSec << std::endl;
      std::cout << "\tNEcrsx: " << outRooTracker->NEcrsx << std::endl;
      std::cout << "\tNEcrsy: " << outRooTracker->NEcrsy << std::endl;
      std::cout << "\tNEcrsz: " << outRooTracker->NEcrsz << std::endl;
      std::cout << "\tNEcrsphi: " << outRooTracker->NEcrsphi << std::endl;
      std::cout << "\tNOutgoing Particles: " << (NGoodStatus-1) << std::endl;
      std::cout << std::endl;
    }

    //Fill the particle info
    outRooTracker->StdHepN = NGoodStatus+1;

    // As in TNeutOutput, to emulate neutgeom
    // StdHepX[1] is the target
    outRooTracker->StdHepPdg[1] =
      MakeNuclearPDG(vector->TargetZ, vector->TargetA);
    outRooTracker->StdHepP4[1][kNStdHepIdxE] = vector->TargetA;

    int storageNum = 0;
    for(int partNum = 0; partNum < vector->Npart(); ++partNum){
      const NeutPart& part = (*vector->PartInfo(partNum));

      if(partNum == 1){ //Skip the target descriptor that we have already
        if(verbosity > 1){
          std::cout << "TARGET"
            << "\n\tA: " << vector->TargetA
            << "\n\tZ: " << vector->TargetZ
            << "\n\tPDG: " << outRooTracker->StdHepPdg[storageNum]
            << "\n\tStatus: " << outRooTracker->StdHepStatus[storageNum]
            << "\n\tHEPP4: " << SayArray(outRooTracker->StdHepP4[storageNum])
            << std::endl;
        }
        storageNum++;   //already filled.
        continue;
      }
      if( partNum && // Incoming neutrino should always be saved
            (!part.fIsAlive ||
              !((part.fStatus == -1)||
               (part.fStatus == 0) ||
               (part.fStatus == 2)
              )
            ) ){
        continue;
      }

      outRooTracker->StdHepPdg[storageNum] = part.fPID;
      outRooTracker->StdHepStatus[storageNum] = (part.fStatus+1)?1:0;
      outRooTracker->StdHepP4[storageNum][kNStdHepIdxPx] = part.fP.X();
      outRooTracker->StdHepP4[storageNum][kNStdHepIdxPy] = part.fP.Y();
      outRooTracker->StdHepP4[storageNum][kNStdHepIdxPz] = part.fP.Z();
      outRooTracker->StdHepP4[storageNum][kNStdHepIdxE] = part.fP.E();


      if(verbosity > 1){
        std::cout << ((partNum)?"Outgoing Particle:":"Incoming Neutrino:")
          << "\n\tStdHEPPDG: " << outRooTracker->StdHepPdg[storageNum]
          << "\n\tStdHEPStatus: " << outRooTracker->StdHepStatus[storageNum]
          << "\n\tStdHEPP4: " << SayArray(outRooTracker->StdHepP4[storageNum])
          << std::endl;
      }

      //Not implemented in NEUT
      (void)outRooTracker->StdHepX4[storageNum][kNStdHepIdxX];
      (void)outRooTracker->StdHepX4[storageNum][kNStdHepIdxY];
      (void)outRooTracker->StdHepX4[storageNum][kNStdHepIdxZ];
      (void)outRooTracker->StdHepX4[storageNum][kNStdHepIdxT];
      (void)outRooTracker->StdHepPolz[storageNum][kNStdHepIdxPx];
      (void)outRooTracker->StdHepPolz[storageNum][kNStdHepIdxPy];
      (void)outRooTracker->StdHepPolz[storageNum][kNStdHepIdxPz];
      (void)outRooTracker->StdHepPolz[storageNum][kNStdHepIdxE];
      storageNum++;
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
    if(verbosity > 1){
      std::cout << "*****************Filled*****************\n" << std::endl;
    }
    outRooTracker->Reset();
  }
  rooTrackerTree->Write();
  outFile->Close();
  return 0;
}

void SayRunLike(const char* invoke_cmd){
  std::cout << "Run like:\n " << invoke_cmd << " <input_file_descriptor>"
   << " [<output_file_name=vector.ntrac.root>] [--OutputObject]"
   << "\n\t\"input_file_descriptor\" = any string that is a valid input"
   << "\n\t\tfor TChain::Add(const char*), e.g. myinputfiles_*.root"
   << "\n\tIf --OutputObject is specified the output tree will contain a single"
   " branch of the type NRooTrackerVtx" << std::endl;
}

bool IsHelp(std::string helpcli){
  if((helpcli == "--help") || (helpcli == "-h")  || (helpcli == "-?") ){
    return true;
  }
  return false;
}

int main(int argc, char* argv[]){
  //Too many args spoil the broth
  if((argc > 4) || (argc == 1)){ SayRunLike(argv[0]); return 1;}
  //If the user is asking for help
  if(argc == 2 && IsHelp(argv[1])){ SayRunLike(argv[0]); return 0; }


  if((std::string(argv[argc-1]) == "--OutputObject")){
    if(argc==2){
      std::cerr << "[ERROR]: You only specified a single argument and it was:\""
      << argv[argc-1] << "\". You must at least include an input file descripto"
      "r." << std::endl;
      SayRunLike(argv[0]);
      return 1;
    }
    ObjectOutput = true;
    std::cout << "Will write output in rootracker object format." << std::endl;
  }

  if(argc == (3 + int(ObjectOutput))){
    outfname = argv[2];
  } else {
    outfname = "vector.ntrac.root";
  }

  int rtncode = 0;
  if((rtncode = NeutToRooTracker(argv[1]))){
    SayRunLike(argv[0]);
  }
  return rtncode;
}
