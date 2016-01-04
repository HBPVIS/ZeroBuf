
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_DYNAMICSUBALLOCATOR_H
#define ZEROBUF_DYNAMICSUBALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/Allocator.h> // base class

namespace zerobuf
{
/**
  * A zerobuf child allocator which manages a dynamic allocation of
  * statically-sized members
  */
template< class A > class DynamicSubAllocatorBase : public Allocator
{
public:
    ZEROBUF_API DynamicSubAllocatorBase( A& parent, size_t headerIndex,
                                         size_t elementIndex, size_t size );
    ZEROBUF_API ~DynamicSubAllocatorBase();

    ZEROBUF_API uint8_t* getData() final;
    ZEROBUF_API const uint8_t* getData() const final;
    size_t getSize() const  final { return _size; }
    ZEROBUF_API void copyBuffer( const void* data, size_t size ) final;

private:
    A& _parent;
    const size_t _header;
    const size_t _element;
    const size_t _size;

    DynamicSubAllocatorBase( const DynamicSubAllocatorBase< A >& ) = delete;
    DynamicSubAllocatorBase< A >& operator = (
            const DynamicSubAllocatorBase< A >& ) = delete;
};

typedef DynamicSubAllocatorBase< Allocator > DynamicSubAllocator;
typedef DynamicSubAllocatorBase< const Allocator > ConstDynamicSubAllocator;

}

#endif
