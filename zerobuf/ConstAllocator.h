
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel.Nachbaur@epfl.ch
 */

#ifndef ZEROBUF_CONSTALLOCATOR_H
#define ZEROBUF_CONSTALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/Allocator.h> // base class

namespace zerobuf
{
/** A zerobuf root allocator which gives read-only access to its data. */
class ConstAllocator : public Allocator
{
public:
    ZEROBUF_API ConstAllocator( const uint8_t* data, size_t size );

    ZEROBUF_API ~ConstAllocator();

    const uint8_t* getData() const final { return _data; }
    size_t getSize() const final { return _size; }
    bool isMutable() const final { return false; }

private:
    ConstAllocator( const ConstAllocator& ) = delete;
    ConstAllocator& operator = ( const ConstAllocator& ) = delete;

    uint8_t* getData() final
        { throw std::runtime_error( "No mutable getData() for ConstAllocator" ); }
    void copyBuffer( const void*, size_t ) final
        { throw std::runtime_error( "No copyBuffer for ConstAllocator" ); }
    uint8_t* updateAllocation( size_t, bool, size_t ) final
        { throw std::runtime_error( "No updateAllocation for ConstAllocator" ); }
    void compact( float ) final
        { throw std::runtime_error( "No compact for ConstAllocator" ); }

    const uint8_t* _data;
    const size_t _size;
};
}

#endif
