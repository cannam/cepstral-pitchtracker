/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/* Copyright Chris Cannam - All Rights Reserved */

#ifndef _NOTE_HYPOTHESIS_H_
#define _NOTE_HYPOTHESIS_H_

#include "base/RealTime.h"
#include <vector>

namespace Turbot {

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
     * Construct an empty hypothesis. This will be in New state and
     * will provisionally accept any estimate.
     */
    NoteHypothesis();

    /**
     * Destroy the hypothesis
     */
    ~NoteHypothesis();

    struct Estimate {
        Estimate() : freq(0), time(), confidence(0) { }
        Estimate(double _f, RealTime _t, double _c) :
            freq(_f), time(_t), confidence(_c) { }
	double freq;
	RealTime time;
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
        Note(double _f, RealTime _t, RealTime _d) :
            freq(_f), time(_t), duration(_d) { }
	double freq;
	RealTime time;
	RealTime duration;
    };
    
    /**
     * Return a single note roughly matching this hypothesis
     */
    Note getAveragedNote() const;
    
private:
    bool isWithinTolerance(Estimate) const;
    bool isOutOfDateFor(Estimate) const;
    bool isSatisfied() const;
    double getMeanFrequency() const;
    
    State m_state;
    Estimates m_pending;
};

}

#endif
