
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_NONMOVINGBASEALLOCATOR_H
#define ZEROBUF_NONMOVINGBASEALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/Allocator.h> // base class

namespace zerobuf
{
/** Allocator base class which does not move existing fields */
class NonMovingBaseAllocator : public Allocator
{
public:
    ZEROBUF_API NonMovingBaseAllocator( size_t staticSize, size_t numDynamic );

    ZEROBUF_API virtual ~NonMovingBaseAllocator();

    ZEROBUF_API uint8_t* updateAllocation( size_t index, bool copy,
                                           size_t size ) override;

protected:
    virtual void _resize( size_t newSize ) = 0;

private:
    NonMovingBaseAllocator( const NonMovingBaseAllocator& ) = delete;
    NonMovingBaseAllocator& operator = (
        const NonMovingBaseAllocator& ) = delete;

    size_t _staticSize;
    size_t _numDynamic;

    uint8_t* _moveAllocation( size_t index, bool copy, size_t newOffset,
                              size_t newSize );
};
}
#endif
