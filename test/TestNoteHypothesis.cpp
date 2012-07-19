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

#ifndef TEST_NOTE_HYPOTHESIS_H
#define TEST_NOTE_HYPOTHESIS_H

#include "base/NoteHypothesis.h"

#include <QObject>
#include <QtTest>

namespace Turbot {

class TestNoteHypothesis : public QObject
{
    Q_OBJECT

private slots:
    void emptyAccept() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e;
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QVERIFY(h.accept(e));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
    }

    void simpleSatisfy() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 1);
	NoteHypothesis::Estimate e2(500, RealTime::fromMilliseconds(10), 1);
	NoteHypothesis::Estimate e3(500, RealTime::fromMilliseconds(20), 1);
	NoteHypothesis::Estimate e4(500, RealTime::fromMilliseconds(30), 1);
	NoteHypothesis::Estimate e5(500, RealTime::fromMilliseconds(80), 1);
	NoteHypothesis::Estimate e6(500, RealTime::fromMilliseconds(90), 1);
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QVERIFY(h.accept(e1));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e2));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
	QVERIFY(h.accept(e4));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
	QVERIFY(!h.accept(e5));
	QCOMPARE(h.getState(), NoteHypothesis::Expired);
	QVERIFY(!h.accept(e6));
	QCOMPARE(h.getState(), NoteHypothesis::Expired);
    }
	
    void strayReject() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 1);
	NoteHypothesis::Estimate e2(1000, RealTime::fromMilliseconds(10), 1);
	NoteHypothesis::Estimate e3(500, RealTime::fromMilliseconds(20), 1);
	NoteHypothesis::Estimate e4(500, RealTime::fromMilliseconds(30), 1);
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QVERIFY(h.accept(e1));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(!h.accept(e2));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e4));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
    }
		
    void tooSlow() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 1);
	NoteHypothesis::Estimate e2(500, RealTime::fromMilliseconds(50), 1);
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QVERIFY(h.accept(e1));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(!h.accept(e2));
	QCOMPARE(h.getState(), NoteHypothesis::Rejected);
    }
	
    void weakSatisfy() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(500, RealTime::fromMilliseconds(0), 0.5);
	NoteHypothesis::Estimate e2(502, RealTime::fromMilliseconds(10), 0.5);
	NoteHypothesis::Estimate e3(504, RealTime::fromMilliseconds(20), 0.5);
	NoteHypothesis::Estimate e4(506, RealTime::fromMilliseconds(30), 0.5);
	NoteHypothesis::Estimate e5(508, RealTime::fromMilliseconds(40), 0.5);
	NoteHypothesis::Estimate e6(510, RealTime::fromMilliseconds(90), 0.5);
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QVERIFY(h.accept(e1));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e2));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e4));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e5));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
	QVERIFY(!h.accept(e6));
	QCOMPARE(h.getState(), NoteHypothesis::Expired);
    }
	
    void frequencyRange() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(0), 1);
	NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(10), 1);
	NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(20), 1);
	NoteHypothesis::Estimate e4(470, RealTime::fromMilliseconds(30), 1);
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QVERIFY(h.accept(e1));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e2));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
	QVERIFY(!h.accept(e4));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
    }

    void acceptedEstimates() {
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
	QCOMPARE(h.getState(), NoteHypothesis::New);
	QCOMPARE(h.getAcceptedEstimates(), NoteHypothesis::Estimates());
	QVERIFY(h.accept(e1));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QCOMPARE(h.getAcceptedEstimates(), NoteHypothesis::Estimates());
	QVERIFY(h.accept(e2));
	QCOMPARE(h.getState(), NoteHypothesis::Provisional);
	QCOMPARE(h.getAcceptedEstimates(), NoteHypothesis::Estimates());
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
	QCOMPARE(h.getAcceptedEstimates(), es);
	QVERIFY(!h.accept(e4));
	QCOMPARE(h.getState(), NoteHypothesis::Satisfied);
	QCOMPARE(h.getAcceptedEstimates(), es);
	QVERIFY(!h.accept(e5));
	QCOMPARE(h.getState(), NoteHypothesis::Expired);
	QCOMPARE(h.getAcceptedEstimates(), es);
    }
	
    void meanFrequency() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(0), 1);
	NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(10), 1);
	NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(20), 1);
	QVERIFY(h.accept(e1));
	QVERIFY(h.accept(e2));
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getMeanFrequency(), 444.0);
    }

    void averagedNote() {
	NoteHypothesis h;
	NoteHypothesis::Estimate e1(440, RealTime::fromMilliseconds(10), 1);
	NoteHypothesis::Estimate e2(448, RealTime::fromMilliseconds(20), 1);
	NoteHypothesis::Estimate e3(444, RealTime::fromMilliseconds(30), 1);
	QVERIFY(h.accept(e1));
	QVERIFY(h.accept(e2));
	QVERIFY(h.accept(e3));
	QCOMPARE(h.getAveragedNote(), NoteHypothesis::Note
		 (444,
		  RealTime::fromMilliseconds(10),
		  RealTime::fromMilliseconds(20)));
    }

	
    
};

}

#endif
