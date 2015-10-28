
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#include "NonMovingSubAllocator.h"
#include <zerobuf/version.h>

#include <cstring>

namespace zerobuf
{
NonMovingSubAllocator::NonMovingSubAllocator( NonMovingBaseAllocator* parent,
                                              const size_t offset,
                                              const size_t numDynamic,
                                              const size_t staticSize )
    : NonMovingBaseAllocator( staticSize, numDynamic )
    , _parent( parent )
    , _offset( offset )
    , _size( numDynamic > 0 ? parent->getDynamicSize( offset ) : staticSize )
{}

NonMovingSubAllocator::NonMovingSubAllocator( const NonMovingSubAllocator& from)
    : NonMovingBaseAllocator( from )
    , _parent( from._parent )
    , _offset( from._offset )
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
    _offset = rhs._offset;
    _size = rhs._size;
    return *this;
}

uint8_t* NonMovingSubAllocator::getData()
{
    return getNumDynamic() > 0 ? _parent->getDynamic< uint8_t >( _offset ):
                                 _parent->getItemPtr< uint8_t >( _offset );
}

const uint8_t* NonMovingSubAllocator::getData() const
{
    return getNumDynamic() > 0 ? _parent->getDynamic< uint8_t >( _offset ):
                                 _parent->getItemPtr< uint8_t >( _offset );
}

void NonMovingSubAllocator::copyBuffer( const void* data, const size_t size )
{
    void* to = getNumDynamic() > 0 ? _parent->updateAllocation( _offset, size ):
                                     getData();
    _size = size;
    ::memcpy( to, data, size );
}

void NonMovingSubAllocator::_resize( const size_t newSize )
{
    _parent->updateAllocation( _offset, newSize );
    if( _size == 0 )
    {
        uint32_t& version = getItem< uint32_t >( 0 );
        version = ZEROBUF_VERSION_ABI;
    }
    _size = newSize;
}

Allocator* NonMovingSubAllocator::clone() const
{
    return new NonMovingSubAllocator( *this );
}

}
