
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#include "ConstNonMovingSubAllocator.h"
#include "NonMovingSubAllocator.h"
#include <zerobuf/version.h>

#include <cstring>

namespace zerobuf
{
ConstNonMovingSubAllocator::ConstNonMovingSubAllocator( const NonMovingBaseAllocator* parent,
                                                        const size_t offset,
                                                        const size_t index,
                                                        const size_t numDynamic,
                                                        const size_t staticSize )
    : NonMovingBaseAllocator( staticSize, numDynamic )
    , _parent( parent )
    , _offset( offset )
    , _index( index )
    , _size( staticSize )
{}

ConstNonMovingSubAllocator::ConstNonMovingSubAllocator( const ConstNonMovingSubAllocator& from)
    : NonMovingBaseAllocator( from )
    , _parent( from._parent )
    , _offset( from._offset )
    , _index( from._index )
    , _size( from._size )
{}

ConstNonMovingSubAllocator::ConstNonMovingSubAllocator( const NonMovingSubAllocator& from )
    : NonMovingBaseAllocator( from )
    , _parent( from.getParent( ))
    , _offset( from.getOffset( ))
    , _index( from.getIndex( ))
    , _size( from.getSize( ))
{}

ConstNonMovingSubAllocator::~ConstNonMovingSubAllocator()
{}

ConstNonMovingSubAllocator& ConstNonMovingSubAllocator::operator = (
    const ConstNonMovingSubAllocator& rhs )
{
    if( this == &rhs )
        return *this;

    NonMovingBaseAllocator::operator = ( rhs );
    _parent = rhs._parent;
    _offset = rhs._offset;
    _index = rhs._index;
    _size = rhs._size;
    return *this;
}

ConstNonMovingSubAllocator& ConstNonMovingSubAllocator::operator = (
    const NonMovingSubAllocator& rhs )
{
    NonMovingBaseAllocator::operator = ( rhs );
    _parent = rhs.getParent();
    _offset = rhs.getOffset();
    _index = rhs.getIndex();
    _size = rhs.getSize();
    return *this;
}

const uint8_t* ConstNonMovingSubAllocator::getData() const
{
    const uint8_t* ptr = _parent->getData();
    return ptr + _offset;
}

Allocator* ConstNonMovingSubAllocator::clone() const
{
    return new ConstNonMovingSubAllocator( *this );
}

}
