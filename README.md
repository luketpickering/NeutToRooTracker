# NeutToRooTracker
A tool for converting the output of the neut simple generator to a rooTracker-like format.

Background:
-----------
NEUT is a neutrino event generator. 
It is not currently open source, but is in use by a number of collaborations in the neutrino physics community.
It is often useful to compare the predictions of multiple generators to each other, to facilitate this it is useful to have a common format.
The other two major, modern neutrino event generators [GENIE](http://genie.hepforge.org/) (https://github.com/GENIEMC) and [NuWro](http://borg.ift.uni.wroc.pl/nuwro/) (https://github.com/cjusz/nuwro) are both able to have their outputs converter to a 'rooTracker'-like output with bundled executables.
NEUT is able to produce an output in this format, but only when running in full detector geometry simulation mode.
This program aims to allow the output of the simple neutrino event generator module of NEUT to be simpley converted to a compatible format for a unified analysis.

ADVERT: See also for [GiBUU](https://gibuu.hepforge.org/) (https://github.com/luketpickering/GiBUU-t2k-dev).

Building:
=========

Dependencies:
-------------
 - [ROOT](http://root.cern.ch/): https://github.com/root-mirror/root
 - [NEUT](http://dx.doi.org/10.1016/S0920-5632(02)01759-0): Not freely available. Sorry.

To Build:
---------
Fetch subprojects:

    $ git submodule init; git submodule update

Executable:

    $ export NEUTCLASSLOC=</path/to/neut>/src/neutclass
    $ source /<path/to/root>/bin/thisroot.sh
    $ source setup.sh
    $ cd neut2rootracker;

Documentation:
    
    $ cd neut2rootracker; make [docs|latex_docs]


Usage:
======

As generated when ` NeutToRooTracker.exe -h` is invoked:

    Run like:
      NeutToRooTracker.exe -i <TChain::Add descriptor> [-h] [-o <File Name>{default=vector.ntrac.root}] [-n <-1>: Read all {default=-1}] [-v <0-4>{default=0}] [-G] [-O] [-b] [-L] [-E] [-S] [-I <int,int,...> NEUT modes to save output from.]

      -----------------------------------

    [Arg]: (-h|--help)
    [Arg]: (-i|--input-file) <TChain::Add descriptor> [Required]
    [Arg]: (-o|--output-file) <File Name>{default=vector.ntrac.root}
    [Arg]: (-n|--nentries) <-1>: Read all {default=-1}
    [Arg]: (-v|--verbosity) <0-4>{default=0}
    [Arg]: (-G|--GeV-mode)
    [Arg]: (-O|--objectify-output)
    [Arg]: (-b|--save-isbound)
    [Arg]: (-L|--Lite-Mode)
    [Arg]: (-E|--Emulate-NuWro)
    [Arg]: (-S|--Skip-non-FS)
    [Arg]: (-I|--Ignore-NEUT-Modes) <int,int,...> NEUT modes to save output from.

