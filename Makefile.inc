
PLUGIN_EXT	?= .so
PLUGIN	?= cepstral-pitchtracker$(PLUGIN_EXT)
CXX	?= g++
CC	?= gcc

CFLAGS		:= $(CFLAGS) 
CXXFLAGS	:= $(CXXFLAGS) 
LDFLAGS		:= $(LDFLAGS)

HEADERS := CepstralPitchTracker.h \
	   NoteHypothesis.h

SOURCES := CepstralPitchTracker.cpp \
	   NoteHypothesis.cpp \
           libmain.cpp

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

all: $(PLUGIN)

$(PLUGIN):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)

clean:		
		rm $(OBJECTS)

distclean:	clean
		rm $(PLUGIN)

libmain.o:	$(HEADERS) $(SOURCES)
SimpleCepstrum.o:	$(HEADERS) $(SOURCES)
