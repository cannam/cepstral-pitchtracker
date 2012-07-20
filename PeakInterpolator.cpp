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

#include "PeakInterpolator.h"

#include <iostream>

double
PeakInterpolator::findPeakLocation(const double *data, int size)
{
    double maxval;
    int maxidx = 0;
    int i;
    for (i = 0; i < size; ++i) {
        if (i == 0 || data[i] > maxval) {
            maxval = data[i];
            maxidx = i;
        }
    }
    return findPeakLocation(data, size, maxidx);
}

double
PeakInterpolator::findPeakLocation(const double *data, int size, int peakIndex)
{
    // after jos, 
    // https://ccrma.stanford.edu/~jos/sasp/Quadratic_Interpolation_Spectral_Peaks.html

    if (peakIndex < 1 || peakIndex > size - 2) {
        return peakIndex;
    }

    double alpha = data[peakIndex-1];
    double beta  = data[peakIndex];
    double gamma = data[peakIndex+1];

    double denom = (alpha - 2*beta + gamma);

    if (denom == 0) {
        // flat
        return peakIndex;
    }

    double p = ((alpha - gamma) / denom) / 2.0;

    return double(peakIndex) + p;
}

