CXX	?= g++
CC	?= gcc

CFLAGS		:= $(CFLAGS) 
CXXFLAGS	:= $(CXXFLAGS) -I..
LDFLAGS		:= $(LDFLAGS) -lvamp-sdk -lboost_unit_test_framework

SUPER_OBJECTS	:= ../NoteHypothesis.o ../PeakInterpolator.o

OBJECTS		:= TestNoteHypothesis.o TestPeakInterpolator.o

TESTS		:= test-notehypothesis test-peakinterpolator

all:	run-tests

test-notehypothesis:	TestNoteHypothesis.o
	$(CXX) -o $@ $^ $(SUPER_OBJECTS) $(LDFLAGS)

test-peakinterpolator:	TestPeakInterpolator.o
	$(CXX) -o $@ $^ $(SUPER_OBJECTS) $(LDFLAGS)

run-tests:	$(TESTS)
		for t in $(TESTS); do ./"$$t" || exit 1; done

clean:	
	rm -f $(OBJECTS)

distclean:	clean
	rm -f $(TESTS)

