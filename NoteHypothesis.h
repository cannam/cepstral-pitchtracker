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

#ifndef _NOTE_HYPOTHESIS_H_
#define _NOTE_HYPOTHESIS_H_

#include "vamp-sdk/RealTime.h"
#include <vector>

/**
 * An agent used to test an incoming series of instantaneous pitch
 * estimates to see whether they fit a consistent single-note
 * relationship. Contains rules specific to testing note pitch and
 * timing.
 */

class NoteHypothesis
{
public:
    enum State {

        /// Just constructed, will provisionally accept any estimate
        New,

        /// Accepted at least one estimate, but not enough evidence to satisfy
        Provisional,

        /// Could not find enough consistency in offered estimates
        Rejected,

        /// Have accepted enough consistent estimates to satisfy hypothesis
        Satisfied,

        /// Have been satisfied, but evidence has now changed: we're done
        Expired
    };
    
    /**
     * Construct an empty hypothesis. The given slack (in
     * milliseconds) determines how long the hypothesis is prepared to
     * tolerate unacceptable estimates in between accepted estimates
     * before it becomes rejected. A reasonable default is 40ms.
     *
     * This hypothesis will be in New state and will provisionally
     * accept any estimate.
     */
    NoteHypothesis(float slack);

    /**
     * Destroy the hypothesis
     */
    ~NoteHypothesis();

    struct Estimate {
        Estimate() : freq(0), time(), confidence(1) { }
        Estimate(double _f, Vamp::RealTime _t, double _c) :
            freq(_f), time(_t), confidence(_c) { }
        bool operator==(const Estimate &e) const {
            return e.freq == freq && e.time == time && e.confidence == confidence;
        }
        double freq;
        Vamp::RealTime time;
        double confidence;
    };
    typedef std::vector<Estimate> Estimates;

    /**
     * Test the given estimate to see whether it is consistent with
     * this hypothesis, and adjust the hypothesis' internal state
     * accordingly. If the estimate is not inconsistent with the
     * hypothesis, return true.
     */
    bool accept(Estimate);

    /**
     * Return the current state of this hypothesis.
     */
    State getState() const;

    /**
     * If the hypothesis has been satisfied (i.e. is in Satisfied or
     * Expired state), return the series of estimates that it
     * accepted. Otherwise return an empty list
     */
    Estimates getAcceptedEstimates() const;

    struct Note {
        Note() : freq(0), time(), duration() { }
        Note(double _f, Vamp::RealTime _t, Vamp::RealTime _d) :
            freq(_f), time(_t), duration(_d) { }
        bool operator==(const Note &e) const {
            return e.freq == freq && e.time == time && e.duration == duration;
        }
        double freq;
        Vamp::RealTime time;
        Vamp::RealTime duration;
    };
    
    /**
     * Return the time of the first accepted estimate
     */
    Vamp::RealTime getStartTime() const;

    /**
     * Return the mean frequency of the accepted estimates
     */
    double getMeanFrequency() const;

    /**
     * Return a single note roughly matching this hypothesis
     */
    Note getAveragedNote() const;
    
private:
    bool isWithinTolerance(Estimate) const;
    bool isOutOfDateFor(Estimate) const;
    bool isSatisfied() const;
    
    State m_state;
    Estimates m_pending;
    float m_slack;
};

#endif
