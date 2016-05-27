
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_VECTOR_H
#define ZEROBUF_VECTOR_H

#include <zerobuf/DynamicSubAllocator.h> // used inline
#include <zerobuf/Zerobuf.h> // sfinae type
#include <zerobuf/json.h> // used inline

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

    /** @return true if the vector contains no elements. */
    bool empty() const { return _getSize() == 0; }

    /** @return the number of elements in the vector. */
    uint64_t size() const { return _getSize() / _getElementSize< T >(); }

    /** Empty the vector. */
    void clear();

    /** @return The pointer to the current allocation of the vector */
    T* data() { return _alloc->template getDynamic< T >( _index ); }

    /** @return The pointer to the current allocation of the vector */
    const T* data() const
        { return const_cast< const Allocator* >( _alloc )->template getDynamic< T >( _index ); }

    /** @return true if the two vectors of builtins are identical. */
    bool operator == ( const Vector& rhs ) const;
    /** @return false if the two vectors are identical. */
    bool operator != ( const Vector& rhs ) const;

    /** @return a builtin const element */
    template< class Q = T >
    const typename std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type&
    operator[] ( const size_t index ) const;

    /** @return a builtin element */
    template< class Q = T >
    typename std::enable_if< !std::is_base_of< Zerobuf, Q >::value, Q >::type&
    operator[] ( const size_t index );

    /** @return a Zerobuf-derived const element */
    template< class Q = T >
    const typename std::enable_if< std::is_base_of<Zerobuf,Q>::value, Q >::type&
    operator[] ( const size_t index ) const;

    /** @return a Zerobuf-derived element */
    template< class Q = T >
    typename std::enable_if< std::is_base_of< Zerobuf, Q >::value, Q >::type&
    operator[] ( const size_t index );

    /** Insert a builtin element at the end of the vector. */
    template< class Q = T > void
    push_back( const typename std::enable_if<
                                !std::is_base_of<Zerobuf,Q>::value, Q>::type& );

    /** Insert a Zerobuf-derived element at the end of the vector. */
    template< class Q = T > void
    push_back( const typename std::enable_if<
                                  std::is_base_of<Zerobuf,Q>::value, Q>::type&);

    /** @internal */
    void reset( Allocator& alloc ) { _alloc = &alloc; _zerobufs.clear(); }

    /** Remove unused memory from vector and all members. */
    void compact( float ) { /* NOP: elements are static and clear frees */ }

    /** Update this vector from its JSON representation. */
    template< class Q = T > void
    fromJSON( const Json::Value& json, const typename std::enable_if<
                  std::is_base_of< Zerobuf, Q >::value, Q >::type* = nullptr );

    /** Update this vector from its JSON representation. */
    template< class Q = T > void
    fromJSON( const Json::Value& json, const typename std::enable_if<
                  !std::is_base_of< Zerobuf, Q >::value, Q >::type* = nullptr );

    /** @return the JSON representation of this vector. */
    template< class Q = T > void
    toJSON( Json::Value& json, const typename std::enable_if<
             std::is_base_of< Zerobuf, Q >::value, Q >::type* = nullptr ) const;

    /** @return the JSON representation of this vector. */
    template< class Q = T > void
    toJSON( Json::Value& json, const typename std::enable_if<
            !std::is_base_of< Zerobuf, Q >::value, Q >::type* = nullptr ) const;

    /** Update this vector from its JSON, base64-encoded representation. */
    template< class Q = T > void
    fromJSONBinary( const Json::Value& json, const typename std::enable_if<
                    std::is_pod< Q >::value, Q >::type* = nullptr );

    /** @return the JSON representation of this vector, with base64 encoding. */
    template< class Q = T > void
    toJSONBinary( Json::Value& json, const typename std::enable_if<
                  std::is_pod< Q >::value, Q >::type* = nullptr ) const;

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
        return Q::ZEROBUF_STATIC_SIZE();
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

template< class T > inline void Vector< T >::clear()
{
    _alloc->updateAllocation( _index, false, 0 );
    _zerobufs.clear();
}

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

    // memory compare is valid for Zerobufs as well since they are right now
    // static and therefore will have the same layout
    return ::memcmp( static_cast< const void* >( data( )),
                     static_cast< const void* >( rhs.data( )), size_ ) == 0;
}

template< class T > inline
bool Vector< T >::operator != ( const Vector& rhs ) const
{
    return !(operator == ( rhs ));
}

template< class T > template< class Q > inline const typename
std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type&
Vector< T >::operator[] ( const size_t index ) const
{
    if( index >= size( ))
        throw std::runtime_error( "Vector out of bounds read" );

    return data()[ index ];
}

template< class T > template< class Q > inline typename
std::enable_if< !std::is_base_of< Zerobuf, Q >::value, Q >::type&
Vector< T >::operator[] ( const size_t index )
{
    if( index >= size( ))
        throw std::runtime_error( "Vector out of bounds read" );

    return data()[ index ];
}

template< class T > template< class Q > inline const typename
std::enable_if< std::is_base_of<Zerobuf,Q>::value, Q >::type&
Vector< T >::operator[] ( const size_t index ) const
{
    if( index >= size( ))
        throw std::runtime_error( "Vector out of bounds read" );

    while( _zerobufs.size() < index + 1 )
        _zerobufs.emplace_back( AllocatorPtr(
            new ConstDynamicSubAllocator( *_alloc, _index, _zerobufs.size(),
                                          _getElementSize< T >( ))));
    return _zerobufs[ index ];
}

template< class T > template< class Q > inline typename
std::enable_if< std::is_base_of< Zerobuf, Q >::value, Q >::type&
Vector< T >::operator[] ( const size_t index )
{
    if( index >= size( ))
        throw std::runtime_error( "Vector out of bounds read" );

    while( _zerobufs.size() < index + 1 )
        _zerobufs.emplace_back( AllocatorPtr(
            new DynamicSubAllocator( *_alloc, _index, _zerobufs.size(),
                                     _getElementSize< T >( ))));
    return _zerobufs[ index ];
}

template< class T > template< class Q > inline void
Vector< T >::push_back(
    const typename std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type&
        value )
{
    const size_t size_ = _getSize();
    T* newPtr = reinterpret_cast< T* >(
        _alloc->updateAllocation( _index, true /*copy*/, size_ + sizeof( T )));
    newPtr[ size_ / _getElementSize< T >() ] = value;
}

template< class T > template< class Q > inline void
Vector<T>::push_back(
    const typename std::enable_if<std::is_base_of<Zerobuf,Q>::value, Q>::type&
        value )
{
    const size_t size_ = _getSize();
    const zerobuf::Data& zerobuf = value.toBinary();
    uint8_t* newPtr = _alloc->updateAllocation( _index, true /*copy*/,
                                               size_ + zerobuf.size );
    ::memcpy( newPtr + size_, zerobuf.ptr.get(), zerobuf.size );
}

template< class T > template< class Q > inline
void Vector< T >::fromJSON( const Json::Value& json,
    const typename std::enable_if<std::is_base_of<Zerobuf,Q>::value, Q>::type* )
{
    const size_t size_ = getJSONSize( json );
    _alloc->updateAllocation( _index, false, size_ * _getElementSize< T >( ));
    for( size_t i = 0; i < size_; ++i )
        zerobuf::fromJSON( getJSONField( json, i ), (*this)[i] );
}

template< class T > template< class Q > inline
void Vector< T >::fromJSON( const Json::Value& json,
    const typename std::enable_if<!std::is_base_of<Zerobuf,Q>::value, Q>::type*)
{
    const size_t size_ = getJSONSize( json );
    T* array = reinterpret_cast< T* >(
        _alloc->updateAllocation( _index, false /*no copy*/, size_*sizeof( T)));

    for( size_t i = 0; i < size_; ++i )
        array[i] = zerobuf::fromJSON< T >( getJSONField( json, i ));
}

template< class T > template< class Q > inline void
Vector< T >::toJSON( Json::Value& json, const typename std::enable_if<
                        std::is_base_of< Zerobuf, Q >::value, Q >::type* ) const
{
    const size_t size_ = size();
    for( size_t i = 0; i < size_; ++i )
        zerobuf::toJSON( static_cast< const Zerobuf& >(( *this )[ i ]),
                         getJSONField( json, i ));
}

template< class T > template< class Q > inline void
Vector< T >::toJSON( Json::Value& json, const typename std::enable_if<
                       !std::is_base_of< Zerobuf, Q >::value, Q >::type* ) const
{
    const size_t size_ = size();
    for( size_t i = 0; i < size_; ++i )
        zerobuf::toJSON( (*this)[i], getJSONField( json, i ));
}

template< class T > template< class Q > inline void
Vector< T >::fromJSONBinary( const Json::Value& json,
    const typename std::enable_if< std::is_pod< Q >::value, Q >::type* )
{
    const std::string& decoded = zerobuf::fromJSONBinary( json );
    copyBuffer( (uint8_t*)decoded.data(), decoded.length( ));
}

template< class T > template< class Q > inline void
Vector< T >::toJSONBinary( Json::Value& json,
    const typename std::enable_if< std::is_pod< Q >::value, Q >::type* ) const
{
    zerobuf::toJSONBinary( data(), _getSize(), json );
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

template<> inline
std::ostream& operator << ( std::ostream& os, const Vector< char >& string )
{
    return os << string.data();
}

}

#endif
