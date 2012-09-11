
PLUGIN_EXT	?= .so

CXX	?= g++
CC	?= gcc

CFLAGS := $(CFLAGS) 
CXXFLAGS := -I. $(CXXFLAGS)

LDFLAGS := $(LDFLAGS) -lvamp-sdk
PLUGIN_LDFLAGS := $(LDFLAGS) $(PLUGIN_LDFLAGS)
TEST_LDFLAGS := $(LDFLAGS) -lboost_unit_test_framework

PLUGIN := cepstral-pitchtracker$(PLUGIN_EXT)

HEADERS := CepstralPitchTracker.h \
           MeanFilter.h \
	   NoteHypothesis.h \
	   PeakInterpolator.h

SOURCES := CepstralPitchTracker.cpp \
	   NoteHypothesis.cpp \
	   PeakInterpolator.cpp

PLUGIN_MAIN := libmain.cpp

TESTS := test/test-notehypothesis \
         test/test-meanfilter \
         test/test-peakinterpolator

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

PLUGIN_OBJECTS := $(OBJECTS) $(PLUGIN_MAIN:.cpp=.o)

all: $(PLUGIN) $(TESTS)
	for t in $(TESTS); do echo "Running $$t"; ./"$$t" || exit 1; done

$(PLUGIN): $(PLUGIN_OBJECTS)
	$(CXX) -o $@ $^ $(PLUGIN_LDFLAGS)

test/test-notehypothesis: test/TestNoteHypothesis.o $(OBJECTS)
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

test/test-meanfilter: test/TestMeanFilter.o $(OBJECTS)
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

test/test-peakinterpolator: test/TestPeakInterpolator.o $(OBJECTS)
	$(CXX) -o $@ $^ $(TEST_LDFLAGS)

clean:		
		rm -f $(OBJECTS) test/*.o

distclean:	clean
		rm -f $(PLUGIN) $(TESTS)

# DO NOT DELETE

CepstralPitchTracker.o: CepstralPitchTracker.h NoteHypothesis.h
libmain.o: CepstralPitchTracker.h NoteHypothesis.h
NoteHypothesis.o: NoteHypothesis.h
PeakInterpolator.o: PeakInterpolator.h
test/TestNoteHypothesis.o: NoteHypothesis.h
test/TestPeakInterpolator.o: PeakInterpolator.h
