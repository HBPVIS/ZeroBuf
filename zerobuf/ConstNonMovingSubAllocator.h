
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_CONSTNONMOVINGSUBALLOCATOR_H
#define ZEROBUF_CONSTNONMOVINGSUBALLOCATOR_H

#include <zerobuf/api.h>
#include <zerobuf/NonMovingBaseAllocator.h> // base class
#include <stdexcept> // std::runtime_error

namespace zerobuf
{
/** A zerobuf child allocator which does not move existing fields */
class ConstNonMovingSubAllocator : public NonMovingBaseAllocator
{
public:
    ZEROBUF_API ConstNonMovingSubAllocator( const Allocator* parent,
                                            size_t offset,
                                            size_t numDynamic,
                                            size_t staticSize );
    ZEROBUF_API ConstNonMovingSubAllocator( const ConstNonMovingSubAllocator& from );
    ZEROBUF_API ConstNonMovingSubAllocator( const NonMovingSubAllocator& from );
    ZEROBUF_API ~ConstNonMovingSubAllocator();

    ZEROBUF_API
    ConstNonMovingSubAllocator& operator = ( const ConstNonMovingSubAllocator& );
    ZEROBUF_API
    ConstNonMovingSubAllocator& operator = ( const NonMovingSubAllocator& );
    ZEROBUF_API const Allocator* getParent() const { return _parent; }
    ZEROBUF_API size_t getOffset() const { return _offset; }
    ZEROBUF_API size_t getIndex() const { return _index; }

    ZEROBUF_API const uint8_t* getData() const final;
    size_t getSize() const  final { return _size; }

    ZEROBUF_API virtual Allocator* clone() const final;

private:
    const Allocator* _parent;
    size_t _offset;
    size_t _index;
    size_t _size;

    uint8_t* getData() final
        { throw std::runtime_error( "Cannot get non const pointer to data" ); }
    void _resize( size_t ) final
        { throw std::runtime_error( "Cannot resize const vector" ); }
    void copyBuffer( const void*, size_t ) final
        { throw std::runtime_error( "Cannot copy into const vector" ); }

};
}
#endif
