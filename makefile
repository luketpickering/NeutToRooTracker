CXX := g++
RCINT := rootcint
RC := root-config

TOBJS := PureNeutRooTracker.cxx
TOBJH := $(TOBJS:.cxx=.hxx)
TOBJDICTS := $(TOBJS:.cxx=_dict.cxx)
TOBJLINKDEFS := $(TOBJS:.cxx=_linkdef.h)
TOBJSO := $(TOBJS:.cxx=.so)

TARGET := NeutToRooTracker.exe
TARGETSRC := $(TARGET:.exe=.cxx)

NEUTCLASSDIR := ../../neutclass
NEUTDEPSO := neutvect.so neutpart.so neutfsipart.so neutfsivert.so neutvtx.so

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g
LDFLAGS := $(ROOTLDFLAGS) -Wl,-rpath=.

.PHONY: all clean

all: $(TARGET)
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Note you will need to run me from here or add this directory"
	@echo "to your LD_LIBRARY_PATH."
	@echo "*********************************************************************"


$(NEUTDEPSO):
	ln -s $(NEUTCLASSDIR)/$@ $@

$(TARGET): $(TARGETSRC) $(TOBJSO) $(NEUTDEPSO)
	$(CXX) -o $@ -I$(NEUTCLASSDIR) $(CXXFLAGS) $(LDFLAGS) $^

PureNeutRooTracker_dict.cxx: PureNeutRooTracker.hxx

PureNeutRooTracker.so: PureNeutRooTracker.cxx PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h
	$(RCINT) -f PureNeutRooTracker_dict.cxx -c -p PureNeutRooTracker.hxx PureNeutRooTracker_linkdef.h
	$(CXX) $(CXXFLAGS) -shared -I$(NEUTCLASSDIR) PureNeutRooTracker.cxx PureNeutRooTracker_dict.cxx -o $@

clean:
	rm -f $(TOBJDICTS)\
			  $(TOBJSO)\
			  $(TARGET)\
			  $(NEUTDEPSO)
