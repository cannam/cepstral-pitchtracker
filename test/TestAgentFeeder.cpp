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

#include "AgentFeeder.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

static Vamp::RealTime ms(int n) { return Vamp::RealTime::fromMilliseconds(n); }

typedef NoteHypothesis::Estimate Est;

BOOST_AUTO_TEST_SUITE(TestAgentFeeder)

BOOST_AUTO_TEST_CASE(feederEmpty)
{
    AgentFeeder f;
    f.finish();
    AgentFeeder::Hypotheses accepted = f.getAcceptedHypotheses();
    BOOST_CHECK(accepted.empty());
}

BOOST_AUTO_TEST_CASE(feederSingle)
{
    Est e0(1, ms(0), 1);
    Est e10(1, ms(10), 1);
    Est e20(1, ms(20), 1);
    Est e30(1, ms(30), 1);

    AgentFeeder f;
    f.feed(e0);
    f.feed(e10);
    f.feed(e20);
    f.feed(e30);
    f.finish();

    AgentFeeder::Hypotheses accepted = f.getAcceptedHypotheses();
    
    BOOST_CHECK_EQUAL(accepted.size(), size_t(1));
}

BOOST_AUTO_TEST_CASE(feederPairSeparate)
{
    Est e0(1, ms(0), 1);
    Est e10(1, ms(10), 1);
    Est e20(1, ms(20), 1);
    Est e30(1, ms(30), 1);

    Est f0(3, ms(2000), 1);
    Est f10(3, ms(2010), 1);
    Est f20(3, ms(2020), 1);
    Est f30(3, ms(2030), 1);

    AgentFeeder f;
    f.feed(e0);
    f.feed(e10);
    f.feed(e20);
    f.feed(e30);
    f.feed(f0);
    f.feed(f10);
    f.feed(f20);
    f.feed(f30);
    f.finish();

    AgentFeeder::Hypotheses accepted =
	f.getAcceptedHypotheses();
    
    BOOST_CHECK_EQUAL(accepted.size(), size_t(2));
}

BOOST_AUTO_TEST_CASE(feederPairOverlapping)
{
    // eeee
    //   fffffff

    // (With fffffff stopping before eeee has expired.)

    // This should give us one hypothesis, eeee.

    Est e0(1, ms(0), 1);
    Est e10(1, ms(10), 1);

    Est e20(1, ms(20), 1);
    Est f20(3, ms(20), 1);

    Est e30(1, ms(30), 1);
    Est f30(3, ms(30), 1);

    Est f40(3, ms(40), 1);
    Est f50(3, ms(50), 1);
    Est f60(3, ms(60), 1);
    Est f70(3, ms(70), 1);
    Est f80(3, ms(80), 1);

    AgentFeeder f;
    f.feed(e0);
    f.feed(e10);
    f.feed(e20);
    f.feed(f20);
    f.feed(e30);
    f.feed(f30);
    f.feed(f40);
    f.feed(f50);
    f.feed(f60);
    f.feed(f70);
    f.feed(f80);
    f.finish();

    AgentFeeder::Hypotheses accepted = f.getAcceptedHypotheses();
    
    BOOST_CHECK_EQUAL(accepted.size(), size_t(1));

    AgentFeeder::Hypotheses::const_iterator i = accepted.begin();

    BOOST_CHECK_EQUAL(i->getStartTime(), ms(0)); 
    BOOST_CHECK_EQUAL(i->getAcceptedEstimates().size(), size_t(4));
}
        
BOOST_AUTO_TEST_CASE(feederPairOverlappingLong)
{
    // eeee
    //   fffffff

    // (With fffffff continuing until after eeee has expired.)

    // This should give us two overlapping hypotheses. Even though
    // the mono feeder only has one satisfied hypothesis at a
    // time, the eeee hypothesis should become satisfied before
    // the fffffff hypothesis has been, but when the eeee
    // hypothesis ends, the fffffff one should replace it. So,
    // both should be recognised.

    Est e0(1, ms(0), 1);
    Est e10(1, ms(500), 1);

    Est e20(1, ms(1000), 1);
    Est f20(3, ms(1000), 1);

    Est e30(1, ms(1500), 1);
    Est f30(3, ms(1500), 1);

    Est f40(3, ms(2000), 1);
    Est f50(3, ms(2500), 1);
    Est f60(3, ms(3000), 1);
    Est f70(3, ms(3500), 1);
    Est f80(3, ms(4000), 1);

    AgentFeeder f;
    f.feed(e0);
    f.feed(e10);
    f.feed(e20);
    f.feed(f20);
    f.feed(e30);
    f.feed(f30);
    f.feed(f40);
    f.feed(f50);
    f.feed(f60);
    f.feed(f70);
    f.feed(f80);
    f.finish();

    AgentFeeder::Hypotheses accepted = f.getAcceptedHypotheses();
    
    BOOST_CHECK_EQUAL(accepted.size(), size_t(2));

    AgentFeeder::Hypotheses::const_iterator i = accepted.begin();

    BOOST_CHECK_EQUAL(i->getStartTime(), ms(0)); 
    BOOST_CHECK_EQUAL(i->getAcceptedEstimates().size(), size_t(4));
    ++i;

    BOOST_CHECK_EQUAL(i->getStartTime(), ms(1000)); 
    BOOST_CHECK_EQUAL(i->getAcceptedEstimates().size(), size_t(7));
    ++i;
}
        

BOOST_AUTO_TEST_CASE(feederPairContaining)
{
    // eeeeeeee
    //   ffff

    // This should give us eeeeeeee only. The ffff hypothesis
    // (even when satisfied itself) cannot replace the single
    // satisfied hypothesis eeeeeeee while it is still in
    // progress.

    Est e0(1, ms(0), 1);
    Est e10(1, ms(10), 1);
    Est e20(1, ms(20), 1);
    Est e30(1, ms(30), 1);
    Est e40(1, ms(40), 1);
    Est e50(1, ms(50), 1);
    Est e60(1, ms(60), 1);
    Est e70(1, ms(70), 1);

    Est f20(3, ms(20), 1);
    Est f30(3, ms(30), 1);
    Est f40(3, ms(40), 1);
    Est f50(3, ms(50), 1);

    AgentFeeder f;

    f.feed(e0);
    f.feed(e10);

    f.feed(e20);
    f.feed(f20);

    f.feed(e30);
    f.feed(f30);

    f.feed(e40);
    f.feed(f40);

    f.feed(e50);
    f.feed(f50);

    f.feed(e60);
    f.feed(e70);

    f.finish();

    AgentFeeder::Hypotheses accepted = f.getAcceptedHypotheses();
    
    BOOST_CHECK_EQUAL(accepted.size(), size_t(1));
   
    AgentFeeder::Hypotheses::const_iterator i = accepted.begin();

    BOOST_CHECK_EQUAL(i->getStartTime(), ms(0));
    BOOST_CHECK_EQUAL(i->getAcceptedEstimates().size(), size_t(8));
    ++i;
}
        
BOOST_AUTO_TEST_SUITE_END()

