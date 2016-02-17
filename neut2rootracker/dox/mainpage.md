# NeutToRooTracker

A tool for converting the output of the neut simple generator (`neutroot2`) to
a RooTracker-like format.

## Introduction

NEUT is a neutrino event generator.
It is not currently open source, but is in use by a number of collaborations
in the neutrino physics community. It is often useful to compare the predictions
of multiple generators to each other, to facilitate this it is useful to have a
common format. The other two major, modern neutrino event generators
[GENIE](http://genie.hepforge.org/) (https://github.com/GENIEMC) and
[NuWro](http://borg.ift.uni.wroc.pl/nuwro/) (https://github.com/cjusz/nuwro) are
both able to have their outputs converter to a 'rooTracker'-like output with
bundled executables. NEUT is able to produce an output in this format, but only
when running in full detector geometry simulation mode. This program aims to
allow the output of the simple neutrino event generator module of NEUT to be
simply converted to a compatible format for a unified analysis.

## Building:

### Dependencies:

 - [ROOT](http://root.cern.ch/): https://github.com/root-mirror/root
 - [NEUT](http://dx.doi.org/10.1016/S0920-5632%2802%2901759-0): Not freely
 available. Sorry.

### To Build:

#### Clone the repo:

    $ git clone https://github.com/luketpickering/NeutToRooTracker.git

#### Fetch sub-projects:

    $ git submodule init; git submodule update

#### Compile Executable:

Depending on how your `NEUT` install is built export an environment variable to
point to the source or binary install tree.

1. Using default `NEUT` build scripts:

    `$ export NEUT_ROOT=</path/to/neut>`

1. Using cmake built `NEUT`:

    `$ export NEUT_INSTALL_ROOT=</path/to/neut/install>`

Your `NEUT` install must be successfully built, and this environment variable
set before attempting to build `NeutToRooTracker.exe`

    $ source /<path/to/root>/bin/thisroot.sh
    $ source setup.sh

This will add the `bin` directory to your `$PATH` variable if it isn't already
included.

    $ cd neut2rootracker; make

#### Generate the Documentation:

    $ cd neut2rootracker; make [docs|latex_docs]

## Usage -- Command Line Arguments:

Command line options all have a short form and a long form and either take 1 or
0 arguments.

 * `-i|--input-file <TChain::Add descriptor>` [**required**]:

    Input file descriptor.
    Input files are `NEUT` vector format files, as produced by the `neutroot2`
    executable.
    This string is passed to `TChain::Add` so may contain wildcards to
    read multiple input files.
    **N.B.** if wildcards are used the argument should be wrapped in
    double quotes to stop the calling shell expanding the wildcard.

 * `-o|--output-file <output_file.root>`:

    The name of the output `rootracker` file.
    Defaults to vector.ntrac.root.

 * `-n|--nentries <integer>`:

    Maximum number of input file entries to process.
    Defaults to `-1` which specifies that all input entries should be processed.

 * `-v|--verbosity`:
 * `-G|--GeV-mode`:

    Assumes all input units are MeV and scales all energy and momentum
    values by `1.0/1000.0`.

 * `-O|--objectify-output`:

    If enabled, the output tree will contain a single branch of
    `NRooTrackerVtx` (or `NRooTrackerVtxB` in the case of Lite output mode)
    instances with data members exposing the `RooTracker` information.

 * `-b|--save-isbound`:

    For a target with extra free protons, such as CH, `NEUT` will not specify
    a different target PDG code if the event happened on a free proton.
    This option adds an extra branch to the output tree which specifies whether
    the interaction occured off of a bound nucleon.

 * `-L|--Lite-Mode`:

    Output a significantly reduced data set for smaller output files.
    This will not output any of the `NEUT`-specific interaction information or
    FSI history.

 * `-E|--Emulate-NuWro`:

    Outputs a slightly modified `StdHep` flavor which more closesly mimics the
    `NuWro` `RooTracker` output.
    The target nucleon and target nucleus are condensed into a single `StdHep`
    entry with the PDG specifying the target nucleus and the `StdHepP4` being
    that of the target nucleon.

 * `-S|--Skip-non-FS`:

    If enabled, the output `StdHep` tree will contain no final state particles
    that did not make it out of the nucleus
    (`NEUT` `fStatus!=[0,2]`, `fIsAlive!=1`).
    **N.B.** If not using Lite mode then all final state information will be
    available in the `NEUT` NE and NF common block output.

 * `-I|--Ignore-NEUT-Modes <integer,integer,integer,...>`:

    Accepts a comma separated list of `NEUT` interaction mode specifiers to
    ignore.
    Input entries with one of the ingored modes are not processed and not saved
    to the output file.
    A digest of ignored entries is written to `stdout` at the end of processing.

## RooTracker format description:

The basic `StdHep` portion of the `NEUT` flavor RooTracker format is described
here:

  * `Int_t EvtCode //Interaction Mode --- CCQE == 1
  * `Int_t StdHepN //Number of particles stored in other StdHep Arrays
  * `Int_t StdHepStatus[$i<$StdHepN]` //Status code of particle i --- 0 ==
incoming, 11 == incoming nucleus (if not using `-E`), 1 == good final state,
anything else == intermediate or unseeable particle.
  * `Int_t StdHepPDG[i<StdHepN]` //PDG code of particle i --- 14 == nu_mu,
13 == mu, 2212 == p, 2112 == n
  * `Int_t StdHepP4[i<StdHepN][4]` //Four momentum of particle i ---
StdHepP4[i][3] == E_i.

For other branches that are included when not running with `-L` see the member
documentation of NRooTrackerVtx.

## Things to be aware of

 * **Output by default is in units of MeV:** To output in GeV pass `-G` CLI
  option.

 * **If the StdHepStatus != 1 then you didn't see that particle**. Always check
   the StdHepStatus.

 * For input files which contain NEUT flux and event rate histograms the EvtWght
   and NEntriesInFile branches of non-Lite mode are filled.
   `EvtWght = event_rate->Integral()/flux->Integral()`. To correctly normalise
   a flux averaged final state property histogram each entry should be weighted
   by `EvtWght*1E-38/NEntriesInFile`, this will put the y axis in units of
   `cm^2 nucleon^-1`. **N.B.** it is a good idea to also scale by bin
   width^-1.

 * `NEUT` will output Pauli blocked events, these will make it through to the
   final output and will be events with no `StdHepStatus[i]==1` final state
   particles.

 * If not emulating NuWro the struck nucleon entry will have
  `StdHepStatus==11` similar to GENIE.


