
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#include "DynamicSubAllocator.h"
#include <zerobuf/version.h>

#include <cstring>

namespace zerobuf
{
template< class A >
DynamicSubAllocatorBase< A >::DynamicSubAllocatorBase(
    A& parent, const size_t headerIndex, const size_t elementIndex,
    const size_t size )
    : _parent( parent )
    , _header( headerIndex )
    , _element( elementIndex )
    , _size( size )
{}

template< class A >
DynamicSubAllocatorBase< A >::~DynamicSubAllocatorBase()
{}

template< class A > uint8_t* DynamicSubAllocatorBase< A >::getData()
{
    return _parent.template getDynamic< uint8_t >( _header ) + _element * _size;
}

template<> uint8_t* DynamicSubAllocatorBase< const Allocator >::getData()
{
    throw std::runtime_error( "Non-const data access on const data" );
}

template< class A > const uint8_t* DynamicSubAllocatorBase< A >::getData() const
{
    return _parent.template getDynamic< uint8_t >( _header ) + _element * _size;
}

template< class A >
void DynamicSubAllocatorBase< A >::copyBuffer( const void* data,
                                               const size_t size )
{
    if( size != _size )
        throw std::runtime_error(
            "Can't copy buffer of different size into a static-sized member" );
    ::memcpy( getData(), data, size );
}

template class DynamicSubAllocatorBase< Allocator >;
template class DynamicSubAllocatorBase< const Allocator >;
}
