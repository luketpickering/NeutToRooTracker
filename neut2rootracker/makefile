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

NEUTCHECKO := $(NEUT_ROOT)/src/neutclass/neutvect.o
NEUTCLASSO := neutvect.o neutpart.o neutfsipart.o neutfsivert.o neutvtx.o

#Check if we have nucleon fsi tracking
ifneq ("$(wildcard $(NEUT_ROOT)/src/neutclass/neutnucfsistep.h)","")
	NUCLEON_FSI_TRACKING_DEF = -DHAVE_NUCLEON_FSI_TRACKING
	NEUTCLASSO += neutnucfsivert.o neutnucfsistep.o
endif

NEUTDICTO := $(NEUTCLASSO:.o=Dict.o)
NEUTDEPO := $(addprefix $(NEUT_ROOT)/src/neutclass/,$(NEUTCLASSO))
NEUTDEPO += $(addprefix $(NEUT_ROOT)/src/neutclass/,$(NEUTDICTO))

UTILS_BASE := ../utils
UTILSBUILD := $(UTILS_BASE)/build/`uname`
UTILSINCLUDES := $(UTILSBUILD)/include
LIBUTILS := $(UTILSBUILD)/lib/libLUtils.a
LIBUTILSLD := -L$(UTILSBUILD)/lib -lLUtils

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g -std=c++11 -Wall $(NUCLEON_FSI_TRACKING_DEF) -I$(UTILSINCLUDES)
LDFLAGS := $(ROOTLDFLAGS) $(LIBUTILSLD)

.PHONY: all clean clean_docs

all: $(TARGET)_neutbuild
	mkdir -p $(BDIR)
	mv $(TARGET) $(BDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built NeutToRooTracker."
	@echo "*********************************************************************"

wcmakeneut: $(TARGET)
	mkdir -p $(BDIR)
	mv $(TARGET) $(BDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built NeutToRooTracker."
	@echo "*********************************************************************"

$(NEUTCHECKO):
	@echo "Environment variable NEUT_ROOT is not set or NEUT has not been built "
	@echo "as the neutclass objects can not be found."
	@echo "Expected to find $(NEUTDEPO)"
	@echo "Was NEUT built with cmake? Try $ make wcmakeneut"
	exit 1

ROOTCHECK:
	root -l -b -q

$(LIBUTILS):
	mkdir -p $(UTILS_BASE)/build
	cd $(UTILS_BASE)/build; cmake ../; $(MAKE) install -j4

#If the normal in source NEUT build process has been used then this is the correct target to build
$(TARGET)_neutbuild: $(TARGETSRC) $(TOBJO) $(NEUTDEPO) $(NEUTCHECKO) $(LIBUTILS)
	$(CXX) -o $(TARGET) $< $(CXXFLAGS) -I$(NEUT_ROOT)/src/neutclass $(TOBJO) $(NEUTDEPO) $(LDFLAGS)

#By default we assume that NEUT has been built by CMake and installed somewhere
$(TARGET): $(TARGETSRC) $(TOBJO) $(NEUT_INSTALL_ROOT)/lib/static/libneutclass_static.a $(LIBUTILS)
	$(CXX) -o $(TARGET) $< $(CXXFLAGS) -I$(NEUT_INSTALL_ROOT)/include $(TOBJO) -L$(NEUT_INSTALL_ROOT)/lib/static -lneutclass_static $(LDFLAGS)

PureNeutRooTracker_dict.o: PureNeutRooTracker.hxx ROOTCHECK
	$(RCINT) -f PureNeutRooTracker_dict.cxx -c -p PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h
	$(CXX) $(CXXFLAGS) -c PureNeutRooTracker_dict.cxx -o $@ -I$(NEUT_ROOT)/src/neutclass

PureNeutRooTracker.o: PureNeutRooTracker.cxx PureNeutRooTracker.hxx ROOTCHECK $(LIBUTILS)
	$(CXX) $(CXXFLAGS) -c PureNeutRooTracker.cxx -o $@

clean_docs:
	rm -f NeutToRooTracker_dox.pdf
	cd dox; rm -rf html latex

docs: $(TARGETSRC) $(TOBJS) $(TOBJH) dox/NeutToRooTracker.dox.cfg dox/mainpage.md
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

distclean: clean clean_docs
	rm -rf $(BDIR)
	cd ../utils; rm -rf $(UTILS_BASE)/build
	rm -rf dox/html dox/latex NeutToRooTracker_dox.pdf

