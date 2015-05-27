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
NEUTDEPSO := neutvect.so neutpart.so neutfsipart.so neutfsivert.so neutvtx.so

UTILSLOC=../../../utils
UTILSDEPSO=libPureGenUtils.so

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g -std=c++11 -Wall
LDFLAGS := $(ROOTLDFLAGS) -Wl,-rpath=.

.PHONY: all clean

all: $(TARGET)
	mkdir -p $(BDIR) $(LDIR)
	mv $(TARGET) $(BDIR)/
	mv $(UTILSDEPSO) $(NEUTDEPSO) $(TOBJSO) $(LDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built NeutToRooTracker."
	@echo "*********************************************************************"

$(UTILSDEPSO):
	ln -s `readlink -f $(UTILSLOC)/lib/$@` $@
$(NEUTDEPSO):
	ln -s $(NEUTCLASSDIR)/$@ $@

$(TARGET): $(TARGETSRC) $(TOBJSO) $(NEUTDEPSO) $(UTILSDEPSO)
	$(CXX) -o $@ -I$(NEUTCLASSDIR) $(CXXFLAGS) $(LDFLAGS) -I$(UTILSLOC) $^

PureNeutRooTracker_dict.cxx: PureNeutRooTracker.hxx

PureNeutRooTracker.so: PureNeutRooTracker.cxx PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h
	$(RCINT) -f PureNeutRooTracker_dict.cxx -c -p PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h
	$(CXX) $(CXXFLAGS) -shared -I$(UTILSLOC) -I$(NEUTCLASSDIR) PureNeutRooTracker.cxx PureNeutRooTracker_dict.cxx -o $@

clean:
	rm -rf $(TOBJDICTS)\
        $(TDICTHEADERS)\
        $(UTILSDEPSO)\
        $(TOBJSO)\
	      $(TARGET)\
	      $(NEUTDEPSO)\
        $(BDIR) \
        $(LDIR)

