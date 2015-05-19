#include <cstdlib>
#include <cerrno>
#include <climits>

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

enum STR2INT_ERROR { STRINT_SUCCESS,
                     STRINT_OVERFLOW,
                     STRINT_UNDERFLOW,
                     STRINT_INCONVERTIBLE };

///Converts a string to a long, checking for errors.
///See STR2INT_ERROR for error codes.
STR2INT_ERROR str2int (long &i, char const *s, int base=10) {
  char *end;
  long  l;
  errno;
  l = strtol(s, &end, base);
  if ((errno == ERANGE && l == LONG_MAX) || l > LONG_MAX) {
      return STRINT_OVERFLOW;
  }
  if ((errno == ERANGE && l == LONG_MIN) || l < LONG_MIN) {
      return STRINT_UNDERFLOW;
  }
  if (*s == '\0' || *end != '\0') {
      return STRINT_INCONVERTIBLE;
  }
  i = l;
  return STRINT_SUCCESS;
}

///Converts a string to a int, checking for errors.
///See STR2INT_ERROR for error codes.
STR2INT_ERROR str2int (int &i, char const *s, int base=10) {
  long holder;
  STR2INT_ERROR retC = str2int(holder,s,base);
  if(retC != STRINT_SUCCESS){
    return retC;
  }
  if(holder > INT_MAX) {
    return STRINT_OVERFLOW;
  } else if (holder < INT_MIN){
    return STRINT_UNDERFLOW;
  }
  i = holder;
  return retC;
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
    outRooTracker->AddBranches(rooTrackerTree);
  }

  for(long entryNum = 0; entryNum < NEntries; ++entryNum){

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

    if(verbosity > 1){
      std::cout << "Vector #:" << entryNum << std::endl;
      std::cout << "\tEvtCode: " << (*outRooTracker->EvtCode) << std::endl;
      std::cout << "\tEvtXSec: " << outRooTracker->EvtXSec << std::endl;
      std::cout << "\tNEcrsx: " << outRooTracker->NEcrsx << std::endl;
      std::cout << "\tNEcrsy: " << outRooTracker->NEcrsy << std::endl;
      std::cout << "\tNEcrsz: " << outRooTracker->NEcrsz << std::endl;
      std::cout << "\tNEcrsphi: " << outRooTracker->NEcrsphi << std::endl;
      std::cout << "\tNOutgoing Particles: " << vector->Npart() << std::endl;
      std::cout << std::endl;
    }

    //Fill the particle info
    outRooTracker->StdHepN = vector->Npart();

    for(int partNum = 0; partNum < vector->Npart(); ++partNum){
      const NeutPart& part = (*vector->PartInfo(partNum));

      if(partNum == 1){
        // As in TNeutOutput, to emulate neutgeom
        // StdHepX[1] is the target
        outRooTracker->StdHepPdg[1] =
          MakeNuclearPDG(vector->TargetZ, vector->TargetA);
        outRooTracker->StdHepP4[1][kNStdHepIdxE] = vector->TargetA;
        if(verbosity > 1){
          std::cout << "TARGET"
            << "\n\tA: " << vector->TargetA
            << "\n\tZ: " << vector->TargetZ
            << "\n\tPDG: " << outRooTracker->StdHepPdg[partNum]
            << "\n\tStatus: " << outRooTracker->StdHepStatus[partNum]
            << "\n\tHEPP4: " << SayArray(outRooTracker->StdHepP4[partNum])
            << std::endl;
        }
        continue;
      }

      outRooTracker->StdHepPdg[partNum] = part.fPID;

      switch(part.fStatus){
        case -1:{
          outRooTracker->StdHepStatus[partNum] = 0;
          break;
        }
        case 0:{
          outRooTracker->StdHepStatus[partNum] = 1;
          break;
        }
        case 2:{
          outRooTracker->StdHepStatus[partNum] = 1;
          break;
        }
        default:{
          std::cout << "--Found other neutcode: " << part.fStatus << std::endl;
          outRooTracker->StdHepStatus[partNum] = 2;
        }
      }

      outRooTracker->StdHepP4[partNum][kNStdHepIdxPx] = part.fP.X();
      outRooTracker->StdHepP4[partNum][kNStdHepIdxPy] = part.fP.Y();
      outRooTracker->StdHepP4[partNum][kNStdHepIdxPz] = part.fP.Z();
      outRooTracker->StdHepP4[partNum][kNStdHepIdxE] = part.fP.E();

      if(verbosity > 1){
        std::cout << ((partNum>1)?"Particle:":"Incoming Neutrino:")
          << "\n\tStdHEPPDG: " << outRooTracker->StdHepPdg[partNum]
          << "\n\tStdHEPStatus: " << outRooTracker->StdHepStatus[partNum]
          << "\n\tStdHEPP4: " << SayArray(outRooTracker->StdHepP4[partNum])
          << std::endl;
      }

      //Not implemented in NEUT
      (void)outRooTracker->StdHepX4[partNum][kNStdHepIdxX];
      (void)outRooTracker->StdHepX4[partNum][kNStdHepIdxY];
      (void)outRooTracker->StdHepX4[partNum][kNStdHepIdxZ];
      (void)outRooTracker->StdHepX4[partNum][kNStdHepIdxT];
      (void)outRooTracker->StdHepPolz[partNum][kNStdHepIdxPx];
      (void)outRooTracker->StdHepPolz[partNum][kNStdHepIdxPy];
      (void)outRooTracker->StdHepPolz[partNum][kNStdHepIdxPz];
      (void)outRooTracker->StdHepPolz[partNum][kNStdHepIdxE];
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
   << " [<output_file_name=vector.ntrac.root>] [0-3{Verbosity}] [--OutputObject]"
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
  if((argc > 6) || (argc == 1)){ SayRunLike(argv[0]); return 1;}
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

  if(argc >= (3 + int(ObjectOutput))){
    outfname = argv[2];
  } else {
    outfname = "vector.ntrac.root";
  }

  if(argc >= (4 + int(ObjectOutput))){
    if(str2int(verbosity,argv[3]) != STRINT_SUCCESS){
      std::cout << "Failed to parse: " << argv[3]
        << " as an integer verbosity level." << std::endl;
      return 1;
    }
  } else {
    verbosity = 0;
  }

  int rtncode = 0;
  if((rtncode = NeutToRooTracker(argv[1]))){
    SayRunLike(argv[0]);
  }
  return rtncode;
}
