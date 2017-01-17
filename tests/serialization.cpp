
/* Copyright (c) 2015-2017, Human Brain Project
 *                          Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *                          Stefan.Eilemann@epfl.ch
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

    test::TestNestedZerobuf small;
    BOOST_CHECK_EQUAL( small.getDynamic().getIntvalue(), 7 );
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
    BOOST_CHECK_MESSAGE( object.toBinary().size >= 40,
                         object.toBinary().size );

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
    schema.getUint8Tdynamic().clear();
    schema.getUint16Tdynamic().clear();
    schema.getUint32Tdynamic().clear();
    schema.getUint64Tdynamic().clear();
    schema.getUint128Tdynamic().clear();
    schema.getInt8Tdynamic().clear();
    schema.getInt16Tdynamic().clear();
    schema.getInt32Tdynamic().clear();
    schema.getInt64Tdynamic().clear();
    schema.getNesteddynamic().clear();

    // realloc on existing zerobufs
    test::TestNested inner( 1, 2 );
    schema.getNesteddynamic().push_back( inner );
    BOOST_CHECK_EQUAL( inner, schema.getNesteddynamic()[0] );
    schema.check();
    schema.getNesteddynamic()[0].check();
}

BOOST_AUTO_TEST_CASE(vectorOOB)
{
    test::TestSchema schema = getTestObject();
    BOOST_CHECK_THROW( schema.getIntdynamic()[0xC0FFEE], std::runtime_error );
    BOOST_CHECK_THROW( schema.getNesteddynamic()[0xC0FFEE], std::runtime_error);

    const test::TestSchema& chema = schema;
    BOOST_CHECK_THROW( chema.getIntdynamic()[0xC0FFEE], std::runtime_error );
    BOOST_CHECK_THROW( chema.getNesteddynamic()[0xC0FFEE], std::runtime_error);
}


BOOST_AUTO_TEST_CASE(compact)
{
    test::TestSchema schema = getTestObject();
    checkTestObject( schema ); // fill _zerobufs in vectors

    size_t oldSize = schema.toBinary().size;
    schema.compact(); // compaction under threshold
    BOOST_CHECK_EQUAL( schema.toBinary().size, oldSize );
    schema.compact( 0.f ); // force compaction
    BOOST_CHECK_LT( schema.toBinary().size, oldSize );
    checkTestObject( schema ); // fill _zerobufs in vectors

    schema.getNestedMember().setName( "The quick brown fox" );
    schema.getNestedMember().setName( "The quick brown fox jumps" );
    schema.getNestedMember().setName( "The quick brown" );

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
    schema.getUint8Tdynamic().clear();
    schema.getUint16Tdynamic().clear();
    schema.getUint32Tdynamic().clear();
    schema.getUint64Tdynamic().clear();
    schema.getUint128Tdynamic().clear();
    schema.getInt8Tdynamic().clear();
    schema.getInt16Tdynamic().clear();
    schema.getInt32Tdynamic().clear();
    schema.getInt64Tdynamic().clear();
    schema.getNesteddynamic().clear();
    schema.getNestedMember().getName().clear();

    // compact
    oldSize = schema.toBinary().size;

    schema.compact();
    schema.check();
    schema.getNestedMember().check();
    BOOST_CHECK_LT( schema.toBinary().size, oldSize );

    test::TestSchema copy;
    copy.fromJSON( schema.toJSON( ));
    BOOST_CHECK_EQUAL( schema.toBinary().size, copy.toBinary().size );
}
