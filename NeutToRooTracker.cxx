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
    std::cerr << "Couldn't open output file: " << outfname <<" leaving in "
    "a strop." << std::endl;
    return 4;
  } else {
    std::cout << "Created output file: " << outFile->GetName() << std::endl;
  }

  TTree* rooTrackerTree = new TTree("nRooTracker","Pure NEUT RooTracker");
  NRooTrackerVtx* outRooTracker = 0;
  rooTrackerTree->Branch("nRooTracker",&outRooTracker,32000);

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
    outRooTracker->EvtCode = ss.str();
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
    outRooTracker->StdHepPdg = new int[outRooTracker->StdHepN];
    outRooTracker->StdHepStatus = new int[outRooTracker->StdHepN];
    outRooTracker->StdHepFd = new int[outRooTracker->StdHepN];
    outRooTracker->StdHepLd = new int[outRooTracker->StdHepN];
    outRooTracker->StdHepFm = new int[outRooTracker->StdHepN];
    outRooTracker->StdHepLm = new int[outRooTracker->StdHepN];

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
      (void)outRooTracker->StdHepP4[storageNum][kNStdHepIdxPx];
      (void)outRooTracker->StdHepP4[storageNum][kNStdHepIdxPy];
      (void)outRooTracker->StdHepP4[storageNum][kNStdHepIdxPz];
      (void)outRooTracker->StdHepP4[storageNum][kNStdHepIdxE];
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

    outRooTracker->NEipvc = new int[outRooTracker->NEnvc];
    for(int i = 0; i < outRooTracker->NEnvc; ++i){
      (void)outRooTracker->NEpvc[0];
    }
    outRooTracker->NEiorgvc = new int[outRooTracker->NEnvc];
    outRooTracker->NEiflgvc = new int[outRooTracker->NEnvc];
    outRooTracker->NEicrnvc = new int[outRooTracker->NEnvc];

    //**************************************************
    //NEUT Pion FSI interaction history
    //Not yet implemented
    outRooTracker->NEnvert = 1;

    for(int i = 0; i < outRooTracker->NEnvert; ++i){
      (void)outRooTracker->NEposvert[0];
    }
    outRooTracker->NEiflgvert = new int[outRooTracker->NEnvert];

    outRooTracker->NEnvcvert = 1;
    for(int i = 0; i < outRooTracker->NEnvcvert; ++i){
      (void)outRooTracker->NEdirvert[0];
    }
    outRooTracker->NEabspvert = new float[outRooTracker->NEnvcvert];
    outRooTracker->NEabstpvert = new float[outRooTracker->NEnvcvert];
    outRooTracker->NEipvert = new int[outRooTracker->NEnvcvert];
    outRooTracker->NEiverti = new int[outRooTracker->NEnvcvert];
    outRooTracker->NEivertf = new int[outRooTracker->NEnvcvert];

    //**************************************************
    //NEUT Nucleon FSI interaction history
    //Not yet implemented
    outRooTracker->NFnvert = 1;
    outRooTracker->NFiflag = new int[outRooTracker->NFnvert];
    outRooTracker->NFx = new float[outRooTracker->NFnvert];
    outRooTracker->NFy = new float[outRooTracker->NFnvert];
    outRooTracker->NFz = new float[outRooTracker->NFnvert];
    outRooTracker->NFpx = new float[outRooTracker->NFnvert];
    outRooTracker->NFpy = new float[outRooTracker->NFnvert];
    outRooTracker->NFpz = new float[outRooTracker->NFnvert];
    outRooTracker->NFe = new float[outRooTracker->NFnvert];
    outRooTracker->NFfirststep = new int[outRooTracker->NFnvert];

    outRooTracker->NFnstep = 1;
    outRooTracker->NFecms2 = new float[outRooTracker->NFnstep];

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
   << " [output_file_name=vector.ntrac.root]"
   << "\n\t\"input_file_descriptor\" = any string that is a valid input"
   << "\n\t\tfor TChain::Add(const char*)." << std::endl;
}

bool IsHelp(std::string helpcli){
  if((helpcli == "--help") || (helpcli == "-h")  || (helpcli == "-?") ){
    return true;
  }
  return false;
}

int main(int argc, char* argv[]){
  //Too many args spoil the broth
  if((argc > 3) || (argc == 1)){ SayRunLike(argv[0]); return 1;}
  //If the user is asking for help
  if(argc == 2 && IsHelp(argv[1])){ SayRunLike(argv[0]); return 0; }
  //If we have a specified output filename
  if(argc == 3){ outfname = argv[2]; }
  //Otherwise just use the default
  else { outfname = "vector.ntrac.root"; }

  int rtncode = 0;
  if((rtncode = NeutToRooTracker(argv[1]))){
    SayRunLike(argv[0]);
  }
  return rtncode;
}
