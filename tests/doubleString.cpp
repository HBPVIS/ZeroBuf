
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE doubleString

#include <boost/test/unit_test.hpp>

#include <testschema/doubleString.h>
#include <iostream>

BOOST_AUTO_TEST_CASE(reallocation)
{
    // Repro: allocator bug with a specific order of calls
    //  https://github.com/HBPVIS/ZeroBuf/pull/50
    test::DoubleString doubleString;
    doubleString.setKey( std::string( 1, 1 ));
    doubleString.setValue( std::string( 12, 12 ));
    doubleString.setKey( std::string( 8, 8 ));
    doubleString.setValue( std::string( 16, 16 ));
    doubleString.setKey( std::string( 12, 12 ));
    doubleString.setValue( std::string( 17, 17 ));

    // Try to find another combination
    for( size_t i = 0; i < 1000; ++i )
    {
        size_t len = ::rand() % 20;
        doubleString.setKey( std::string( len, 'a' ));

        len = ::rand() % 20;
        doubleString.setValue( std::string( len, 'a' ));
        BOOST_CHECK_LE( doubleString.toBinary().size,
                        36/*header*/ + 19/*biggest hole*/ + 40/*20b strings*/ );
    }
}
