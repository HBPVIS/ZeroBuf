
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "Zerobuf.h"

#include "detail/JsonSerialization.h"
#include "Allocator.h"
#include "Schema.h"

#include <iostream>


namespace zerobuf
{

Zerobuf::~Zerobuf()
{
    delete _alloc;
}

const void* Zerobuf::getZerobufData() const
{
    return _alloc ? _alloc->getData() : 0;
}

size_t Zerobuf::getZerobufSize() const
{
    return _alloc ? _alloc->getSize() : 0;
}

void Zerobuf::setZerobufData( const void* data, size_t size )
{
    if( _alloc )
        _alloc->copyBuffer( data, size );
    else
        std::cerr << "Can't copy data into empty zerobuf" << std::endl;
}

std::string Zerobuf::toJSON() const
{
    Json::Value rootJSON;
    for( const auto& valueSchema : getSchema().fields )
        addValueToJSON( rootJSON, valueSchema )
    return rootJSON.toStyledString();
}

void Zerobuf::fromJSON( const std::string& json )
{
    Json::Value rootJSON;
    Json::Reader reader;
    reader.parse( json, rootJSON );

    for( const auto& valueSchema : getSchema().fields )
        getValueFromJSON( rootJSON, valueSchema )
}

bool Zerobuf::operator==( const Zerobuf& rhs ) const
{
    if( this == &rhs )
        return true;
    // not optimal, but correct and easy
    return toJSON() == rhs.toJSON();
}

bool Zerobuf::operator!=( const Zerobuf& rhs ) const
{
    return !(*this == rhs);
}

void Zerobuf::_setZerobufArray( const void* data, const size_t size,
                                const size_t arrayNum )
{
    void* array = _alloc->updateAllocation( arrayNum, size );
    ::memcpy( array, data, size );
}

Zerobuf& Zerobuf::operator = ( const Zerobuf& rhs )
{
    if( this != &rhs )
        _alloc->copyBuffer( rhs._alloc->getData(), rhs._alloc->getSize( ));
    return *this;
}

}
