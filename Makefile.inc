
PLUGIN_EXT	?= .so
PLUGIN	?= cepstral-pitchtracker$(PLUGIN_EXT)
CXX	?= g++
CC	?= gcc

CFLAGS		:= $(CFLAGS) 
CXXFLAGS	:= $(CXXFLAGS) 
LDFLAGS		:= $(LDFLAGS)

HEADERS := CepstralPitchTracker.h

SOURCES := CepstralPitchTracker.cpp \
           libmain.cpp

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

$(PLUGIN):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)

clean:		
		rm $(OBJECTS)

distclean:	clean
		rm $(PLUGIN)

libmain.o:	$(HEADERS) $(SOURCES)
SimpleCepstrum.o:	$(HEADERS) $(SOURCES)
