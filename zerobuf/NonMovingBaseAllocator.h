
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
    ZEROBUF_API
    explicit NonMovingBaseAllocator( const NonMovingBaseAllocator& from );
    ZEROBUF_API virtual ~NonMovingBaseAllocator();

    ZEROBUF_API
    NonMovingBaseAllocator& operator = ( const NonMovingBaseAllocator& rhs );

    ZEROBUF_API uint8_t* updateAllocation( size_t index, size_t size ) override;

protected:
    virtual void _resize( size_t newSize ) = 0;
    ZEROBUF_API size_t getDynamic() const { return _numDynamic; }

private:
    size_t _staticSize;
    size_t _numDynamic;
};
}
#endif
