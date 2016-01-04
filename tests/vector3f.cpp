
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE vector3f

#include <boost/test/unit_test.hpp>

#include <zerobuf/Generic.h>
#include <zerobuf/render/camera.h>
#include <utility>

BOOST_AUTO_TEST_CASE(emptyVector)
{
    const zerobuf::render::Vector3f empty;
    BOOST_CHECK_EQUAL( empty.getX(), 0.f );
    BOOST_CHECK_EQUAL( empty.getY(), 0.f );
    BOOST_CHECK_EQUAL( empty.getZ(), 0.f );
    BOOST_CHECK_EQUAL( empty.getZerobufStaticSize(), 16 );
    BOOST_CHECK_EQUAL( empty.getZerobufNumDynamics(), 0 );
}

BOOST_AUTO_TEST_CASE(initializeVector)
{
    const zerobuf::render::Vector3f vector( 1, -2, 4.5f );
    BOOST_CHECK_EQUAL( vector.getX(), 1.f );
    BOOST_CHECK_EQUAL( vector.getY(), -2.f );
    BOOST_CHECK_EQUAL( vector.getZ(), 4.5f );
}

BOOST_AUTO_TEST_CASE(copyConstructVector)
{
    const zerobuf::render::Vector3f temporary( 1, 0, 0 );
    zerobuf::render::Vector3f vec( temporary );
    BOOST_CHECK( vec == zerobuf::render::Vector3f( 1, 0, 0 ));
    BOOST_CHECK( vec == temporary );
}

BOOST_AUTO_TEST_CASE(moveConstructVector)
{
    zerobuf::render::Vector3f temporary( 1, 0, 0 );
    zerobuf::render::Vector3f vec( std::move( temporary ));
    BOOST_CHECK( vec == zerobuf::render::Vector3f( 1, 0, 0 ));
    BOOST_CHECK( vec != temporary );
}

BOOST_AUTO_TEST_CASE(changeVector)
{
    zerobuf::render::Vector3f vector;
    vector.setX( 2.0f );
    vector.setY( -0.5f );
    vector.setZ( 1.42f );
    BOOST_CHECK_EQUAL( vector.getX(), 2.f );
    BOOST_CHECK_EQUAL( vector.getY(), -.5f );
    BOOST_CHECK_EQUAL( vector.getZ(), 1.42f );
}

const std::string expectedJSON = "{\n"
                                 "   \"x\" : 1,\n"
                                 "   \"y\" : -2,\n"
                                 "   \"z\" : 4.5\n"
                                 "}\n";

BOOST_AUTO_TEST_CASE(vectorToGeneric)
{
    const zerobuf::render::Vector3f vector( 1, -2, 4.5f );
    const void* data = vector.getZerobufData();
    const size_t size = vector.getZerobufSize();
    const zerobuf::Schemas& schemas = zerobuf::render::Vector3f::schemas();

    zerobuf::Generic generic( schemas );
    generic.copyZerobufData( data, size );
    const std::string& json = generic.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );
}

BOOST_AUTO_TEST_CASE(genericToVector)
{
    const zerobuf::Schemas& schemas = zerobuf::render::Vector3f::schemas();
    zerobuf::Generic generic( schemas );
    generic.fromJSON( expectedJSON );

    const zerobuf::render::Vector3f vector( generic );
    BOOST_CHECK_EQUAL( vector.getX(), 1.f );
    BOOST_CHECK_EQUAL( vector.getY(), -2.f );
    BOOST_CHECK_EQUAL( vector.getZ(), 4.5f );
    BOOST_CHECK_EQUAL( vector.getZerobufNumDynamics(),
                       generic.getZerobufNumDynamics( ));
    BOOST_CHECK_EQUAL( vector.getZerobufStaticSize(),
                       generic.getZerobufStaticSize( ));
}

BOOST_AUTO_TEST_CASE(vectorJSON)
{
    const zerobuf::render::Vector3f vector( 1, -2, 4.5f );

    const std::string& json = vector.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );

    zerobuf::render::Vector3f vector2;
    vector2.fromJSON( json );
    BOOST_CHECK( vector == vector2 );
}
