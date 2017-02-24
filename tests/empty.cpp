
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#define BOOST_TEST_MODULE empty

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <testschema/testSchema.h>

BOOST_AUTO_TEST_CASE(empty)
{
    const test::TestEmpty empty1;
    test::TestEmpty empty2(empty1);
    test::TestEmpty empty3 = empty1;

    BOOST_CHECK_EQUAL(empty1.getZerobufStaticSize(), 0);
    BOOST_CHECK_EQUAL(empty1.getZerobufNumDynamics(), 0);
    BOOST_CHECK_EQUAL(empty1.toBinary().size, 0);
    BOOST_CHECK(empty1.toBinary().ptr == nullptr);
    BOOST_CHECK(empty1 == empty2);
    BOOST_CHECK(empty3 == empty2);

    BOOST_CHECK_THROW(empty2.fromBinary(&empty1, 42), std::runtime_error);
    BOOST_CHECK_EQUAL(empty2.toJSON(), "{}");
    BOOST_CHECK(empty2.fromJSON("{}"));
    BOOST_CHECK(empty2.fromJSON("blubb"));
    BOOST_CHECK_NO_THROW(std::cout << empty1);
}
