CXX := g++
RCINT := rootcint
RC := root-config

BDIR :=bin

TOBJSRC := PureNeutRooTracker.cxx
TOBJH := $(TOBJSRC:.cxx=.hxx)
TOBJDICTS := $(TOBJSRC:.cxx=_dict.cxx)
TDICTHEADERS := $(TOBJSRC:.cxx=_dict.h)
TOBJLINKDEFS := $(TOBJSRC:.cxx=_linkdef.h)
TOBJO := $(TOBJSRC:.cxx=.o)
TOBJO += $(TOBJDICTS:.cxx=.o)

TARGET := NeutToRooTracker.exe
TARGETSRC := $(TARGET:.exe=.cxx)

NEUTCHECKO := $(NEUTCLASSLOC)/neutvect.o
NEUTCLASSO := neutvect.o neutpart.o neutfsipart.o neutfsivert.o neutvtx.o
NEUTDICTO := $(NEUTCLASSO:.o=Dict.o)
NEUTDEPO := $(addprefix $(NEUTCLASSLOC)/,$(NEUTCLASSO))
NEUTDEPO += $(addprefix $(NEUTCLASSLOC)/,$(NEUTDICTO))

UTILSLOC := ../utils
LIBUTILS := $(UTILSLOC)/lib/libPureGenUtils.a
LIBUTILSLD := -L$(UTILSLOC)/lib -lPureGenUtils

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g -std=c++11 -Wall
LDFLAGS := $(ROOTLDFLAGS) $(LIBUTILSLD) -Wl,-rpath=. -Wl,--start-group

.PHONY: all clean

all: $(TARGET)
	mkdir -p $(BDIR)
	mv $(TARGET) $(BDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built NeutToRooTracker."
	@echo "*********************************************************************"

$(LIBUTILS):
	cd $(UTILSLOC); $(MAKE)

$(NEUTCHECKO):
	@echo "env NEUTCLASSLOC is not set or NEUT has not been built as the neutclass libraries cannot be found."
	`exit 1`

ROOTCHECK:
	root -l -b -q

$(TARGET): $(TARGETSRC) $(TOBJO) $(NEUTDEPO) $(NEUTCHECKO) $(LIBUTILS)
	$(CXX) -o $@ $< $(CXXFLAGS) -I$(NEUTCLASSLOC) -I$(UTILSLOC) $(TOBJO) $(NEUTDEPO) $(LDFLAGS)

PureNeutRooTracker_dict.o: PureNeutRooTracker.hxx ROOTCHECK
	$(RCINT) -f PureNeutRooTracker_dict.cxx -c -p PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h
	$(CXX) $(CXXFLAGS) -c PureNeutRooTracker_dict.cxx -o $@ -I$(UTILSLOC) -I$(NEUTCLASSLOC)

PureNeutRooTracker.o: PureNeutRooTracker.cxx PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h PureNeutRooTracker_dict.cxx ROOTCHECK
	$(CXX) $(CXXFLAGS) -c PureNeutRooTracker.cxx -o $@ -I$(UTILSLOC) -I$(NEUTCLASSLOC)

docs: $(TARGETSRC) $(TOBJS) $(TOBJH) dox/NeutToRooTracker.dox.cfg
	rm -f *dict.*
	cd dox; doxygen NeutToRooTracker.dox.cfg;

latex_docs: docs
	cd dox/latex; $(MAKE); $(MAKE);
	cp dox/latex/refman.pdf ./NeutToRooTracker_dox.pdf

clean:
	rm -f $(TOBJDICTS)\
        $(TDICTHEADERS)\
				$(TARGET)\
				$(TOBJO)

distclean: clean
	rm -rf $(BDIR)
	cd ../utils; $(MAKE) clean
	rm -rf dox/html dox/latex NeutToRooTracker_dox.pdf

