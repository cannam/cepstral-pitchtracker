/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    This file is Copyright (c) 2012 Chris Cannam
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "NoteHypothesis.h"

std::ostream &operator<<(std::ostream &out, const NoteHypothesis::Estimate &n)
{
    return out << "[" << n.freq << "@" << n.time << ":" << n.confidence << "]" << std::endl;
}

std::ostream &operator<<(std::ostream &out, const NoteHypothesis::Estimates &e)
{
    out << "( ";
    for (int i = 0; i < (int)e.size(); ++i) out << e[i] << "; ";
    out << " )";
    return out;
}

std::ostream &operator<<(std::ostream &out, const NoteHypothesis::Note &n)
{
    return out << "[" << n.freq << "@" << n.time << ":" << n.duration << "]" << std::endl;
}

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

using Vamp::RealTime;

BOOST_AUTO_TEST_SUITE(TestNoteHypothesis)

BOOST_AUTO_TEST_CASE(emptyAccept)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e;
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK(h.accept(e));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
}

BOOST_AUTO_TEST_CASE(simpleSatisfy)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 1);
    NoteHypothesis::Estimate e2(500, RealTime::fromMilliseconds(10), 1);
    NoteHypothesis::Estimate e3(500, RealTime::fromMilliseconds(20), 1);
    NoteHypothesis::Estimate e4(500, RealTime::fromMilliseconds(30), 1);
    NoteHypothesis::Estimate e5(500, RealTime::fromMilliseconds(80), 1);
    NoteHypothesis::Estimate e6(500, RealTime::fromMilliseconds(90), 1);
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e2));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
    BOOST_CHECK(h.accept(e4));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
    BOOST_CHECK(!h.accept(e5));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Expired);
    BOOST_CHECK(!h.accept(e6));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Expired);
}
	
BOOST_AUTO_TEST_CASE(strayReject)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 1);
    NoteHypothesis::Estimate e2(1000, RealTime::fromMilliseconds(10), 1);
    NoteHypothesis::Estimate e3(500, RealTime::fromMilliseconds(20), 1);
    NoteHypothesis::Estimate e4(500, RealTime::fromMilliseconds(30), 1);
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(!h.accept(e2));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e4));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
}
		
BOOST_AUTO_TEST_CASE(tooSlow)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 1);
    NoteHypothesis::Estimate e2(500, RealTime::fromMilliseconds(50), 1);
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(!h.accept(e2));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Rejected);
}
	
BOOST_AUTO_TEST_CASE(weakSatisfy)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 0.5);
    NoteHypothesis::Estimate e2(502, RealTime::fromMilliseconds(10), 0.5);
    NoteHypothesis::Estimate e3(504, RealTime::fromMilliseconds(20), 0.5);
    NoteHypothesis::Estimate e4(506, RealTime::fromMilliseconds(30), 0.5);
    NoteHypothesis::Estimate e5(508, RealTime::fromMilliseconds(40), 0.5);
    NoteHypothesis::Estimate e6(510, RealTime::fromMilliseconds(90), 0.5);
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e2));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e4));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e5));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
    BOOST_CHECK(!h.accept(e6));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Expired);
}
	
BOOST_AUTO_TEST_CASE(frequencyRange)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(0), 1);
    NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(10), 1);
    NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(20), 1);
    NoteHypothesis::Estimate e4(470, RealTime::fromMilliseconds(30), 1);
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e2));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
    BOOST_CHECK(!h.accept(e4));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
}

BOOST_AUTO_TEST_CASE(acceptedEstimates)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(0), 1);
    NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(10), 1);
    NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(20), 1);
    NoteHypothesis::Estimate e4(470, RealTime::fromMilliseconds(30), 1);
    NoteHypothesis::Estimate e5(444, RealTime::fromMilliseconds(90), 1);
    NoteHypothesis::Estimates es;
    es.push_back(e1);
    es.push_back(e2);
    es.push_back(e3);
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::New);
    BOOST_CHECK_EQUAL(h.getAcceptedEstimates(), NoteHypothesis::Estimates());
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK_EQUAL(h.getAcceptedEstimates(), NoteHypothesis::Estimates());
    BOOST_CHECK(h.accept(e2));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Provisional);
    BOOST_CHECK_EQUAL(h.getAcceptedEstimates(), NoteHypothesis::Estimates());
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
    BOOST_CHECK_EQUAL(h.getAcceptedEstimates(), es);
    BOOST_CHECK(!h.accept(e4));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Satisfied);
    BOOST_CHECK_EQUAL(h.getAcceptedEstimates(), es);
    BOOST_CHECK(!h.accept(e5));
    BOOST_CHECK_EQUAL(h.getState(), NoteHypothesis::Expired);
    BOOST_CHECK_EQUAL(h.getAcceptedEstimates(), es);
}
	
BOOST_AUTO_TEST_CASE(meanFrequency)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(0), 1);
    NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(10), 1);
    NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(20), 1);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK(h.accept(e2));
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getMeanFrequency(), 444.0);
}

BOOST_AUTO_TEST_CASE(averagedNote)
{
    NoteHypothesis h;
    NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(10), 1);
    NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(20), 1);
    NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(30), 1);
    BOOST_CHECK(h.accept(e1));
    BOOST_CHECK(h.accept(e2));
    BOOST_CHECK(h.accept(e3));
    BOOST_CHECK_EQUAL(h.getAveragedNote(), NoteHypothesis::Note
                      (444,
                       RealTime::fromMilliseconds(10),
                       RealTime::fromMilliseconds(20)));
}

BOOST_AUTO_TEST_SUITE_END()

