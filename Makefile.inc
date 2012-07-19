
PLUGIN_EXT	?= .so
PLUGIN	?= cepstral-pitchtracker$(PLUGIN_EXT)
CXX	?= g++
CC	?= gcc

CFLAGS		:= $(CFLAGS) 
CXXFLAGS	:= $(CXXFLAGS) 
LDFLAGS		:= $(LDFLAGS) -lvamp-sdk

HEADERS := CepstralPitchTracker.h \
	   NoteHypothesis.h \
	   PeakInterpolator.h

SOURCES := CepstralPitchTracker.cpp \
	   NoteHypothesis.cpp \
	   PeakInterpolator.cpp \
           libmain.cpp

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

all: $(PLUGIN)

$(PLUGIN):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)
		$(MAKE) -C test

clean:		
		rm -f $(OBJECTS)
		$(MAKE) -C test clean

distclean:	clean
		rm -f $(PLUGIN)
		$(MAKE) -C test distclean

libmain.o:	$(HEADERS) $(SOURCES)
CepstralPitchTracker.o:	$(HEADERS) $(SOURCES)
NoteHypothesis.o:	$(HEADERS) $(SOURCES)
PeakInterpolator.o:	$(HEADERS) $(SOURCES)
