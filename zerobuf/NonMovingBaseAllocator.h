
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_NONMOVINGBASEALLOCATOR_H
#define ZEROBUF_NONMOVINGBASEALLOCATOR_H

#include <zerobuf/Allocator.h>

namespace zerobuf
{
/** Allocator base class which does not move existing fields */
class NonMovingBaseAllocator : public Allocator
{
public:
    ZEQ_API NonMovingBaseAllocator( size_t staticSize, size_t numDynamic );
    ZEQ_API NonMovingBaseAllocator( const NonMovingBaseAllocator& from );
    ZEQ_API virtual ~NonMovingBaseAllocator();

    ZEQ_API NonMovingBaseAllocator& operator = (
        const NonMovingBaseAllocator& rhs );

    ZEQ_API virtual uint8_t* updateAllocation( size_t index, size_t size );

protected:
    virtual void _resize( size_t newSize ) = 0;

private:
    size_t _staticSize;
    size_t _numDynamic;
};
}
#endif
