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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestPeakInterpolator)

BOOST_AUTO_TEST_CASE(peakAtSample_N3)
{
    // Peak exactly at sample index
    double data[] = { 0.0, 10.0, 0.0 };
    PeakInterpolator p;
    // Asked to find peak at index 1, should return index 1
    double result = p.findPeakLocation(data, 3, 1);
    BOOST_CHECK_EQUAL(result, 1.0);
    // Asked to find any peak, should return index 1
    result = p.findPeakLocation(data, 3);
    BOOST_CHECK_EQUAL(result, 1.0);
}

BOOST_AUTO_TEST_CASE(peakAtSample_N5)
{
    // Peak exactly at sample index
    double data[] = { 0.0, 10.0, 20.0, 10.0, 0.0 };
    PeakInterpolator p;
    // Asked to find peak at index 2, should return index 2
    double result = p.findPeakLocation(data, 5, 2);
    BOOST_CHECK_EQUAL(result, 2.0);
    // Asked to find any peak, should return index 2
    result = p.findPeakLocation(data, 5);
    BOOST_CHECK_EQUAL(result, 2.0);
}

BOOST_AUTO_TEST_CASE(flat)
{
    // No peak
    double data[] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
    PeakInterpolator p;
    // Asked to find peak at index N, should return N (no superior neighbours)
    double result = p.findPeakLocation(data, 5, 2);
    BOOST_CHECK_EQUAL(result, 2.0);
    // Asked to find any peak, should return 0 (first value as good as any)
    result = p.findPeakLocation(data, 5);
    BOOST_CHECK_EQUAL(result, 0.0);
}

BOOST_AUTO_TEST_CASE(multiPeak)
{
    // More than one peak
    double data[] = { 1.0, 2.0, 1.0, 2.0, 1.0 };
    PeakInterpolator p;
    // Asked to find peak at index 3, should return index 3
    double result = p.findPeakLocation(data, 5, 3);
    BOOST_CHECK_EQUAL(result, 3.0);
    // But asked to find any peak, should return 1 (first peak)
    result = p.findPeakLocation(data, 5);
    BOOST_CHECK_EQUAL(result, 1.0);
}

BOOST_AUTO_TEST_CASE(start)
{
    // Can't meaningfully interpolate if we're identifying element 0
    // as the peak (nothing to its left)
    double data[] = { 1.0, 1.0, 0.0, 0.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 0);
    BOOST_CHECK_EQUAL(result, 0.0);
}

BOOST_AUTO_TEST_CASE(end)
{
    // Likewise for the final element
    double data[] = { 0.0, 0.0, 1.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 3);
    BOOST_CHECK_EQUAL(result, 3.0);
    // But when asked to find any peak, we expect idx 2 to be picked,
    // not idx 3, so that will result in interpolation
    result = p.findPeakLocation(data, 4);
    BOOST_CHECK(result > 2.0 && result < 3.0);
}

BOOST_AUTO_TEST_CASE(longHalfway)
{
    // Peak is exactly half-way between indices
    double data[] = { 1.0, 1.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0 };
    PeakInterpolator p;
    // Asked to find peak for either index 3 or 4, should return 3.5
    double result = p.findPeakLocation(data, 8, 3);
    BOOST_CHECK_EQUAL(result, 3.5);
    result = p.findPeakLocation(data, 8, 4);
    BOOST_CHECK_EQUAL(result, 3.5);
    // Likewise if asked to find any peak
    result = p.findPeakLocation(data, 8);
    BOOST_CHECK_EQUAL(result, 3.5);
}

BOOST_AUTO_TEST_CASE(shortHalfway)
{
    // As longHalfway, but with fewer points
    double data[] = { 1.0, 2.0, 2.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 1);
    BOOST_CHECK_EQUAL(result, 1.5);
    result = p.findPeakLocation(data, 4);
    BOOST_CHECK_EQUAL(result, 1.5);
}

BOOST_AUTO_TEST_CASE(aboveHalfway)
{
    // Peak is nearer to one index than its neighbour. (Exact position
    // depends on the peak interpolation method in use; we only know
    // that it must be beyond the half way point)
    double data[] = { 1.0, 1.5, 2.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 2);
    BOOST_CHECK(result > 1.5 && result < 2.0);
    result = p.findPeakLocation(data, 4);
    BOOST_CHECK(result > 1.5 && result < 2.0);
}

BOOST_AUTO_TEST_CASE(belowHalfway)
{
    // Peak is nearer to one index than its neighbour. (Exact position
    // depends on the peak interpolation method in use; we only know
    // that it must be before the half way point)
    double data[] = { 1.0, 2.0, 1.5, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 1);
    BOOST_CHECK(result > 1.0 && result < 1.5);
    result = p.findPeakLocation(data, 4);
    BOOST_CHECK(result > 1.0 && result < 1.5);
}

BOOST_AUTO_TEST_SUITE_END()

