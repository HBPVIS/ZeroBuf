
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_JSON_H
#define ZEROBUF_JSON_H

#include <zerobuf/api.h>
#include <zerobuf/types.h>

namespace zerobuf
{
/** @cond IGNORE functions used by generated JSON code. Needed to keep jsoncpp
 * and base64 API internal */
ZEROBUF_API const Json::Value& getJSONField( const Json::Value& json,
                                             const std::string& field );
ZEROBUF_API Json::Value& getJSONField( Json::Value& json,
                                       const std::string& field );
ZEROBUF_API const Json::Value& getJSONField( const Json::Value& json,
                                             size_t index );
ZEROBUF_API Json::Value& getJSONField( Json::Value& json, size_t index );
ZEROBUF_API size_t getJSONSize( const Json::Value& json );

template< class T > T fromJSON( const Json::Value& json );
template< class T > void toJSON( const T& value, Json::Value& json);
ZEROBUF_API void fromJSON( const Json::Value& json, Zerobuf& zerobuf );
ZEROBUF_API void toJSON( const Zerobuf& zerobuf, Json::Value& json );
ZEROBUF_API std::string fromJSONBinary( const Json::Value& json );
ZEROBUF_API void toJSONBinary( const uint8_t* data, const size_t size,
                               Json::Value& json );
ZEROBUF_API void emptyJSONArray( Json::Value& value );
/** @endcond */
}

#endif
