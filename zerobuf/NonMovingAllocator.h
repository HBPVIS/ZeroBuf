
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_NONMOVINGALLOCATOR_H
#define ZEROBUF_NONMOVINGALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/NonMovingBaseAllocator.h> // base class

namespace zerobuf
{
/** A zerobuf root allocator which does not move existing fields */
class NonMovingAllocator : public NonMovingBaseAllocator
{
public:
    ZEROBUF_API NonMovingAllocator( size_t staticSize, size_t numDynamic );
    ZEROBUF_API NonMovingAllocator( const NonMovingAllocator& from );
    ZEROBUF_API ~NonMovingAllocator();

    ZEROBUF_API NonMovingAllocator& operator = ( const NonMovingAllocator& rhs );

    uint8_t* getData() final { return _data; }
    const uint8_t* getData() const final { return _data; }
    size_t getSize() const final { return _size; }
    ZEROBUF_API void copyBuffer( const void* data, size_t size ) final;

private:
    uint8_t* _data;
    size_t _size;

    void _resize( size_t newSize ) final;
};
}
#endif
