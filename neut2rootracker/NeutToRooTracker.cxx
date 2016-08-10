#include <iostream>
#include <sstream>

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"

#include "neutvect.h"
#include "neutvtx.h"

#include "neutfsipart.h"
#include "neutfsivert.h"

#ifdef HAVE_NUCLEON_FSI_TRACKING

#include "neutnucfsistep.h"
#include "neutnucfsivert.h"

#endif

#include "LUtils/CLITools.hxx"
#include "LUtils/Debugging.hxx"
#include "LUtils/Utils.hxx"

#include "PureNeutRooTracker.hxx"

namespace {

std::ostream& operator<<(std::ostream& o, const TLorentzVector& tlv) {
  return o << "[" << tlv.X() << ", " << tlv.Y() << ", " << tlv.Z() << ", "
           << tlv.T() << "]";
}
}

/// Contains the variables affected by the CLI options.
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
/// expand the wildcard before passing the CLI options to \c main.
///
/// Passed like <tt>NeutToRooTracker.exe -i "FilesToRead.root"</tt>
std::string InpFDescript;
///\brief Whether to objectify output.
///
///\details Due to the way <tt>TObject</tt>s are output this will implicitly
/// turn on \c IsBound and \c StruckNucleonPDG branches.
///
/// Enabled with the \c -O CLI option.
bool ObjectOutput = false;
///\brief Whether to output in GeV, rather than the NEUT native MeV.
///
///\details Enabled with the \c -G CLI option.
bool OutputInGev = false;
///\brief Whether to save the <tt>IsBound</tt> flag for interactions.
///
///\details Demarcates whether the interaction took place on a proton or a
/// bound nucleus.
///
/// Enabled with the \c -s CLI option
bool SaveIsBound = false;
///\brief Whether to run in Lite mode. Outputs a much smaller tree, with fewer
/// variables.
///
///\details Enabled with the \c -L CLI option.
bool LiteMode = false;
///\brief Whether to skip particles which NEUT decides should not enter the
/// detector.
///
///\details Sometimes NEUT outputs particles demarcated as not still alive or
/// having some other non-good status code.
///
/// Enabling this with the \c -S CLI option will cause them to be not saved.
bool SkipNonFS = false;
///\brief Whether to try and emulate the NuWro flavor of rooTracker.
///
///\details This means that <tt>StdHepPdg[1]</tt> will contain the nuclear
/// target PDG code, <tt>StdHepP4[1]</tt> will contain the struck nucleon
/// 4Momentum. This can be used to determine the invariant mass of the struck
/// nucleon and thus it's species. However activating this option will also
/// cause
/// the <tt>StruckNucleonPDG</tt> branch to be written to the output file.
///
/// Enabled with the \c -E CLI option.
bool EmulateNuWro = false;
/// Increases the verbosity, settable with the \c -v CLI option.
int verbosity = 0;
/// Max entries to read before exiting, settable with the \c -n CLI option.
long MaxEntries = 0;
///\brief Will not save events which identify as one of these NEUT modes.
///
///\details Modes to ignore are added via like
/// <tt>NeutToRooTracker.exe  -I 1,2,27</tt>.
std::vector<int> ModeIgnores;
}

int NeutToRooTracker(const char* InputFileDescriptor) {
  // Input stuff
  TChain* NeutTree = new TChain("neuttree");

  int nFiles = NeutTree->Add(InputFileDescriptor);

  if (!nFiles) {
    UDBError("\"" << InputFileDescriptor << "\" matched 0 input files.");
    return 2;
  }

  NeutVect* vector = new NeutVect();
  NeutTree->SetBranchAddress("vectorbranch", &vector);
  NeutVtx* vtx = new NeutVtx();
  NeutTree->SetBranchAddress("vertexbranch", &vtx);

  long NEntries = NeutTree->GetEntries();
  long FilledEntries = 0;
  long IgnoredEntries = 0;

  if (!NEntries) {
    UDBError("Failed to find any entries (" << NEntries << ").");
    return 4;
  }
  UDBLog("Reading " << nFiles << " input files with " << NEntries
                    << " entries.");

  // Output stuff
  TFile* outFile =
      new TFile(NeutToRooTrackerOpts::OutFName.c_str(), "RECREATE");

  if (!outFile->IsOpen()) {
    UDBError("Couldn't open output file: " << NeutToRooTrackerOpts::OutFName);
    return 8;
  } else {
    UDBInfo("Created output file: " << outFile->GetName());
  }

  TTree* rooTrackerTree = new TTree("nRooTracker", "Pure NEUT RooTracker");
  NRooTrackerVtxB* outRooTracker = nullptr;
  NRooTrackerVtx* FullRooTracker = nullptr;
  if (NeutToRooTrackerOpts::LiteMode) {
    outRooTracker = new NRooTrackerVtxB();
  } else {
    FullRooTracker = new NRooTrackerVtx();
    outRooTracker = FullRooTracker;
  }

  if (NeutToRooTrackerOpts::ObjectOutput) {
    rooTrackerTree->Branch("nRooTracker", &outRooTracker);
  } else {
    outRooTracker->AddBranches(rooTrackerTree,
                               NeutToRooTrackerOpts::SaveIsBound,
                               NeutToRooTrackerOpts::EmulateNuWro);
  }

  float EUnitScaleFactor = 1.0;
  if (NeutToRooTrackerOpts::OutputInGev) {
    EUnitScaleFactor = 1.0 / 1000;
  }

  long long doEntries =
      (NeutToRooTrackerOpts::MaxEntries == -1)
          ? NEntries
          : (std::min(NeutToRooTrackerOpts::MaxEntries, NEntries));
  UInt_t TFileUID = 0;
  Double_t EvtWght = 0, EvtHistWght = 0;
  Double_t NEntriesInFile = 0;
  for (long entryNum = 0; entryNum < doEntries; ++entryNum) {
    if (entryNum && (!(entryNum % 10000))) {
      UDBInfo("Read " << entryNum << " entries.");
    }

    if (!entryNum) {
      UDBDebug("Reading first entry... ");
    }

    NeutTree->GetEntry(entryNum);
    if (!entryNum) {
      UDBDebug("Read first entry!");
    }

    if (NeutTree->GetFile()->GetUniqueID() != TFileUID) {
      TFileUID = NeutTree->GetFile()->GetUniqueID();
      TH1D* flux_numu =
          dynamic_cast<TH1D*>(NeutTree->GetFile()->Get("flux_numu"));
      TH1D* evtrt_numu =
          dynamic_cast<TH1D*>(NeutTree->GetFile()->Get("evtrt_numu"));

      NEntriesInFile = NeutTree->GetTree()->GetEntries();
      UDBLog("Opened new file: "
             << NeutTree->GetFile()->GetName() << " on entry " << entryNum
             << "/" << NEntries << " (" << NEntriesInFile
             << " entries in this file), EvtWght: " << EvtWght);

      if (!flux_numu || !evtrt_numu) {
        EvtHistWght = 0;
        EvtWght = 0;
      } else {
        EvtHistWght = evtrt_numu->Integral() / double(NEntriesInFile);
        EvtWght = evtrt_numu->Integral() / (flux_numu->Integral()*double(NEntriesInFile));
      }
    }

    if (NeutToRooTrackerOpts::ModeIgnores.size()) {
      bool found = false;
      for (int const& mode : NeutToRooTrackerOpts::ModeIgnores) {
        if (mode == vector->Mode) {
          IgnoredEntries++;
          found = true;
          break;
        }
      }
      if (found) {
        continue;
      }
    }

    //**************************************************
    // Event Level
    std::stringstream ss("");
    ss << vector->Mode;
    outRooTracker->EvtCode->SetString(ss.str().c_str());
    outRooTracker->EvtNum = vector->EventNo;

    if (!NeutToRooTrackerOpts::LiteMode) {
      FullRooTracker->EvtXSec = vector->Totcrs;
      FullRooTracker->EvtWght = EvtWght;
      FullRooTracker->EvtHistWght = EvtHistWght;
      FullRooTracker->NEntriesInFile = NEntriesInFile;

      FullRooTracker->NEcrsx = vector->Crsx;
      FullRooTracker->NEcrsy = vector->Crsy;
      FullRooTracker->NEcrsz = vector->Crsz;
      FullRooTracker->NEcrsphi = vector->Crsphi;

      if (vtx->Nvtx() != 1) {
        UDBWarn("Vertex entry " << entryNum << " had " << vtx->Nvtx()
                                << " entries, expected 1.");
        FullRooTracker->EvtVtx[0] = 0.0;
        FullRooTracker->EvtVtx[1] = 0.0;
        FullRooTracker->EvtVtx[2] = 0.0;
        FullRooTracker->EvtVtx[3] = 0.0;
      } else {
        FullRooTracker->EvtVtx[0] = vtx->Pos(0)->X();
        FullRooTracker->EvtVtx[1] = vtx->Pos(0)->Y();
        FullRooTracker->EvtVtx[2] = vtx->Pos(0)->Z();
        FullRooTracker->EvtVtx[3] = vtx->Pos(0)->T();
      }
    }

    //**************************************************
    // StdHepN Particles

    if (NeutToRooTrackerOpts::verbosity > 3) {
      UDBInfo(
          "**********************************************************"
          "**********************");
      vector->Dump();
      vtx->Dump();
      UDBInfo(
          "**********************************************************"
          "**********************");
    }

    outRooTracker->IsBound = vector->Ibound;

    int saveInd = 0;
    for (int partNum = 0; partNum < vector->Npart(); ++partNum) {
      const NeutPart& part = (*vector->PartInfo(partNum));

      if ((partNum == 1) && NeutToRooTrackerOpts::EmulateNuWro) {
        // As in nuwro2rootracker partnum 1 should have P4 of the struck
        // nucleon but the PDG of the target
        outRooTracker->StdHepPdg[saveInd] =
            Utils::MakeNuclearPDG(vector->TargetZ, vector->TargetA);
        // Now save the struck nucleon properties
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPx] =
            part.fP.Px() * EUnitScaleFactor;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPy] =
            part.fP.Py() * EUnitScaleFactor;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxPz] =
            part.fP.Pz() * EUnitScaleFactor;
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] =
            part.fP.E() * EUnitScaleFactor;

        outRooTracker->StruckNucleonPDG = part.fPID;

        // Not implemented in NEUT
        if (!NeutToRooTrackerOpts::LiteMode) {
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
      } else if (partNum == 1) {
        // As in TNeutOutput, to emulate neutgeom
        // StdHepX[1] is the target
        outRooTracker->StdHepPdg[saveInd] =
            Utils::MakeNuclearPDG(vector->TargetZ, vector->TargetA);
        outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] = vector->TargetA;
        // Now incremebent the saveInd to save the struck nucleon properties
        // but don't continue
        // Not implemented in NEUT
        if (!NeutToRooTrackerOpts::LiteMode) {
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
      }

      outRooTracker->StdHepPdg[saveInd] = part.fPID;

      switch (part.fStatus) {
        case -1: {  // Initial state
          outRooTracker->StdHepStatus[saveInd] = 0;
          break;
        }
        case 0: {  // Good
          if (part.fIsAlive == 1) {
            outRooTracker->StdHepStatus[saveInd] = 1;
          } else {  // But also bad!?
            outRooTracker->StdHepStatus[saveInd] = 2;
            if (NeutToRooTrackerOpts::SkipNonFS) {
              UDBVerbose(
                  "Not saving particle status ("
                  << part.fStatus << ":\"" << NEUTStatusCodes[part.fStatus]
                  << "\") as it was not 'IsAlive' in event: " << entryNum);
              continue;
            }
          }
          break;
        }
        case 2: {  // Escaped detector == Good
          if (part.fIsAlive == 1) {
            outRooTracker->StdHepStatus[saveInd] = 1;
            UDBWarn("Found NEUT status 2:\""
                    << NEUTStatusCodes[part.fStatus]
                    << "\" which was marked as "
                       "IsAlive. (PDG:"
                    << outRooTracker->StdHepPdg[saveInd]
                    << ") in event: " << entryNum);
          } else {  // But also bad.
            outRooTracker->StdHepStatus[saveInd] = 2;
            if (NeutToRooTrackerOpts::SkipNonFS) {
              UDBVerbose("Not saving particle status("
                         << part.fStatus << ":\""
                         << NEUTStatusCodes[part.fStatus]
                         << "\") as it was not 'IsAlive'. (PDG:"
                         << outRooTracker->StdHepPdg[saveInd]
                         << ") in event: " << entryNum);
              continue;
            }
          }
          break;
        }
        default: {
          UDBWarn("Found unexpected neut fStatus code: "
                  << part.fStatus << ":\"" << NEUTStatusCodes[part.fStatus]
                  << "\" in event: " << entryNum);
          outRooTracker->StdHepStatus[saveInd] = part.fStatus;
          if (NeutToRooTrackerOpts::SkipNonFS) {
            continue;
          }
        }
      }
      // TODO Check MEC events
      if ((!NeutToRooTrackerOpts::EmulateNuWro) && (partNum == 1) &&
          (part.fStatus == -1)) {
        outRooTracker->StdHepStatus[saveInd] =
            11;  // To sync with GENIE code for
        // Struck Nucleon.
      }

      outRooTracker->StdHepP4[saveInd][kNStdHepIdxPx] =
          part.fP.Px() * EUnitScaleFactor;
      outRooTracker->StdHepP4[saveInd][kNStdHepIdxPy] =
          part.fP.Py() * EUnitScaleFactor;
      outRooTracker->StdHepP4[saveInd][kNStdHepIdxPz] =
          part.fP.Pz() * EUnitScaleFactor;
      outRooTracker->StdHepP4[saveInd][kNStdHepIdxE] =
          part.fP.E() * EUnitScaleFactor;

      // Not implemented in NEUT
      if (!NeutToRooTrackerOpts::LiteMode) {
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
    }  // end for(int partNum = 0; partNum < vector->Npart(); ++partNum)

    outRooTracker->StdHepN = saveInd;

    UDBVerbose("(Interation Mode: " << vector->Mode << ")");
    for (int it = 0; it < outRooTracker->StdHepN; ++it) {
      UDBVerbose(((it > 0) ? "Particle:" : "Incoming Neutrino:")
                 << " " << it << "/" << outRooTracker->StdHepN
                 << "(VectNPart:" << vector->Npart() << ")"
                 << "\n\tStdHEPPDG: " << outRooTracker->StdHepPdg[it]
                 << "\n\tStdHEPStatus: " << outRooTracker->StdHepStatus[it]
                 << "\n\tStdHEPP4: "
                 << Utils::PrintArray(outRooTracker->StdHepP4[it]));
    }

    if (!NeutToRooTrackerOpts::LiteMode) {
      //**************************************************
      // NEUT VCWork Particles
      FullRooTracker->NEnvc = vector->Npart();
      for (int partNum = 0; partNum < FullRooTracker->NEnvc; ++partNum) {
        const NeutPart& part = (*vector->PartInfo(partNum));

        FullRooTracker->NEpvc[partNum][0] = part.fP.Px() * EUnitScaleFactor;
        FullRooTracker->NEpvc[partNum][1] = part.fP.Py() * EUnitScaleFactor;
        FullRooTracker->NEpvc[partNum][2] = part.fP.Pz() * EUnitScaleFactor;
        FullRooTracker->NEipvc[partNum] = part.fPID;
        FullRooTracker->NEiorgvc[partNum] = 0;
        FullRooTracker->NEiflgvc[partNum] = part.fStatus;
        FullRooTracker->NEicrnvc[partNum] = part.fIsAlive ? 1 : 0;
      }

      //**************************************************
      // NEUT Pion FSI interaction history
      FullRooTracker->NEnvert = vector->NfsiVert();

      for (int FSIVertNum = 0; FSIVertNum < FullRooTracker->NEnvert;
           ++FSIVertNum) {
        const NeutFsiVert& fsiVert = (*vector->FsiVertInfo(FSIVertNum));
        FullRooTracker->NEposvert[FSIVertNum][0] = fsiVert.fPos.X();
        FullRooTracker->NEposvert[FSIVertNum][1] = fsiVert.fPos.Y();
        FullRooTracker->NEposvert[FSIVertNum][2] = fsiVert.fPos.Z();
        FullRooTracker->NEiflgvert[FSIVertNum] = fsiVert.fVertID;
      }

      FullRooTracker->NEnvcvert = vector->NfsiPart();
      for (int FSIPartNum = 0; FSIPartNum < FullRooTracker->NEnvcvert;
           ++FSIPartNum) {
        const NeutFsiPart& fsiPart = (*vector->FsiPartInfo(FSIPartNum));

        FullRooTracker->NEdirvert[FSIPartNum][0] = fsiPart.fDir.X();
        FullRooTracker->NEdirvert[FSIPartNum][1] = fsiPart.fDir.Y();
        FullRooTracker->NEdirvert[FSIPartNum][2] = fsiPart.fDir.Z();
        FullRooTracker->NEabspvert[FSIPartNum] =
            fsiPart.fMomLab * EUnitScaleFactor;
        FullRooTracker->NEabstpvert[FSIPartNum] =
            fsiPart.fMomNuc * EUnitScaleFactor;
        FullRooTracker->NEipvert[FSIPartNum] = fsiPart.fPID;
        FullRooTracker->NEiverti[FSIPartNum] = fsiPart.fVertStart;
        FullRooTracker->NEivertf[FSIPartNum] = fsiPart.fVertEnd;
      }

#ifdef HAVE_NUCLEON_FSI_TRACKING

      //**************************************************
      // NEUT Nucleon FSI interaction history
      FullRooTracker->NFnvert = vector->NnucFsiVert();
      for (int NucFSIPartNum = 0; NucFSIPartNum < FullRooTracker->NFnvert;
           ++NucFSIPartNum) {
        const NeutNucFsiVert& nucFSIPart =
            (*vector->NucFsiVertInfo(NucFSIPartNum));

        FullRooTracker->NFiflag[NucFSIPartNum] = nucFSIPart.fVertFlag;
        FullRooTracker->NFx[NucFSIPartNum] = nucFSIPart.fPos.X();
        FullRooTracker->NFy[NucFSIPartNum] = nucFSIPart.fPos.Y();
        FullRooTracker->NFz[NucFSIPartNum] = nucFSIPart.fPos.Z();
        FullRooTracker->NFpx[NucFSIPartNum] = nucFSIPart.fMom.X();
        FullRooTracker->NFpy[NucFSIPartNum] = nucFSIPart.fMom.X();
        FullRooTracker->NFpz[NucFSIPartNum] = nucFSIPart.fMom.X();
        FullRooTracker->NFe[NucFSIPartNum] = nucFSIPart.fMom.X();
        FullRooTracker->NFfirststep[NucFSIPartNum] = nucFSIPart.fVertFirstStep;
      }

      FullRooTracker->NFnstep = vector->NnucFsiStep();
      for (int NucFSIStepNum = 0; NucFSIStepNum < FullRooTracker->NFnstep;
           ++NucFSIStepNum) {
        const NeutNucFsiStep& nucFSIStep =
            (*vector->NucFsiStepInfo(NucFSIStepNum));
        FullRooTracker->NFecms2[NucFSIStepNum] = nucFSIStep.fECMS2;
        FullRooTracker->NFProb[NucFSIStepNum] = nucFSIStep.fProb;
      }

#endif
    }  // end if(!NeutToRooTrackerOpts::LiteMode)

    rooTrackerTree->Fill();
    FilledEntries++;
    UDBVerbose("*****************Filled*****************\n");
    outRooTracker->Reset();
  }
  UDBLog("Wrote " << FilledEntries << " events to disk.");
  if (NeutToRooTrackerOpts::ModeIgnores.size()) {
    UDBLog("Ignored " << IgnoredEntries << " entries based on "
                                           "interaction mode. ");
  }

  rooTrackerTree->Write();
  outFile->Close();
  return 0;
}

namespace NeutToRooTrackerOpts {

/// CLI option and value handling implementation.
void SetOpts() {
  CLIArgs::AddOpt("-i", "--input-file", true,
                  [&](std::string const& opt) -> bool {
                    std::cout << "\t--Reading from file descriptor : " << opt
                              << std::endl;
                    InpFDescript = opt;
                    return true;
                  },
                  true, []() {}, "<TChain::Add descriptor>");

  CLIArgs::AddOpt("-o", "--output-file", true,
                  [&](std::string const& opt) -> bool {
                    std::cout << "\t--Writing to File: " << opt << std::endl;
                    OutFName = opt;
                    return true;
                  },
                  false, [&]() { OutFName = "vector.ntrac.root"; },
                  "<File Name>{default=vector.ntrac.root}");

  CLIArgs::AddOpt(
      "-n", "--nentries", true,
      [&](std::string const& opt) -> bool {
        long vbhold;
        if (Utils::str2int(vbhold, opt.c_str()) == Utils::STRINT_SUCCESS) {
          if (vbhold != -1) {
            std::cout << "\t--Looking at, at most, " << vbhold << " entries."
                      << std::endl;
          }
          MaxEntries = vbhold;
          return true;
        }
        return false;
      },
      false, [&]() { MaxEntries = -1; }, "<-1>: Read all {default=-1}");

  CLIArgs::AddOpt(
      "-v", "--verbosity", true,
      [&](std::string const& opt) -> bool {
        int vbhold;
        if (Utils::str2int(vbhold, opt.c_str()) == Utils::STRINT_SUCCESS) {
          std::cout << "\t--Verbosity: " << vbhold << std::endl;
          verbosity = vbhold;
          return true;
        }
        return false;
      },
      false, [&]() { verbosity = 0; }, "<0-4>{default=0}");

  CLIArgs::AddOpt("-G", "--GeV-mode", false,
                  [&](std::string const& opt) -> bool {
                    std::cout << "\t--Outputting in GeV." << std::endl;
                    OutputInGev = true;
                    return true;
                  },
                  false, [&]() { OutputInGev = false; },
                  "Use GeV rather than MeV.");

  CLIArgs::AddOpt("-O", "--objectify-output", false,
                  [&](std::string const& opt) -> bool {
                    std::cout << "\t--Using simple tree." << std::endl;
                    ObjectOutput = true;
                    return true;
                  },
                  false, [&]() { ObjectOutput = false; },
                  "Output object tree.");

  CLIArgs::AddOpt(
      "-b", "--save-isbound", false,
      [&](std::string const& opt) -> bool {
        std::cout << "\t--Adding IsBound branch to output tree." << std::endl;
        SaveIsBound = true;
        return true;
      },
      false, [&]() { SaveIsBound = false; }, "Output IsBound Branch");

  CLIArgs::AddOpt("-L", "--Lite-Mode", false,
                  [&](std::string const& opt) -> bool {
                    std::cout << "\t--Running in Lite Mode." << std::endl;
                    LiteMode = true;
                    return true;
                  },
                  false, [&]() { LiteMode = false; }, "Run in Lite Mode");

  CLIArgs::AddOpt("-E", "--Emulate-NuWro", false,
                  [&](std::string const& opt) -> bool {
                    std::cout << "\t--Emulating the output of nuwro2rootracker."
                              << std::endl;
                    EmulateNuWro = true;
                    return true;
                  },
                  false, [&]() { EmulateNuWro = false; },
                  "Emulate nuwro2rootracker more closely.");

  CLIArgs::AddOpt(
      "-S", "--Skip-non-FS", false,
      [&](std::string const& opt) -> bool {
        std::cout << "\t--Not saving non-FS particles." << std::endl;
        SkipNonFS = true;
        return true;
      },
      false, [&]() { SkipNonFS = false; }, "Don't save non-FS particles.");

  CLIArgs::AddOpt(
      "-I", "--Ignore-NEUT-Modes", true,
      [&](std::string const& opt) -> bool {
        ModeIgnores = Utils::StringVToIntV(Utils::SplitStringByDelim(opt, ","));

        if (ModeIgnores.size()) {
          std::cout << "\t--Ignoring interactions with NEUT modes:  "
                    << std::flush;
          for (auto const& mi : ModeIgnores) {
            std::cout << mi << ", " << std::flush;
          }
          std::cout << std::endl;
          return true;
        }
        return false;
      },
      false, []() {}, "<int,int,...> NEUT modes to save output from.");
}
}

int main(int argc, char const* argv[]) {
  NeutToRooTrackerOpts::SetOpts();

  CLIArgs::AddArguments(argc, argv);
  if (!CLIArgs::HandleArgs()) {
    CLIArgs::SayRunLike();
    return 1;
  }

  UDBSetDebuggingLevel(NeutToRooTrackerOpts::verbosity);
  UDBSetInfoLevel(NeutToRooTrackerOpts::verbosity);

  int rtncode = 0;
  if ((rtncode =
           NeutToRooTracker(NeutToRooTrackerOpts::InpFDescript.c_str()))) {
    CLIArgs::SayRunLike();
  }

  UDBTearDown();
  return rtncode;
}
