
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_STATICSUBALLOCATOR_H
#define ZEROBUF_STATICSUBALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/Allocator.h> // base class

namespace zerobuf
{
/** A zerobuf child allocator which manages a static sub-struct. */
template< class A > class StaticSubAllocatorBase : public Allocator
{
public:
    ZEROBUF_API StaticSubAllocatorBase( A& parent, size_t offset,
                                        size_t size );
    ZEROBUF_API ~StaticSubAllocatorBase();

    ZEROBUF_API uint8_t* getData() final;
    ZEROBUF_API const uint8_t* getData() const final;
    size_t getSize() const  final { return _size; }
    ZEROBUF_API void copyBuffer( const void* data, size_t size ) final;

private:
    A& _parent;
    const size_t _offset;
    const size_t _size;

    StaticSubAllocatorBase( const StaticSubAllocatorBase< A >& ) = delete;
    StaticSubAllocatorBase< A >& operator = (
            const StaticSubAllocatorBase< A >& ) = delete;
};

typedef StaticSubAllocatorBase< Allocator > StaticSubAllocator;
typedef StaticSubAllocatorBase< const Allocator > ConstStaticSubAllocator;

}

#endif
