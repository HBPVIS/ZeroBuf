
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#include "Zerobuf.h"

#include "NonMovingSubAllocator.h"
#include "Schema.h"
#include "StaticSubAllocator.h"
#include "detail/JsonConverter.h"
#include <zerobuf/version.h>

#include <iostream>

namespace zerobuf
{

Zerobuf::Zerobuf( AllocatorPtr alloc )
    : _allocator( std::move( alloc ))
{
    if( _allocator &&
        ((const Allocator*)_allocator.get( ))->getItem< uint32_t >( 0 ) !=
        ZEROBUF_VERSION_ABI )
    {
        _allocator->getItem< uint32_t >( 0 ) = ZEROBUF_VERSION_ABI;
    }
}

Zerobuf::Zerobuf( Zerobuf&& rhs )
{
    rhs.notifyChanging();
    _allocator = std::move( rhs._allocator );
    rhs._allocator.reset( new NonMovingAllocator( rhs.getZerobufStaticSize(),
                                                 rhs.getZerobufNumDynamics( )));
    uint32_t& version = rhs._allocator->getItem< uint32_t >( 0 );
    version = ZEROBUF_VERSION_ABI;
}

Zerobuf::~Zerobuf()
{}

Zerobuf& Zerobuf::operator = ( const Zerobuf& rhs )
{
    if( this == &rhs || !_allocator || !rhs._allocator )
        return *this;

    if( getTypeIdentifier() != rhs.getTypeIdentifier( ))
        throw std::runtime_error( "Can't assign Zerobuf of a different type" );

    notifyChanging();
    _allocator->copyBuffer( rhs._allocator->getData(),
                            rhs._allocator->getSize( ));
    return *this;
}

Zerobuf& Zerobuf::operator = ( Zerobuf&& rhs )
{
    if( this == &rhs || !rhs._allocator )
        return *this;

    if( getTypeIdentifier() != rhs.getTypeIdentifier( ))
        throw std::runtime_error( "Can't assign Zerobuf of a different type" );

    notifyChanging();
    rhs.notifyChanging();

    if( _allocator->isMovable() && rhs._allocator->isMovable( ))
        _allocator = std::move( rhs._allocator );
    else // Sub allocator data can't be moved - need to copy
        _allocator->copyBuffer( rhs._allocator->getData(),
                                rhs._allocator->getSize( ));

    rhs._allocator.reset( new NonMovingAllocator( rhs.getZerobufStaticSize(),
                                                 rhs.getZerobufNumDynamics( )));
    return *this;
}

void Zerobuf::compact( const float threshold )
{
    if( _allocator && getZerobufNumDynamics() > 0 )
        _allocator->compact( threshold );
}

bool Zerobuf::_fromBinary( const void* data, const size_t size )
{
    if( !_allocator )
        throw std::runtime_error(
            "Can't copy data into empty Zerobuf object" );

    if( size < 4 )
    {
        std::cerr << "zerobuf too small" << std::endl;
        return false;
    }

    const uint32_t version = *reinterpret_cast< const uint32_t* >( data );
    if( version != ZEROBUF_VERSION_ABI )
    {
        std::cerr << "Version mismatch, got zerobuf v" << version
                  << " running v" << ZEROBUF_VERSION_ABI << std::endl;
        return false;
    }

    notifyChanging();
    _allocator->copyBuffer( data, size );
    return true;
}

Data Zerobuf::_toBinary() const
{
    if( !_allocator )
        return Data();

    Data data;
    data.ptr = std::shared_ptr< const void >( _allocator->getData(),
                                              []( const void* ){} );
    data.size = _allocator->getSize();
    return data;
}

bool Zerobuf::_fromJSON( const std::string& string )
{
    if( !_allocator )
        throw std::runtime_error(
            "Can't convert empty Zerobuf object from JSON" );

    Json::Value json;
    Json::Reader reader;
    if( !reader.parse( string, json ))
    {
        std::cerr << "Error parsing JSON: "
                  << reader.getFormattedErrorMessages() << std::endl;
        return false;
    }

    notifyChanging();
    JSONConverter converter( getSchemas( ));
    if( !converter.fromJSON( *_allocator, json ))
        return false;
    compact();
    return true;
}

std::string Zerobuf::_toJSON() const
{
    if( !_allocator )
        return "{}";

    Json::Value json;
    JSONConverter converter( getSchemas( ));

    if( converter.toJSON( *_allocator, json ))
        return json.toStyledString();

    std::cerr << "Internal error converting to JSON, got so far:\n"
              << json.toStyledString() << std::endl;
    return std::string();
}

bool Zerobuf::operator == ( const Zerobuf& rhs ) const
{
    if( this == &rhs ||
        ( !_allocator && getTypeIdentifier() == rhs.getTypeIdentifier( )))
    {
        return true;
    }
    return toJSON() == rhs.toJSON();
}

bool Zerobuf::operator != ( const Zerobuf& rhs ) const
{
    return !(*this == rhs);
}

Allocator& Zerobuf::getAllocator()
{
    if( !_allocator )
        throw std::runtime_error( "Empty Zerobuf has no allocator" );

    notifyChanging();
    return *_allocator;
}

const Allocator& Zerobuf::getAllocator() const
{
    if( !_allocator )
        throw std::runtime_error( "Empty Zerobuf has no allocator" );

    return *_allocator;
}

void Zerobuf::_copyZerobufArray( const void* data, const size_t size,
                                 const size_t arrayNum )
{
    if( !_allocator )
        throw std::runtime_error(
            "Can't copy data into empty Zerobuf object" );

    notifyChanging();
    void* array = _allocator->updateAllocation( arrayNum, false /*no copy*/,
                                                size );
    ::memcpy( array, data, size );
}

void Zerobuf::check() const
{
    if( _allocator )
        getAllocator().check( getZerobufNumDynamics( ));
}

}
