
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_NONMOVINGSUBALLOCATOR_H
#define ZEROBUF_NONMOVINGSUBALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/NonMovingBaseAllocator.h> // base class

namespace zerobuf
{
/** A zerobuf child allocator which does not move existing fields */
template< class A >
class NonMovingSubAllocatorBase : public NonMovingBaseAllocator
{
public:
    ZEROBUF_API NonMovingSubAllocatorBase( A& parent, size_t index,
                                           size_t numDynamic,
                                           size_t staticSize );
    ZEROBUF_API ~NonMovingSubAllocatorBase();


    ZEROBUF_API uint8_t* getData() final;
    ZEROBUF_API const uint8_t* getData() const final;
    ZEROBUF_API size_t getSize() const final;
    ZEROBUF_API void copyBuffer( const void* data, size_t size ) final;

private:
    A& _parent;
    const size_t _index;

    NonMovingSubAllocatorBase( const NonMovingSubAllocatorBase< A >& ) = delete;
    NonMovingSubAllocatorBase< A >& operator = (
        const NonMovingSubAllocatorBase< A >& ) = delete;
    void _resize( size_t newSize ) final;
};

typedef NonMovingSubAllocatorBase< Allocator > NonMovingSubAllocator;
typedef NonMovingSubAllocatorBase< const Allocator > ConstNonMovingSubAllocator;

}
#endif
