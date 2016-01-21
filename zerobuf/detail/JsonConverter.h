
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include <zerobuf/Schema.h>
#include <zerobuf/version.h>
#include <zerobuf/jsoncpp/json/json.h>
#include "base64.h"

#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

namespace zerobuf
{
namespace
{
static const uint128_t _byteType = servus::make_uint128( "::zerobuf::byte_t" );

const uint128_t& _getType( const Schema::Field& field )
{
    return std::get< Schema::FIELD_TYPE >( field );
}

const std::string& _getName( const Schema::Field& field )
{
    return std::get< Schema::FIELD_NAME >( field );
}

bool _isStaticElement( const Schema::Field& field )
{
    return std::get< Schema::FIELD_SIZE >( field ) > 0;
}

bool _isStatic( const Schema::Field& field )
{
    return std::get< Schema::FIELD_SIZE >( field ) > 0 &&
           std::get< Schema::FIELD_ELEMENTS >( field ) > 0;
}

size_t _getOffset( const Schema::Field& field )
{
    return std::get< Schema::FIELD_OFFSET >( field );
}

size_t _getIndex( const Schema::Field& field )
{
    if( _isStatic( field ))
        throw std::runtime_error( "Static elements don't have an index" );

    const size_t offset = _getOffset( field );
    if( offset < 4 || (( offset - 4 ) % 16 ) != 0 )
        throw std::runtime_error( "Not an offset of a dynamic member" );
    return (offset - 4) / 16;
}

size_t _getSize( const Schema::Field& field )
{
    return std::get< Schema::FIELD_SIZE >( field );
}

size_t _getNElements( const Schema::Field& field )
{
    return std::get< Schema::FIELD_ELEMENTS >( field );
}

class JSONConverter
{
public:
    JSONConverter( const Schemas& schemas )
        : _schemas( schemas ) // copy since lifetime of param is not defined
    {
        if( schemas.empty( ))
            throw std::runtime_error( "No schemas given" );

        addConverter< int8_t >( "int8_t" );
        addConverter< uint8_t >( "uint8_t" );
        addConverter< byte_t >( "::zerobuf::byte_t" );
        addConverter< int16_t >( "int16_t" );
        addConverter< uint16_t >( "uint16_t" );
        addConverter< int32_t >( "int32_t" );
        addConverter< uint32_t >( "uint32_t" );
        addConverter< int64_t >( "int64_t" );
        addConverter< uint64_t >( "uint64_t" );
        addConverter< uint128_t >( "::zerobuf::uint128_t" );
        addConverter< float >( "float" );
        addConverter< double >( "double" );
        addConverter< bool >( "bool" );
        addConverter< char >( "char" );
        for( const Schema& schema : _schemas )
            addConverter( schema );
    }

    bool toJSON( const Allocator& allocator, Json::Value& jsonValue ) const
    {
        const Schema& root = _schemas.front();
        const Schema::Field field( "root", root.type, 0,
                                   root.numDynamics == 0 ? root.staticSize : 0,
                                   1 );
        return _fromZeroBuf( allocator, field, root, jsonValue );
    }

    bool fromJSON( Allocator& allocator, const Json::Value& jsonValue ) const
    {
        const Schema& root = _schemas.front();
        const Schema::Field field( "root", root.type, 0,
                                   root.numDynamics == 0 ? root.staticSize : 0,
                                   1 );
        return _toZeroBuf( allocator, field, root, jsonValue );
    }

private:
    typedef std::function< bool( const Allocator&, const Schema::Field&,
                                 Json::Value& ) > ToJSON_f;
    typedef std::function< bool( const Allocator&, const Schema::Field&,
                                 Json::Value& ) const > ToJSONConst_f;
    typedef std::function< bool( Allocator&, const Schema::Field&,
                                 const Json::Value& ) > FromJSON_f;

    typedef std::unordered_map< uint128_t, ToJSON_f > ToJSONMap;
    typedef std::unordered_map< uint128_t, FromJSON_f > FromJSONMap;

    ToJSONMap _toJSONMap;
    FromJSONMap _fromJSONMap;
    Schemas _schemas;

    void addConverter( const Schema& schema )
    {
        _toJSONMap[ schema.type ] = std::bind( &JSONConverter::_fromZeroBuf,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2, schema,
                                               std::placeholders::_3 );
        _fromJSONMap[ schema.type ] = std::bind( &JSONConverter::_toZeroBuf,
                                                 this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2, schema,
                                                 std::placeholders::_3 );
    }

    template< class T > void addConverter( const std::string& name )
    {
        const uint128_t& type = servus::make_uint128( name );
        _toJSONMap[ type ] = std::bind( &JSONConverter::_fromPOD< T >,
                                        this,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3 );
        _fromJSONMap[ type ] = std::bind( &JSONConverter::_toPOD< T >, this,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3 );
    }

    bool _toJSONFunc( const Allocator& allocator, const Schema::Field& field,
                      Json::Value& jsonValue ) const
    {
        const uint128_t& type = _getType( field );
        const auto& func = _toJSONMap.find( type );
        if( func == _toJSONMap.end( ))
        {
            std::cerr << "Missing converter for type " << type.getShortString()
                      << " of field " << _getName( field ) << std::endl;
            return false;
        }
        return func->second( allocator, field, jsonValue );
    }

    bool _fromJSONFunc( Allocator& allocator, const Schema::Field& field,
                        const Json::Value& jsonValue ) const
    {
        const uint128_t& type = _getType( field );
        const auto& func = _fromJSONMap.find( type );
        if( func == _fromJSONMap.end( ))
        {
            std::cerr << "Missing converter for type " << type.getShortString()
                      << " of field " << _getName( field ) << std::endl;
            return false;
        }
        return func->second( allocator, field, jsonValue );
    }

    // ZeroBuf object conversion
    bool _fromZeroBuf( const Allocator& allocator, const Schema::Field& field,
                       const Schema& schema, Json::Value& jsonValue ) const
    {
        const size_t nElements = _getNElements( field );

        switch( nElements )
        {
        case 0: // dynamic array
        {
            if( !_isStaticElement( field ))
                throw std::runtime_error(
                    "Dynamic arrays of dynamic Zerobufs not supported" );

            const size_t elemSize = _getSize( field );
            const size_t index = _getIndex( field );
            const size_t arraySize = allocator.getDynamicSize( index ) /
                                     elemSize ;
            const size_t offset = allocator.getDynamicOffset( index );
            jsonValue.resize( arraySize );

            for( size_t i = 0; i < arraySize; ++i )
            {
                const ConstStaticSubAllocator subAllocator( allocator,
                                                            offset + i*elemSize,
                                                            elemSize );
                if( !_fromZeroBufItem( subAllocator, schema,
                                       jsonValue[ uint32_t( i )]))
                {
                    return false;
                }
            }
            return true;
        }

        case 1: // single element
        {
            const size_t offset = _getOffset( field );
            const bool isRoot = offset == 0;
            if( _isStaticElement( field ))
            {
                if( isRoot )
                    return _fromZeroBufItem( allocator, schema, jsonValue );

                const ConstStaticSubAllocator subAllocator( allocator, offset,
                                                            _getSize( field ));
                return _fromZeroBufItem( subAllocator, schema, jsonValue );
            }

            if( isRoot )
                return _fromZeroBufItem( allocator, schema, jsonValue );

            const ConstNonMovingSubAllocator subAllocator( allocator,
                                                           _getIndex( field ),
                                                           schema.numDynamics,
                                                           schema.staticSize );
            return _fromZeroBufItem( subAllocator, schema, jsonValue );
        }

        default: // static array
        {
            if( !_isStaticElement( field ))
                throw std::runtime_error(
                    "Static arrays of dynamic Zerobufs not supported" );

            jsonValue.resize( nElements );
            const size_t offset = _getOffset( field );
            const size_t elemSize = _getSize( field );

            for( size_t i = 0; i < nElements; ++i )
            {
                const ConstStaticSubAllocator subAllocator( allocator,
                                                            offset + i*elemSize,
                                                            elemSize );
                if( !_fromZeroBufItem( subAllocator, schema,
                                       jsonValue[ uint32_t( i )]))
                {
                    return false;
                }
            }
            return true;
        }
        }
    }

    bool _fromZeroBufItem( const Allocator& allocator, const Schema& schema,
                           Json::Value& jsonValue ) const
    {
        for( const Schema::Field& field : schema.fields )
        {
            const std::string& name = _getName( field );
            jsonValue[ name ] = Json::Value();
            if( !_toJSONFunc( allocator, field, jsonValue[ name ]))
                return false;
        }
        return true;
    }

    bool _toZeroBuf( Allocator& allocator, const Schema::Field& field,
                     const Schema& schema, const Json::Value& jsonValue ) const
    {
        const size_t nElements = _getNElements( field );

        switch( nElements )
        {
        case 0: // dynamic array
        {
            if( !_isStaticElement( field ))
                throw std::runtime_error(
                    "Dynamic arrays of dynamic Zerobufs not supported" );

            const size_t index = _getIndex( field );
            const size_t arraySize = jsonValue.size();
            const size_t elemSize = _getSize( field );
            allocator.updateAllocation( index, false /*no copy*/,
                                        arraySize * elemSize );

            const size_t offset = allocator.getDynamicOffset( index );
            for( size_t i = 0; i < arraySize; ++i )
            {
                StaticSubAllocator subAllocator( allocator, offset + i*elemSize,
                                                 elemSize );
                if( !_toZeroBufItem( subAllocator, schema,
                                     jsonValue[ uint32_t( i )]))
                {
                    return false;
                }
            }
            return true;
        }

        case 1: // single element
        {
            const size_t offset = _getOffset( field );
            const bool isRoot = offset == 0;
            if( _isStaticElement( field ))
            {
                if( isRoot )
                    return _toZeroBufItem( allocator, schema, jsonValue );

                StaticSubAllocator subAllocator( allocator, offset,
                                                 _getSize( field ));
                return _toZeroBufItem( subAllocator, schema, jsonValue );
            }

            if( isRoot )
                return _toZeroBufItem( allocator, schema, jsonValue );

            NonMovingSubAllocator subAllocator( allocator, _getIndex( field ),
                                                schema.numDynamics,
                                                schema.staticSize );
            return _toZeroBufItem( subAllocator, schema, jsonValue );
        }

        default: // static array
        {
            if( !_isStaticElement( field ))
                throw std::runtime_error(
                    "Static arrays of dynamic Zerobufs not supported" );

            if( jsonValue.size() != nElements )
            {
                std::cerr << "JSON array '" << _getName( field ) << "': '"
                          << jsonValue << "' does not match array size "
                          << nElements << std::endl;
                return false;
            }

            const size_t offset = _getOffset( field );
            const size_t elemSize = _getSize( field );
            for( size_t i = 0; i < nElements; ++i )
            {
                StaticSubAllocator subAllocator( allocator, offset + i*elemSize,
                                                 elemSize );
                if( !_toZeroBufItem( subAllocator, schema,
                                     jsonValue[ uint32_t( i )] ))
                {
                    return false;
                }
            }
            return true;
        }
        }
    }

    bool _toZeroBufItem( Allocator& allocator, const Schema& schema,
                         const Json::Value& jsonValue ) const
    {
        allocator.getItem< uint32_t >( 0 ) = ZEROBUF_VERSION_ABI;
        for( const Schema::Field& field : schema.fields )
        {
            const std::string& name = _getName( field );
            if( !_fromJSONFunc( allocator, field, jsonValue[ name ]))
                return false;
        }
        return true;
    }

    // builtin conversions
    template< class T >
    bool _fromPOD( const Allocator& allocator, const Schema::Field& field,
                  Json::Value& jsonValue ) const
    {
        const size_t nElements = _getNElements( field );
        switch( nElements )
        {
        case 0: // dynamic array
            return _fromPODDynamic< T >( allocator, field, jsonValue );

        case 1: // single element
        {
            const size_t offset = _getOffset( field );
            return _fromPODItem< T >( allocator.getItem< T >( offset ),
                                     jsonValue );
        }
        default: // static array
            return _fromPODArray< T >( allocator, field, jsonValue );
        }
    }

    template< class T >
    bool _fromPODItem( const T& value, Json::Value& jsonValue ) const
    {
        if( std::is_floating_point< T >::value )
            jsonValue = Json::Value( double( value ));
        else if( std::is_same< bool, T >::value )
            jsonValue = Json::Value( bool( value ));
        else if( std::is_signed< T >::value )
            jsonValue = Json::Value( Json::Int64( value ));
        else
            jsonValue = Json::Value( Json::UInt64( value ));
        return true;
    }

    template< class T >
    bool _fromPODDynamic( const Allocator& allocator, const Schema::Field& field,
                          Json::Value& jsonValue ) const
    {
        return _fromPODDynamicSimple< T >( allocator, field, jsonValue );
    }

    template< class T >
    bool _fromPODDynamicSimple( const Allocator& allocator,
                                const Schema::Field& field,
                                Json::Value& jsonValue ) const
    {
        if( _isStatic( field ))
            throw std::runtime_error( "Internal JSON converter error" );

        const size_t index = _getIndex( field );
        const T* data = allocator.getDynamic< T >( index );
        const size_t size = allocator.getDynamicSize( index ) / sizeof(T);
        for( size_t i = 0; i < size; ++i )
            if( !_fromPODItem( data[i], jsonValue[ uint32_t( i )]))
                return false;
        return true;
    }

    template< class T >
    bool _fromPODArray( const Allocator& allocator, const Schema::Field& field,
                        Json::Value& jsonValue ) const
    {
        return _fromPODArraySimple< T >( allocator, field, jsonValue );
    }

    template< class T >
    bool _fromPODArraySimple( const Allocator& allocator,
                              const Schema::Field& field,
                              Json::Value& jsonValue ) const
    {
        const size_t nElements = _getNElements( field );
        const size_t offset = _getOffset( field );
        const T* data = allocator.getItemPtr< T >( offset );

        jsonValue.resize( nElements );
        for( size_t i = 0; i < nElements; ++i )
            if( !_fromPODItem< T >( data[ i ], jsonValue[ uint32_t( i )]))
                return false;
        return true;
    }

    template< class T >
    bool _toPOD( Allocator& allocator, const Schema::Field& field,
                 const Json::Value& jsonValue )
    {
        const size_t nElements = _getNElements( field );
        switch( nElements )
        {
        case 0: // dynamic array
            return _toPODDynamic< T >( allocator, field, jsonValue );

        case 1: // single element
        {
            const size_t offset = _getOffset( field );
            return _toPODItem< T >( allocator.getItem<T>( offset ),
                                    jsonValue );
        }
        default: // static array
            return _toPODArray< T >( allocator, field, jsonValue );
        }
    }

    template< class T >
    bool _toPODItem( T& value, const Json::Value& jsonValue )
    {
        if( std::is_floating_point< T >( ))
            value = T( jsonValue.asDouble( ));
        else if( std::is_same< bool, T >::value )
            value = jsonValue.asBool();
        else if( std::is_signed< T >::value )
            value = T( jsonValue.asInt64( ));
        else
            value = T( jsonValue.asUInt64( ));
        return true;
    }

    template< class T >
    bool _toPODDynamic( Allocator& allocator, const Schema::Field& field,
                        const Json::Value& jsonValue )
    {
        return _toPODDynamicSimple< T >( allocator, field, jsonValue );
    }

    template< class T >
    bool _toPODDynamicSimple( Allocator& allocator, const Schema::Field& field,
                        const Json::Value& jsonValue )
    {
        if( _isStatic( field ))
            throw std::runtime_error( "Internal JSON converter error" );

        const size_t index = _getIndex( field );
        const size_t size = jsonValue.size();
        T* data = reinterpret_cast< T* >(
            allocator.updateAllocation( index, false /*no copy*/,
                                        size * sizeof( T )));

        for( size_t i = 0; i < size; ++i )
            if( !_toPODItem< T >( data[i], jsonValue[ uint32_t( i )]))
                return false;
        return true;
    }

    template< class T >
    bool _toPODArray( Allocator& allocator, const Schema::Field& field,
                      const Json::Value& jsonValue )
    {
        return _toPODArraySimple< T >( allocator, field, jsonValue );
    }

    template< class T >
    bool _toPODArraySimple( Allocator& allocator, const Schema::Field& field,
                            const Json::Value& jsonValue )
    {
        const size_t nElements = _getNElements( field );
        if( jsonValue.size() != nElements )
        {
            std::cerr << "JSON array '" << _getName( field ) << "': '"
                      << jsonValue << "' does not match array size " << nElements
                      << std::endl;
            return false;
        }

        const size_t offset = _getOffset( field );
        T* data = allocator.getItemPtr< T >( offset );
        for( size_t i = 0; i < nElements; ++i )
            if( !_toPODItem< T >( data[ i ], jsonValue[ uint32_t( i )]))
                return false;
        return true;
    }
};

template<> bool JSONConverter::_fromPODItem< std::string >(
    const std::string& value, Json::Value& jsonValue ) const
{
    jsonValue = Json::Value( value );
    return true;
}

template<> bool JSONConverter::_fromPODItem< uint128_t >(
    const uint128_t& value, Json::Value& jsonValue ) const
{
    jsonValue["high"] = Json::UInt64( value.high( ));
    jsonValue["low"] = Json::UInt64( value.low( ));
    return true;
}

template<>
bool JSONConverter::_fromPODDynamic< char >( const Allocator& allocator,
                                             const Schema::Field& field,
                                             Json::Value& jsonValue ) const
{
    if( _isStatic( field ))
        throw std::runtime_error( "Internal JSON converter error" );

    const size_t index = _getIndex( field );
    const size_t size = allocator.getDynamicSize( index );

    const uint8_t* ptr =  allocator.getDynamic< const uint8_t >( index );
    const std::string value( ptr, ptr + size );
    return _fromPODItem( value, jsonValue );
}

template<>
bool JSONConverter::_toPODDynamic< char >( Allocator& allocator,
                                           const Schema::Field& field,
                                           const Json::Value& jsonValue )
{
    if( _isStatic( field ))
        throw std::runtime_error( "Internal JSON converter error" );

    const size_t index = _getIndex( field );
    const std::string& value = jsonValue.asString();

    void* array = allocator.updateAllocation( index, false /*no copy*/,
                                              value.length( ));
    ::memcpy( array, value.c_str(), value.length( ));
    return true;
}

template<>
bool JSONConverter::_fromPODDynamic< uint8_t >( const Allocator& allocator,
                                                const Schema::Field& field,
                                                Json::Value& jsonValue ) const
{
    if( _isStatic( field ))
        throw std::runtime_error( "Internal JSON converter error" );

    if( _getType( field ) != _byteType )
        return _fromPODDynamicSimple< uint8_t >( allocator, field, jsonValue );

    const size_t index = _getIndex( field );
    const size_t size = allocator.getDynamicSize( index );
    const uint8_t* data =  allocator.getDynamic< const uint8_t >( index );
    const std::string& value = base64_encode( data, size );
    return _fromPODItem( value, jsonValue );
}

template<>
bool JSONConverter::_toPODDynamic< uint8_t >( Allocator& allocator,
                                              const Schema::Field& field,
                                              const Json::Value& jsonValue )
{
    if( _isStatic( field ))
        throw std::runtime_error( "Internal JSON converter error" );

    if( _getType( field ) != _byteType )
        return _toPODDynamicSimple< uint8_t >( allocator, field, jsonValue );

    const size_t index = _getIndex( field );
    const std::string& value = jsonValue.asString();
    const std::string& decoded = base64_decode( value );
    void* array = allocator.updateAllocation( index, false /*no copy*/,
                                              decoded.length( ));
    ::memcpy( array, decoded.c_str(), decoded.length( ));
    return true;
}

template<>
bool JSONConverter::_fromPODArray< uint8_t >( const Allocator& allocator,
                                              const Schema::Field& field,
                                              Json::Value& jsonValue ) const
{
    if( _getType( field ) != _byteType )
        return _fromPODArraySimple< uint8_t >( allocator, field, jsonValue );

    const size_t nElements = _getNElements( field );
    const size_t offset = _getOffset( field );
    const uint8_t* data = allocator.getItemPtr< uint8_t >( offset );
    const std::string& value = base64_encode( data, nElements );
    return _fromPODItem( value, jsonValue );
}

template<>
bool JSONConverter::_toPODArray< uint8_t >( Allocator& allocator,
                                            const Schema::Field& field,
                                            const Json::Value& jsonValue )
{
    if( _getType( field ) != _byteType )
        return _toPODArraySimple< uint8_t >( allocator, field, jsonValue );

    const size_t nElements = _getNElements( field );
    const std::string& value = jsonValue.asString();
    const std::string& decoded = base64_decode( value );

    if( decoded.length() != nElements )
    {
        std::cerr << "JSON array '" << _getName( field ) << "': '" << jsonValue
                  << "' (" << decoded.length()
                  << " bytes) does not match array size " << nElements
                  << std::endl;
        return false;
    }

    const size_t offset = _getOffset( field );
    uint8_t* data = allocator.getItemPtr< uint8_t >( offset );
    ::memcpy( data, decoded.c_str(), decoded.length( ));
    return true;
}

template<> bool JSONConverter::_toPODItem< uint128_t >(
    uint128_t& value, const Json::Value& jsonValue )
{
    value = uint128_t( jsonValue["high"].asUInt64(),
                       jsonValue["low"].asUInt64( ));
    return true;
}

template<> bool JSONConverter::_toPODItem< std::string >(
    std::string& value, const Json::Value& jsonValue )
{
    value = jsonValue.asString();
    return true;
}

}
} // zerobuf
