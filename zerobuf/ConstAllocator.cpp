
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel.Nachbaur@epfl.ch
 */

#include "ConstAllocator.h"

namespace zerobuf
{

ConstAllocator::ConstAllocator( const uint8_t* data, size_t size )
    : Allocator()
    , _data( data )
    , _size( size )
{}

ConstAllocator::~ConstAllocator()
{}

}
