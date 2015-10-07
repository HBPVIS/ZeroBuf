
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#include "NonMovingAllocator.h"
#include <zerobuf/version.h>

#include <cstring>
#include <stdlib.h>

namespace zerobuf
{

NonMovingAllocator::NonMovingAllocator( const size_t staticSize,
                                        const size_t numDynamic )
    : NonMovingBaseAllocator( staticSize, numDynamic )
    , _data( (uint8_t*)::calloc( 1, staticSize ))
    , _size( staticSize )
{
    uint32_t& version = getItem< uint32_t >( 0 );
    version = ZEROBUF_VERSION_ABI;
}

NonMovingAllocator::NonMovingAllocator( const NonMovingAllocator& from )
    : NonMovingBaseAllocator( from )
    , _data( (uint8_t*)malloc( from._size ))
    , _size( from._size )
{
    ::memcpy( _data, from._data, from._size );
}

NonMovingAllocator& NonMovingAllocator::operator = (
    const NonMovingAllocator& rhs )
{
    if( this == &rhs )
        return *this;

    NonMovingBaseAllocator::operator = ( rhs );
    copyBuffer( rhs._data, rhs._size );
    return *this;
}

NonMovingAllocator::~NonMovingAllocator()
{
    ::free( _data );
}

void NonMovingAllocator::copyBuffer( const void* data, size_t size )
{
    _data = (uint8_t*)::realloc( _data, size );
    _size = size;
    ::memcpy( _data, data, size );
}

Allocator* NonMovingAllocator::clone() const
{
    return new NonMovingAllocator( *this );
}

void NonMovingAllocator::_resize( const size_t newSize )
{
    _data = (uint8_t*)::realloc( _data, newSize );
    _size = newSize;
}

}
