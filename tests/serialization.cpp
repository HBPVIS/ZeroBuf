
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE serialization

#include <boost/test/unit_test.hpp>

#include "serialization.h"


BOOST_AUTO_TEST_CASE(defaultValues)
{
    test::TestSchema object;
    BOOST_CHECK_EQUAL( object.getIntvalue(), 0 );
    BOOST_CHECK_EQUAL( object.getUintvalue(), 42 );
    BOOST_CHECK_EQUAL( object.getFloatvalue(), 4.2f );
    BOOST_CHECK( !object.getFalseBool( ));
    BOOST_CHECK( object.getTrueBool( ));
}

BOOST_AUTO_TEST_CASE(initialized)
{
    const test::TestSchema& schema = getTestObject();
    checkTestObject( schema );

    const test::TestSchema copy( schema );
    checkTestObject( copy );
}

BOOST_AUTO_TEST_CASE(test_string)
{
    test::TestSchema object;

    const std::string message( "The quick brown fox" );
    object.setStringvalue( message );
    BOOST_CHECK( !object.getStringvalue().empty( ));
    BOOST_CHECK_EQUAL( object.getStringvalue().size(), 19 );
    BOOST_CHECK_EQUAL( message, object.getStringvalueString( ));
    BOOST_CHECK_EQUAL( message.length(),
                       object.getStringvalueString().length( ));

    test::TestSchema::Stringvalue& objectString = object.getStringvalue();
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
}

BOOST_AUTO_TEST_CASE(mutablePODArrays)
{
    test::TestSchema object;

    object.setIntdynamic( std::vector< int32_t >{ 3, 4, 5 } );
    object.getIntdynamic()[1] = 6;
    BOOST_CHECK_EQUAL( object.getIntdynamic()[0], 3 );
    BOOST_CHECK_EQUAL( object.getIntdynamic()[1], 6 );
    BOOST_CHECK_EQUAL( object.getIntdynamic()[2], 5 );

    object.getIntarray()[1] = 7;
    BOOST_CHECK_EQUAL( object.getIntarray()[1], 7 );
}

BOOST_AUTO_TEST_CASE(movedVector)
{
    test::TestSchema schema = getTestObject();
    checkTestObject( schema ); // fill _zerobufs in vectors

    // Clear allocated memory
    schema.getIntdynamic().clear();
    schema.getUintdynamic().clear();
    schema.getFloatdynamic().clear();
    schema.getDoubledynamic().clear();
    schema.getBytedynamic().clear();
    schema.getShortdynamic().clear();
    schema.getUbytedynamic().clear();
    schema.getUshortdynamic().clear();
    schema.getUlongdynamic().clear();
    schema.getUint8_tdynamic().clear();
    schema.getUint16_tdynamic().clear();
    schema.getUint32_tdynamic().clear();
    schema.getUint64_tdynamic().clear();
    schema.getUint128_tdynamic().clear();
    schema.getInt8_tdynamic().clear();
    schema.getInt16_tdynamic().clear();
    schema.getInt32_tdynamic().clear();
    schema.getInt64_tdynamic().clear();
    schema.getNesteddynamic().clear();

    // realloc on existing zerobufs
    test::TestNested inner( 1, 2 );
    schema.getNesteddynamic().push_back( inner );
    BOOST_CHECK_EQUAL( inner, schema.getNesteddynamic()[0] );
}
