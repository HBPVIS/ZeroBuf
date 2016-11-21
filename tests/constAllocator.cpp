
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel@Nachbaur@epf.ch
 */

#define BOOST_TEST_MODULE constAllocator

#include <boost/test/unit_test.hpp>

#include "serialization.h"

BOOST_AUTO_TEST_CASE(construction)
{
    const std::vector< uint8_t > data( 10 );
    std::unique_ptr< const zerobuf::Allocator > alloc(
                new zerobuf::ConstAllocator( data.data(), data.size( )));

    BOOST_CHECK_EQUAL( alloc->getData(), data.data( ));
    BOOST_CHECK_EQUAL( alloc->getSize(), data.size( ));

    std::unique_ptr< zerobuf::Allocator > mutableBaseAlloc(
                new zerobuf::ConstAllocator( data.data(), data.size( )));
    BOOST_CHECK_THROW( mutableBaseAlloc->getData(), std::runtime_error );
    BOOST_CHECK_THROW( mutableBaseAlloc->copyBuffer( nullptr, 0 ), std::runtime_error );
    BOOST_CHECK_THROW( mutableBaseAlloc->updateAllocation( 0, false, 0 ), std::runtime_error );
    BOOST_CHECK_THROW( mutableBaseAlloc->compact( 42 ), std::runtime_error );
}

BOOST_AUTO_TEST_CASE(create)
{
    test::TestSchema object;

    const auto& data = object.toBinary();
    test::ConstTestSchemaPtr newObject = test::TestSchema::create( data.ptr.get(), data.size );

    BOOST_CHECK_EQUAL( object, *newObject );
}

BOOST_AUTO_TEST_CASE(zerobufVectorAccessFromConstAllocatorParent)
{
    test::TestNestedZerobuf object;
    test::TestNested nested( 1, 2 );
    object.getNested().push_back( nested );

    const auto& binary = object.toBinary();
    test::ConstTestNestedZerobufPtr constObject(
              test::TestNestedZerobuf::create( binary.ptr.get(), binary.size ));
    BOOST_CHECK_NO_THROW( constObject->getNested()[0] );
}
