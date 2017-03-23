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

#include "Cepstrum.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TestCepstrum)

BOOST_AUTO_TEST_CASE(cosine)
{
    // input format is re0, im0, re1, im1...
    float in[] = { 0,0, 10,0, 0,0 };
    double out[4];
    double mm = Cepstrum(4).process(in, out);
    BOOST_CHECK_SMALL(out[0] - (-4.5), 1e-10);
    BOOST_CHECK_EQUAL(out[1], 0);
    BOOST_CHECK_SMALL(out[2] - (-5.5), 1e-10);
    BOOST_CHECK_EQUAL(out[3], 0);
    BOOST_CHECK_EQUAL(mm, 10.0/3.0);
}

BOOST_AUTO_TEST_CASE(symmetry)
{
    // Cepstrum output bins 1..n-1 are symmetric about bin n/2
    float in[] = { 1,2,3,4,5,6,7,8,9,10 };
    double out[8];
    double mm = Cepstrum(8).process(in, out);
    BOOST_CHECK_SMALL(out[1] - out[7], 1e-14);
    BOOST_CHECK_SMALL(out[2] - out[6], 1e-14);
    BOOST_CHECK_SMALL(out[3] - out[5], 1e-14);
    double mmcheck = 0;
    for (int i = 0; i < 5; ++i) {
        mmcheck += sqrt(in[i*2] * in[i*2] + in[i*2+1] * in[i*2+1]);
    }
    mmcheck /= 5;
    BOOST_CHECK_EQUAL(mm, mmcheck);
}

BOOST_AUTO_TEST_CASE(oneHarmonic)
{
    // input format is re0, im0, re1, im1, re2, im2
    // freq for bin i is i * samplerate / n
    // freqs:        0  sr/n  2sr/n  3sr/n  4sr/n  5sr/n  6sr/n  7sr/n  sr/2
    float in[] = { 0,0,  0,0,  10,0,   0,0,  10,0,   0,0,  10,0,   0,0,  0,0 };
    double out[16];
    double mm = Cepstrum(16).process(in, out);
    BOOST_CHECK_EQUAL(mm, 30.0/9.0);
    // peak is at 8
    BOOST_CHECK(out[8] > 0);
    // odd bins are all zero
    BOOST_CHECK_EQUAL(out[1], 0);
    BOOST_CHECK_EQUAL(out[3], 0);
    BOOST_CHECK_EQUAL(out[5], 0);
    BOOST_CHECK_EQUAL(out[7], 0);
    BOOST_CHECK_EQUAL(out[9], 0);
    BOOST_CHECK_EQUAL(out[11], 0);
    BOOST_CHECK_EQUAL(out[13], 0);
    BOOST_CHECK_EQUAL(out[15], 0);
    // the rest are negative
    BOOST_CHECK(out[0] < 0);
    BOOST_CHECK(out[2] < 0);
    BOOST_CHECK(out[4] < 0);
    BOOST_CHECK(out[6] < 0);
    BOOST_CHECK(out[10] < 0);
    BOOST_CHECK(out[12] < 0);
    BOOST_CHECK(out[14] < 0);
}

BOOST_AUTO_TEST_SUITE_END()

