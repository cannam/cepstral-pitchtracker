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

#ifndef _PEAK_INTERPOLATOR_H_
#define _PEAK_INTERPOLATOR_H_

class PeakInterpolator
{
public:
    PeakInterpolator() { } 
    ~PeakInterpolator() { }

    /**
     * Return the interpolated location (i.e. possibly between sample
     * point indices) of the peak in the given sampled range.
     *
     * "The peak" is defined as the (approximate) location of the
     * maximum of a function interpolating between the points
     * neighbouring the sample index with the maximum value in the
     * range.
     *
     * If multiple local peak samples in the input range are equal,
     * i.e. there is more than one apparent peak in the range, the one
     * with the lowest index will be used. This is the case even if a
     * later peak would be of superior height after interpolation.
     */
    double findPeakLocation(const double *data, int size);

    /**
     * Return the interpolated location (i.e. between sample point
     * indices) of the peak whose nearest sample is found at peakIndex
     * in the given sampled range. This method allows you to specify
     * which peak to find, if local rather than global peaks are of
     * interest.
     */
    double findPeakLocation(const double *data, int size, int peakIndex);
};

#endif
