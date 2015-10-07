
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
class NonMovingSubAllocator : public NonMovingBaseAllocator
{
public:
    ZEROBUF_API NonMovingSubAllocator( NonMovingBaseAllocator* parent,
                                       size_t offset,
                                       size_t numDynamic,
                                       size_t staticSize );
    ZEROBUF_API NonMovingSubAllocator( const NonMovingSubAllocator& from );
    ZEROBUF_API ~NonMovingSubAllocator();

    ZEROBUF_API
    NonMovingSubAllocator& operator = ( const NonMovingSubAllocator& );
    ZEROBUF_API const NonMovingBaseAllocator* getParent() const { return _parent; }
    ZEROBUF_API size_t getOffset() const { return _offset; }
    ZEROBUF_API size_t getIndex() const { return _index; }

    ZEROBUF_API uint8_t* getData() final;
    ZEROBUF_API const uint8_t* getData() const final;
    size_t getSize() const  final { return _size; }
    ZEROBUF_API void copyBuffer( const void* data, size_t size ) final;

    ZEROBUF_API virtual Allocator* clone() const final;

private:

    NonMovingBaseAllocator* _parent;
    size_t _offset;
    size_t _index;
    size_t _size;

    void _resize( size_t newSize ) final;
};
}
#endif
