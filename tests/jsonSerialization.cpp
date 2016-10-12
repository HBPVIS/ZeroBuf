
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE jsonSerialization

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "serialization.h"

const std::string expectedNestedJson( "{\n"
                                      "   \"intvalue\" : 42,\n"
                                      "   \"uintvalue\" : 4200\n"
                                      "}\n" );
const std::string expectedJson( "{\n"
                                "   \"boolvalue\" : true,\n"
                                "   \"bytearray\" : \"AQECAw==\",\n"
                                "   \"bytedynamic\" : \"AQECAw==\",\n"
                                "   \"bytevalue\" : 42,\n"
                                "   \"doublearray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"doubledynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"doublevalue\" : 42,\n"
                                "   \"enumeration\" : \"SECOND\",\n"
                                "   \"enumerations\" : [ 0, 1 ],\n"
                                "   \"falseBool\" : false,\n"
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
                                "   \"nested\" : {\n"
                                "      \"intvalue\" : 42,\n"
                                "      \"uintvalue\" : 4200\n"
                                "   },\n"
                                "   \"nestedMember\" : {\n"
                                "      \"intvalue\" : 7,\n"
                                "      \"name\" : \"Hugo\"\n"
                                "   },\n"
                                "   \"nestedarray\" : [\n"
                                "      {\n"
                                "         \"intvalue\" : 42,\n"
                                "         \"uintvalue\" : 4200\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 43,\n"
                                "         \"uintvalue\" : 4201\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 44,\n"
                                "         \"uintvalue\" : 4202\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 45,\n"
                                "         \"uintvalue\" : 4203\n"
                                "      }\n"
                                "   ],\n"
                                "   \"nesteddynamic\" : [\n"
                                "      {\n"
                                "         \"intvalue\" : 42,\n"
                                "         \"uintvalue\" : 4200\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 43,\n"
                                "         \"uintvalue\" : 4201\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 44,\n"
                                "         \"uintvalue\" : 4202\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 45,\n"
                                "         \"uintvalue\" : 4203\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 46,\n"
                                "         \"uintvalue\" : 4204\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 47,\n"
                                "         \"uintvalue\" : 4205\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 48,\n"
                                "         \"uintvalue\" : 4206\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 49,\n"
                                "         \"uintvalue\" : 4207\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 50,\n"
                                "         \"uintvalue\" : 4208\n"
                                "      },\n"
                                "      {\n"
                                "         \"intvalue\" : 51,\n"
                                "         \"uintvalue\" : 4209\n"
                                "      }\n"
                                "   ],\n"
                                "   \"shortarray\" : [ 1, 1, 2, 3 ],\n"
                                "   \"shortdynamic\" : [ 1, 1, 2, 3 ],\n"
                                "   \"shortvalue\" : 42,\n"
                                "   \"stringvalue\" : \"testmessage\",\n"
                                "   \"trueBool\" : true,\n"
                                "   \"ubytearray\" : \"AQECAw==\",\n"
                                "   \"ubytedynamic\" : \"AQECAw==\",\n"
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

    const test::TestNested& constNested = object.getNested();
    BOOST_CHECK_EQUAL( constNested.toJSON(), expectedNestedJson );

    test::TestNested nested = object.getNested();
    BOOST_CHECK_EQUAL( nested.toJSON(), expectedNestedJson );
}

BOOST_AUTO_TEST_CASE(zerobufFromJSON)
{
    test::TestNested nested;
    nested.fromJSON( expectedNestedJson );
    checkTestObject( nested );

    test::TestSchema object;
    object.fromJSON( expectedJson );
    checkTestObject( object );
}

BOOST_AUTO_TEST_CASE(enum_string_conversion)
{
    test::TestEnum testEnum = test::TestEnum::FIRST;
    BOOST_CHECK_EQUAL( test::to_string( testEnum ), "FIRST" );
    testEnum = test::from_string( "SECOND" );
    BOOST_CHECK_EQUAL( testEnum, test::TestEnum::SECOND );
    BOOST_CHECK_NO_THROW( std::cout << testEnum << std::endl );

    BOOST_CHECK_THROW( test::from_string( "wrong" ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE(json_schema_empty)
{
    BOOST_CHECK_EQUAL( test::TestEmpty::ZEROBUF_SCHEMA(),
                       "{\"$schema\": \"http://json-schema.org/schema#\", "
                       "\"additionalProperties\": false, "
                       "\"description\": \"Class TestEmpty of namespace ['test']\", "
                       "\"title\": \"TestEmpty\", "
                       "\"type\": \"object\"}" );
}

BOOST_AUTO_TEST_CASE(json_schema_nested)
{
    BOOST_CHECK_EQUAL( test::TestNested::ZEROBUF_SCHEMA(),
                       "{\"$schema\": \"http://json-schema.org/schema#\", "
                       "\"additionalProperties\": false, "
                       "\"description\": \"Class TestNested of namespace ['test']\", "
                       "\"properties\": {"
                       "\"intvalue\": {\"type\": \"integer\"}, "
                       "\"uintvalue\": {\"type\": \"integer\"}}, "
                       "\"title\": \"TestNested\", "
                       "\"type\": \"object\"}" );
}

BOOST_AUTO_TEST_CASE(json_schema_dynamic)
{
    BOOST_CHECK_EQUAL( test::TestDynamic::ZEROBUF_SCHEMA(),
                       "{\"$schema\": \"http://json-schema.org/schema#\", "
                       "\"additionalProperties\": false, "
                       "\"description\": \"Class TestDynamic of namespace ['test']\", "
                       "\"properties\": {"
                       "\"intvalue\": {\"type\": \"integer\"}, "
                       "\"name\": {\"type\": \"string\"}}, "
                       "\"title\": \"TestDynamic\", "
                       "\"type\": \"object\"}" );
}

BOOST_AUTO_TEST_CASE(json_schema_nested_zerobuf)
{
    BOOST_CHECK_EQUAL( test::TestNestedZerobuf::ZEROBUF_SCHEMA(),
                       "{\"$schema\": \"http://json-schema.org/schema#\", "
                       "\"additionalProperties\": false, "
                       "\"description\": \"Class TestNestedZerobuf of namespace ['test']\", "
                       "\"properties\": {"
                         "\"dynamic\": {"
                           "\"$schema\": \"http://json-schema.org/schema#\", "
                           "\"additionalProperties\": false, "
                           "\"description\": \"Class TestDynamic of namespace ['test']\", "
                           "\"properties\": {"
                             "\"intvalue\": {\"type\": \"integer\"}, "
                             "\"name\": {\"type\": \"string\"}}, "
                           "\"title\": \"TestDynamic\", "
                           "\"type\": \"object\"}, "
                         "\"nest\": {"
                           "\"$schema\": \"http://json-schema.org/schema#\", "
                           "\"additionalProperties\": false, "
                           "\"description\": \"Class TestNested of namespace ['test']\", "
                           "\"properties\": {"
                             "\"intvalue\": {\"type\": \"integer\"}, "
                             "\"uintvalue\": {\"type\": \"integer\"}}, "
                           "\"title\": \"TestNested\", "
                           "\"type\": \"object\"}, "
                         "\"nested\": {"
                           "\"items\": {"
                             "\"$schema\": \"http://json-schema.org/schema#\", "
                             "\"additionalProperties\": false, "
                             "\"description\": \"Class TestNested of namespace ['test']\", "
                             "\"properties\": {"
                               "\"intvalue\": {\"type\": \"integer\"}, "
                               "\"uintvalue\": {\"type\": \"integer\"}}, "
                             "\"title\": \"TestNested\", "
                             "\"type\": \"object\"}, "
                           "\"type\": \"array\"}}, "
                         "\"title\": \"TestNestedZerobuf\", "
                         "\"type\": \"object\"}" );
}

BOOST_AUTO_TEST_CASE(json_schema_enum)
{
    BOOST_CHECK_EQUAL( test::TestEnumTable::ZEROBUF_SCHEMA(),
                       "{\"$schema\": \"http://json-schema.org/schema#\", "
                       "\"additionalProperties\": false, "
                       "\"description\": \"Class TestEnumTable of namespace ['test']\", "
                       "\"properties\": {"
                         "\"value\": {"
                           "\"$schema\": \"http://json-schema.org/schema#\", "
                           "\"additionalProperties\": false, "
                           "\"description\": \"Enum TestEnum of type uint\", "
                           "\"enum\": [\"FIRST\", \"SECOND\", \"THIRD_UNDERSCORE\"], "
                           "\"title\": \"TestEnum\", "
                           "\"type\": \"string\"}}, "
                       "\"title\": \"TestEnumTable\", "
                       "\"type\": \"object\"}" );
}
