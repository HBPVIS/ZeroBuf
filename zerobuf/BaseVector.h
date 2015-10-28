
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_BASEVECTOR_H
#define ZEROBUF_BASEVECTOR_H

#include <zerobuf/Types.h>
#include <zerobuf/NonMovingAllocator.h>
#include <zerobuf/Zerobuf.h>

#include <cstring> // memcmp
#include <stdexcept> // std::runtime_error
#include <typeinfo> // typeid
#include <ostream>

namespace zerobuf
{
/*
 * Base class for const and non-const vectors.
 *
 * @param A allocator type (needed for constness)
 * @param T element type
 */
template< class A, class T >
class BaseVector
{
public:
    /**
     * @param alloc The parent allocator that contains the data.
     * @param index Index of the vector in the parent allocator dynamic storage
     */
    BaseVector( A* alloc, size_t index, size_t elemSize = sizeof( T ));
    virtual ~BaseVector() {}

    template<class Q = T>
    const typename std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type&
    operator[] ( const size_t index ) const
    {
        if( index >= size( ))
            throw std::runtime_error( "Vector out of bounds read" );

        return data()[ index ];
    }

    template<class Q = T>
    typename std::enable_if<std::is_base_of<Zerobuf,Q>::value, Q>::type
    operator[] ( const size_t index ) const
    {
        if( index >= size( ))
            throw std::runtime_error( "Vector out of bounds read" );

        // TODO OPT: Create a COW allocator, which uses the existing memory for
        // gets until the first set is done, upon which it replaces itself with
        // a copy to a NonMovingAllocator as below:
        const uint8_t* base = _parent->template getDynamic< uint8_t >( _index );
        NonMovingAllocator* alloc = new NonMovingAllocator( _elemSize, 0 );

        alloc->copyBuffer( base + index * _elemSize, _elemSize );
        return Q( alloc );
    }

    bool empty() const { return _getSize() == 0; }
    uint64_t size() const { return _getSize() / _elemSize; }

    template<class Q = T>
    const typename std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type* data() const
        { return _parent->template getDynamic< const T >( _index ); }

    template<class Q = T>
    const typename std::enable_if<std::is_base_of<Zerobuf,Q>::value, Q>::type* data() const
        { throw std::runtime_error( "No raw array pointer for Zerobuf elements"); }

    bool operator == ( const BaseVector& rhs ) const;
    bool operator != ( const BaseVector& rhs ) const;

protected:
    A* _parent;
    const size_t _index;
    const size_t _elemSize;

    BaseVector();
    size_t _getSize() const { return _parent->getDynamicSize( _index ); }
};

// Implementation
template< class A, class T > inline
BaseVector< A, T >::BaseVector( A* alloc, const size_t index,
                                const size_t elemSize )
    : _parent( alloc )
    , _index( index )
    , _elemSize( elemSize )
{}


template< class A, class T > inline
bool BaseVector< A, T >::operator == ( const BaseVector& rhs ) const
{
    if( this == &rhs )
        return true;
    const size_t size_ = _getSize();
    if( size_ == 0 || size_ != rhs._getSize( ))
        return false;
    return ::memcmp( data(), rhs.data(), size_ ) == 0;
}

template< class A, class T > inline
bool BaseVector< A, T >::operator != ( const BaseVector& rhs ) const
{
    return !(operator == ( rhs ));
}

template< class A, class T > inline
std::ostream& operator << ( std::ostream& os,
                            const BaseVector< A, T >& vector )
{
    return os << typeid( vector ).name() << " of size " << vector.size();
}

}

#endif
