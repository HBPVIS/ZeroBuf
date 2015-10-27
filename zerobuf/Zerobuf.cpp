
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "Zerobuf.h"

#include "detail/JsonSerialization.h"
#include "ConstNonMovingSubAllocator.h"
#include "NonMovingSubAllocator.h"
#include "Schema.h"
#include "Vector.h"
#include "ConstVector.h"

#include <iostream>

namespace zerobuf
{

namespace detail
{

template<>
void convertStaticToJSONValue<Zerobuf>( const Allocator* allocator,
                                        const Schema::Field& valueSchema,
                                        Json::Value& value )
{
    const size_t offset =
        std::get< Schema::FIELD_DATAOFFSET >( valueSchema );
    const size_t size = std::get< Schema::FIELD_SIZE >( valueSchema );
    const Schema::SchemaFunction& schemaFunc =
            std::get< Schema::FIELD_SCHEMAFUNC >( valueSchema );

    const Schema& schema = schemaFunc();
    const size_t staticSize = schema.staticSize;

    const NonMovingBaseAllocator* parentAllocator =
               static_cast< const NonMovingBaseAllocator* >( allocator );

    if( size == 0 )
    {
        ConstNonMovingSubAllocator nonMovingAllocator( parentAllocator,
                                                       offset,
                                                       0,
                                                       schema.staticSize );

        detail::toJSONValue( &nonMovingAllocator, schema, value );

    }
    else /* static array */
    if( std::get< Schema::FIELD_ISSTATIC >( valueSchema ))
    {
        for( size_t i = 0; i < size; ++i )
        {
            ConstNonMovingSubAllocator nonMovingAllocator( parentAllocator,
                                                           offset + i * staticSize,
                                                           0,
                                                           schema.staticSize );
            Json::Value jsonVal;
            detail::toJSONValue( &nonMovingAllocator, schema, jsonVal );
            value.append( jsonVal );
        }
    }
}

template<>
void convertToJSONValue<Zerobuf>( const Allocator* allocator,
                                  const Schema::Field& valueSchema,
                                  Json::Value& value )
{
    if( std::get< Schema::FIELD_ISSTATIC >( valueSchema ))
       convertStaticToJSONValue<Zerobuf>( allocator, valueSchema, value );
    else /* dynamic variables */
    {
        const size_t offset =
            std::get< Schema::FIELD_DATAOFFSET >( valueSchema );

        const Schema::SchemaFunction& schemaFunc =
                std::get< Schema::FIELD_SCHEMAFUNC >( valueSchema );

        const Schema& schema = schemaFunc();

        const NonMovingBaseAllocator* parentAllocator =
                   static_cast< const NonMovingBaseAllocator* >( allocator );

        const size_t size = parentAllocator->getDynamicSize( offset ) / schema.staticSize;
        for( size_t i = 0; i < size; ++i )
        {
            const size_t dynOff = (size_t)parentAllocator->getDynamicPtr<const uint8_t>( offset )
                                  - (size_t)parentAllocator->getData();

            ConstNonMovingSubAllocator nonMovingAllocator( parentAllocator,
                                                           dynOff + i * schema.staticSize,
                                                           0,
                                                           schema.staticSize );

            detail::toJSONValue( &nonMovingAllocator, schema,  value[uint32_t(i)] );
        }

    }
}

template<>
void convertStaticFromJSONValue<Zerobuf>( Allocator* allocator,
                                          const Schema::Field& valueSchema,
                                          const Json::Value& value )
{
    const size_t offset =
        std::get< Schema::FIELD_DATAOFFSET >( valueSchema );
    const size_t size =
        std::get< Schema::FIELD_SIZE >( valueSchema );

    const Schema::SchemaFunction& schemaFunc =
           std::get< Schema::FIELD_SCHEMAFUNC >( valueSchema );

    const Schema& schema = schemaFunc();
    const size_t staticSize = schema.staticSize;

    NonMovingBaseAllocator* parentAllocator =
               static_cast< NonMovingBaseAllocator* >( allocator );

    if( size == 0 )
    {
        NonMovingSubAllocator nonMovingAllocator( parentAllocator,
                                                  offset,
                                                  0,
                                                  schema.staticSize );

        detail::fromJSONValue( &nonMovingAllocator, schema, value );
    }
    /* static array */
    else if( std::get< Schema::FIELD_ISSTATIC >( valueSchema ))
    {
        if( value.size() > size )
        {
            const std::string& key = std::get< Schema::FIELD_NAME >( valueSchema );
            std::cerr << "fromJSON: JSON array '" << key
                      << "' too big; got " << value.size()
                      << ", allowed is " << size << std::endl;
        }
        else
        {
            size_t i = 0;
            for( const auto& jsonVal : value )
            {
                NonMovingSubAllocator nonMovingAllocator( parentAllocator,
                                                          offset + i * staticSize,
                                                          0,
                                                          schema.staticSize );
                detail::fromJSONValue( &nonMovingAllocator, schema, jsonVal );
                ++i;
            }
        }
    }
}

template<>
void convertFromJSONValue<Zerobuf>( Allocator* allocator,
                                    const Schema::Field& valueSchema,
                                    const Json::Value& value )
{
    /* static variable(s) */
    if( std::get< Schema::FIELD_ISSTATIC >( valueSchema ))
        convertStaticFromJSONValue<Zerobuf>( allocator, valueSchema, value );
    else /* dynamic variables */
    {
        NonMovingBaseAllocator* parentAllocator =
                   static_cast< NonMovingBaseAllocator* >( allocator );

        const size_t offset =
            std::get< Schema::FIELD_DATAOFFSET >( valueSchema );

        const Schema::SchemaFunction& schemaFunc =
                std::get< Schema::FIELD_SCHEMAFUNC >( valueSchema );

        const Schema& schema = schemaFunc();

        const size_t size = parentAllocator->getDynamicSize( offset ) / schema.staticSize;
        for( size_t i = 0; i < size; ++i )
        {
            NonMovingSubAllocator nonMovingAllocator( parentAllocator,
                                                      i,
                                                      size,
                                                      schema.staticSize );

            detail::fromJSONValue( &nonMovingAllocator, schema,  value[ uint32_t(i) ] );
        }
    }

}


JSONRegisterer< Zerobuf > registerJSON( "Zerobuf" );

}

struct Zerobuf::Impl
{

    Impl( Zerobuf& zerobuf, Allocator* allocator )
        : _alloc( allocator )
        , _zerobuf( zerobuf )
    {}

    Impl( const Impl& impl )
        : _alloc( impl._alloc->clone() )
        , _zerobuf( impl._zerobuf )
    {
    }

    ~Impl()
    {
        delete _alloc;
    }

    const void* getZerobufData() const
    {
        return _alloc ? _alloc->getData() : 0;
    }

    size_t getZerobufSize() const
    {
        return _alloc ? _alloc->getSize() : 0;
    }

    void setZerobufData( const void* data, size_t size )
    {
        if( _alloc )
            _alloc->copyBuffer( data, size );
        else
            std::cerr << "Can't copy data into empty zerobuf" << std::endl;
    }

    void toJSON( Json::Value& rootJSON ) const
    {
        detail::toJSONValue( _alloc, _zerobuf.getSchema(), rootJSON );
    }

    std::string toJSONString() const
    {
        Json::Value rootJSON;
        toJSON( rootJSON );
        return rootJSON.toStyledString();
    }

    void fromJSON( const std::string& json )
    {
        Json::Value rootJSON;
        Json::Reader reader;
        reader.parse( json, rootJSON );
        detail::fromJSONValue( _alloc, _zerobuf.getSchema(), rootJSON );
    }

    Impl& operator=( const Impl& rhs )
    {
        const Allocator* from = rhs._alloc;
        _alloc->copyBuffer( from->getData(),from->getSize( ));
        return *this;
    }

    void setZerobufArray( const void* data,
                          const size_t size,
                          const size_t arrayNum )
    {
        void* array = _alloc->updateAllocation( arrayNum, size );
        ::memcpy( array, data, size );
    }

    Allocator* _alloc;
    Zerobuf& _zerobuf;

};

template<>
void Vector<Zerobuf>::push_back( const Zerobuf& value )
{
    const size_t size_ =  Super::_getSize();
    const uint8_t* oldPtr = reinterpret_cast<const uint8_t*>( data( ));
    uint8_t* newPtr = Vector::_parent->updateAllocation( Super::_index,
                                                         size_ + value.getZerobufSize( ));
    if( oldPtr != newPtr )
        ::memcpy( newPtr, oldPtr, size_ );

    ::memcpy( newPtr + size_,
              value.getZerobufData(),
              value.getZerobufSize());
}

Zerobuf::Zerobuf( )
    : _impl( new Zerobuf::Impl( *this, 0 ))
{}

Zerobuf::Zerobuf( Allocator* alloc )
    : _impl( new Zerobuf::Impl( *this, alloc ))
{}

Zerobuf::Zerobuf( const Zerobuf& zerobuf )
{
    if( this != &zerobuf )
        *_impl = *zerobuf._impl;
}

Zerobuf::~Zerobuf()
{
}

const void* Zerobuf::getZerobufData() const
{
    return _impl->getZerobufData();
}

size_t Zerobuf::getZerobufSize() const
{
    return _impl->getZerobufSize();
}

void Zerobuf::setZerobufData( const void* data, size_t size )
{
    _impl->setZerobufData( data, size );
}

std::string Zerobuf::toJSON() const
{
    return _impl->toJSONString();
}

void Zerobuf::fromJSON( const std::string& json )
{
    _impl->fromJSON( json );
}

bool Zerobuf::operator==( const Zerobuf& rhs ) const
{
    if( this == &rhs )
        return true;
    return toJSON() == rhs.toJSON();
}

bool Zerobuf::operator!=( const Zerobuf& rhs ) const
{
    return !(*this == rhs);
}

Zerobuf& Zerobuf::operator=( const Zerobuf& rhs )
{
    if( this != &rhs )
        *_impl = *rhs._impl;
    return *this;
}

Allocator* Zerobuf::getAllocator()
{
    return _impl->_alloc;
}

const Allocator* Zerobuf::getAllocator() const
{
    return _impl->_alloc;
}

void Zerobuf::_setZerobufArray( const void* data,
                                const size_t size,
                                const size_t arrayNum )
{
    _impl->setZerobufArray( data,
                            size,
                            arrayNum );
}

}
