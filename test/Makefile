CXX	?= g++
CC	?= gcc

CFLAGS		:= $(CFLAGS) 
CXXFLAGS	:= $(CXXFLAGS) -I..
LDFLAGS		:= $(LDFLAGS) -lvamp-sdk -lboost_unit_test_framework

SUPER_OBJECTS	:= ../NoteHypothesis.o

tests:	TestNoteHypothesis.o
	$(CXX) -o $@ $^ $(SUPER_OBJECTS) $(LDFLAGS)
