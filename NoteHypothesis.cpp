/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/* Copyright Chris Cannam - All Rights Reserved */

#include "NoteHypothesis.h"

#include <cmath>

#include "system/sysutils.h"

namespace Turbot {

NoteHypothesis::NoteHypothesis()
{
    m_state = New;
}

NoteHypothesis::~NoteHypothesis()
{
}

bool
NoteHypothesis::isWithinTolerance(Estimate s) const
{
    if (m_pending.empty()) {
        return true;
    }

    // check we are within a relatively close tolerance of the last
    // candidate
    Estimate last = m_pending[m_pending.size()-1];
    double r = s.freq / last.freq;
    int cents = lrint(1200.0 * (log(r) / log(2.0)));
    if (cents < -60 || cents > 60) return false;

    // and within a slightly bigger tolerance of the current mean
    double meanFreq = getMeanFrequency();
    r = s.freq / meanFreq;
    cents = lrint(1200.0 * (log(r) / log(2.0)));
    if (cents < -80 || cents > 80) return false;
    
    return true;
}

bool
NoteHypothesis::isOutOfDateFor(Estimate s) const
{
    if (m_pending.empty()) return false;

    return ((s.time - m_pending[m_pending.size()-1].time) > 
            RealTime::fromMilliseconds(40));
}

bool 
NoteHypothesis::isSatisfied() const
{
    if (m_pending.empty()) return false;
    
    double meanConfidence = 0.0;
    for (int i = 0; i < (int)m_pending.size(); ++i) {
        meanConfidence += m_pending[i].confidence;
    }
    meanConfidence /= m_pending.size();

    int lengthRequired = 10000;
    if (meanConfidence > 0.0) {
        lengthRequired = int(2.0 / meanConfidence + 0.5);
    }

    return ((int)m_pending.size() > lengthRequired);
}

bool
NoteHypothesis::accept(Estimate s)
{
    bool accept = false;

    switch (m_state) {

    case New:
        m_state = Provisional;
        accept = true;
        break;

    case Provisional:
        if (isOutOfDateFor(s)) {
            m_state = Rejected;
        } else if (isWithinTolerance(s)) {
            accept = true;
        }
        break;
        
    case Satisfied:
        if (isOutOfDateFor(s)) {
            m_state = Expired;
        } else if (isWithinTolerance(s)) {
            accept = true;
        }
        break;

    case Rejected:
        break;

    case Expired:
        break;
    }

    if (accept) {
        m_pending.push_back(s);
        if (m_state == Provisional && isSatisfied()) {
            m_state = Satisfied;
        }
    }

    return accept;
}        

NoteHypothesis::State
NoteHypothesis::getState() const
{
    return m_state;
}

NoteHypothesis::Estimates
NoteHypothesis::getAcceptedEstimates() const
{
    if (m_state == Satisfied || m_state == Expired) {
        return m_pending;
    } else {
        return Estimates();
    }
}

double
NoteHypothesis::getMeanFrequency() const
{
    double acc = 0.0;
    for (int i = 0; i < (int)m_pending.size(); ++i) {
        acc += m_pending[i].freq;
    }
    acc /= m_pending.size();
    return acc;
}

NoteHypothesis::Note
NoteHypothesis::getAveragedNote() const
{
    Note n;

    if (!(m_state == Satisfied || m_state == Expired)) {
        n.freq = 0.0;
        n.time = RealTime::zeroTime;
        n.duration = RealTime::zeroTime;
        return n;
    }

    n.time = m_pending.begin()->time;

    Estimates::const_iterator i = m_pending.end();
    --i;
    n.duration = i->time - n.time;

    // just mean frequency for now, but this isn't at all right perceptually
    n.freq = getMeanFrequency();
    
    return n;
}

}

