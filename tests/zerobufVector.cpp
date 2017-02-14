
/* Copyright (c) 2017, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE zerobufvector

#include <boost/test/unit_test.hpp>
#include <testschema/testSchema.h>

BOOST_AUTO_TEST_CASE(resize)
{
    test::TestSchema object;

    BOOST_CHECK_EQUAL( object.getNesteddynamic().size(), 0 );
    object.getNesteddynamic().resize( 5 );
    BOOST_CHECK_EQUAL( object.getNesteddynamic().size(), 5 );
    object.getNesteddynamic()[0].setIntvalue( 5 );
    object.getNesteddynamic().resize( 3 );
    BOOST_CHECK_EQUAL( object.getNesteddynamic().size(), 3 );
    BOOST_CHECK_EQUAL( object.getNesteddynamic()[0].getIntvalue(), 5 );

    object.getNesteddynamic().push_back( test::TestNested( 1, 2 ));
    BOOST_CHECK_EQUAL( object.getNesteddynamic().size(), 4 );
    BOOST_CHECK_EQUAL( object.getNesteddynamic()[3].getIntvalue(), 1 );
}

BOOST_AUTO_TEST_CASE(iterate)
{
    test::TestSchema object;

    object.getNesteddynamic().push_back( test::TestNested( 1, 2 ));
    object.getNesteddynamic().push_back( test::TestNested( 2, 3 ));
    object.getNesteddynamic().push_back( test::TestNested( 1, 2 ));
    for( auto& nested : object.getNesteddynamic( ))
        nested.setIntvalue( 5 );

    BOOST_CHECK_EQUAL( object.getNesteddynamic()[2].getIntvalue(), 5 );
}

BOOST_AUTO_TEST_CASE(const_iterate)
{
    test::TestSchema object;

    object.getNesteddynamic().push_back( test::TestNested( 1, 2 ));
    object.getNesteddynamic().push_back( test::TestNested( 1, 3 ));

    const auto& constObject = object;
    for( const auto& nested : constObject.getNesteddynamic( ))
        BOOST_CHECK_EQUAL( nested.getIntvalue(), 1 );
}
