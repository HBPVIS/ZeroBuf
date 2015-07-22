
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_NONMOVINGSUBALLOCATOR_H
#define ZEROBUF_NONMOVINGSUBALLOCATOR_H

#include <zerobuf/NonMovingBaseAllocator.h>

namespace zerobuf
{
/** A zerobuf child allocator which does not move existing fields */
class NonMovingSubAllocator : public NonMovingBaseAllocator
{
public:
    ZEQ_API NonMovingSubAllocator( NonMovingBaseAllocator* parent,
                                   size_t index, size_t numDynamic,
                                   size_t staticSize );
    ZEQ_API NonMovingSubAllocator( const NonMovingSubAllocator& from );
    ZEQ_API virtual ~NonMovingSubAllocator();

    ZEQ_API NonMovingSubAllocator& operator = ( const NonMovingSubAllocator& );

    ZEQ_API virtual uint8_t* getData();
    ZEQ_API virtual const uint8_t* getData() const;
    virtual size_t getSize() const { return _size; }
    ZEQ_API virtual void copyBuffer( const void* data, size_t size );

private:
    NonMovingBaseAllocator* _parent;
    size_t _index;
    size_t _size;

    virtual void _resize( size_t newSize );
};
}
#endif
