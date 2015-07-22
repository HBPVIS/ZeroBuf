
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_NONMOVINGALLOCATOR_H
#define ZEROBUF_NONMOVINGALLOCATOR_H

#include <zerobuf/NonMovingBaseAllocator.h>

namespace zerobuf
{
/** A zerobuf root allocator which does not move existing fields */
class NonMovingAllocator : public NonMovingBaseAllocator
{
public:
    ZEQ_API NonMovingAllocator( size_t staticSize, size_t numDynamic );
    ZEQ_API NonMovingAllocator( const NonMovingAllocator& from );
    ZEQ_API virtual ~NonMovingAllocator();

    ZEQ_API NonMovingAllocator& operator = ( const NonMovingAllocator& rhs );

    virtual uint8_t* getData() { return _data; }
    virtual const uint8_t* getData() const { return _data; }
    virtual size_t getSize() const { return _size; }
    ZEQ_API virtual void copyBuffer( const void* data, size_t size );

private:
    uint8_t* _data;
    size_t _size;

    virtual void _resize( size_t newSize );
};
}
#endif
