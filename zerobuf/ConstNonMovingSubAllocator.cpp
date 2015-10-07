
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
                                                        const size_t numDynamic,
                                                        const size_t staticSize )
    : NonMovingBaseAllocator( staticSize, numDynamic )
    , _parent( parent )
    , _offset( offset )
    , _size( numDynamic > 0 ? parent->getDynamicSize( offset ) : staticSize )
{}

ConstNonMovingSubAllocator::ConstNonMovingSubAllocator( const ConstNonMovingSubAllocator& from)
    : NonMovingBaseAllocator( from )
    , _parent( from._parent )
    , _offset( from._offset )
    , _size( from._size )
{}

ConstNonMovingSubAllocator::ConstNonMovingSubAllocator( const NonMovingSubAllocator& from )
    : NonMovingBaseAllocator( from )
    , _parent( from.getParent( ))
    , _offset( from.getOffset( ))
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
    _size = rhs._size;
    return *this;
}

ConstNonMovingSubAllocator& ConstNonMovingSubAllocator::operator = (
    const NonMovingSubAllocator& rhs )
{
    NonMovingBaseAllocator::operator = ( rhs );
    _parent = rhs.getParent();
    _offset = rhs.getOffset();
    _size = rhs.getSize();
    return *this;
}

const uint8_t* ConstNonMovingSubAllocator::getData() const
{
    return getDynamic() > 0 ? _parent->getDynamicPtr< uint8_t >( _offset ):
                              _parent->getItemPtr< uint8_t >( _offset );
}

Allocator* ConstNonMovingSubAllocator::clone() const
{
    return new ConstNonMovingSubAllocator( *this );
}

}
