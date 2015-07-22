
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_CONSTVECTOR_H
#define ZEROBUF_CONSTVECTOR_H

#include <zerobuf/BaseVector.h>

namespace zerobuf
{

/**
 * Const vector
 *
 * @param T element type
 * @param I Index of the vector in the parent allocator dynamic storage
 */
template< class T, size_t I >
class ConstVector : public BaseVector< const Allocator, T, I >
{
    typedef BaseVector< const Allocator, T, I > Super;

public:
    ConstVector( const Allocator* alloc );
    virtual ~ConstVector() {}

private:
    ConstVector();
    virtual void _resize( size_t )
        { throw std::runtime_error( "Cannot resize const vector" ); }
    virtual void copyBuffer( uint8_t*, size_t )
        { throw std::runtime_error( "Cannot copy into const vector" ); }
};

// Implementation
template< class T, size_t I > inline
ConstVector< T, I >::ConstVector( const Allocator* alloc )
    : BaseVector< const Allocator, T, I >( alloc )
{}

}

#endif
