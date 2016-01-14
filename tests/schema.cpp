
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE schema

#include <boost/test/unit_test.hpp>

#include <zerobuf/Schema.h>
#include <zerobuf/render/camera.h>
#include <testSchema.h>

BOOST_AUTO_TEST_CASE(compareSchemas)
{
    const zerobuf::Schemas& schemas1 = zerobuf::render::Camera::schemas();
    const zerobuf::Schemas& schemas2 = test::TestEmpty::schemas();
    const zerobuf::Schemas schemas3{{ 0, 0, test::TestEmpty::TYPE_IDENTIFIER(),
                                    {}}};
    const zerobuf::Schemas schemas4;

    BOOST_CHECK_NE( schemas1.size(), schemas2.size( ));
    BOOST_CHECK_NE( schemas1.size(), schemas4.size( ));
    BOOST_CHECK_EQUAL_COLLECTIONS( schemas2.begin(), schemas2.end(),
                                   schemas3.begin(), schemas3.end( ));
    BOOST_CHECK_NE( schemas1.front(), schemas2.front( ));
    BOOST_CHECK_EQUAL( schemas2.front(), schemas3.front( ));
    BOOST_CHECK_EQUAL( schemas2.front(), schemas2.front( ));
    BOOST_CHECK( schemas1.front().ZEROBUF_TYPE().isUUID( ));
}
