
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "Zerobuf.h"
#include <zerobuf/Allocator.h>
#include <zerobuf/jsoncpp/json/json.h>
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
        std::cerr << "Con't copy data into empty zerobuf" << std::endl;
}

Zerobuf& Zerobuf::operator = ( const Zerobuf& rhs )
{
    if( this != &rhs )
        _alloc->copyBuffer( rhs._alloc->getData(), rhs._alloc->getSize( ));
    return *this;
}

bool Zerobuf::_parseJSON( const std::string& input, Json::Value& output )
{
    Json::Reader reader;
    return reader.parse( input, output );
}

}
