
/* Copyright (c) 2016, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#include "Schema.h"
#include <zerobuf/version.h>

namespace zerobuf
{
const uint128_t& Schema::ZEROBUF_TYPE()
{
    static const uint128_t type =
        servus::make_uint128( "zerobuf::Schema" +
                              std::to_string( ZEROBUF_VERSION_ABI ));
    return type;
}

bool Schema::operator == ( const Schema& rhs ) const
{
    if( this == &rhs )
        return true;
    return staticSize == rhs.staticSize && numDynamics == rhs.numDynamics &&
        type == rhs.type && fields == rhs.fields;
}

bool Schema::operator != ( const Schema& rhs ) const
{
    return !( *this == rhs );
}

}
