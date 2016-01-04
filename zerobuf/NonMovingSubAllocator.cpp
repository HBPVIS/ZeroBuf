
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#include "NonMovingSubAllocator.h"
#include <zerobuf/version.h>

#include <cstring>

namespace zerobuf
{
template< class A >
NonMovingSubAllocatorBase< A >::NonMovingSubAllocatorBase(
    A& parent, const size_t index, const size_t numDynamic,
    const size_t staticSize )
    : NonMovingBaseAllocator( staticSize, numDynamic )
    , _parent( parent )
    , _index( index )
{}

template< class A > NonMovingSubAllocatorBase< A >::~NonMovingSubAllocatorBase()
{}

template< class A > uint8_t* NonMovingSubAllocatorBase< A >::getData()
{
    return _parent.template getDynamic< uint8_t >( _index );
}

template<> uint8_t* NonMovingSubAllocatorBase< const Allocator >::getData()
{
    throw std::runtime_error( "Non-const access on const allocator" );
}

template< class A >
const uint8_t* NonMovingSubAllocatorBase< A >::getData() const
{
    return _parent.template getDynamic< uint8_t >( _index );
}

template< class A > size_t NonMovingSubAllocatorBase< A >::getSize() const
{
    return _parent.getDynamicSize( _index );
}

template< class A >
void NonMovingSubAllocatorBase< A >::copyBuffer( const void* data,
                                                 const size_t size )
{
    void* to = _parent.updateAllocation( _index, false /*no copy*/, size );
    ::memcpy( to, data, size );
}

template<> void
NonMovingSubAllocatorBase< const Allocator >::copyBuffer( const void*,
                                                          const size_t )
{
    throw std::runtime_error( "Non-const access on const allocator" );
}

template< class A >
void NonMovingSubAllocatorBase< A >::_resize( const size_t newSize )
{
    _parent.updateAllocation( _index, true /*copy*/, newSize );
}

template<>
void NonMovingSubAllocatorBase< const Allocator >::_resize( const size_t )
{
    throw std::runtime_error( "Non-const access on const allocator" );
}

template class NonMovingSubAllocatorBase< Allocator >;
template class NonMovingSubAllocatorBase< const Allocator >;

}
