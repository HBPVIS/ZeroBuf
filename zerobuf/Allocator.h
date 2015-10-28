
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 *                     grigori.chevtchenko@epfl.ch
 */

#ifndef ZEROBUF_ALLOCATOR_H
#define ZEROBUF_ALLOCATOR_H

#include <zerobuf/Types.h>

namespace zerobuf
{
/**
 * Allocator base class and interface
 *
 * Implements part of the binary data layout by providing access to the stored
 * elements.
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

    /** Update allocation of the dynamic elem @arg index to have newSize bytes.
     * Updates the offset and size fields in the static section as needed.
     * Does not copy the old data to the new location.
     * @return the pointer to the elem at the new place.
     */
    virtual uint8_t* updateAllocation( size_t index,  size_t newSize ) = 0;

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

    virtual Allocator* clone() const = 0;

protected:
    uint64_t& _getOffset( const size_t i )
        { return getItem< uint64_t >( 4 + i * 16 ); }
    uint64_t _getOffset( const size_t i ) const
        { return getItem< uint64_t >( 4 + i * 16 ); }
    uint64_t& _getSize( const size_t i )
        { return getItem< uint64_t >( 4 + 8 + i * 16 ); }
    uint64_t _getSize( const size_t i ) const
        { return getItem< uint64_t >( 4 + 8 + i * 16 ); }
};
}

#endif
