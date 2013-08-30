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

#include "AgentFeeder.h"

void AgentFeeder::feed(NoteHypothesis::Estimate e)
{
    if (m_haveCurrent) {
        if (m_current.accept(e)) {
            return;
        }
        if (m_current.getState() == NoteHypothesis::Expired) {
            m_accepted.push_back(m_current);
            m_haveCurrent = false;
        }
    }

    bool swallowed = false;

    Hypotheses newCandidates;

    for (Hypotheses::iterator i = m_candidates.begin();
         i != m_candidates.end(); ++i) {
        
        NoteHypothesis h = *i;
        
        if (swallowed) {
            
            // don't offer: each observation can only belong to one
            // satisfied hypothesis
            newCandidates.push_back(h);
            
        } else {
            
            if (h.accept(e)) {
                
                if (h.getState() == NoteHypothesis::Satisfied) {
                    
                    swallowed = true;
                    
                    if (!m_haveCurrent ||
                        m_current.getState() == NoteHypothesis::Expired ||
                        m_current.getState() == NoteHypothesis::Rejected) {
                        m_current = h;
                        m_haveCurrent = true;
                    } else {
                        newCandidates.push_back(h);
                    }
                    
                } else {
                    newCandidates.push_back(h);
                }
            }
        }
    }
    
    if (!swallowed) {
        NoteHypothesis h(m_slack);
        if (h.accept(e)) {
            newCandidates.push_back(h);
        }
    }
    
    m_candidates = reap(newCandidates);
}

AgentFeeder::Hypotheses
AgentFeeder::reap(Hypotheses candidates)
{
    // reap rejected/expired hypotheses from list of candidates

    Hypotheses survived;
    for (Hypotheses::const_iterator i = candidates.begin();
         i != candidates.end(); ++i) {
        NoteHypothesis h = *i;
        if (h.getState() != NoteHypothesis::Rejected && 
            h.getState() != NoteHypothesis::Expired) {
            survived.push_back(h);
        }
    }

    return survived;
}
    
void
AgentFeeder::finish()
{
    if (m_current.getState() == NoteHypothesis::Satisfied) {
	m_accepted.push_back(m_current);
    }
}

