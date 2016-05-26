
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 *                          grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_ALLOCATOR_H
#define ZEROBUF_ALLOCATOR_H

#include <zerobuf/types.h>

namespace zerobuf
{
/**
 * Allocator base class and interface.
 *
 * Implements part of the binary data layout by providing access to the stored
 * elements. Not intended to be used by application code.
 */
class Allocator
{
public:
    Allocator() {}
    virtual ~Allocator() {}

    virtual uint8_t* getData() = 0;
    virtual const uint8_t* getData() const = 0;
    virtual size_t getSize() const = 0;
    virtual void copyBuffer( const void* data, size_t size ) = 0;
    virtual void compact( float /*threshold*/ )
        { throw std::runtime_error( "Compaction not implemented" ); }
    virtual bool isMovable() const { return false; } // allocation is moveable
    virtual bool isMutable() const { return true; } // data is mutable

    /**
     * Update allocation of the dynamic elem at index to have newSize bytes.
     *
     * The offset and size fields in the static section are updated as needed.
     * Does not copy the old data to the new location.
     *
     * @return the pointer to the elem at the new place.
     */
    virtual uint8_t* updateAllocation( size_t /*index*/, bool /*copy*/,
                                       size_t /*newSize*/ )
        { throw std::runtime_error( "Dynamic allocation not implemented" ); }


    template< class T > T* getItemPtr( const size_t offset )
        { return reinterpret_cast< T* >( getData() + offset ); }

    template< class T > const T* getItemPtr( const size_t offset ) const
        { return reinterpret_cast< const T* >( getData() + offset ); }

    template< class T > T& getItem( const size_t offset )
        { return *getItemPtr< T >( offset ); }

    template< class T > T getItem( const size_t offset ) const
        { return *getItemPtr< T >( offset ); }

    template< class T > T* getDynamic( const size_t index )
        { return reinterpret_cast< T* >( getData() + _getOffset( index )); }

    template< class T > const T* getDynamic( const size_t index ) const
        { return reinterpret_cast< const T* >( getData() + _getOffset(index)); }

    uint64_t getDynamicOffset( const size_t index ) const
        { return _getOffset( index ); }

    size_t getDynamicSize( const size_t index ) const
        { return _getSize( index ); }

    void check( const size_t numDynamics ) const
    {
        for( size_t i = 0; i < numDynamics; ++i )
            _checkIndex( i );
    }

protected:
    uint64_t& _getOffset( const size_t i )
        { _checkIndex( i ); return getItem< uint64_t >( 4 + i * 16 ); }
    uint64_t _getOffset( const size_t i ) const
        { _checkIndex( i ); return getItem< uint64_t >( 4 + i * 16 ); }
    uint64_t& _getSize( const size_t i )
        { _checkIndex( i ); return getItem< uint64_t >( 4 + 8 + i * 16 ); }
    uint64_t _getSize( const size_t i ) const
        { _checkIndex( i ); return getItem< uint64_t >( 4 + 8 + i * 16 ); }

    void _checkIndex( const size_t i ) const
    {
        const uint64_t offset = getItem< uint64_t >( 4 + i * 16 );
        const uint64_t size = getItem< uint64_t >( 4 + 8 + i * 16 );
        if( offset + size > getSize( ))
            throw std::runtime_error(
                "Internal allocator error: dynamic #" + std::to_string( i ) +
                " at " + std::to_string( offset ) + " size " +
                std::to_string( size ) + " exceeds allocation size " +
                std::to_string( getSize( )));
        if( offset != 0 && offset < 4 + (i+1) * 16 )
            throw std::runtime_error(
                "Internal allocator error: dynamic #" + std::to_string( i ) +
                " at " + std::to_string(offset) + " is within static section" );
    }
};
}

#endif
