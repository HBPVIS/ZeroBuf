
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "json.h"
#include "Zerobuf.h"
#include "jsoncpp/json/json.h"
#include "detail/base64.h"

namespace zerobuf
{
template<> ZEROBUF_INL bool fromJSON( const Json::Value& json )
    { return json.asBool(); }
template<> ZEROBUF_INL int8_t fromJSON( const Json::Value& json )
    { return json.asInt(); }
template<> ZEROBUF_INL uint8_t fromJSON( const Json::Value& json )
    { return json.asUInt(); }
template<> ZEROBUF_INL int16_t fromJSON( const Json::Value& json )
    { return json.asInt(); }
template<> ZEROBUF_INL uint16_t fromJSON( const Json::Value& json )
    { return json.asUInt(); }
template<> ZEROBUF_INL int32_t fromJSON( const Json::Value& json )
    { return json.asInt(); }
template<> ZEROBUF_INL uint32_t fromJSON( const Json::Value& json )
    { return json.asUInt(); }
template<> ZEROBUF_INL int64_t fromJSON( const Json::Value& json )
    { return json.asInt64(); }
template<> ZEROBUF_INL uint64_t fromJSON( const Json::Value& json )
    { return json.asUInt64(); }
template<> ZEROBUF_INL float fromJSON( const Json::Value& json )
    { return json.asFloat(); }
template<> ZEROBUF_INL double fromJSON( const Json::Value& json )
    { return json.asDouble(); }
template<> ZEROBUF_INL std::string fromJSON( const Json::Value& json )
    { return json.asString(); }
template<> ZEROBUF_INL uint128_t fromJSON( const Json::Value& json )
    { return { json[ "high" ].asUInt64(), json[ "low" ].asUInt64( )}; }

template< class T > void toJSON( const T& value, Json::Value& json )
    { json = Json::Value( value ); }
template ZEROBUF_INL void toJSON( const bool&, Json::Value& );
template ZEROBUF_INL void toJSON( const int8_t&, Json::Value& );
template ZEROBUF_INL void toJSON( const int16_t&, Json::Value& );
template ZEROBUF_INL void toJSON( const int32_t&, Json::Value& );
template ZEROBUF_INL void toJSON( const uint32_t&, Json::Value& );
template ZEROBUF_INL void toJSON( const float&, Json::Value& );
template ZEROBUF_INL void toJSON( const double&, Json::Value& );
template ZEROBUF_INL void toJSON( const std::string&, Json::Value& );
template<> ZEROBUF_INL void toJSON( const uint8_t& value, Json::Value& json )
    { json = Json::UInt( value ); }
template<> ZEROBUF_INL void toJSON( const uint16_t& value, Json::Value& json )
    { json = Json::UInt( value ); }
template<> ZEROBUF_INL void toJSON( const int64_t& value, Json::Value& json )
    { json = Json::Int64( value ); }
template<> ZEROBUF_INL void toJSON( const uint64_t& value, Json::Value& json )
    { json = Json::UInt64( value ); }
template<> ZEROBUF_INL void toJSON( const uint128_t& value, Json::Value& json )
{
    json[ "high" ] = Json::UInt64( value.high( ));
    json[ "low" ] = Json::UInt64( value.low( ));
}

void emptyJSONArray( Json::Value& value )
{
    value = Json::Value( Json::arrayValue );
}

void fromJSON( const Json::Value& json, Zerobuf& zerobuf )
{
    zerobuf._parseJSON( json );
}

void toJSON( const Zerobuf& zerobuf, Json::Value& json )
{
    zerobuf._createJSON( json );
}

std::string fromJSONBinary( const Json::Value& json )
{
    return base64_decode( json.asString( ));
}

void toJSONBinary( const uint8_t* data, const size_t size, Json::Value& json )
{
    json = Json::Value( base64_encode( data, uint32_t( size )));
}

const Json::Value& getJSONField( const Json::Value& json,
                                 const std::string& field )
{
    return json[ field ];
}

Json::Value& getJSONField( Json::Value& json, const std::string& field )
{
    return json[ field ];
}

const Json::Value& getJSONField( const Json::Value& json, const size_t index )
{
    return json[ unsigned( index )];
}

Json::Value& getJSONField( Json::Value& json, const size_t index )
{
    return json[ unsigned( index )];
}

size_t getJSONSize( const Json::Value& json )
{
    return json.size();
}

}
