
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#include "StaticSubAllocator.h"
#include <zerobuf/version.h>

#include <cstring>

namespace zerobuf
{
template< class A >
StaticSubAllocatorBase< A >::StaticSubAllocatorBase( A& parent,
                                                     const size_t offset,
                                                     const size_t size )
    : _parent( parent )
    , _offset( offset )
    , _size( size )
{}

template< class A >
StaticSubAllocatorBase< A >::~StaticSubAllocatorBase()
{}

template< class A > uint8_t* StaticSubAllocatorBase< A >::getData()
{
    return _parent.getData() + _offset;
}

template<> uint8_t* StaticSubAllocatorBase< const Allocator >::getData()
{
    throw std::runtime_error( "Non-const data access on const data" );
}

template< class A > const uint8_t* StaticSubAllocatorBase< A >::getData() const
{
    return _parent.getData() + _offset;
}

template< class A >
void StaticSubAllocatorBase< A >::copyBuffer( const void* data,
                                              const size_t size )
{
    if( size != _size )
        throw std::runtime_error(
            "Can't copy buffer of different size into a static-sized member" );
    ::memcpy( getData(), data, size );
}

template class StaticSubAllocatorBase< Allocator >;
template class StaticSubAllocatorBase< const Allocator >;
}
