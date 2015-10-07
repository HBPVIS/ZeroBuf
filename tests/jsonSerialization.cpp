
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE jsonSerialization

#include <zerobuf/Generic.h>

#include <boost/test/unit_test.hpp>

#include "serialization.h"

const std::string expectedJson( "{\n"
                                "   \"boolvalue\" : true,\n"
                                "   \"bytearray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"bytedynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"bytevalue\" : 42,\n"
                                "   \"doublearray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"doubledynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"doublevalue\" : 42,\n"
                                "   \"enumeration\" : 1,\n"
                                "   \"enumerations\" : [ 0, 1 ],\n"
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
                                "   \"table\" : {\n"
                                "      \"intvalue\" : 42,\n"
                                "      \"uintvalue\" : 43\n"
                                "   },\n"
                                "   \"tables\" : [\n"
                                "      {\n"
                                "         \"intvalue\" : 42,\n"
                                "         \"uintvalue\" : 43\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 43,\n"
                                "         \"uintvalue\" : 44\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 44,\n"
                                "         \"uintvalue\" : 45\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 45,\n"
                                "         \"uintvalue\" : 46\n"
                                "      }\n"
                                "   ],\n"
                                "   \"tables_dynamic\" : [\n"
                                "      {\n"
                                "         \"intvalue\" : 42,\n"
                                "         \"uintvalue\" : 43\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 43,\n"
                                "         \"uintvalue\" : 44\n"
                                "      }\n"
                                "   ],\n"
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

BOOST_AUTO_TEST_CASE(zerobufToJSON)
{
    const test::TestSchema& object( getTestObject( ));

    const std::string& json = object.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJson );
}

BOOST_AUTO_TEST_CASE(zerobufFromJSON)
{
    test::TestSchema object;
    object.fromJSON( expectedJson );
    checkTestObject( object );
}

BOOST_AUTO_TEST_CASE(rawZerobufToJSON)
{
    const test::TestSchema& object( getTestObject( ));
    const void* data = object.getZerobufData();
    const size_t size = object.getZerobufSize();
    const zerobuf::Schema& schema = test::TestSchema::schema();

    zerobuf::Generic generic( schema );
    generic.setZerobufData( data, size );
    const std::string& json = generic.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJson );
}

BOOST_AUTO_TEST_CASE(rawZerobufFromJSON)
{
    zerobuf::Schema schema = test::TestSchema::schema();
    zerobuf::Generic generic( schema );
    generic.fromJSON( expectedJson );

    const test::TestSchema object( generic );
    checkTestObject( object );
}
