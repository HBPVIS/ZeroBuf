
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE camera

#include <boost/test/unit_test.hpp>

#include <zerobuf/render/camera.h>


BOOST_AUTO_TEST_CASE(emptyCamera)
{
    const zerobuf::render::Camera camera;
    const std::vector< float > empty( 3, 0 );
    const std::vector< float >& origin = camera.getOriginVector();
    const std::vector< float >& lookAt = camera.getLookAtVector();
    const std::vector< float >& up = camera.getUpVector();
    BOOST_CHECK_EQUAL_COLLECTIONS( origin.begin(), origin.end(),
                                   empty.begin(), empty.end( ));
    BOOST_CHECK_EQUAL_COLLECTIONS( lookAt.begin(), lookAt.end(),
                                   empty.begin(), empty.end( ));
    BOOST_CHECK_EQUAL_COLLECTIONS( up.begin(), up.end(),
                                   empty.begin(), empty.end( ));
}

BOOST_AUTO_TEST_CASE(changeCamera)
{
    zerobuf::render::Camera camera;
    camera.setOrigin( std::vector< float > { 1.f, 0.f, 0.f } );
    camera.setLookAt( std::vector< float > { 0.f, 1.f, 0.f } );
    camera.setUp( std::vector< float > { 0.f, 0.f, 1.f } );
    BOOST_CHECK_EQUAL( camera.getOrigin()[0], 1.f );
    BOOST_CHECK_EQUAL( camera.getLookAt()[1], 1.f );
    BOOST_CHECK_EQUAL( camera.getUp()[2], 1.f );
}

BOOST_AUTO_TEST_CASE(cameraJSON)
{
    zerobuf::render::Camera camera;
    camera.setOrigin( std::vector< float > { 1.f, 0.f, 0.f } );
    camera.setLookAt( std::vector< float > { 0.f, 1.f, 0.f } );
    camera.setUp( std::vector< float > { 0.f, 0.f, 1.f } );

    const std::string expectedJSON = "{\n"
                                     "   \"lookAt\" : [ 0, 1, 0 ],\n"
                                     "   \"origin\" : [ 1, 0, 0 ],\n"
                                     "   \"up\" : [ 0, 0, 1 ]\n"
                                     "}\n";
    const std::string& json = camera.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );

    zerobuf::render::Camera camera2;
    camera2.fromJSON( json );
    BOOST_CHECK( camera == camera2 );
}
