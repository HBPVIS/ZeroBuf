
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE nestedDynamicZerobuf

#include <boost/test/unit_test.hpp>
#include <testSchema.h>

#include <zerobuf/Generic.h>
#include <utility>

BOOST_AUTO_TEST_CASE(emptyTestNestedZerobuf)
{
    const test::TestNestedZerobuf testNestedZerobuf;
    BOOST_CHECK( testNestedZerobuf.getNested().empty( ));
}

BOOST_AUTO_TEST_CASE(copyConstructTestNestedZerobuf)
{
    test::TestNestedZerobuf temporary;
    temporary.getNested().push_back( test::TestNested( 1, 2 ));
    BOOST_CHECK_EQUAL( temporary.getNested().size(), 1 );

    const test::TestNestedZerobuf testNestedZerobuf( temporary );
    BOOST_CHECK_EQUAL( testNestedZerobuf.getNested().size(), 1 );

    const test::TestNestedZerobuf::Nested& nested=testNestedZerobuf.getNested();
    BOOST_CHECK_EQUAL( nested[0], test::TestNested( 1, 2 ));
    BOOST_CHECK_EQUAL( temporary, testNestedZerobuf );
}

BOOST_AUTO_TEST_CASE(moveConstructTestNestedZerobuf)
{
    test::TestNestedZerobuf temporary;
    temporary.getNested().push_back( test::TestNested( 1, 2 ));
    test::TestNestedZerobuf testNestedZerobuf( std::move( temporary ));

    BOOST_CHECK_EQUAL( testNestedZerobuf.getNested().size(), 1 );
    BOOST_CHECK_EQUAL( testNestedZerobuf.getNested()[0],
                       test::TestNested( 1, 2 ));
    BOOST_CHECK_NE( temporary, testNestedZerobuf );
    BOOST_CHECK( temporary.getNested().empty( ));

    temporary = std::move( testNestedZerobuf );
    BOOST_CHECK_EQUAL( temporary.getNested().size(), 1 );
    BOOST_CHECK_EQUAL( temporary.getNested()[0],
                       test::TestNested( 1, 2 ));
    BOOST_CHECK_NE( temporary, testNestedZerobuf );
    BOOST_CHECK( testNestedZerobuf.getNested().empty( ));

    temporary = std::move( temporary );
    BOOST_CHECK_EQUAL( temporary.getNested().size(), 1 );
    BOOST_CHECK_EQUAL( temporary.getNested()[0],
                       test::TestNested( 1, 2 ));
    BOOST_CHECK_NE( temporary, testNestedZerobuf );
    BOOST_CHECK( testNestedZerobuf.getNested().empty( ));

    BOOST_CHECK_THROW( temporary = std::move( test::TestNested( )),
                       std::runtime_error );
}

BOOST_AUTO_TEST_CASE(changeTestNestedZerobuf)
{
    test::TestNestedZerobuf object;
    object.getNested().push_back( test::TestNested( 1, 2 ));

    test::TestNested threeFour( 3, 4 );
    object.getNested()[0] = threeFour;
    BOOST_CHECK_EQUAL( object.getNested()[0], test::TestNested( 3, 4 ));
    BOOST_CHECK_EQUAL( threeFour, test::TestNested( 3, 4 ));
    BOOST_CHECK_EQUAL( object.getNested()[0], threeFour );

    object.getNested()[0] = test::TestNested( 8, 9 );
    BOOST_CHECK_EQUAL( object.getNested()[0], test::TestNested( 8, 9 ));

    object.getNested()[0] = std::move( test::TestNested( 10, 11 ));
    BOOST_CHECK_EQUAL( object.getNested()[0], test::TestNested( 10, 11 ));

    object.getNested()[0] = std::move( threeFour );
    BOOST_CHECK_EQUAL( object.getNested()[0], test::TestNested( 3, 4 ));
    BOOST_CHECK_NE( object.getNested()[0], threeFour );

    object.setNested( { test::TestNested( 6, 7 ) });
    const test::TestNestedZerobuf& constObject = object;
    BOOST_CHECK_EQUAL( constObject.getNested()[0], test::TestNested( 6, 7 ));
    BOOST_CHECK_EQUAL( object.getNested()[0], test::TestNested( 6, 7 ));

    object.getNested()[0].setIntvalue( 5 );
    BOOST_CHECK_EQUAL( constObject.getNested()[0], test::TestNested( 5, 7 ));
    BOOST_CHECK_EQUAL( object.getNested()[0], test::TestNested( 5, 7 ));
    BOOST_CHECK_EQUAL( object.getNested()[0].getZerobufSize(), 12 );
}

BOOST_AUTO_TEST_CASE(vector)
{
    test::TestNestedZerobuf temporary;
    const test::TestNestedZerobuf& constTemp = temporary;
    temporary.getNested().push_back( test::TestNested( 1, 2 ));
    temporary.getNested().push_back( test::TestNested( 3, 4 ));


    BOOST_CHECK_EQUAL( temporary.getNested().size(), 2 );
    BOOST_CHECK_EQUAL( temporary.getNested(), constTemp.getNested( ));
    BOOST_CHECK_THROW( temporary.getNested()[42], std::runtime_error );
    BOOST_CHECK_THROW( constTemp.getNested()[42], std::runtime_error );

    test::TestNestedZerobuf temporary2;
    BOOST_CHECK_NE( temporary.getNested(), temporary2.getNested( ));

    temporary.getNested().clear();
    BOOST_CHECK_EQUAL( temporary.getNested().size(), 0 );
    BOOST_CHECK( temporary.getNested().empty( ));
    BOOST_CHECK_EQUAL( temporary.getNested(), temporary2.getNested( ));
}

const std::string expectedJSON =
    "{\n"
    "   \"nested\" : [\n"
    "      {\n"
    "         \"intvalue\" : 1,\n"
    "         \"uintvalue\" : 2\n"
    "      },\n"
    "      {\n"
    "         \"intvalue\" : 10,\n"
    "         \"uintvalue\" : 20\n"
    "      }\n"
    "   ]\n"
    "}\n";

BOOST_AUTO_TEST_CASE(testNestedZerobufToGeneric)
{
    test::TestNestedZerobuf testNestedZerobuf;
    testNestedZerobuf.getNested().push_back( test::TestNested( 1, 2 ));
    testNestedZerobuf.getNested().push_back( test::TestNested( 10, 20 ));
    const void* data = testNestedZerobuf.getZerobufData();
    const size_t size = testNestedZerobuf.getZerobufSize();
    const zerobuf::Schemas& schemas = test::TestNestedZerobuf::schemas();

    zerobuf::Generic generic( schemas );
    generic.copyZerobufData( data, size );
    const std::string& json = generic.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );
}

BOOST_AUTO_TEST_CASE(genericToTestNestedZerobuf)
{
    const zerobuf::Schemas& schemas = test::TestNestedZerobuf::schemas();
    zerobuf::Generic generic( schemas );
    generic.fromJSON( expectedJSON );

    test::TestNestedZerobuf expected;
    expected.getNested().push_back( test::TestNested( 1, 2 ));
    expected.getNested().push_back( test::TestNested( 10, 20 ));
    const test::TestNestedZerobuf testNestedZerobuf( generic );
    BOOST_CHECK_EQUAL( testNestedZerobuf, expected );
    BOOST_CHECK_EQUAL( testNestedZerobuf.getZerobufNumDynamics(),
                       generic.getZerobufNumDynamics( ));
    BOOST_CHECK_EQUAL( testNestedZerobuf.getZerobufStaticSize(),
                       generic.getZerobufStaticSize( ));
}

BOOST_AUTO_TEST_CASE(testNestedZerobufJSON)
{
    test::TestNestedZerobuf testNestedZerobuf;
    testNestedZerobuf.getNested().push_back( test::TestNested( 1, 2 ));
    testNestedZerobuf.getNested().push_back( test::TestNested( 10, 20 ));

    const std::string& json = testNestedZerobuf.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );

    test::TestNestedZerobuf testNestedZerobuf2;
    testNestedZerobuf2.fromJSON( json );
    BOOST_CHECK_EQUAL( testNestedZerobuf, testNestedZerobuf2 );
}
