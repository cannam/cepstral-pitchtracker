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

#ifndef _CEPSTRUM_H_
#define _CEPSTRUM_H_

#include "vamp-sdk/FFT.h"
#include <cmath>

#include <iostream>
#include <exception>

class Cepstrum
{
public:
    /**
     * Construct a cepstrum converter based on an n-point FFT.
     */
    Cepstrum(int n) : m_n(n) {
	if (n & (n-1)) {
	    throw "N must be a power of two";
	}
    }
    ~Cepstrum() { }
    
    /**
     * Convert the given frequency-domain data to the cepstral domain.
     *
     * The input must be in the format used for FrequencyDomain data
     * in the Vamp SDK: n/2+1 consecutive pairs of real and imaginary
     * component floats corresponding to bins 0..(n/2) of the FFT
     * output. Thus, n+2 values in total.
     *
     * The output consists of the raw cepstrum of length n.
     *
     * The cepstrum is calculated as the inverse FFT of a
     * synthetically symmetrical base-10 log magnitude spectrum.
     *
     * Returns the mean magnitude of the input spectrum.
     */
    double process(const float *in, double *out) {

	int hs = m_n/2 + 1;
	double *io = new double[m_n];
	double *logmag = new double[m_n];
	double epsilon = 1e-10;

	double magmean = 0.0;

	for (int i = 0; i < hs; ++i) {

	    double power = in[i*2] * in[i*2] + in[i*2+1] * in[i*2+1];
	    double mag = sqrt(power);
	    magmean += mag;

	    logmag[i] = log10(mag + epsilon);

	    if (i > 0) {
		// make the log magnitude spectrum symmetrical
		logmag[m_n - i] = logmag[i];
	    }
	}
	
	magmean /= hs;
	/*
	std::cerr << "logmags:" << std::endl;
	for (int i = 0; i < m_n; ++i) {
	    std::cerr << logmag[i] << " ";
	}
	std::cerr << std::endl;
	*/
	Vamp::FFT::inverse(m_n, logmag, 0, out, io);
    
	delete[] logmag;
	delete[] io;

	return magmean;
    }

private:
    int m_n;
};

#endif
