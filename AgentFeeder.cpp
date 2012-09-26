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
    if (!m_current.accept(e)) {

	if (m_current.getState() == NoteHypothesis::Expired) {
	    m_accepted.push_back(m_current);
	}

	bool swallowed = false;

	Hypotheses newCandidates;

	for (typename Hypotheses::iterator i = m_candidates.begin();
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
        
			if (m_current.getState() == NoteHypothesis::Expired ||
			    m_current.getState() == NoteHypothesis::Rejected) {
			    m_current = h;
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
	    NoteHypothesis h;
	    h.accept(e); // must succeed, as h is new
	    newCandidates.push_back(h);
	}

	// reap rejected/expired hypotheses from candidates list,
	// and assign back to m_candidates

	m_candidates.clear();

	for (typename Hypotheses::const_iterator i = newCandidates.begin();
	     i != newCandidates.end(); ++i) {
	    NoteHypothesis h = *i;
	    if (h.getState() != NoteHypothesis::Rejected && 
		h.getState() != NoteHypothesis::Expired) {
		m_candidates.push_back(h);
	    }
	}
    }  
}

void
AgentFeeder::finish()
{
    if (m_current.getState() == NoteHypothesis::Satisfied) {
	m_accepted.push_back(m_current);
    }
}

