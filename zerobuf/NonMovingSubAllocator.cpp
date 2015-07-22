
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "NonMovingSubAllocator.h"
#include <zeq/version.h>

namespace zerobuf
{
NonMovingSubAllocator::NonMovingSubAllocator( NonMovingBaseAllocator* parent,
                                              const size_t index,
                                              const size_t numDynamic,
                                              const size_t staticSize )
    : NonMovingBaseAllocator( staticSize, numDynamic )
    , _parent( parent )
    , _index( index )
    , _size( parent->getDynamicSize( index ))
{}

NonMovingSubAllocator::NonMovingSubAllocator( const NonMovingSubAllocator& from)
    : NonMovingBaseAllocator( from )
    , _parent( from._parent )
    , _index( from._index )
    , _size( from._size )
{}

NonMovingSubAllocator::~NonMovingSubAllocator()
{}

NonMovingSubAllocator& NonMovingSubAllocator::operator = (
    const NonMovingSubAllocator& rhs )
{
    if( this == &rhs )
        return *this;

    NonMovingBaseAllocator::operator = ( rhs );
    _parent = rhs._parent;
    _index = rhs._index;
    _size = rhs._size;
    return *this;
}

uint8_t* NonMovingSubAllocator::getData()
{
    return _parent->getDynamic< uint8_t >( _index );
}

const uint8_t* NonMovingSubAllocator::getData() const
{
    return _parent->getDynamic< const uint8_t >( _index );
}

void NonMovingSubAllocator::copyBuffer( const void* data, size_t size )
{
    void* to = _parent->updateAllocation( _index, size );
    _size = size;
    ::memcpy( to, data, size );
}

void NonMovingSubAllocator::_resize( const size_t newSize )
{
    _parent->updateAllocation( _index, newSize );
    if( _size == 0 )
    {
        uint32_t& version = getItem< uint32_t >( 0 );
        version = ZEQ_VERSION_ABI;
    }
    _size = newSize;
}

}
