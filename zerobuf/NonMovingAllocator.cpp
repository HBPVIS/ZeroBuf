
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
{}

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

void NonMovingAllocator::_resize( const size_t newSize )
{
    _data = (uint8_t*)::realloc( _data, newSize );
    _size = newSize;
}

}
