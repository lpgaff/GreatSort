# Makefile for GreatSort
.PHONY: clean all doc

PWD			:= $(shell pwd)
BIN_DIR     := ./bin
SRC_DIR     := ./src
LIB_DIR     := ./lib
INC_DIR     := ./include
DOC_DIR		:= ./doc
CUR_DIR		:= \"$(PWD)/\"
AME_FILE    := \"$(PWD)/data/mass_1.mas20\"

ROOTVER     := $(shell root-config --version | head -c1)
ifeq ($(ROOTVER),5)
	ROOTDICT  := rootcint
	DICTEXT   := .h
else
	ROOTDICT  := rootcling
	DICTEXT   := _rdict.pcm
endif

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM),Darwin)
SHAREDSWITCH = -Qunused-arguments -shared -undefined dynamic_lookup -dynamiclib -Wl,-install_name,'@executable_path/../lib/'# NO ENDING SPACE
OSDEF = -DMACOSX
else
SHAREDSWITCH = -shared -Wl,-soname,# NO ENDING SPACE
OSDEF = -DLINUX
LIBEXTRA = -lrt
endif

# Documentation
DOC			:= doxygen
DOC_FILE	:= Doxyfile
DOC_HTML	:= documentation.html

ROOTCPPFLAGS	:= $(shell root-config --cflags)
ROOTLDFLAGS		:= $(shell root-config --ldflags)
ROOTLIBS		:= $(shell root-config --glibs) -lRHTTP -lThread
LIBS			:= $(ROOTLIBS) $(LIBEXTRA)

# Compiler.
CXX          = $(shell root-config --cxx)
CC           = $(shell root-config --cc)

# Flags for compiler.
CPPFLAGS	 = -c -Wall -Wextra $(ROOTCPPFLAGS) -g -fPIC -O3
CPPFLAGS	+= -DUNIX -DPOSIX $(OSDEF)
INCLUDES	+= -I$(INC_DIR) -I.

# Pass in the data file locations
CPPFLAGS		+= -DAME_FILE=$(AME_FILE)
CPPFLAGS		+= -DSRIM_DIR=$(SRIM_DIR)
CPPFLAGS		+= -DCUR_DIR=$(CUR_DIR)

# Linker.
LD          = $(shell root-config --ld)
# Flags for linker.
LDFLAGS 	+= $(ROOTLDFLAGS) -g

# The object files.
OBJECTS =  		$(SRC_DIR)/Calibration.o \
				$(SRC_DIR)/CommandLineInterface.o \
				$(SRC_DIR)/Converter.o \
				$(SRC_DIR)/DataPackets.o \
				$(SRC_DIR)/DataSpy.o \
				$(SRC_DIR)/EventBuilder.o \
				$(SRC_DIR)/Histogrammer.o \
				$(SRC_DIR)/GreatEvts.o \
				$(SRC_DIR)/GreatGUI.o \
				$(SRC_DIR)/Reaction.o \
				$(SRC_DIR)/Settings.o

# The header files.
DEPENDENCIES =  $(INC_DIR)/Calibration.hh \
				$(INC_DIR)/CommandLineInterface.hh \
				$(INC_DIR)/Converter.hh \
				$(INC_DIR)/DataPackets.hh \
				$(INC_DIR)/DataSpy.hh \
				$(INC_DIR)/EventBuilder.hh \
				$(INC_DIR)/Histogrammer.hh \
				$(INC_DIR)/GreatEvts.hh \
				$(INC_DIR)/GreatGUI.hh \
				$(INC_DIR)/Reaction.hh \
				$(INC_DIR)/Settings.hh

all: $(BIN_DIR)/great_sort $(LIB_DIR)/libgreat_sort.so
 
$(LIB_DIR)/libgreat_sort.so: great_sort.o $(OBJECTS) great_sortDict.o
	mkdir -p $(LIB_DIR)
	$(LD) great_sort.o $(OBJECTS) great_sortDict.o $(SHAREDSWITCH)$@ $(LIBS) -o $@

$(BIN_DIR)/great_sort: great_sort.o $(OBJECTS) great_sortDict.o
	mkdir -p $(BIN_DIR)
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)

great_sort.o: great_sort.cc
	$(CXX) $(CPPFLAGS) $(INCLUDES) $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(INC_DIR)/%.hh
	$(CXX) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

great_sortDict.o: great_sortDict.cc great_sortDict$(DICTEXT) $(INC_DIR)/RootLinkDef.h
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	$(CXX) -fPIC $(CPPFLAGS) $(INCLUDES) -c $<
	cp great_sortDict$(DICTEXT) $(BIN_DIR)/
	cp great_sortDict$(DICTEXT) $(LIB_DIR)/

great_sortDict.cc: $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h
	$(ROOTDICT) -f $@ -c $(INCLUDES) $(DEPENDENCIES) $(INC_DIR)/RootLinkDef.h

# Generated by the rule above.
great_sortDict$(DICTEXT): great_sortDict.cc

test:
	tests/test_settings.sh

clean:
	rm -vf $(BIN_DIR)/great_sort $(SRC_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*.gch *.o $(BIN_DIR)/*.pcm *.pcm $(BIN_DIR)/*Dict* *Dict* $(LIB_DIR)/*

doc:
	mkdir -p $(DOC_DIR)
	$(DOC) $(DOC_FILE)
	ln -sf $(DOC_DIR)/index.html $(DOC_HTML)

doc-clean:
	rm -rvf $(DOC_DIR)/* $(DOC_DIR)/.??* $(DOC_HTML) 
