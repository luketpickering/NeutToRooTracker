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
}

///Contains the variables affected by the CLI options.
namespace NeutToRooTrackerOpts {
///\brief The name to write the output \c TFile to.
///
///\details Passed with the \c -o CLI option.
std::string OutFName;
///\brief The input file descriptor string.
///
///\details This may contain wild cards which can be evaluated by \c TChain::Add
///\warning If wildcards are needed it should be passed wrapped in quotes,
///<tt>NeutToRooTracker.exe -i "Files*.root"</tt>, otherwise the shell will
///expand the wildcard before passing the CLI options to \c main.
///
///Passed like <tt>NeutToRooTracker.exe -i "FilesToRead.root"</tt>
std::string InpFDescript;
///\brief Whether to objectify output.
///
///\details Due to the way <tt>TObject</tt>s are output this will implicitly
///turn on \c IsBound and \c StruckNucleonPDG branches.
///
///Enabled with the \c -O CLI option.
bool ObjectOutput = false;
///\brief Whether to output in GeV, rather than the NEUT native MeV.
///
///\details Enabled with the \c -G CLI option.
bool OutputInGev = false;
///\brief Whether to save the <tt>IsBound</tt> flag for interactions.
///
///\details Demarcates whether the interaction took place on a proton or a
///bound nucleus.
///
///Enabled with the \c -s CLI option
bool SaveIsBound = false;
///\brief Whether to run in Lite mode. Outputs a much smaller tree, with fewer
///variables.
///
///\details Enabled with the \c -L CLI option.
bool LiteMode = false;
///\brief Whether to skip particles which NEUT decides should not enter the
///detector.
///
///\details Sometimes NEUT outputs particles demarcated as not still alive or
///having some other non-good status code.
///
///Enabling this with the \c -S CLI option will cause them to be not saved.
bool SkipNonFS = false;
///\brief Whether to try and emulate the NuWro flavor of rooTracker.
///
///\details This means that <tt>StdHepPdg[1]</tt> will contain the nuclear
///target PDG code, <tt>StdHepP4[1]</tt> will contain the struck nucleon
///4Momentum. This can be used to determine the invariant mass of the struck
///nucleon and thus it's species. However activating this option will also cause
///the <tt>StruckNucleonPDG</tt> branch to be written to the output file.
///
///Enabled with the \c -E CLI option.
bool EmulateNuWro = false;
///Increases the verbosity, settable with the \c -v CLI option.
int verbosity = 0;
///Max entries to read before exiting, settable with the \c -n CLI option.
long MaxEntries = 0;
///\brief Will not save events which identify as one of these NEUT modes.
///
///\details Modes to ingore are added via like
/// <tt>NeutToRooTracker.exe  -I 1,2,27</tt>.
std::vector<int> ModeIgnores;
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
  long FilledEntries = 0; long IgnoredEntries = 0;

  if(!NEntries){
    std::cout << "Failed to find any entries (" << NEntries
      << ")." << std::endl;
    return 4;
  }
  std::cout << "Reading " << nFiles << " input files with "
    << NEntries << " entries." << std::endl;

//Output stuff
  TFile* outFile = new TFile(NeutToRooTrackerOpts::OutFName.c_str(),"CREATE");

  if(!outFile->IsOpen()){
    std::cerr << "[ERROR]: Couldn't open output file: "
      << NeutToRooTrackerOpts::OutFName << std::endl;
    return 8;
  } else {
    std::cout << "Created output file: " << outFile->GetName() << std::endl;
  }

  TTree* rooTrackerTree = new TTree("nRooTracker","Pure NEUT RooTracker");
  NRooTrackerVtxB* outRooTracker = nullptr;
  NRooTrackerVtx* FullRooTracker = nullptr;
  if(NeutToRooTrackerOpts::LiteMode){
    outRooTracker = new NRooTrackerVtxB();
  } else {
    FullRooTracker = new NRooTrackerVtx();
    outRooTracker = FullRooTracker;
  }

  if(NeutToRooTrackerOpts::ObjectOutput){
    rooTrackerTree->Branch("nRooTracker", &outRooTracker);
  } else {
    outRooTracker->AddBranches(rooTrackerTree,
      NeutToRooTrackerOpts::SaveIsBound,
      NeutToRooTrackerOpts::EmulateNuWro);
  }

  long long doEntries = (NeutToRooTrackerOpts::MaxEntries==-1) ?
    NEntries : (std::min(NeutToRooTrackerOpts::MaxEntries, NEntries));

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

    if(NeutToRooTrackerOpts::ModeIgnores.size()){
      bool found = false;
      for(int const & mode: NeutToRooTrackerOpts::ModeIgnores){
        if(mode == vector->Mode){
          IgnoredEntries++;
          found = true;
          break;
        }
      }
      if(found){
        continue;
      }
    }

    //**************************************************
    //Event Level
    std::stringstream ss("");
    ss << vector->Mode;
    outRooTracker->EvtCode->SetString(ss.str().c_str());
    outRooTracker->EvtNum = vector->EventNo;

    if(!NeutToRooTrackerOpts::LiteMode){
      FullRooTracker->EvtXSec = vector->Totcrs;

      FullRooTracker->NEcrsx = vector->Crsx;
      FullRooTracker->NEcrsy = vector->Crsy;
      FullRooTracker->NEcrsz = vector->Crsz;
      FullRooTracker->NEcrsphi = vector->Crsphi;

      (void)FullRooTracker->EvtVtx[0];
      (void)FullRooTracker->EvtVtx[0];
      (void)FullRooTracker->EvtVtx[0];
      (void)FullRooTracker->EvtVtx[0];
    }

    //**************************************************
    //StdHepN Particles

    if(NeutToRooTrackerOpts::verbosity > 3){
      std::cout << "**********************************************************"
        "**********************"
        << std::endl;
      vector->Dump();
      std::cout << "**********************************************************"
        "**********************"
        << std::endl;
    }

    outRooTracker->IsBound = vector->Ibound;

    int saveInd = 0;
    for(int partNum = 0; partNum < vector->Npart(); ++partNum){

      const NeutPart& part = (*vector->PartInfo(partNum));
      if((partNum == 1) && NeutToRooTrackerOpts::EmulateNuWro){
        // As in nuwro2rootracker partnum 1 should have P4 of the struck
        // nucleon but the PDG of the target
        outRooTracker->StdHepPdg[saveInd] =
          PGUtils::MakeNuclearPDG(vector->TargetZ, vector->TargetA);
        //Now save the struck nucleon properties
        if(NeutToRooTrackerOpts::OutputInGev){
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
        outRooTracker->StruckNucleonPDG = part.fPID;

        //Not implemented in NEUT
        if(!NeutToRooTrackerOpts::LiteMode){
          (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxX];
          (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxY];
          (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxZ];
          (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxT];
          (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxPx];
          (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxPy];
          (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxPz];
          (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxE];
        }
        ++saveInd;
        continue;
      } else if(partNum == 1){
        // As in TNeutOutput, to emulate neutgeom
        // StdHepX[1] is the target
        outRooTracker->StdHepPdg[saveInd] =
          PGUtils::MakeNuclearPDG(vector->TargetZ, vector->TargetA);
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] = vector->TargetA;
        //Now incremebent the saveInd to save the struck nucleon properties
        //but don't continue
        ++saveInd;
      }

      outRooTracker->StdHepPdg[saveInd] = part.fPID;

      switch(part.fStatus){
        case -1:{ // Initial state
          outRooTracker->StdHepStatus[saveInd] = 0;
          break;
        }
        case 0:{ // Good
          if(part.fIsAlive == 1){
            outRooTracker->StdHepStatus[saveInd] = 1;
          } else { //But also bad!?
            outRooTracker->StdHepStatus[saveInd] = 2;
            if(NeutToRooTrackerOpts::SkipNonFS){
              if(NeutToRooTrackerOpts::verbosity){
                std::cout << "[INFO]: Not saving particle status("
                  << part.fStatus << ") as it was not 'IsAlive'." << std::endl;
            }
              continue;
            }
          }
          break;
        }
        case 2:{ // Escaped detector == Good
          if(part.fIsAlive == 1){
            outRooTracker->StdHepStatus[saveInd] = 1;
            if(NeutToRooTrackerOpts::verbosity){
              std::cout << "[INFO]: Found NEUT status 2 which was marked as "
                "IsAlive. (PDG:" << outRooTracker->StdHepPdg[saveInd] << ")"
                << std::endl;
            }
          } else { // But also bad.
            outRooTracker->StdHepStatus[saveInd] = 2;
            if(NeutToRooTrackerOpts::SkipNonFS){
              if(NeutToRooTrackerOpts::verbosity){
                std::cout << "[INFO]: Not saving particle status("
                  << part.fStatus << ") as it was not 'IsAlive'. (PDG:"
                  << outRooTracker->StdHepPdg[saveInd] << ")" << std::endl;
              }
              continue;
            }
          }
          break;
        }
        default:{
          if(NeutToRooTrackerOpts::verbosity > 1){
            std::cout << "--Found unexpected neutcode: " << part.fStatus
              << std::endl;
          }
            outRooTracker->StdHepStatus[saveInd] = part.fStatus;
          if(NeutToRooTrackerOpts::SkipNonFS){
            continue;
          }
        }
      }
      if( (!NeutToRooTrackerOpts::EmulateNuWro) &&
          (partNum == 1) && (part.fStatus == -1)){
        outRooTracker->StdHepStatus[saveInd] = 11; //To sync with GENIE code for
        //Struck Nucleon.
      }

      if(NeutToRooTrackerOpts::OutputInGev){
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

      //Not implemented in NEUT
      if(!NeutToRooTrackerOpts::LiteMode){
        (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxX];
        (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxY];
        (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxZ];
        (void)FullRooTracker->StdHepX4[saveInd][kNStdHepIdxT];
        (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxPx];
        (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxPy];
        (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxPz];
        (void)FullRooTracker->StdHepPolz[saveInd][kNStdHepIdxE];
      }
      saveInd++;
    }

    outRooTracker->StdHepN = saveInd;

    if(NeutToRooTrackerOpts::verbosity > 1){
      std::cout <<  "(Int Mode: " << vector->Mode << ")" << std::endl;
      for(int it = 0; it < outRooTracker->StdHepN; ++it){
        std::cout
          << ((it>0)? "Particle:":"Incoming Neutrino:") << " "
          << it << "/" << outRooTracker->StdHepN << "(VectNPart:"
          << vector->Npart() << ")"
          << "\n\tStdHEPPDG: " << outRooTracker->StdHepPdg[it]
          << "\n\tStdHEPStatus: " << outRooTracker->StdHepStatus[it]
          << "\n\tStdHEPP4: " << PGUtils::PrintArray(
            outRooTracker->StdHepP4[it])
          << std::endl;
      }
    }

    if(!NeutToRooTrackerOpts::LiteMode){
      //**************************************************
      //NEUT VCWork Particles
      //Not yet implemented.
      FullRooTracker->NEnvc = 1;
      for(int i = 0; i < FullRooTracker->NEnvc; ++i){
        (void)FullRooTracker->NEpvc[i];
        (void)FullRooTracker->NEiorgvc[i];
        (void)FullRooTracker->NEiflgvc[i];
        (void)FullRooTracker->NEicrnvc[i];
      }

      //**************************************************
      //NEUT Pion FSI interaction history
      //Not yet implemented
      FullRooTracker->NEnvert = 1;

      for(int i = 0; i < FullRooTracker->NEnvert; ++i){
        (void)FullRooTracker->NEposvert[i];
        (void)FullRooTracker->NEiflgvert[i];
      }

      FullRooTracker->NEnvcvert = 1;
      for(int i = 0; i < FullRooTracker->NEnvcvert; ++i){
        (void)FullRooTracker->NEdirvert[i];
        (void)FullRooTracker->NEabspvert[i];
        (void)FullRooTracker->NEabstpvert[i];
        (void)FullRooTracker->NEipvert[i];
        (void)FullRooTracker->NEiverti[i];
        (void)FullRooTracker->NEivertf[i];
      }

      //**************************************************
      //NEUT Nucleon FSI interaction history
      //Not yet implemented
      FullRooTracker->NFnvert = 1;
      (void)FullRooTracker->NFiflag[0];
      (void)FullRooTracker->NFx[0];
      (void)FullRooTracker->NFy[0];
      (void)FullRooTracker->NFz[0];
      (void)FullRooTracker->NFpx[0];
      (void)FullRooTracker->NFpy[0];
      (void)FullRooTracker->NFpz[0];
      (void)FullRooTracker->NFe[0];
      (void)FullRooTracker->NFfirststep[0];

      FullRooTracker->NFnstep = 1;
      (void)FullRooTracker->NFecms2[0];
    }
    rooTrackerTree->Fill();
    FilledEntries++;
    if(NeutToRooTrackerOpts::verbosity > 1){
      std::cout << "*****************Filled*****************\n" << std::endl;
    }
    outRooTracker->Reset();
  }
  std::cout << "Wrote " << FilledEntries << " events to disk." << std::flush;
  if(NeutToRooTrackerOpts::ModeIgnores.size()){
    std::cout << " Ignored " << IgnoredEntries << " entries based on "
      "interaction mode. " << std::flush;
  } std::cout << std::endl;

  rooTrackerTree->Write();
  outFile->Close();
  return 0;
}

namespace NeutToRooTrackerOpts {

///CLI option and value handling implementation.
void SetOpts(){

  CLIArgs::AddOpt("-i", "--input-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Reading from file descriptor : " << opt << std::endl;
      InpFDescript = opt;
      return true;
    }, true,[](){},"<TChain::Add descriptor>");

  CLIArgs::AddOpt("-o", "--output-file", true,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Writing to File: " << opt << std::endl;
      OutFName = opt;
      return true;
    }, false,
    [&](){OutFName = "vector.ntrac.root";},
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

  CLIArgs::AddOpt("-L", "--Lite-Mode", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Running in Lite Mode." << std::endl;
      LiteMode = true;
      return true;
    }, false,
    [&](){LiteMode = false;}, "Run in Lite Mode");

  CLIArgs::AddOpt("-E", "--Emulate-NuWro", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Emulating the output of nuwro2rootracker." << std::endl;
      EmulateNuWro = true;
      return true;
    }, false,
    [&](){EmulateNuWro = false;}, "Emulate nuwro2rootracker more closely.");

  CLIArgs::AddOpt("-S", "--Skip-non-FS", false,
    [&] (std::string const &opt) -> bool {
      std::cout << "\t--Not saving non-FS particles." << std::endl;
      SkipNonFS = true;
      return true;
    }, false,
    [&](){SkipNonFS = false;}, "Don't save non-FS particles.");

  CLIArgs::AddOpt("-I", "--Ignore-NEUT-Modes", true,
    [&] (std::string const &opt) -> bool {
      ModeIgnores =
        PGUtils::StringVToIntV(PGUtils::SplitStringByDelim(opt,","));

      if(ModeIgnores.size()){
        std::cout << "\t--Ignoring interactions with NEUT modes:  "
          << std::flush;
        for(auto const &mi : ModeIgnores){
          std::cout << mi << ", " << std::flush;
        }
        std::cout << std::endl;
        return true;
      }
      return false;
    }, false,
    [](){},
    "<int,int,...> NEUT modes to save output from.");
}
}

int main(int argc, char const * argv[]){
  NeutToRooTrackerOpts::SetOpts();

  CLIArgs::AddArguments(argc,argv);
  if(!CLIArgs::HandleArgs()){
    CLIArgs::SayRunLike();
    return 1;
  }

  int rtncode = 0;
  if((rtncode = NeutToRooTracker(NeutToRooTrackerOpts::InpFDescript.c_str()))){
    CLIArgs::SayRunLike();
  }
  return rtncode;
}
