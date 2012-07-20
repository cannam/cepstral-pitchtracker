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

static double cubicInterpolate(const double y[4], double x)
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
PeakInterpolator::findPeakLocation(const double *data, int size, int peakIndex)
{
    std::cerr << "findPeakLocation: size " << size << ", peakIndex " << peakIndex << std::endl;

    if (peakIndex < 1 || peakIndex > size - 2) {
        std::cerr << "returning " << peakIndex << ", data too short" << std::endl;
        return peakIndex;
    }

    double maxval = 0.0;
    double location = peakIndex;

    const int divisions = 10;
    double y[4];

    y[0] = data[peakIndex-1];
    y[1] = data[peakIndex];
    y[2] = data[peakIndex+1];
    if (peakIndex < size - 2) {
        y[3] = data[peakIndex+2];
    } else {
        y[3] = y[2];
    }
    std::cerr << "a y: " << y[0] << " " << y[1] << " " << y[2] << " " << y[3] << std::endl;
    for (int i = 0; i < divisions; ++i) {
        double probe = double(i) / double(divisions);
        double value = cubicInterpolate(y, probe);
        std::cerr << "probe = " << probe << ", value = " << value << " for location " << peakIndex + probe << std::endl;
        if (value > maxval) {
            maxval = value; 
            location = peakIndex + probe;
        }
    }

    y[3] = y[2];
    y[2] = y[1];
    y[1] = y[0];
    if (peakIndex > 1) {
        y[0] = data[peakIndex-2];
    } else {
        y[0] = y[1];
    }
    std::cerr << "b y: " << y[0] << " " << y[1] << " " << y[2] << " " << y[3] << std::endl;
    for (int i = 0; i < divisions; ++i) {
        double probe = double(i) / double(divisions);
        double value = cubicInterpolate(y, probe);
        std::cerr << "probe = " << probe << ", value = " << value << " for location " << peakIndex - 1 + probe << std::endl;
        if (value > maxval) {
            maxval = value; 
            location = peakIndex - 1 + probe;
        }
    }

    std::cerr << "returning " << location << std::endl;

    return location;
}

