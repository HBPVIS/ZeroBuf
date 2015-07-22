
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_VECTOR_H
#define ZEROBUF_VECTOR_H

#include <zerobuf/BaseVector.h>

namespace zerobuf
{
/**
 * Non-const vector
 *
 * @param T element type
 * @param I Index of the vector in the parent allocator dynamic storage
 */
template< class T, size_t I >
class Vector : public BaseVector< Allocator, T, I >
{
    typedef BaseVector< Allocator, T, I > Super;

public:
    Vector( Allocator* alloc );
    virtual ~Vector() {}

    void push_back( const T& value );
    T* data() { return Super::_parent->template getDynamic< T >( I ); }

private:
    Vector();
    virtual void _resize( const size_t size )
        { Super::_parent->updateAllocation( I, size ); }
    virtual void copyBuffer( uint8_t* data, size_t size );
};

// Implementation
template< class T, size_t I > inline
Vector< T, I >::Vector( Allocator* alloc )
    : BaseVector< Allocator, T, I >( alloc )
{}

template< class T, size_t I > inline
void Vector< T, I >::push_back( const T& value )
{
    const size_t size = Super::_getSize();
    const T* oldPtr = data();
    T* newPtr = reinterpret_cast< T* >(
        Super::_parent->updateAllocation( I, size + sizeof( T )));
    if( oldPtr != newPtr )
        ::memcpy( newPtr, oldPtr, size );

    newPtr[ size / sizeof(T) ] = value;
}

template< class T, size_t I > inline
void Vector< T, I >::copyBuffer( uint8_t* data, size_t size )
{
    void* to = Super::_parent->updateAllocation( I, size );
    ::memcpy( to, data, size );
}

}

#endif
