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

#ifndef _AGENT_FEEDER_H_
#define _AGENT_FEEDER_H_

#include "NoteHypothesis.h"

#include <vector>

/**
 * Take a series of estimates (one at a time) and feed them to a set
 * of note hypotheses, creating a new candidate hypothesis for each
 * observation and also testing the observation against the existing
 * set of hypotheses.
 *
 * One satisfied hypothesis is considered to be "accepted" at any
 * moment (that is, the earliest contemporary hypothesis to have
 * become satisfied). The series of accepted and completed hypotheses
 * from construction to the present time can be queried through
 * getAcceptedHypotheses().
 *
 * Call feed() to provide a new observation. Call finish() when all
 * observations have been provided. The set of hypotheses returned by
 * getAcceptedHypotheses() will not be complete unless finish() has
 * been called.
 */
class AgentFeeder
{
public:
    AgentFeeder() : m_haveCurrent(false) { }

    void feed(NoteHypothesis::Estimate);
    void finish();

    typedef std::vector<NoteHypothesis> Hypotheses;

    const Hypotheses &getAcceptedHypotheses() const {
        return m_accepted;
    }

    Hypotheses reap(Hypotheses);

private:
    Hypotheses m_candidates;
    NoteHypothesis m_current;
    bool m_haveCurrent;
    Hypotheses m_accepted;
};


#endif

