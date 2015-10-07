
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#include <zerobuf/ConstVector.h>
#include <zerobuf/Zerobuf.h>
#include <zerobuf/jsoncpp/json/json.h>

namespace zerobuf
{
namespace detail
{

/**
 * Convert any C++ value from its type to the corresponding JSON type.
 *
 * @tparam CT any C++ type
 * @tparam JT JSON type, either POD or Json::Value
 * @param value the C++ value to convert
 * @return the converted JSON value
 */
template< typename CT, typename JT >
Json::Value _toJSON( const CT& value )
{
    return JT(value);
}

/** Specialization for servus::uint128_t, represented as an object in JSON */
template<>
Json::Value _toJSON< servus::uint128_t,
                     Json::UInt64 >( const servus::uint128_t& value )
{
    Json::Value json;
    json["high"] = Json::UInt64(value.high( ));
    json["low"] = Json::UInt64(value.low( ));
    return json;
}

/** Specialization for servus::uint128_t, represented as an object in JSON */
template<>
Json::Value _toJSON< ::zerobuf::Zerobuf,
                     Json::UInt64 >( const ::zerobuf::Zerobuf& value )
{
    return value.getJSON();
}

/**
 * Convert any JSON value from its type to the corresponding C++ type.
 *
 * @tparam CT any C++ type
 * @tparam JT JSON type, either POD or Json::Value
 * @param jsonValue the JSON value to convert
 * @return the converted C++ value
 */
template< typename CT, typename JT >
CT _fromJSON( const JT& jsonValue )
{
    return jsonValue;
}

/** Specialization for servus::uint128_t, represented as an object in JSON */
template<>
servus::uint128_t _fromJSON( const Json::Value& jsonValue )
{
    return servus::uint128_t( jsonValue["high"].asUInt64(),
                              jsonValue["low"].asUInt64( ));
}

/**
 * Extracts the value from the ZeroBuf allocator as described by valueSchema,
 * performs necessary conversion to create a JSON value and adds the value to
 * rootJSON.
 */
#define convertToJSON(cpptype, jsontype) \
{ \
    const std::string& type = std::get< Schema::FIELD_TYPE >( valueSchema ); \
    if( type == #cpptype ) \
    { \
        const size_t offset = \
            std::get< Schema::FIELD_DATAOFFSET >( valueSchema ); \
        const size_t size = std::get< Schema::FIELD_SIZE >( valueSchema ); \
        Json::Value& jsonValue = \
            rootJSON[std::get< Schema::FIELD_NAME >( valueSchema )]; \
        \
        /* static variable */ \
        if( size == 0 )\
        { \
            jsonValue = _toJSON< cpptype, jsontype >(\
                _alloc->getItem< cpptype >( offset )); \
        } \
        /* static array */ \
        else if( std::get< Schema::FIELD_ISSTATIC >( valueSchema )) \
        { \
            const cpptype* ptr = _alloc->getItemPtr< cpptype >( offset ); \
            jsonValue.resize( size ); \
            for( size_t i = 0; i < size; ++i ) \
                jsonValue[uint32_t(i)] = _toJSON< cpptype, jsontype >( ptr[i] ); \
        } \
        else \
        { \
            /* std::string */ \
            if( type == "char" ) \
            { \
                const uint8_t* ptr = \
                    _alloc->getDynamic< const uint8_t >( offset ); \
                const std::string value( \
                    ptr, ptr + _alloc->getItem< uint64_t >( size )); \
                if( !value.empty( )) \
                    jsonValue = _toJSON< std::string, std::string >( value ); \
            } \
            /* dynamic array */ \
            else \
            { \
                ConstVector< cpptype > values( _alloc, offset ); \
                for( size_t i = 0; i < values.size(); ++i ) \
                { \
                    jsonValue[uint32_t(i)] = \
                        _toJSON< cpptype, jsontype >( values[i] ); \
                } \
            } \
        } \
    } \
}

/**
 * Gets the value from rootJSON, performs necessary conversions and sets the
 * value into the ZeroBuf allocator, as described by valueSchema.
 */
#define convertFromJSON(cpptype, jsontype) \
{ \
    const std::string& type = std::get< Schema::FIELD_TYPE >( valueSchema ); \
    if( type == #cpptype ) \
    { \
        const size_t offset = \
            std::get< Schema::FIELD_DATAOFFSET >( valueSchema ); \
        const size_t size = std::get< Schema::FIELD_SIZE >( valueSchema ); \
        const std::string& key = std::get< Schema::FIELD_NAME >( valueSchema ); \
        const Json::Value& jsonValue = rootJSON[key]; \
        \
        /* static variable */ \
        if( size == 0 ) \
        { \
            _alloc->getItem< cpptype >( offset ) = \
                _fromJSON< cpptype >( jsonValue jsontype ); \
        } \
        /* static array */ \
        else if( std::get< Schema::FIELD_ISSTATIC >( valueSchema )) \
        { \
            if( jsonValue.size() > size ) \
                std::cerr << "fromJSON: JSON array '" << key \
                          << "' too big; got " << jsonValue.size() \
                          << ", allowed is " << size << std::endl; \
            else \
            { \
                size_t i = 0; \
                for( const auto& value : jsonValue ) \
                { \
                    _alloc->getItemPtr< cpptype >( offset )[i++] = \
                        _fromJSON< cpptype >( value jsontype ); \
                } \
            } \
        } \
        else \
        { \
            /* std::string */ \
            if( type == "char" ) \
            { \
                const std::string& value = jsonValue.asString(); \
                _setZerobufArray( value.c_str(), value.length(), offset ); \
            } \
            /* dynamic array*/ \
            else \
            { \
                cpptype* array = reinterpret_cast< cpptype* >( \
                    _alloc->updateAllocation( offset, jsonValue.size() * \
                                                      sizeof( cpptype ))); \
                size_t i = 0; \
                for( const auto& value : jsonValue ) \
                    array[i++] = _fromJSON< cpptype >( value jsontype ); \
            } \
        } \
    } \
}

/**
 * Get the value from this (the ZeroBuf object calling this macro) as described
 * by valueSchema and add it to rootJSON. Does a trial-and-error type matching
 * + macro magic to mimic type-to-string mapping.
 */
#define addValueToJSON( rootJSON, valueSchema ) \
{ \
    using detail::_toJSON; \
    convertToJSON(int8_t, int8_t) \
    convertToJSON(uint8_t, uint8_t) \
    convertToJSON(int16_t, int16_t) \
    convertToJSON(uint16_t, uint16_t) \
    convertToJSON(int32_t, int32_t) \
    convertToJSON(uint32_t, uint32_t) \
    convertToJSON(int64_t, Json::Int64) \
    convertToJSON(uint64_t, Json::UInt64) \
    convertToJSON(servus::uint128_t, Json::UInt64) \
    convertToJSON(float, float) \
    convertToJSON(double, double) \
    convertToJSON(bool, bool) \
    convertToJSON(char, char) \
    convertToJSON(::zerobuf::Zerobuf, Json::UInt64) \
}

/**
 * Get the value from rootJSON as described by valueSchema and set it to this
 * (the ZeroBuf object calling this macro). Does a trial-and-error type matching
 * + macro magic to mimic type-to-string mapping.
 */
#define getValueFromJSON( rootJSON, valueSchema ) \
{ \
    if( !rootJSON.isMember( std::get<Schema::FIELD_NAME>( valueSchema ))) \
        continue; \
    using detail::_fromJSON; \
    convertFromJSON(int8_t, .asInt()) \
    convertFromJSON(uint8_t, .asUInt()) \
    convertFromJSON(int16_t, .asInt()) \
    convertFromJSON(uint16_t, .asUInt()) \
    convertFromJSON(int32_t, .asInt()) \
    convertFromJSON(uint32_t, .asUInt()) \
    convertFromJSON(int64_t, .asInt64()) \
    convertFromJSON(uint64_t, .asUInt64()) \
    convertFromJSON(servus::uint128_t, /* empty; extra handling for uint128_t*/) \
    convertFromJSON(float, .asFloat()) \
    convertFromJSON(double, .asDouble()) \
    convertFromJSON(bool, .asBool()) \
    convertFromJSON(char, .asInt() /* irrelevant, string has extra handling */) \
}

} // detail
} // zerobuf
