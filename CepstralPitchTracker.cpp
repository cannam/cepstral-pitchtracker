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

#include "CepstralPitchTracker.h"
#include "MeanFilter.h"
#include "PeakInterpolator.h"

#include "vamp-sdk/FFT.h"

#include <vector>
#include <algorithm>

#include <cstdio>
#include <cmath>
#include <complex>

using std::string;
using std::vector;
using Vamp::RealTime;


CepstralPitchTracker::CepstralPitchTracker(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_channels(0),
    m_stepSize(256),
    m_blockSize(1024),
    m_fmin(50),
    m_fmax(900),
    m_vflen(1),
    m_binFrom(0),
    m_binTo(0),
    m_bins(0)
{
}

CepstralPitchTracker::~CepstralPitchTracker()
{
}

string
CepstralPitchTracker::getIdentifier() const
{
    return "cepstral-pitchtracker";
}

string
CepstralPitchTracker::getName() const
{
    return "Cepstral Pitch Tracker";
}

string
CepstralPitchTracker::getDescription() const
{
    return "Estimate f0 of monophonic material using a cepstrum method.";
}

string
CepstralPitchTracker::getMaker() const
{
    return "Chris Cannam";
}

int
CepstralPitchTracker::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
CepstralPitchTracker::getCopyright() const
{
    return "Freely redistributable (BSD license)";
}

CepstralPitchTracker::InputDomain
CepstralPitchTracker::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
CepstralPitchTracker::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
CepstralPitchTracker::getPreferredStepSize() const
{
    return 256;
}

size_t
CepstralPitchTracker::getMinChannelCount() const
{
    return 1;
}

size_t
CepstralPitchTracker::getMaxChannelCount() const
{
    return 1;
}

CepstralPitchTracker::ParameterList
CepstralPitchTracker::getParameterDescriptors() const
{
    ParameterList list;
    return list;
}

float
CepstralPitchTracker::getParameter(string identifier) const
{
    return 0.f;
}

void
CepstralPitchTracker::setParameter(string identifier, float value) 
{
}

CepstralPitchTracker::ProgramList
CepstralPitchTracker::getPrograms() const
{
    ProgramList list;
    return list;
}

string
CepstralPitchTracker::getCurrentProgram() const
{
    return ""; // no programs
}

void
CepstralPitchTracker::selectProgram(string name)
{
}

CepstralPitchTracker::OutputList
CepstralPitchTracker::getOutputDescriptors() const
{
    OutputList outputs;

    OutputDescriptor d;

    d.identifier = "f0";
    d.name = "Estimated f0";
    d.description = "Estimated fundamental frequency";
    d.unit = "Hz";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = true;
    d.minValue = m_fmin;
    d.maxValue = m_fmax;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = (m_inputSampleRate / m_stepSize);
    d.hasDuration = false;
    outputs.push_back(d);

    d.identifier = "notes";
    d.name = "Notes";
    d.description = "Derived fixed-pitch note frequencies";
    d.unit = "Hz";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = true;
    d.minValue = m_fmin;
    d.maxValue = m_fmax;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::FixedSampleRate;
    d.sampleRate = (m_inputSampleRate / m_stepSize);
    d.hasDuration = true;
    outputs.push_back(d);

    return outputs;
}

bool
CepstralPitchTracker::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

//    std::cerr << "CepstralPitchTracker::initialise: channels = " << channels
//	      << ", stepSize = " << stepSize << ", blockSize = " << blockSize
//	      << std::endl;

    m_channels = channels;
    m_stepSize = stepSize;
    m_blockSize = blockSize;

    m_binFrom = int(m_inputSampleRate / m_fmax);
    m_binTo = int(m_inputSampleRate / m_fmin); 

    if (m_binTo >= (int)m_blockSize / 2) {
        m_binTo = m_blockSize / 2 - 1;
    }

    m_bins = (m_binTo - m_binFrom) + 1;

    reset();

    return true;
}

void
CepstralPitchTracker::reset()
{
}

void
CepstralPitchTracker::addFeaturesFrom(NoteHypothesis h, FeatureSet &fs)
{
    NoteHypothesis::Estimates es = h.getAcceptedEstimates();

    for (int i = 0; i < (int)es.size(); ++i) {
	Feature f;
	f.hasTimestamp = true;
	f.timestamp = es[i].time;
	f.values.push_back(es[i].freq);
	fs[0].push_back(f);
    }

    Feature nf;
    nf.hasTimestamp = true;
    nf.hasDuration = true;
    NoteHypothesis::Note n = h.getAveragedNote();
    nf.timestamp = n.time;
    nf.duration = n.duration;
    nf.values.push_back(n.freq);
    fs[1].push_back(nf);
}

CepstralPitchTracker::FeatureSet
CepstralPitchTracker::process(const float *const *inputBuffers, RealTime timestamp)
{
    FeatureSet fs;

    int bs = m_blockSize;
    int hs = m_blockSize/2 + 1;

    double *rawcep = new double[bs];
    double *io = new double[bs];
    double *logmag = new double[bs];

    // The "inverse symmetric" method. Seems to be the most reliable
        
    double magmean = 0.0;

    for (int i = 0; i < hs; ++i) {

	double power =
	    inputBuffers[0][i*2  ] * inputBuffers[0][i*2  ] +
	    inputBuffers[0][i*2+1] * inputBuffers[0][i*2+1];
	double mag = sqrt(power);

        magmean += mag;

	double lm = log(mag + 0.00000001);
	
	logmag[i] = lm;
	if (i > 0) logmag[bs - i] = lm;
    }

    magmean /= hs;
    double threshold = 0.1; // for magmean
    
    Vamp::FFT::inverse(bs, logmag, 0, rawcep, io);
    
    delete[] logmag;
    delete[] io;

    int n = m_bins;
    double *data = new double[n];
    MeanFilter(m_vflen).filterSubsequence(rawcep, data, m_blockSize, n, m_binFrom);
    delete[] rawcep;

    double maxval = 0.0;
    int maxbin = -1;

    for (int i = 0; i < n; ++i) {
        if (data[i] > maxval) {
            maxval = data[i];
            maxbin = i;
        }
    }

    if (maxbin < 0) {
        delete[] data;
        return fs;
    }

    double nextPeakVal = 0.0;
    for (int i = 1; i+1 < n; ++i) {
        if (data[i] > data[i-1] &&
            data[i] > data[i+1] &&
            i != maxbin &&
            data[i] > nextPeakVal) {
            nextPeakVal = data[i];
        }
    }

    PeakInterpolator pi;
    double cimax = pi.findPeakLocation(data, m_bins, maxbin);
    double peakfreq = m_inputSampleRate / (cimax + m_binFrom);

    double confidence = 0.0;
    if (nextPeakVal != 0.0) {
        confidence = (maxval - nextPeakVal) * 10.0;
        if (magmean < threshold) confidence = 0.0;
//        std::cerr << "magmean = " << magmean << ", confidence = " << confidence << std::endl;
    }

    NoteHypothesis::Estimate e;
    e.freq = peakfreq;
    e.time = timestamp;
    e.confidence = confidence;

    if (!m_good.accept(e)) {

        int candidate = -1;
        bool accepted = false;

        for (int i = 0; i < (int)m_possible.size(); ++i) {
            if (m_possible[i].accept(e)) {
                if (m_possible[i].getState() == NoteHypothesis::Satisfied) {
                    accepted = true;
                    candidate = i;
                }
                break;
            }
        }

        if (!accepted) {
            NoteHypothesis h;
            h.accept(e); //!!! must succeed as h is new, so perhaps there should be a ctor for this
            m_possible.push_back(h);
        }

        if (m_good.getState() == NoteHypothesis::Expired) {
            addFeaturesFrom(m_good, fs);
        }
        
        if (m_good.getState() == NoteHypothesis::Expired ||
            m_good.getState() == NoteHypothesis::Rejected) {
            if (candidate >= 0) {
                m_good = m_possible[candidate];
            } else {
                m_good = NoteHypothesis();
            }
        }

        // reap rejected/expired hypotheses from possible list
        Hypotheses toReap = m_possible;
        m_possible.clear();
        for (int i = 0; i < (int)toReap.size(); ++i) {
            NoteHypothesis h = toReap[i];
            if (h.getState() != NoteHypothesis::Rejected && 
                h.getState() != NoteHypothesis::Expired) {
                m_possible.push_back(h);
            }
        }
    }  

    delete[] data;
    return fs;
}

CepstralPitchTracker::FeatureSet
CepstralPitchTracker::getRemainingFeatures()
{
    FeatureSet fs;
    if (m_good.getState() == NoteHypothesis::Satisfied) {
        addFeaturesFrom(m_good, fs);
    }
    return fs;
}
