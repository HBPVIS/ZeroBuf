
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE camera

#include <boost/test/unit_test.hpp>

#include <zerobuf/Generic.h>
#include <zerobuf/render/camera.h>
#include <utility>

BOOST_AUTO_TEST_CASE(emptyCamera)
{
    const zerobuf::render::Camera camera;
    const zerobuf::render::Vector3f empty;
    BOOST_CHECK( camera.getOrigin() == empty );
    BOOST_CHECK( camera.getLookAt() == empty );
    BOOST_CHECK( camera.getUp() == empty );
}

BOOST_AUTO_TEST_CASE(initializeCamera)
{
    const zerobuf::render::Camera camera( zerobuf::render::Vector3f( 1, 0, 0 ),
                                          zerobuf::render::Vector3f( -1, 1, 0 ),
                                          zerobuf::render::Vector3f( 0, 0, 1 ));
    BOOST_CHECK( camera.getOrigin() == zerobuf::render::Vector3f( 1, 0, 0 ));
    BOOST_CHECK( camera.getLookAt() == zerobuf::render::Vector3f( -1, 1, 0 ));
    BOOST_CHECK( camera.getUp() == zerobuf::render::Vector3f( 0, 0, 1 ));
}

BOOST_AUTO_TEST_CASE(copyConstructCamera)
{
    const zerobuf::render::Camera temporary( zerobuf::render::Vector3f( 1, 0, 0 ),
                                             zerobuf::render::Vector3f( -1, 1, 0 ),
                                             zerobuf::render::Vector3f( 0, 0, 1 ));
    const zerobuf::render::Camera camera( temporary );
    BOOST_CHECK( camera.getOrigin() == zerobuf::render::Vector3f( 1, 0, 0 ));
    BOOST_CHECK( camera.getLookAt() == zerobuf::render::Vector3f( -1, 1, 0 ));
    BOOST_CHECK( camera.getUp() == zerobuf::render::Vector3f( 0, 0, 1 ));
    BOOST_CHECK( camera == temporary );
}

BOOST_AUTO_TEST_CASE(moveConstructCamera)
{
    zerobuf::render::Camera temporary( zerobuf::render::Vector3f( 1, 0, 0 ),
                                       zerobuf::render::Vector3f( -1, 1, 0 ),
                                       zerobuf::render::Vector3f( 0, 0, 1 ));
    const zerobuf::render::Camera camera( std::move( temporary ));
    BOOST_CHECK( camera.getOrigin() == zerobuf::render::Vector3f( 1, 0, 0 ));
    BOOST_CHECK( camera.getLookAt() == zerobuf::render::Vector3f( -1, 1, 0 ));
    BOOST_CHECK( camera.getUp() == zerobuf::render::Vector3f( 0, 0, 1 ));
    BOOST_CHECK_NE( camera, temporary );
}

BOOST_AUTO_TEST_CASE(moveCamera)
{
    zerobuf::render::Camera temporary;
    zerobuf::render::Vector3f origin( 1, 0, 0 );
    temporary.getOrigin() = std::move( origin );
    temporary.setLookAt( std::move( zerobuf::render::Vector3f( -1, 1, 0 )));
    temporary.setUp( std::move( zerobuf::render::Vector3f( 0, 0, 1 )));

    zerobuf::render::Camera camera;
    camera = std::move( temporary );
    BOOST_CHECK( camera.getOrigin() == zerobuf::render::Vector3f( 1, 0, 0 ));
    BOOST_CHECK( camera.getLookAt() == zerobuf::render::Vector3f( -1, 1, 0 ));
    BOOST_CHECK( camera.getUp() == zerobuf::render::Vector3f( 0, 0, 1 ));
    BOOST_CHECK_NE( camera, temporary );
}

BOOST_AUTO_TEST_CASE(changeCamera)
{
    zerobuf::render::Camera camera;
    camera.setOrigin( zerobuf::render::Vector3f( 1.f, 0.f, 0.f ));
    camera.setLookAt( zerobuf::render::Vector3f( 0.f, 1.f, 0.f ));
    camera.setUp( zerobuf::render::Vector3f( 0.f, 0.f, 1.f ));
    BOOST_CHECK_EQUAL( camera.getOrigin().getX(), 1.f );
    BOOST_CHECK_EQUAL( camera.getLookAt().getY(), 1.f );
    BOOST_CHECK_EQUAL( camera.getUp().getZ(), 1.f );
}

BOOST_AUTO_TEST_CASE(assignOther)
{
    zerobuf::render::Camera camera;
    const zerobuf::render::Vector3f vector;
    BOOST_CHECK_THROW( camera = vector, std::runtime_error );
    BOOST_CHECK_THROW( camera = std::move( vector ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE(assignSelf)
{
    zerobuf::render::Camera camera;
    camera.setOrigin( zerobuf::render::Vector3f( 1.f, 0.f, 0.f ));

    camera = camera;
    BOOST_CHECK_EQUAL( camera.getOrigin(),
                       zerobuf::render::Vector3f( 1.f, 0.f, 0.f ));

}

const std::string expectedJSON = "{\n"
                                 "   \"lookAt\" : {\n"
                                 "      \"x\" : 0,\n"
                                 "      \"y\" : 1,\n"
                                 "      \"z\" : 0\n"
                                 "   },\n"
                                 "   \"origin\" : {\n"
                                 "      \"x\" : 1,\n"
                                 "      \"y\" : 0,\n"
                                 "      \"z\" : 0\n"
                                 "   },\n"
                                 "   \"up\" : {\n"
                                 "      \"x\" : 0,\n"
                                 "      \"y\" : 0,\n"
                                 "      \"z\" : 1\n"
                                 "   }\n"
                                 "}\n";

BOOST_AUTO_TEST_CASE(cameraToGeneric)
{
    const zerobuf::render::Camera camera( zerobuf::render::Vector3f( 1, 0, 0 ),
                                          zerobuf::render::Vector3f( 0, 1, 0 ),
                                          zerobuf::render::Vector3f( 0, 0, 1 ));
    const void* data = camera.getZerobufData();
    const size_t size = camera.getZerobufSize();
    const zerobuf::Schemas& schemas = zerobuf::render::Camera::schemas();

    zerobuf::Generic generic( schemas );
    generic.copyZerobufData( data, size );
    const std::string& json = generic.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );
}

BOOST_AUTO_TEST_CASE(genericToCamera)
{
    const zerobuf::Schemas& schemas = zerobuf::render::Camera::schemas();
    zerobuf::Generic generic( schemas );
    generic.fromJSON( expectedJSON );

    const zerobuf::render::Camera expectedCamera( zerobuf::render::Vector3f( 1, 0, 0 ),
                                                  zerobuf::render::Vector3f( 0, 1, 0 ),
                                                  zerobuf::render::Vector3f( 0, 0, 1 ));
    const zerobuf::render::Camera camera( generic );
    BOOST_CHECK( camera == expectedCamera );
    BOOST_CHECK_EQUAL( camera.getZerobufNumDynamics(),
                       generic.getZerobufNumDynamics( ));
    BOOST_CHECK_EQUAL( camera.getZerobufStaticSize(),
                       generic.getZerobufStaticSize( ));
}

BOOST_AUTO_TEST_CASE(cameraJSON)
{
    zerobuf::render::Camera camera;
    camera.setOrigin( zerobuf::render::Vector3f( 1.f, 0.f, 0.f ));
    camera.setLookAt( zerobuf::render::Vector3f( 0.f, 1.f, 0.f ));
    camera.setUp( zerobuf::render::Vector3f( 0.f, 0.f, 1.f ));

    const std::string& json = camera.toJSON();
    BOOST_CHECK_EQUAL( json, expectedJSON );

    zerobuf::render::Camera camera2;
    camera2.fromJSON( json );
    BOOST_CHECK( camera == camera2 );

    BOOST_CHECK( !camera.fromJSON( "blubb" ));
}
