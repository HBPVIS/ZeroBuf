
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE serialization

#include <boost/test/unit_test.hpp>

#include "serialization.h"


BOOST_AUTO_TEST_CASE(checkDefault)
{
    checkTestObject( getTestObject( ));
}

BOOST_AUTO_TEST_CASE(test_string)
{
    test::TestSchema object;

    const std::string message( "The quick brown fox" );
    object.setStringvalue( message );
    BOOST_CHECK( !object.getStringvalue().empty( ));
    BOOST_CHECK_EQUAL( object.getStringvalue().size(), 19 );
    BOOST_CHECK_EQUAL( message, object.getStringvalueString( ));
    BOOST_CHECK_EQUAL( message.length(), object.getStringvalueString().length( ));

    test::TestSchema::Stringvalue objectString = object.getStringvalue();
    BOOST_CHECK_EQUAL( objectString[2], 'e' );
    BOOST_CHECK( !objectString.empty( ));
    BOOST_CHECK_EQUAL( objectString.size(), 19 );

    objectString.push_back( '!' );
    BOOST_CHECK_EQUAL( objectString.size(), 20 );
    BOOST_CHECK_EQUAL( std::string( objectString.data(), objectString.size( )),
                       message + "!" );
    BOOST_CHECK_EQUAL( object.getStringvalueString(), message + "!" );
    BOOST_CHECK_MESSAGE( object.getZerobufSize() >= 40,
                         object.getZerobufSize( ));

    const std::string longMessage( "So long, and thanks for all the fish!" );
    object.setStringvalue( longMessage );
    BOOST_CHECK( !object.getStringvalue().empty( ));
    BOOST_CHECK_EQUAL( object.getStringvalue().size(), 37 );
    BOOST_CHECK_EQUAL( longMessage, object.getStringvalueString( ));

    const std::string shortMessage( "The fox" );
    object.setStringvalue( shortMessage );
    BOOST_CHECK( !object.getStringvalue().empty( ));
    BOOST_CHECK_EQUAL( object.getStringvalue().size(), 7 );
    BOOST_CHECK_EQUAL( shortMessage, object.getStringvalueString( ));

    object.setStringvalue( message );
    BOOST_CHECK( !object.getStringvalue().empty( ));
    BOOST_CHECK_EQUAL( object.getStringvalue().size(), 19 );
    BOOST_CHECK_EQUAL( message, object.getStringvalueString( ));

    test::TestSchema object2( object );
    BOOST_CHECK_EQUAL( message, object2.getStringvalueString( ));
    BOOST_CHECK_EQUAL( object.getStringvalue(), object2.getStringvalue( ));

    const std::string emptyMessage( "" );
    object.setStringvalue( emptyMessage );
    BOOST_CHECK( object.getStringvalue().empty( ));
    BOOST_CHECK_EQUAL( object.getStringvalue().size(), 0 );
    BOOST_CHECK_EQUAL( emptyMessage, object.getStringvalueString( ));

    object.setEnumeration( test::TestEnum_SECOND );
    BOOST_CHECK_EQUAL( object.getEnumeration(), test::TestEnum_SECOND );

    const std::vector<test::TestEnum> testEnums = { test::TestEnum_FIRST, test::TestEnum_SECOND };
    object.setEnumerations( testEnums );
    const std::vector<test::TestEnum>& result = object.getEnumerationsVector();
    BOOST_CHECK_EQUAL_COLLECTIONS( testEnums.begin(), testEnums.end(),
                                   result.begin(), result.end( ));
}
