
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#define BOOST_TEST_MODULE nestedDynamicZerobuf

#include <boost/test/unit_test.hpp>
#include <testschema/testSchema.h>

#include <utility>

BOOST_AUTO_TEST_CASE(defaultValues)
{
    const test::TestNestedZerobuf testNestedZerobuf;
    BOOST_CHECK(testNestedZerobuf.getNested().empty());
    BOOST_CHECK_EQUAL(testNestedZerobuf.getNest().getIntvalue(), -17);
    BOOST_CHECK_EQUAL(testNestedZerobuf.getNest().getUintvalue(), 17);
}

BOOST_AUTO_TEST_CASE(copyConstructTestNestedZerobuf)
{
    test::TestNestedZerobuf temporary;
    temporary.getDynamic().setName("Hugo");
    temporary.getNested().push_back(test::TestNested(1, 2));
    BOOST_CHECK_EQUAL(temporary.getNested().size(), 1);
    BOOST_CHECK_EQUAL(temporary.getDynamic().getNameString(), "Hugo");

    const test::TestNestedZerobuf testNestedZerobuf(temporary);
    BOOST_CHECK_EQUAL(testNestedZerobuf.getNested().size(), 1);
    BOOST_CHECK_EQUAL(testNestedZerobuf.getDynamic().getNameString(), "Hugo");

    const test::TestNestedZerobuf::Nested& nested =
        testNestedZerobuf.getNested();
    BOOST_CHECK_EQUAL(nested[0], test::TestNested(1, 2));
    BOOST_CHECK_EQUAL(temporary, testNestedZerobuf);
}

BOOST_AUTO_TEST_CASE(moveConstructTestNestedZerobuf)
{
    test::TestNestedZerobuf temporary;
    temporary.getDynamic().setName("Hugo");
    temporary.getNested().push_back(test::TestNested(1, 2));
    test::TestNestedZerobuf testNestedZerobuf(std::move(temporary));

    BOOST_CHECK_EQUAL(testNestedZerobuf.getNested().size(), 1);
    BOOST_CHECK_EQUAL(testNestedZerobuf.getNested()[0], test::TestNested(1, 2));
    BOOST_CHECK_EQUAL(testNestedZerobuf.getDynamic().getNameString(), "Hugo");
    BOOST_CHECK_NE(temporary, testNestedZerobuf);
    BOOST_CHECK(temporary.getNested().empty());
    BOOST_CHECK_MESSAGE(temporary.getDynamic().getNameString().empty(),
                        temporary.getDynamic().getNameString());

    temporary = std::move(testNestedZerobuf);
    BOOST_CHECK_EQUAL(temporary.getNested().size(), 1);
    BOOST_CHECK_EQUAL(temporary.getNested()[0], test::TestNested(1, 2));
    BOOST_CHECK_EQUAL(temporary.getDynamic().getNameString(), "Hugo");
    BOOST_CHECK_NE(temporary, testNestedZerobuf);
    BOOST_CHECK(testNestedZerobuf.getNested().empty());

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
    temporary = std::move(temporary);
#pragma clang diagnostic pop
    BOOST_CHECK_EQUAL(temporary.getNested().size(), 1);
    BOOST_CHECK_EQUAL(temporary.getNested()[0], test::TestNested(1, 2));
    BOOST_CHECK_NE(temporary, testNestedZerobuf);
    BOOST_CHECK(testNestedZerobuf.getNested().empty());

    BOOST_CHECK_THROW(temporary = std::move(test::TestNested()),
                      std::runtime_error);
}

BOOST_AUTO_TEST_CASE(changeTestNestedZerobuf)
{
    test::TestNestedZerobuf object;
    object.getNested().push_back(test::TestNested(1, 2));

    test::TestNested threeFour(3, 4);
    object.getNested()[0] = threeFour;
    BOOST_CHECK_EQUAL(object.getNested()[0], test::TestNested(3, 4));
    BOOST_CHECK_EQUAL(threeFour, test::TestNested(3, 4));
    BOOST_CHECK_EQUAL(object.getNested()[0], threeFour);

    object.getNested()[0] = test::TestNested(8, 9);
    BOOST_CHECK_EQUAL(object.getNested()[0], test::TestNested(8, 9));

    object.getNested()[0] = test::TestNested(10, 11);
    BOOST_CHECK_EQUAL(object.getNested()[0], test::TestNested(10, 11));

    object.getNested()[0] = std::move(threeFour);
    BOOST_CHECK_EQUAL(object.getNested()[0], test::TestNested(3, 4));
    BOOST_CHECK_NE(object.getNested()[0], threeFour);

    object.setNested({test::TestNested(6, 7)});
    const test::TestNestedZerobuf& constObject = object;
    BOOST_CHECK_EQUAL(constObject.getNested()[0], test::TestNested(6, 7));
    BOOST_CHECK_EQUAL(object.getNested()[0], test::TestNested(6, 7));

    object.getNested()[0].setIntvalue(5);
    BOOST_CHECK_EQUAL(constObject.getNested()[0], test::TestNested(5, 7));
    BOOST_CHECK_EQUAL(object.getNested()[0], test::TestNested(5, 7));
    BOOST_CHECK_EQUAL(object.getNested()[0].toBinary().size, 12);
}

BOOST_AUTO_TEST_CASE(vector)
{
    test::TestNestedZerobuf object;
    const test::TestNestedZerobuf& constObject = object;
    object.getNested().push_back(test::TestNested(1, 2));
    object.getNested().push_back(test::TestNested(3, 4));

    BOOST_CHECK_EQUAL(object.getNested().size(), 2);
    BOOST_CHECK_EQUAL(object.getNested(), constObject.getNested());
    BOOST_CHECK_THROW(object.getNested()[42], std::runtime_error);
    BOOST_CHECK_THROW(constObject.getNested()[42], std::runtime_error);

    test::TestNestedZerobuf object2;
    BOOST_CHECK_NE(object.getNested(), object2.getNested());

    object.getNested().clear();
    BOOST_CHECK_EQUAL(object.getNested().size(), 0);
    BOOST_CHECK(object.getNested().empty());
    BOOST_CHECK_EQUAL(object.getNested(), object2.getNested());
}

BOOST_AUTO_TEST_CASE(compact)
{
    test::TestNestedZerobuf object;
    object.getNested().push_back(test::TestNested(1, 2));
    object.getNested().push_back(test::TestNested(3, 4));

    object.getDynamic().setName("The quick brown fox");
    object.getDynamic().setName("The quick brown fox jumps");
    object.getDynamic().setName("The quick brown");

    size_t minSize = test::TestNestedZerobuf::ZEROBUF_STATIC_SIZE() +
                     2 * test::TestNested::ZEROBUF_STATIC_SIZE() +
                     test::TestDynamic::ZEROBUF_STATIC_SIZE() +
                     object.getDynamic().getName().size();

    BOOST_CHECK_LT(minSize, object.toBinary().size);
    object.compact();
    object.check();
    object.getDynamic().check();
    BOOST_CHECK_EQUAL(minSize, object.toBinary().size);

    minSize = test::TestNestedZerobuf::ZEROBUF_STATIC_SIZE() +
              test::TestDynamic::ZEROBUF_STATIC_SIZE();
    object.getDynamic().getName().clear();
    object.getNested().clear();

    BOOST_CHECK_LT(minSize, object.toBinary().size);
    object.compact();
    object.check();
    object.getDynamic().check();
    BOOST_CHECK_EQUAL(minSize, object.toBinary().size);
}

const std::string expectedJSON =
    "{\n"
    "   \"dynamic\" : {\n"
    "      \"intvalue\" : 7,\n"
    "      \"name\" : \"Hugo\"\n"
    "   },\n"
    "   \"nest\" : {\n"
    "      \"intvalue\" : -17,\n"
    "      \"uintvalue\" : 17\n"
    "   },\n"
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

BOOST_AUTO_TEST_CASE(testNestedZerobufJSON)
{
    test::TestNestedZerobuf testNestedZerobuf;
    testNestedZerobuf.getDynamic().setName("Hugo");
    testNestedZerobuf.getNested().push_back(test::TestNested(1, 2));
    testNestedZerobuf.getNested().push_back(test::TestNested(10, 20));

    const std::string& json = testNestedZerobuf.toJSON();
    BOOST_CHECK_EQUAL(json, expectedJSON);

    test::TestNestedZerobuf testNestedZerobuf2;
    testNestedZerobuf2.fromJSON(json);
    BOOST_CHECK_EQUAL(testNestedZerobuf, testNestedZerobuf2);
}
