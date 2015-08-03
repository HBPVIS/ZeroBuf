
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE jsonSerialization

#include <zerobuf/Generic.h>

#include <test_json.h>

#include <boost/test/unit_test.hpp>

const std::string expectedJson( "{\n"
                                "   \"boolvalue\" : true,\n"
                                "   \"bytearray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"bytedynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"bytevalue\" : 42,\n"
                                "   \"doublearray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"doubledynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"doublevalue\" : 42,\n"
                                "   \"floatarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"floatdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"floatvalue\" : 42,\n"
                                "   \"int16_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int16_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int16_tvalue\" : 42,\n"
                                "   \"int32_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int32_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int32_tvalue\" : 42,\n"
                                "   \"int64_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int64_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int64_tvalue\" : 42,\n"
                                "   \"int8_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int8_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"int8_tvalue\" : 42,\n"
                                "   \"intarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"intdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"intvalue\" : 42,\n"
                                "   \"shortarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"shortdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"shortvalue\" : 42,\n"
                                "   \"stringvalue\" : \"testmessage\",\n"
                                "   \"ubytearray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"ubytedynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"ubytevalue\" : 42,\n"
                                "   \"uint128_tarray\" : [\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 1\n"
                                "      },\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 1\n"
                                "      },\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 2\n"
                                "      },\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 3\n"
                                "      }\n"
                                "   ],\n"
                                "   \"uint128_tdynamic\" : [\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 1\n"
                                "      },\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 1\n"
                                "      },\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 2\n"
                                "      },\n"
                                "      {\n"
                                "         \"high\" : 0,\n"
                                "         \"low\" : 3\n"
                                "      }\n"
                                "   ],\n"
                                "   \"uint128_tvalue\" : {\n"
                                "      \"high\" : 0,\n"
                                "      \"low\" : 42\n"
                                "   },\n"
                                "   \"uint16_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint16_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint16_tvalue\" : 42,\n"
                                "   \"uint32_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint32_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint32_tvalue\" : 42,\n"
                                "   \"uint64_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint64_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint64_tvalue\" : 42,\n"
                                "   \"uint8_tarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint8_tdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uint8_tvalue\" : 42,\n"
                                "   \"uintarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uintdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"uintvalue\" : 42,\n"
                                "   \"ulongarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"ulongdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"ulongvalue\" : 42,\n"
                                "   \"ushortarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"ushortdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"ushortvalue\" : 42\n"
                                "}\n" );

#define SETVALUES(type, name) \
    const std::vector< type > name##Vector { type(1), type(1), type(2), type(3) }; \
    const type name##value( 42 ); \
    \
    object.set##name##dynamic( name##Vector ); \
    object.set##name##array( name##Vector ); \
    object.set##name##value( name##value );

#define TESTVALUES(type, name) \
    const std::vector< type > expected##name##Vector { type(1), type(1), type(2), type(3) }; \
    const type expected##name##value( 42 ); \
    \
    const std::vector< type >& name##Dynamic( object.get##name##dynamicVector( )); \
    const std::vector< type >& name##Array( object.get##name##arrayVector( )); \
    const type& name##Value( object.get##name##value( )); \
    \
    BOOST_CHECK_EQUAL_COLLECTIONS( expected##name##Vector.begin(), expected##name##Vector.end(),\
                                   name##Dynamic.begin(), name##Dynamic.end( )); \
    BOOST_CHECK_EQUAL_COLLECTIONS( expected##name##Vector.begin(), expected##name##Vector.end(),\
                                   name##Array.begin(), name##Array.end( )); \
    BOOST_CHECK_EQUAL( expected##name##value, name##Value );


test::TestJSON getTestObject()
{
    test::TestJSON object;
    SETVALUES(int32_t, Int);
    SETVALUES(uint32_t, Uint);
    SETVALUES(float, Float);
    SETVALUES(double, Double);
    SETVALUES(int8_t, Byte);
    SETVALUES(int16_t, Short);
    SETVALUES(uint8_t, Ubyte);
    SETVALUES(uint16_t, Ushort);
    SETVALUES(uint64_t, Ulong);
    SETVALUES(uint8_t, Uint8_t);
    SETVALUES(uint16_t, Uint16_t);
    SETVALUES(uint32_t, Uint32_t);
    SETVALUES(uint64_t, Uint64_t);
    SETVALUES(servus::uint128_t, Uint128_t);
    SETVALUES(int8_t, Int8_t);
    SETVALUES(int16_t, Int16_t);
    SETVALUES(int32_t, Int32_t);
    SETVALUES(int64_t, Int64_t);
    object.setBoolvalue( true );
    object.setStringvalue( "testmessage" );
    return object;
}

void checkTestObject( const test::TestJSON& object )
{
    TESTVALUES(int32_t, Int);
    TESTVALUES(uint32_t, Uint);
    TESTVALUES(float, Float);
    TESTVALUES(double, Double);
    TESTVALUES(int8_t, Byte);
    TESTVALUES(int16_t, Short);
    TESTVALUES(uint8_t, Ubyte);
    TESTVALUES(uint16_t, Ushort);
    TESTVALUES(uint64_t, Ulong);
    TESTVALUES(uint8_t, Uint8_t);
    TESTVALUES(uint16_t, Uint16_t);
    TESTVALUES(uint32_t, Uint32_t);
    TESTVALUES(uint64_t, Uint64_t);
    TESTVALUES(servus::uint128_t, Uint128_t);
    TESTVALUES(int8_t, Int8_t);
    TESTVALUES(int16_t, Int16_t);
    TESTVALUES(int32_t, Int32_t);
    TESTVALUES(int64_t, Int64_t);
    BOOST_CHECK( object.getBoolvalue( ));
    BOOST_CHECK_EQUAL( object.getStringvalueString(), "testmessage" );
}

BOOST_AUTO_TEST_CASE(zerobufToJSON)
{
    const test::TestJSON& object( getTestObject( ));

    const std::string& json = object.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJson );
}

BOOST_AUTO_TEST_CASE(zerobufFromJSON)
{
    test::TestJSON object;
    object.fromJSON( expectedJson );
    checkTestObject( object );
}

BOOST_AUTO_TEST_CASE(rawZerobufToJSON)
{
    const test::TestJSON& object( getTestObject( ));
    const void* data = object.getZerobufData();
    const size_t size = object.getZerobufSize();
    const zerobuf::Schema& schema = test::TestJSON::schema();

    zerobuf::Generic generic( schema );
    generic.setZerobufData( data, size );
    const std::string& json = generic.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJson );
}

BOOST_AUTO_TEST_CASE(rawZerobufFromJSON)
{
    zerobuf::Schema schema = test::TestJSON::schema();
    zerobuf::Generic generic( schema );
    generic.fromJSON( expectedJson );

    const test::TestJSON object( generic );
    checkTestObject( object );
}
