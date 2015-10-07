
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_VECTOR_H
#define ZEROBUF_VECTOR_H

#include <zerobuf/BaseVector.h> // base class

namespace zerobuf
{
/**
 * Non-const vector
 *
 * @param T element type
 */
template< class T >
class Vector : public BaseVector< Allocator, T >
{
    typedef BaseVector< Allocator, T > Super;

public:
    /**
     * @param alloc The parent allocator that contains the data.
     * @param index Index of the vector in the parent allocator dynamic storage
     */
    Vector( Allocator* alloc, size_t index, size_t staticSize = sizeof( T ));
    ~Vector() {}

    void push_back( const T& value );
    T* data()
        { return Super::_parent->template getDynamicPtr< T >( Super::_index ); }

private:
    Vector();
    void _resize( const size_t size )
        { Super::_parent->updateAllocation( Super::_index, size ); }
    void copyBuffer( uint8_t* data, size_t size );
};

// Implementation
template< class T > inline
Vector< T >::Vector( Allocator* alloc,
                     const size_t index,
                     const size_t staticSize )
    : BaseVector< Allocator, T >( alloc, index, staticSize )
{}

template< class T > inline
void Vector< T >::push_back( const T& value )
{
    const size_t size_ = Super::_getSize();
    const T* oldPtr = data();
    T* newPtr = reinterpret_cast< T* >(
        Super::_parent->updateAllocation( Super::_index, size_ + sizeof( T )));
    if( oldPtr != newPtr )
        ::memcpy( newPtr, oldPtr, size_ );

    newPtr[ size_ / Super::_elemSize ] = value;
}

template< class T > inline
void Vector< T >::copyBuffer( uint8_t* data_, size_t size )
{
    void* to = Super::_parent->updateAllocation( Super::_index, size );
    ::memcpy( to, data_, size );
}

template<>
void Vector<Zerobuf>::push_back( const Zerobuf& value );

}

#endif
