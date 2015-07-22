
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_BASEVECTOR_H
#define ZEROBUF_BASEVECTOR_H

#include <zerobuf/types.h>

namespace zerobuf
{
/*
 * Base class for const and non-const vectors.
 *
 * @param A allocator type (needed for constness)
 * @param T element type
 * @param I Index of the vector in the parent allocator dynamic storage
 */
template< class A, class T, size_t I >
class BaseVector
{
public:
    BaseVector( A* alloc );
    virtual ~BaseVector() {}

    const T& operator[] ( size_t index ) const;

    bool empty() const { return _getSize() == 0; }
    uint64_t size() const { return _getSize() / sizeof(T); }
    const T* data() const { return _parent->template getDynamic< T >( I ); }

    bool operator == ( const BaseVector& rhs ) const;
    bool operator != ( const BaseVector& rhs ) const;

protected:
    A* _parent;

    BaseVector();
    virtual size_t _getSize() const { return _parent->getDynamicSize( I ); }
};

// Implementation
template< class A, class T, size_t I > inline
BaseVector< A, T, I >::BaseVector( A* alloc )
    : _parent( alloc )
{}

template< class A, class T, size_t I > inline
const T& BaseVector< A, T, I >::operator[] ( const size_t index ) const
{
    if( index >= size( ))
        throw std::runtime_error( "Vector out of bounds read" );

    const T* data = _parent->template getDynamic< const T >( I );
    return data[ index ];
}

template< class A, class T, size_t I > inline
bool BaseVector< A, T, I >::operator == ( const BaseVector& rhs ) const
{
    const size_t size = _getSize();
    if( size == 0 || size != rhs._getSize( ))
        return false;
    return ::memcmp( data(), rhs.data(), size ) == 0;
}

template< class A, class T, size_t I > inline
bool BaseVector< A, T, I >::operator != ( const BaseVector& rhs ) const
{
    return !(operator == ( rhs ));
}

template< class A, class T, size_t I > inline
std::ostream& operator << ( std::ostream& os,
                            const BaseVector< A, T, I >& vector )
{
    return os << typeid( vector ).name() << " of size " << vector.size();
}

}

#endif
