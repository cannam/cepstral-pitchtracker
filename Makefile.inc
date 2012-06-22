
PLUGIN_EXT	?= .so
PLUGIN	?= simple-cepstrum$(PLUGIN_EXT)
CXX	?= g++
CC	?= gcc

CFLAGS		:= $(CFLAGS) 
CXXFLAGS	:= $(CXXFLAGS) 
LDFLAGS		:= $(LDFLAGS)

HEADERS := SimpleCepstrum.h

SOURCES := SimpleCepstrum.cpp \
           libmain.cpp

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

$(PLUGIN):	$(OBJECTS)
		$(CXX) -o $@ $^ $(LDFLAGS)

clean:		
		rm $(OBJECTS)

distclean:	clean
		rm $(PLUGIN)
