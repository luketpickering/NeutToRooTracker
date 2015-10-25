CXX := g++
RCINT := rootcint
RC := root-config

BDIR :=bin
LDIR :=lib

TOBJS := PureNeutRooTracker.cxx
TOBJH := $(TOBJS:.cxx=.hxx)
TOBJDICTS := $(TOBJS:.cxx=_dict.cxx)
TDICTHEADERS := $(TOBJS:.cxx=_dict.h)
TOBJLINKDEFS := $(TOBJS:.cxx=_linkdef.h)
TOBJSO := $(TOBJS:.cxx=.so)

TARGET := NeutToRooTracker.exe
TARGETSRC := $(TARGET:.exe=.cxx)

NEUTCLASSDIR := $(NEUTCLASSLOC)
NEUTCHECKSO := $(NEUTCLASSDIR)/neutvect.so
NEUTDEPSO := neutvect.so neutpart.so neutfsipart.so neutfsivert.so neutvtx.so

UTILSLOC=../utils
LIBUTILS=libPureGenUtils.so
UTILSDEPSO:=$(UTILSLOC)/lib/$(LIBUTILS)

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g -std=c++11 -Wall
LDFLAGS := $(ROOTLDFLAGS) -Wl,-rpath=.

.PHONY: all clean

all: $(TARGET)
	mkdir -p $(BDIR) $(LDIR)
	mv $(TARGET) $(BDIR)/
	mv $(LIBUTILS) $(NEUTDEPSO) $(TOBJSO) $(LDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built NeutToRooTracker."
	@echo "*********************************************************************"

$(LIBUTILS):
	cd $(UTILSLOC); $(MAKE)
	ln -s `readlink -f $(UTILSDEPSO)` $@

$(NEUTCHECKSO):
	@echo "env NEUTCLASSLOC is not set or NEUT has not been built as the neutclass libraries cannot be found."
	`exit 1`

$(NEUTDEPSO): $(NEUTCHECKSO)
	ln -s $(NEUTCLASSDIR)/$@ $@

ROOTCHECK:
	root -l -b -q

$(TARGET): $(TARGETSRC) $(TOBJSO) $(NEUTDEPSO) $(LIBUTILS)
	$(CXX) -o $@ -I$(NEUTCLASSDIR) $(CXXFLAGS) $(LDFLAGS) -I$(UTILSLOC) $^

PureNeutRooTracker_dict.cxx: PureNeutRooTracker.hxx ROOTCHECK
	$(RCINT) -f PureNeutRooTracker_dict.cxx -c -p PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h

PureNeutRooTracker.so: PureNeutRooTracker.cxx PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h PureNeutRooTracker_dict.cxx ROOTCHECK
	$(CXX) $(CXXFLAGS) -shared -I$(UTILSLOC) -I$(NEUTCLASSDIR) PureNeutRooTracker.cxx PureNeutRooTracker_dict.cxx -o $@

docs: $(TARGETSRC) $(TOBJS) $(TOBJH) dox/NeutToRooTracker.dox.cfg
	rm -f *dict.*
	cd dox; doxygen NeutToRooTracker.dox.cfg;

latex_docs: docs
	cd dox/latex; $(MAKE); $(MAKE);
	cp dox/latex/refman.pdf ./NeutToRooTracker_dox.pdf

clean:
	rm -rf $(TOBJDICTS)\
        $(TDICTHEADERS)\
        $(LIBUTILS)\
        $(TOBJSO)\
	$(TARGET)\
	$(NEUTDEPSO)\
        $(BDIR) \
        $(LDIR)
	cd ../utils; $(MAKE) clean
	rm -rf dox/html dox/latex NeutToRooTracker_dox.pdf

