
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "NonMovingBaseAllocator.h"

#include <map>
#include <cassert>

namespace zerobuf
{

NonMovingBaseAllocator::NonMovingBaseAllocator( const size_t staticSize,
                                                const size_t numDynamic )
    : _staticSize( staticSize )
    , _numDynamic( numDynamic )
{}

NonMovingBaseAllocator::NonMovingBaseAllocator(
    const NonMovingBaseAllocator& from )
    : _staticSize( from._staticSize )
    , _numDynamic( from._numDynamic )
{}

NonMovingBaseAllocator& NonMovingBaseAllocator::operator = (
    const NonMovingBaseAllocator& rhs )
{
    if( this == &rhs )
        return *this;

    _staticSize = rhs._staticSize;
    _numDynamic = rhs._numDynamic;
    return *this;
}

NonMovingBaseAllocator::~NonMovingBaseAllocator()
{}

uint8_t* NonMovingBaseAllocator::updateAllocation( const size_t index,
                                                   const size_t newSize )
{
    uint64_t& oldOffset = _getOffset( index );
    uint64_t& oldSize = _getSize( index );

    if( oldSize >= newSize ) // enough space, update and return
    {
        oldSize = newSize;
        return getData() + oldOffset;
    }

    if( oldOffset != 0 ) // Check for space in current place
    {
        uint64_t nextOffset = getSize();

        for( size_t i = 0; i < _numDynamic; ++i )
        {
            const uint64_t offset = _getOffset( i );
            if( offset != 0 && offset > oldOffset )
                nextOffset = std::min( nextOffset, offset );
        }

        if( oldOffset + newSize < nextOffset ) // enough space, update, return
        {
            oldSize = newSize;
            return getData() + oldOffset;
        }
    }

    // Check for a big enough hole
    typedef std::map< size_t, size_t > ArrayMap;
    ArrayMap arrays; // sort arrays by position
    for( size_t i = 0; i < _numDynamic; ++i )
    {
        const uint64_t offset = _getOffset( i );
        if( i != index && offset >= _staticSize )
            arrays[ offset ] = _getSize( i );
    }

    uint64_t start = _staticSize;
    for( ArrayMap::const_iterator i = arrays.begin(); i != arrays.end(); ++i )
    {
        assert( i->first >= start );
        if( i->first - start >= newSize )
        {
            oldOffset = start;
            oldSize = newSize;
            return getData() + start;
        }

        start = i->first + i->second;
    }

    // realloc space at the end
    _resize( start + newSize );

    // reload address of [offset, size] due to potential realloc
    _getOffset( index ) = start;
    _getSize( index ) = newSize;
    return getData() + start;
}

}
