#include "TObject.h"
#include "TString.h"
#include "TObjString.h"
#include "TTree.h"

const int kNStdHepNPmax = 100;
const int kNStdHepIdxPx = 0;
const int kNStdHepIdxPy = 1;
const int kNStdHepIdxPz = 2;
const int kNStdHepIdxE = 3;
const int kNStdHepIdxX = 0;
const int kNStdHepIdxY = 1;
const int kNStdHepIdxZ = 2;
const int kNStdHepIdxT = 3;

const int kNEmaxvc = 100;
const int kNEmaxvert = 100;
const int kNEmaxvertp = 300;

///Most simple NEUT rooTracker like output format
class NRooTrackerVtxB : public TObject {
public:
  NRooTrackerVtxB();
  ///Resets all data members to default values.
  virtual void Reset();
  virtual ~NRooTrackerVtxB();
  ///Adds data member branches andly sets branch addresses on passed
  ///tree.
  ///
  ///Additionally can be told to add branches for IsBound and StruckNucleonPDG.
  virtual void AddBranches(TTree* &tree,
    bool SaveIsBound=false, bool SaveStruckNucleonPDG=false);
  ///Generator-specific string with 'event code'.
  TObjString* EvtCode;
  ///Event num.
  Int_t EvtNum;
  ///Number of particles in the StdHep particle array.
  Int_t StdHepN;
  //******************* stdhep-like particle array

  ///StdHep particle array of Pdg codes (& generator specific codes for
  ///pseudoparticles)
  Int_t* StdHepPdg; //[StdHepN]
  ///StdHep particle array of generator-specific status code
  ///
  ///Employed here:
  ///-0: Initial State particle
  ///-1: Final State particle
  ///\note: Any others states are being bled in by NEUT not outputting according
  ///to it's documentation. It is safe to say that these are not
  ///'good particles'.
  ///
  ///You can turn off writing of bad particles with the CLI opt '-S'.
  Int_t* StdHepStatus; //[StdHepN]
  ///StdHep particle array of 4-p (px,py,pz,E) of particle in LAB frame (GeV)
  Double_t StdHepP4 [kNStdHepNPmax][4];

  ///Whether this interaction was on a bound, or unbound (H), nucleon.
  Int_t IsBound;

  ///The PDG of the struck nucleon.
  ///
  ///\note This is only useful if running in NuWro rooTracker emulation mode.
  ///otherwise this is just a duplciate of StdHepPdg[1].
  Int_t StruckNucleonPDG;

  ClassDef(NRooTrackerVtxB, 1);
};

///A subclass of NRooTrackerVtxB that adds additional output information.
///
///\warning None of this information is currently filled.
///it is left as an excercise to the user to add this in.
class NRooTrackerVtx : public NRooTrackerVtxB {

///\brief Maximum possible number of saved NFNucleonVertices
///
/// This is set to mirror the equivalent parameter in the NEUT
/// FSI hist header file nucleonfsihist.h
static const int kNFMaxNucleonVert = 200;
///\brief Maximum possible number of saved NFNucleonSteps
///
/// This is set to mirror the equivalent parameter in the NEUT
/// FSI hist header file nucleonfsihist.h
static const int kNFMaxNucleonSteps = 2000;

public:
  NRooTrackerVtx();
  ///Resets all data members to default values.
  void Reset();
  ~NRooTrackerVtx();
  ///Adds data member branches andly sets branch addresses on passed
  ///tree.
  ///
  ///Additionally can be told to add branches for IsBound and StruckNucleonPDG.
  void AddBranches(TTree* &tree,
    bool SaveIsBound=false, bool SaveStruckNucleonPDG=false);

  //****************** Define the output rootracker tree branches

  ///cross section for selected event (1E-38 cm2)
  Double_t EvtXSec;
  ///cross section for selected event kinematics (1E-38 cm2 /{K^n})
  Double_t EvtDXSec;
  ///weight for that event
  Double_t EvtWght;
  ///probability for that event (given cross section, path lengths, etc)
  Double_t EvtProb;
  ///event vertex position in detector coord syst (SI)
  Double_t EvtVtx[4];

  //******************* stdhep-like particle array

  /// 4-x (x, y, z, t) of particle in hit nucleus frame (fm)
  Double_t StdHepX4 [kNStdHepNPmax][4];
  /// polarization vector
  Double_t StdHepPolz [kNStdHepNPmax][3];

  /// first daughter
  Int_t* StdHepFd; //[StdHepN]
  /// last daughter
  Int_t* StdHepLd; //[StdHepN]
  /// first mother
  Int_t* StdHepFm; //[StdHepN]
  /// last mother
  Int_t* StdHepLm; //[StdHepN]

  /// NEUT native VCWORK information
  /// Number of particles
  Int_t NEnvc;
  /// PDG particle code
  Int_t* NEipvc; //[NEnvc]
  /// 3-momentum (MeV/c)
  Float_t NEpvc[kNEmaxvc][3];
  /// Index of parent (Fortran convention: starting at 1)
  Int_t* NEiorgvc; //[NEnvc]

  ///\brief Flag of final state
  ///\details Values:
  /// * 0 : DETERMINED LATER PROCEDURE
  /// * 1 : DECAY TO OTHER PARTICLE
  /// * 2 : ESCAPE FROM DETECTOR
  /// * 3 : ABSORPTION
  /// * 4 : CHARGE EXCHANGE
  /// * 5 : STOP AND NOT CONSIDER IN M.C.
  /// * 6 : E.M. SHOWER
  /// * 7 : HADRON PRODUCTION
  /// * 8 : QUASI-ELASTIC SCATTER
  /// * 9 : FORWARD (ELASTIC-LIKE) SCATTER
  Int_t* NEiflgvc; //[NEnvc]
  /// Escaped nucleus (1) or not (0)
  Int_t* NEicrnvc; //[NEnvc]


  //******** Rest of the NEUT variables below are mainly for internal
  //********************** reweighting routines

  ///\brief Cross section calculation variables (currently used for coherent
  ///interactions)
  Float_t NEcrsx;
  ///\brief Cross section calculation variables (currently used for coherent
  ///interactions)
  Float_t NEcrsy;
  ///\brief Cross section calculation variables (currently used for coherent
  ///interactions)
  Float_t NEcrsz;
  ///\brief Cross section calculation variables (currently used for coherent
  ///interactions)
  Float_t NEcrsphi;


  //**************** NEUT FSIHIST pion interaction history

  /// Number of vertices (including production and exit points)
  Int_t NEnvert;
  /// Position of vertex within nucleus (fm)
  Float_t NEposvert[kNEmaxvert][3];

  ///\brief Interaction type
  ///\details Values:
  /// * (*10 FOR HI-NRG interaction, >~400 MeV/c)
  /// * -1 : ESCAPE
  /// * 0 : INITIAL (or unmatched parent vertex if I>1)
  /// * 3 : ABSORPTION
  /// * 4 : CHARGE EXCHANGE
  /// * 7 : HADRON PRODUCTION (hi-nrg only, i.e. 70)
  /// * 8 : QUASI-ELASTIC SCATTER
  /// * 9 : FORWARD (ELASTIC-LIKE) SCATTER
  Int_t* NEiflgvert; //[NEnvert]

  /// Number of intermediate particles (including initial and final)
  Int_t NEnvcvert;
  /// Direction of particle
  Float_t NEdirvert[kNEmaxvertp][3];


  /// Absolute momentum in the lab frame (MeV/c)
  Float_t* NEabspvert; //[NEnvcvert]
  /// Absolute momentum in the nucleon rest frame (MeV/c)
  Float_t* NEabstpvert; //[NEnvcvert]
  /// PDG particle code
  Int_t*  NEipvert; //[NEnvcvert]
  /// Index of initial vertex (pointing to nvert array above)
  Int_t*  NEiverti; //[NEnvcvert]
  /// Index of final vertex (pointing to nvert array above)
  Int_t*  NEivertf; //[NEnvcvert]

  //**************** NEUT FSIHIST nucleon interaction history

  ///\brief Number of "vertices"
  ///\details Remarks:
  /// *  - a "vertex" is actually better described as a start, end or
  /// *    scattering point of a track
  /// *  - at each scattering point, the first nucleon will be followed in
  /// *    the same track, while the
  /// *    second one will create a new track
  /// *  - each track consists of a series of consecutive vertices. The first
  /// *    vertex has P=0, the last P=4. In between may be any number
  /// *    (including 0) vertices where an actual scattering
  /// *    took place (P=1,2,3).
  /// *  - it is not possible (and not needed) to connect the second track
  /// *    of a scattering vertex with the original one. Note that "first" and
  /// *    "second" is purely arbitrary. For nucleon FSI uncertainties,
  /// *    only the probabilities of the scattering processes have to be
  /// *    calculated, so it is not important to know which tracks belong to
  /// *    each other.
  Int_t NFnvert;
  ///\brief 4-digit flag for interaction type at i-th vertex, in the form "BNTP":
  ///\details Values:
  /// * N: charge nucleon propagated through nucleus (0 = neutron, 1 = proton)
  /// * T: charge "target" nucleon the interaction is taking place on
  /// * P: scattering process:
  /// *    P=0: start tracking of nucleon (i.e. gets "created")
  /// *    P=1: elastic scattering
  /// *    P=2: single pion production
  /// *    P=3: double pion production
  /// *    P=4: stop tracking of nucleon (i.e. leaves nucleus)
  /// * B: Pauli blocking flag (0 = not blocked, 1 = interaction was Pauli
  /// *    blocked and actually did not take place)
  /// Examples:
  /// *  - 103 means double pion production when a proton scattered on a neutron
  /// *  - 1011 means elastic scattering of a neutron on a proton did not take
  /// *    place due to Pauli blocking
  /// \note For P=0 and P=4, "T" is without meaning and always set to 0.
  Int_t* NFiflag; //[NFnvert]

  /// x-component of i-th vertex position inside nucleus
  Float_t* NFx; //[NFnvert]
  /// y-component of i-th vertex position inside nucleus
  Float_t* NFy; //[NFnvert]
  /// z-component of i-th vertex position inside nucleus
  Float_t* NFz; //[NFnvert]
  /// x-component of momentum of nucleon leaving the i-th vertex
  Float_t* NFpx; //[NFnvert]
  /// y-component of momentum of nucleon leaving the i-th vertex
  Float_t* NFpy; //[NFnvert]
  /// z-component of momentum of nucleon leaving the i-th vertex
  Float_t* NFpz; //[NFnvert]
  /// energy of nucleon leaving the i-th vertex
  Float_t* NFe; //[NFnvert]
  /// first step index of this track (to obtain the CMS energies for each step)
  Int_t* NFfirststep; //[NFnvert]
  ///number of steps
  Int_t NFnstep;
  ///CMS energy squared of collision at k-th step (i.e. before interacting).
  /// The sign of this value indicates the charge of the target nucleon:
  ///  NFecms2 > 0: proton,  NFecms2 < 0: neutron (same as "T" in NFiflag)
  Float_t* NFecms2; //[NFnstep]

  ///The name of the generator, in this case always NEUT.
  TString* GeneratorName;

  ClassDef(NRooTrackerVtx, 1);
};
