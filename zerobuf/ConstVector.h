
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_CONSTVECTOR_H
#define ZEROBUF_CONSTVECTOR_H

#include <zerobuf/BaseVector.h> // base class

namespace zerobuf
{

/**
 * Const vector
 *
 * @param T element type
 */
template< class T >
class ConstVector : public BaseVector< const Allocator, T >
{
    typedef BaseVector< const Allocator, T > Super;

public:
    /**
     * @param alloc The parent allocator that contains the data.
     * @param index Index of the vector in the parent allocator dynamic storage
     */
    ConstVector( const Allocator* alloc, size_t index,
                 const size_t staticSize = sizeof( T ));
    ~ConstVector() {}

private:
    ConstVector();
    void _resize( size_t )
        { throw std::runtime_error( "Cannot resize const vector" ); }
    void copyBuffer( uint8_t*, size_t )
        { throw std::runtime_error( "Cannot copy into const vector" ); }
};

// Implementation
template< class T > inline
ConstVector< T >::ConstVector( const Allocator* alloc, const size_t index,
                               const size_t staticSize )
    : BaseVector< const Allocator, T >( alloc, index, staticSize )
{}

}

#endif
