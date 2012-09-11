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

#ifndef _MEAN_FILTER_H_
#define _MEAN_FILTER_H_

class MeanFilter
{
public:
    /**
     * Construct a non-causal mean filter with filter length flen,
     * that replaces each sample N with the mean of samples
     * [N-floor(F/2) .. N+floor(F/2)] where F is the filter length.
     * Only odd F are supported.
     */
    MeanFilter(int flen) : m_flen(flen) { }
    ~MeanFilter() { }

    /**
     * Filter the n samples in "in" and place the results in "out"
     */
    void filter(const double *in, double *out, const int n) {
	filterSubsequence(in, out, n, n, 0);
    }

    /**
     * Filter the n samples starting at the given offset in the
     * m-element array "in" and place the results in the n-element
     * array "out"
     */
    void filterSubsequence(const double *in, double *out,
			   const int m, const int n,
			   const int offset) {
	int half = m_flen/2;
	for (int i = 0; i < n; ++i) {
	    double v = 0;
	    int n = 0;
	    for (int j = -half; j <= half; ++j) {
		int ix = i + j + offset;
		if (ix >= 0 && ix < m) {
		    v += in[ix];
		    ++n;
		}
	    }
	    out[i] = v / n;
	}
    }

private:
    int m_flen;
};

#endif
