
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include "NonMovingBaseAllocator.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <string.h>

namespace zerobuf
{
NonMovingBaseAllocator::NonMovingBaseAllocator(const size_t staticSize,
                                               const size_t numDynamic)
    : _staticSize(staticSize)
    , _numDynamic(numDynamic)
{
}

NonMovingBaseAllocator::~NonMovingBaseAllocator()
{
}

uint8_t* NonMovingBaseAllocator::_moveAllocation(const size_t index,
                                                 const bool copy,
                                                 const size_t newOffset,
                                                 const size_t newSize)
{
    uint64_t& offset = _getOffset(index);
    uint64_t& size = _getSize(index);
    uint8_t* base = getData();

    if (copy && offset > 0)
        ::memmove(base + newOffset, base + offset,
                  std::min(size, uint64_t(newSize)));

    offset = newOffset;
    size = newSize;
    return base + newOffset;
}

uint8_t* NonMovingBaseAllocator::updateAllocation(const size_t index,
                                                  const bool copy,
                                                  const size_t newSize)
{
    uint64_t& oldOffset = _getOffset(index);
    uint64_t& oldSize = _getSize(index);

    if (oldSize >= newSize) // enough space, update and return
    {
        oldSize = newSize;
        if (newSize > 0)
            return getData() + oldOffset;

        oldOffset = 0;
        return nullptr;
    }

    if (oldOffset != 0) // Check for space in current place
    {
        uint64_t nextOffset = getSize();

        for (size_t i = 0; i < _numDynamic; ++i)
        {
            const uint64_t offset = _getOffset(i);
            if (offset != 0 && offset > oldOffset)
                nextOffset = std::min(nextOffset, offset);
        }

        if (oldOffset + newSize < nextOffset) // enough space, update, return
        {
            oldSize = newSize;
            return getData() + oldOffset;
        }
    }

    // Check for a big enough hole
    //-- record allocations
    typedef std::map<size_t, size_t> ArrayMap; // offset -> size
    ArrayMap arrays;                           // sort arrays by position
    for (size_t i = 0; i < _numDynamic; ++i)
    {
        const uint64_t offset = _getOffset(i);
        if (i != index && offset >= _staticSize)
            arrays[offset] = _getSize(i);
    }

    //-- find hole
    uint64_t start = _staticSize;
    for (auto i = arrays.cbegin(); i != arrays.cend(); ++i)
    {
        assert(i->first >= start);
        if (i->first - start >= newSize)
            return _moveAllocation(index, copy, start, newSize);

        start = i->first + i->second;
    }

    //-- check for space after last allocation
    if (getSize() - start >= newSize)
        return _moveAllocation(index, copy, start, newSize);

    // realloc space at the end
    if (start + newSize <= getSize())
        throw std::runtime_error(
            "Internal allocator error: allocation shrinks from " +
            std::to_string(getSize()) + " to " + std::to_string(start) + " + " +
            std::to_string(newSize));

    _resize(start + newSize);
    return _moveAllocation(index, copy, start, newSize);
}

void NonMovingBaseAllocator::compact(const float threshold)
{
    uint64_t dynamicSize = 0;
    for (size_t i = 0; i < _numDynamic; ++i)
        if (_getOffset(i) > 0)
            dynamicSize += _getSize(i);

    const uint64_t minSize = _staticSize + dynamicSize;
    if (float(getSize() - minSize) / float(minSize) < threshold)
        return;

    // OPT: Surely there is a clever algo to do this in place, but for now use
    //      two copies:
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[dynamicSize]);
    uint8_t* iter = buffer.get();
    for (size_t i = 0; i < _numDynamic; ++i)
    {
        uint64_t& offset = _getOffset(i);
        if (offset == 0)
            continue;

        const uint64_t size = _getSize(i);
        const uint64_t newOffset = iter - buffer.get() + _staticSize;
        ::memcpy(iter, getData() + offset, size);
        iter += size;
        offset = newOffset;
    }

    _resize(minSize);
    ::memcpy(getData() + _staticSize, buffer.get(), dynamicSize);
}
}
