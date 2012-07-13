/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
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

#include "CepstrumPitchTracker.h"

#include "vamp-sdk/FFT.h"

#include <vector>
#include <algorithm>

#include <cstdio>
#include <cmath>
#include <complex>

using std::string;
using std::vector;
using Vamp::RealTime;

CepstrumPitchTracker::Hypothesis::Hypothesis()
{
    m_state = New;
}

CepstrumPitchTracker::Hypothesis::~Hypothesis()
{
}

bool
CepstrumPitchTracker::Hypothesis::isWithinTolerance(Estimate s)
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
CepstrumPitchTracker::Hypothesis::isOutOfDateFor(Estimate s)
{
    if (m_pending.empty()) return false;

    return ((s.time - m_pending[m_pending.size()-1].time) > 
            RealTime::fromMilliseconds(40));
}

bool 
CepstrumPitchTracker::Hypothesis::isSatisfied()
{
    if (m_pending.empty()) return false;
    
    double meanConfidence = 0.0;
    for (int i = 0; i < m_pending.size(); ++i) {
        meanConfidence += m_pending[i].confidence;
    }
    meanConfidence /= m_pending.size();

    int lengthRequired = 10000;
    if (meanConfidence > 0.0) {
        lengthRequired = int(2.0 / meanConfidence + 0.5);
    }
    std::cerr << "meanConfidence = " << meanConfidence << ", lengthRequired = " << lengthRequired << ", length = " << m_pending.size() << std::endl;

    return (m_pending.size() > lengthRequired);
}

bool
CepstrumPitchTracker::Hypothesis::accept(Estimate s)
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

CepstrumPitchTracker::Hypothesis::State
CepstrumPitchTracker::Hypothesis::getState()
{
    return m_state;
}

CepstrumPitchTracker::Hypothesis::Estimates
CepstrumPitchTracker::Hypothesis::getAcceptedEstimates()
{
    if (m_state == Satisfied || m_state == Expired) {
        return m_pending;
    } else {
        return Estimates();
    }
}

double
CepstrumPitchTracker::Hypothesis::getMeanFrequency()
{
    double acc = 0.0;
    for (int i = 0; i < m_pending.size(); ++i) {
        acc += m_pending[i].freq;
    }
    acc /= m_pending.size();
    return acc;
}

CepstrumPitchTracker::Hypothesis::Note
CepstrumPitchTracker::Hypothesis::getAveragedNote()
{
    Note n;

    if (!(m_state == Satisfied || m_state == Expired)) {
        n.freq = 0.0;
        n.time = RealTime::zeroTime;
        n.duration = RealTime::zeroTime;
        return n;
    }

    n.time = m_pending.begin()->time;

    Estimates::iterator i = m_pending.end();
    --i;
    n.duration = i->time - n.time;

    // just mean frequency for now, but this isn't at all right perceptually
    n.freq = getMeanFrequency();
    
    return n;
}

void
CepstrumPitchTracker::Hypothesis::addFeatures(FeatureSet &fs)
{
    for (int i = 0; i < m_pending.size(); ++i) {
	Feature f;
	f.hasTimestamp = true;
	f.timestamp = m_pending[i].time;
	f.values.push_back(m_pending[i].freq);
	fs[0].push_back(f);
    }

    Feature nf;
    nf.hasTimestamp = true;
    nf.hasDuration = true;
    Note n = getAveragedNote();
    nf.timestamp = n.time;
    nf.duration = n.duration;
    nf.values.push_back(n.freq);
    fs[1].push_back(nf);
}

CepstrumPitchTracker::CepstrumPitchTracker(float inputSampleRate) :
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

CepstrumPitchTracker::~CepstrumPitchTracker()
{
}

string
CepstrumPitchTracker::getIdentifier() const
{
    return "cepstrum-pitch";
}

string
CepstrumPitchTracker::getName() const
{
    return "Cepstrum Pitch Tracker";
}

string
CepstrumPitchTracker::getDescription() const
{
    return "Estimate f0 of monophonic material using a cepstrum method.";
}

string
CepstrumPitchTracker::getMaker() const
{
    return "Chris Cannam";
}

int
CepstrumPitchTracker::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
CepstrumPitchTracker::getCopyright() const
{
    return "Freely redistributable (BSD license)";
}

CepstrumPitchTracker::InputDomain
CepstrumPitchTracker::getInputDomain() const
{
    return FrequencyDomain;
}

size_t
CepstrumPitchTracker::getPreferredBlockSize() const
{
    return 1024;
}

size_t 
CepstrumPitchTracker::getPreferredStepSize() const
{
    return 256;
}

size_t
CepstrumPitchTracker::getMinChannelCount() const
{
    return 1;
}

size_t
CepstrumPitchTracker::getMaxChannelCount() const
{
    return 1;
}

CepstrumPitchTracker::ParameterList
CepstrumPitchTracker::getParameterDescriptors() const
{
    ParameterList list;
    return list;
}

float
CepstrumPitchTracker::getParameter(string identifier) const
{
    return 0.f;
}

void
CepstrumPitchTracker::setParameter(string identifier, float value) 
{
}

CepstrumPitchTracker::ProgramList
CepstrumPitchTracker::getPrograms() const
{
    ProgramList list;
    return list;
}

string
CepstrumPitchTracker::getCurrentProgram() const
{
    return ""; // no programs
}

void
CepstrumPitchTracker::selectProgram(string name)
{
}

CepstrumPitchTracker::OutputList
CepstrumPitchTracker::getOutputDescriptors() const
{
    OutputList outputs;

    int n = 0;

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
CepstrumPitchTracker::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

//    std::cerr << "CepstrumPitchTracker::initialise: channels = " << channels
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
CepstrumPitchTracker::reset()
{
}

void
CepstrumPitchTracker::filter(const double *cep, double *data)
{
    for (int i = 0; i < m_bins; ++i) {
        double v = 0;
        int n = 0;
        // average according to the vertical filter length
        for (int j = -m_vflen/2; j <= m_vflen/2; ++j) {
            int ix = i + m_binFrom + j;
            if (ix >= 0 && ix < m_blockSize) {
                v += cep[ix];
                ++n;
            }
        }
        data[i] = v / n;
    }
}

double
CepstrumPitchTracker::cubicInterpolate(const double y[4], double x)
{
    double a0 = y[3] - y[2] - y[0] + y[1];
    double a1 = y[0] - y[1] - a0;
    double a2 = y[2] - y[0];
    double a3 = y[1];
    return
        a0 * x * x * x +
        a1 * x * x +
        a2 * x +
        a3;
}

double
CepstrumPitchTracker::findInterpolatedPeak(const double *in, int maxbin)
{
    if (maxbin < 2 || maxbin > m_bins - 3) {
        return maxbin;
    }

    double maxval = 0.0;
    double maxidx = maxbin;

    const int divisions = 10;
    double y[4];

    y[0] = in[maxbin-1];
    y[1] = in[maxbin];
    y[2] = in[maxbin+1];
    y[3] = in[maxbin+2];
    for (int i = 0; i < divisions; ++i) {
        double probe = double(i) / double(divisions);
        double value = cubicInterpolate(y, probe);
        if (value > maxval) {
            maxval = value; 
            maxidx = maxbin + probe;
        }
    }

    y[3] = y[2];
    y[2] = y[1];
    y[1] = y[0];
    y[0] = in[maxbin-2];
    for (int i = 0; i < divisions; ++i) {
        double probe = double(i) / double(divisions);
        double value = cubicInterpolate(y, probe);
        if (value > maxval) {
            maxval = value; 
            maxidx = maxbin - 1 + probe;
        }
    }

/*
    std::cerr << "centre = " << maxbin << ": ["
              << in[maxbin-2] << ","
              << in[maxbin-1] << ","
              << in[maxbin] << ","
              << in[maxbin+1] << ","
              << in[maxbin+2] << "] -> " << maxidx << std::endl;
*/

    return maxidx;
}

CepstrumPitchTracker::FeatureSet
CepstrumPitchTracker::process(const float *const *inputBuffers, RealTime timestamp)
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
    filter(rawcep, data);
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

    double cimax = findInterpolatedPeak(data, maxbin);
    double peakfreq = m_inputSampleRate / (cimax + m_binFrom);

    double confidence = 0.0;
    if (nextPeakVal != 0.0) {
        confidence = (maxval - nextPeakVal) * 10.0;
        if (magmean < threshold) confidence = 0.0;
        std::cerr << "magmean = " << magmean << ", confidence = " << confidence << std::endl;
    }

    Hypothesis::Estimate e;
    e.freq = peakfreq;
    e.time = timestamp;
    e.confidence = confidence;

//    m_good.advanceTime();
    for (int i = 0; i < m_possible.size(); ++i) {
//        m_possible[i].advanceTime();
    }

    if (!m_good.accept(e)) {

        int candidate = -1;
        bool accepted = false;

        for (int i = 0; i < m_possible.size(); ++i) {
            if (m_possible[i].accept(e)) {
                if (m_possible[i].getState() == Hypothesis::Satisfied) {
                    accepted = true;
                    candidate = i;
                }
                break;
            }
        }

        if (!accepted) {
            Hypothesis h;
            h.accept(e); //!!! must succeed as h is new, so perhaps there should be a ctor for this
            m_possible.push_back(h);
        }

        if (m_good.getState() == Hypothesis::Expired) {
            m_good.addFeatures(fs);
        }
        
        if (m_good.getState() == Hypothesis::Expired ||
            m_good.getState() == Hypothesis::Rejected) {
            if (candidate >= 0) {
                m_good = m_possible[candidate];
            } else {
                m_good = Hypothesis();
            }
        }

        // reap rejected/expired hypotheses from possible list
        Hypotheses toReap = m_possible;
        m_possible.clear();
        for (int i = 0; i < toReap.size(); ++i) {
            Hypothesis h = toReap[i];
            if (h.getState() != Hypothesis::Rejected && 
                h.getState() != Hypothesis::Expired) {
                m_possible.push_back(h);
            }
        }
    }  

    delete[] data;
    return fs;
}

CepstrumPitchTracker::FeatureSet
CepstrumPitchTracker::getRemainingFeatures()
{
    FeatureSet fs;
    if (m_good.getState() == Hypothesis::Satisfied) {
        m_good.addFeatures(fs);
    }
    return fs;
}
