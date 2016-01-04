
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_VECTOR_H
#define ZEROBUF_VECTOR_H

#include <zerobuf/DynamicSubAllocator.h> // used inline
#include <zerobuf/Schema.h> // used inline
#include <zerobuf/Zerobuf.h> // sfinae type

#include <cstring> // memcmp
#include <stdexcept> // std::runtime_error
#include <typeinfo> // typeid

namespace zerobuf
{
/**
 * STL-like vector abstraction for dynamic arrays in a zerobuf.
 *
 * @param T element type
 */
template< class T >
class Vector
{
public:
    /** @internal
     * @param alloc The parent allocator that contains the data.
     * @param index Index of the vector in the parent allocator dynamic storage
     */
    Vector( Allocator& alloc, size_t index );
    ~Vector() {}

    /** @return true if the two vectors are identical. */
    bool operator == ( const Vector& rhs ) const;
    /** @return false if the two vectors are identical. */
    bool operator != ( const Vector& rhs ) const;

    /** @return true if the vector contains no elements. */
    bool empty() const { return _getSize() == 0; }

    /** @return the number of elements in the vector. */
    uint64_t size() const { return _getSize() / _getElementSize< T >(); }

    /** Empty the vector. */
    void clear()
    {
        _alloc->updateAllocation( _index, false, 0 );
        _zerobufs.clear();
    }

    /** @return The pointer to the current allocation of the vector */
    T* data() { return _alloc->template getDynamic< T >( _index ); }

    /** @return The pointer to the current allocation of the vector */
    const T* data() const { return _alloc->template getDynamic< T >( _index ); }

    /** @return a builtin const element */
    template< class Q = T >
    const typename std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type&
    operator[] ( const size_t index ) const
    {
        if( index >= size( ))
            throw std::runtime_error( "Vector out of bounds read" );

        return data()[ index ];
    }

    /** @return a builtin element */
    template< class Q = T >
    typename std::enable_if< !std::is_base_of< Zerobuf, Q >::value, Q >::type&
    operator[] ( const size_t index )
    {
        if( index >= size( ))
            throw std::runtime_error( "Vector out of bounds read" );

        return data()[ index ];
    }

    /** @return a Zerobuf-derived const element */
    template< class Q = T >
    const typename std::enable_if< std::is_base_of<Zerobuf,Q>::value, Q >::type&
    operator[] ( const size_t index ) const
    {
        if( index >= size( ))
            throw std::runtime_error( "Vector out of bounds read" );

        while( _zerobufs.size() < index + 1 )
            _zerobufs.emplace_back( AllocatorPtr(
                new ConstDynamicSubAllocator( *_alloc, _index, _zerobufs.size(),
                                              _getElementSize< T >( ))));
        return _zerobufs[ index ];
    }

    /** @return a Zerobuf-derived element */
    template< class Q = T >
    typename std::enable_if< std::is_base_of< Zerobuf, Q >::value, Q >::type&
    operator[] ( const size_t index )
    {
        if( index >= size( ))
            throw std::runtime_error( "Vector out of bounds read" );

        while( _zerobufs.size() < index + 1 )
            _zerobufs.emplace_back( AllocatorPtr(
                new DynamicSubAllocator( *_alloc, _index, _zerobufs.size(),
                                         _getElementSize< T >( ))));
        return _zerobufs[ index ];
    }

    /** Insert a builtin element at the end of the vector. */
    template< class Q = T >
    void push_back( const typename
                    std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type&
                    value )
    {
        const size_t size_ = _getSize();
        T* newPtr = reinterpret_cast< T* >(
            _alloc->updateAllocation( _index, true /*copy*/,
                                      size_ + sizeof( T )));
        newPtr[ size_ / _getElementSize< T >() ] = value;
    }

    /** Insert a Zerobuf-derived element at the end of the vector. */
    template< class Q = T >
    void push_back( const typename
                    std::enable_if<std::is_base_of<Zerobuf,Q>::value, Q>::type&
                    value )
    {
        const size_t size_ =  _getSize();
        uint8_t* newPtr = _alloc->updateAllocation( _index, true /*copy*/,
                                               size_ + value.getZerobufSize( ));
        ::memcpy( newPtr + size_, value.getZerobufData(),
                  value.getZerobufSize( ));
    }

    /** @internal */
    void reset( Allocator& alloc ) { _alloc = &alloc; _zerobufs.clear(); }

private:
    Allocator* _alloc;
    const size_t _index;
    mutable std::vector< T > _zerobufs;

    Vector() = delete;
    Vector( const Vector& rhs ) = delete;
    Vector( const Vector&& rhs ) = delete;

    size_t _getSize() const { return _alloc->getDynamicSize( _index ); }
    void _resize( const size_t size_ )
       { _alloc->updateAllocation( _index, true /*copy*/, size_ ); }
    void copyBuffer( uint8_t* data, size_t size );

    template< class Q = T > size_t _getElementSize(
        typename std::enable_if< std::is_base_of< Zerobuf, Q >::value,
                                                  Q >::type* = 0 ) const
    {
        return Q::schema().staticSize;
    }

    template< class Q = T > size_t _getElementSize(
        typename std::enable_if< !std::is_base_of< Zerobuf, Q >::value,
                                                   Q >::type* = 0 ) const
    {
        return sizeof( Q );
    }
};

// Implementation
template< class T > inline
Vector< T >::Vector( Allocator& alloc, const size_t index )
    : _alloc( &alloc )
    , _index( index )
{}

template< class T > inline
bool Vector< T >::operator == ( const Vector& rhs ) const
{
    if( this == &rhs )
        return true;
    const size_t size_ = _getSize();
    if( size_ != rhs._getSize( ))
        return false;
    if( size_ == 0 )
        return true;
    return ::memcmp( data(), rhs.data(), size_ ) == 0;
}

template< class T > inline
bool Vector< T >::operator != ( const Vector& rhs ) const
{
    return !(operator == ( rhs ));
}

template< class T > inline
void Vector< T >::copyBuffer( uint8_t* data_, size_t size_ )
{
    void* to = _alloc->updateAllocation( _index, false /*no copy*/, size_ );
    ::memcpy( to, data_, size_ );
}

template< class T > inline
std::ostream& operator << ( std::ostream& os, const Vector< T >& vector )
{
    return os << typeid( vector ).name() << " of size " << vector.size();
}

}

#endif
