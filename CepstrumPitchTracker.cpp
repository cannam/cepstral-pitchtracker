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

#include <vector>
#include <algorithm>

#include <cstdio>
#include <cmath>
#include <complex>

using std::string;
using std::vector;

CepstrumPitchTracker::Hypothesis::Hypothesis()
{
    m_state = New;
    m_age = 0;
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
    Estimate last = m_pending[m_pending.size()-1];
    double r = s.freq / last.freq;
    int cents = lrint(1200.0 * (log(r) / log(2.0)));
    return (cents > -200 && cents < 200);
}

bool 
CepstrumPitchTracker::Hypothesis::isSatisfied()
{
    return (m_pending.size() > 2);
}

void
CepstrumPitchTracker::Hypothesis::advanceTime()
{
    ++m_age;
}

bool
CepstrumPitchTracker::Hypothesis::test(Estimate s)
{
    bool accept = false;

    switch (m_state) {

    case New:
        m_state = Provisional;
        accept = true;
        break;

    case Provisional:
        if (m_age > 3) {
            m_state = Rejected;
        } else if (isWithinTolerance(s)) {
            accept = true;
        }
        break;
        
    case Satisfied:
        if (m_age > 3) {
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
        m_age = 0;
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

int
CepstrumPitchTracker::Hypothesis::getPendingLength()
{
    return m_pending.size();
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

void
CepstrumPitchTracker::Hypothesis::addFeatures(FeatureList &fl)
{
    for (int i = 0; i < m_pending.size(); ++i) {
	Feature f;
	f.hasTimestamp = true;
	f.timestamp = m_pending[i].time;
	f.values.push_back(m_pending[i].freq);
	fl.push_back(f);
    }
}

CepstrumPitchTracker::CepstrumPitchTracker(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_channels(0),
    m_stepSize(256),
    m_blockSize(1024),
    m_fmin(50),
    m_fmax(1000),
    m_histlen(1),
    m_vflen(3),
    m_binFrom(0),
    m_binTo(0),
    m_bins(0),
    m_history(0),
    m_prevpeak(0),
    m_prevprop(0)
{
}

CepstrumPitchTracker::~CepstrumPitchTracker()
{
    if (m_history) {
        for (int i = 0; i < m_histlen; ++i) {
            delete[] m_history[i];
        }
        delete[] m_history;
    }
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

    m_history = new double *[m_histlen];
    for (int i = 0; i < m_histlen; ++i) {
        m_history[i] = new double[m_bins];
    }

    reset();

    return true;
}

void
CepstrumPitchTracker::reset()
{
    for (int i = 0; i < m_histlen; ++i) {
        for (int j = 0; j < m_bins; ++j) {
            m_history[i][j] = 0.0;
        }
    }
}

void
CepstrumPitchTracker::filter(const double *cep, double *result)
{
    int hix = m_histlen - 1; // current history index

    // roll back the history
    if (m_histlen > 1) {
        double *oldest = m_history[0];
        for (int i = 1; i < m_histlen; ++i) {
            m_history[i-1] = m_history[i];
        }
        // and stick this back in the newest spot, to recycle
        m_history[hix] = oldest;
    }

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
        m_history[hix][i] = v / n;
    }

    for (int i = 0; i < m_bins; ++i) {
        double mean = 0.0;
        for (int j = 0; j < m_histlen; ++j) {
            mean += m_history[j][i];
        }
        mean /= m_histlen;
        result[i] = mean;
    }
}

double
CepstrumPitchTracker::calculatePeakProportion(const double *data, double abstot, int n)
{
    double aroundPeak = data[n];
    double peakProportion = 0.0;

    int i = n - 1;
    while (i > 0 && data[i] <= data[i+1]) {
        aroundPeak += fabs(data[i]);
        --i;
    }
    i = n + 1;
    while (i < m_bins && data[i] <= data[i-1]) {
        aroundPeak += fabs(data[i]);
        ++i;
    }
    peakProportion = aroundPeak / abstot;

    return peakProportion;
}

bool
CepstrumPitchTracker::acceptPeak(int n, double peakProportion)
{
    bool accept = false;

    if (abs(n - m_prevpeak) < 10) { //!!! should depend on bin count
        accept = true;
    } else if (peakProportion > m_prevprop * 2) {
        accept = true;
    }

    return accept;
}

CepstrumPitchTracker::FeatureSet
CepstrumPitchTracker::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    FeatureSet fs;

    int bs = m_blockSize;
    int hs = m_blockSize/2 + 1;

    double *rawcep = new double[bs];
    double *io = new double[bs];
    double *logmag = new double[bs];

    // The "inverse symmetric" method. Seems to be the most reliable
        
    for (int i = 0; i < hs; ++i) {

	double power =
	    inputBuffers[0][i*2  ] * inputBuffers[0][i*2  ] +
	    inputBuffers[0][i*2+1] * inputBuffers[0][i*2+1];
	double mag = sqrt(power);
	
	double lm = log(mag + 0.00000001);
	
	logmag[i] = lm;
	if (i > 0) logmag[bs - i] = lm;
    }

    fft(bs, true, logmag, 0, rawcep, io);
    
    delete[] logmag;
    delete[] io;

    int n = m_bins;
    double *data = new double[n];
    filter(rawcep, data);
    delete[] rawcep;

    double abstot = 0.0;

    for (int i = 0; i < n; ++i) {
	abstot += fabs(data[i]);
    }

    double maxval = 0.0;
    int maxbin = -1;

    for (int i = 0; i < n; ++i) {
        if (data[i] > maxval) {
            maxval = data[i];
            maxbin = i;
        }
    }

    if (maxbin < 0) return fs;

    double peakfreq = m_inputSampleRate / (maxbin + m_binFrom);
    Hypothesis::Estimate e;
    e.freq = peakfreq;
    e.time = timestamp;

    m_accepted.advanceTime();

    for (int i = 0; i < m_possible.size(); ++i) {
        m_possible[i].advanceTime();
    }

    if (!m_accepted.test(e)) {
        int candidate = -1;
        for (int i = 0; i < m_possible.size(); ++i) {
            if (m_possible[i].test(e)) {
                if (m_possible[i].getState() == Hypothesis::Satisfied) {
                    candidate = i;
                }
                break;
            }
        }

        if (m_accepted.getState() == Hypothesis::Expired) {
            m_accepted.addFeatures(fs[0]);
        }
        
        if (m_accepted.getState() == Hypothesis::Expired ||
            m_accepted.getState() == Hypothesis::Rejected) {
            if (candidate >= 0) {
                m_accepted = m_possible[candidate];
            } else {
                m_accepted = Hypothesis();
            }
        }

        std::cerr << "accepted length = " << m_accepted.getPendingLength()
                  << ", state = " << m_accepted.getState()
                  << ", hypothesis count = " << m_possible.size() << std::endl;

        //!!! and also need to reap rejected/expired hypotheses from the list
    }  
            

/*
    bool accepted = false;

    if (maxbin >= 0) {
        double pp = calculatePeakProportion(data, abstot, maxbin);
        if (acceptPeak(maxbin, pp)) {
            accepted = true;
        } else {
            // try a secondary peak
            maxval = 0.0;
            int secondbin = 0;
            for (int i = 1; i < n-1; ++i) {
                if (i != maxbin &&
                    data[i] > data[i-1] &&
                    data[i] > data[i+1] &&
                    data[i] > maxval) {
                    maxval = data[i];
                    secondbin = i;
                }
            }
            double spp = calculatePeakProportion(data, abstot, secondbin);
            if (acceptPeak(secondbin, spp)) {
                maxbin = secondbin;
                pp = spp;
                accepted = true;
            }
        }
        if (accepted) {
            m_prevpeak = maxbin;
            m_prevprop = pp;
        }
    }
*/
//    std::cerr << "peakProportion = " << peakProportion << std::endl;
//    std::cerr << "peak = " << m_inputSampleRate / (maxbin + m_binFrom) << std::endl;
//    std::cerr << "bins = " << m_bins << std::endl;

//    if (peakProportion >= (0.00006 * m_bins)) {
/*
    if (accepted) {
	Feature f;
	f.hasTimestamp = true;
	f.timestamp = timestamp;
	f.values.push_back(m_inputSampleRate / (maxbin + m_binFrom));
	fs[0].push_back(f);
    }
*/
    delete[] data;
    return fs;
}

CepstrumPitchTracker::FeatureSet
CepstrumPitchTracker::getRemainingFeatures()
{
    FeatureSet fs;
    if (m_accepted.getState() != Hypothesis::New) {
        m_accepted.addFeatures(fs[0]);
    }
    return fs;
}

void
CepstrumPitchTracker::fft(unsigned int n, bool inverse,
                    double *ri, double *ii, double *ro, double *io)
{
    if (!ri || !ro || !io) return;

    unsigned int bits;
    unsigned int i, j, k, m;
    unsigned int blockSize, blockEnd;

    double tr, ti;

    if (n < 2) return;
    if (n & (n-1)) return;

    double angle = 2.0 * M_PI;
    if (inverse) angle = -angle;

    for (i = 0; ; ++i) {
	if (n & (1 << i)) {
	    bits = i;
	    break;
	}
    }

    static unsigned int tableSize = 0;
    static int *table = 0;

    if (tableSize != n) {

	delete[] table;

	table = new int[n];

	for (i = 0; i < n; ++i) {
	
	    m = i;

	    for (j = k = 0; j < bits; ++j) {
		k = (k << 1) | (m & 1);
		m >>= 1;
	    }

	    table[i] = k;
	}

	tableSize = n;
    }

    if (ii) {
	for (i = 0; i < n; ++i) {
	    ro[table[i]] = ri[i];
	    io[table[i]] = ii[i];
	}
    } else {
	for (i = 0; i < n; ++i) {
	    ro[table[i]] = ri[i];
	    io[table[i]] = 0.0;
	}
    }

    blockEnd = 1;

    for (blockSize = 2; blockSize <= n; blockSize <<= 1) {

	double delta = angle / (double)blockSize;
	double sm2 = -sin(-2 * delta);
	double sm1 = -sin(-delta);
	double cm2 = cos(-2 * delta);
	double cm1 = cos(-delta);
	double w = 2 * cm1;
	double ar[3], ai[3];

	for (i = 0; i < n; i += blockSize) {

	    ar[2] = cm2;
	    ar[1] = cm1;

	    ai[2] = sm2;
	    ai[1] = sm1;

	    for (j = i, m = 0; m < blockEnd; j++, m++) {

		ar[0] = w * ar[1] - ar[2];
		ar[2] = ar[1];
		ar[1] = ar[0];

		ai[0] = w * ai[1] - ai[2];
		ai[2] = ai[1];
		ai[1] = ai[0];

		k = j + blockEnd;
		tr = ar[0] * ro[k] - ai[0] * io[k];
		ti = ar[0] * io[k] + ai[0] * ro[k];

		ro[k] = ro[j] - tr;
		io[k] = io[j] - ti;

		ro[j] += tr;
		io[j] += ti;
	    }
	}

	blockEnd = blockSize;
    }
}


