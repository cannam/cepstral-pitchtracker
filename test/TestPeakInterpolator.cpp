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
    double data[] = { 0.0, 1.0, 0.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 3, 1);
    BOOST_CHECK_EQUAL(result, 1.0);
}

BOOST_AUTO_TEST_CASE(peakAtSample_N5)
{
    double data[] = { 0.0, 1.0, 2.0, 1.0, 0.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 5, 2);
    BOOST_CHECK_EQUAL(result, 2.0);
}

BOOST_AUTO_TEST_CASE(flat)
{
    double data[] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 5, 2);
    BOOST_CHECK_EQUAL(result, 2.0);
}

BOOST_AUTO_TEST_CASE(multiPeak)
{
    double data[] = { 1.0, 2.0, 1.0, 2.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 5, 3);
    BOOST_CHECK_EQUAL(result, 3.0);
}

BOOST_AUTO_TEST_CASE(start)
{
    // Can't meaningfully interpolate if we're identifying element 0
    // as the peak
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
}

BOOST_AUTO_TEST_CASE(longHalfway)
{
    double data[] = { 1.0, 1.0, 1.0, 2.0, 2.0, 1.0, 1.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 8, 4);
    BOOST_CHECK_EQUAL(result, 3.5);
}

BOOST_AUTO_TEST_CASE(shortHalfway)
{
    double data[] = { 1.0, 2.0, 2.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 1);
    BOOST_CHECK_EQUAL(result, 1.5);
}

BOOST_AUTO_TEST_CASE(aboveHalfway)
{
    double data[] = { 1.0, 1.5, 2.0, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 2);
    BOOST_CHECK(result > 1.5 && result < 2.0);
}

BOOST_AUTO_TEST_CASE(belowHalfway)
{
    double data[] = { 1.0, 2.0, 1.5, 1.0 };
    PeakInterpolator p;
    double result = p.findPeakLocation(data, 4, 1);
    BOOST_CHECK(result > 1.0 && result < 1.5);
}

BOOST_AUTO_TEST_SUITE_END()

