
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE data_progress

#include <boost/test/unit_test.hpp>

#include <zerobuf/data/Progress.h>

using zerobuf::data::Progress;

BOOST_AUTO_TEST_CASE(defaults)
{
    const Progress progress0( 0 );
    BOOST_CHECK_EQUAL( progress0.getAmount(), 1.f );

    const Progress progress1( 42 );
    BOOST_CHECK_EQUAL( progress1.getAmount(), 0.f );
    BOOST_CHECK( progress1.getOperationString().empty( ));

    const Progress progress2( "foo", 17 );
    BOOST_CHECK_EQUAL( progress2.getAmount(), 0.f );
    BOOST_CHECK_EQUAL( progress2.getOperationString(), "foo" );
}

BOOST_AUTO_TEST_CASE(update)
{
    Progress progress( 4 );
    BOOST_CHECK_EQUAL( progress.getAmount(), 0.f );

    ++progress;
    BOOST_CHECK_EQUAL( progress.getAmount(), 0.25f );

    progress += 2;
    BOOST_CHECK_EQUAL( progress.getAmount(), 0.75f );

    progress += 42;
    BOOST_CHECK_EQUAL( progress.getAmount(), 1.f );
}

BOOST_AUTO_TEST_CASE(restart)
{
    Progress progress( 4 );
    ++progress;
    BOOST_CHECK_EQUAL( progress.getAmount(), 0.25f );

    progress.restart( 10 );
    BOOST_CHECK_EQUAL( progress.getAmount(), 0.f );
    BOOST_CHECK_EQUAL( progress.count(), 0 );

    ++progress;
    BOOST_CHECK_EQUAL( progress.getAmount(), 0.1f );
    BOOST_CHECK_EQUAL( progress.count(), 1 );
}
