
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#ifndef ZEROBUF_SCHEMA_H
#define ZEROBUF_SCHEMA_H

#include <zerobuf/api.h>
#include <zerobuf/NonMovingAllocator.h>
#include <servus/uint128_t.h>
#include <tuple>

namespace zerobuf
{

/**
 * A runtime and optimized representation of the schema file input, which
 * describes the data layout of a ZeroBuf object.
 */
struct Schema
{
    /**
     * The static size of the object.
     * @sa NonMovingBaseAllocator
     */
    const size_t staticSize;

    /**
     * The number of dynamic fields in the object.
     * @sa NonMovingBaseAllocator
     */
    const size_t numDynamic;

    /**
     * The type identifier of the object.
     * @sa ZeroBuf::getZerobufType()
     */
    const servus::uint128_t type;

    /** Access enums for the field tuple in the object.*/
    enum FieldEnum
    {
        FIELD_NAME,       //!< the name of the field
        FIELD_TYPE,       //!< the type of the field
        FIELD_DATAOFFSET, //!< the data offset in the allocator
        FIELD_SIZE,       /**< size of static arrays, size offset in the
                               allocator for dynamic arrays */
        FIELD_ISSTATIC,    //!< bool if the field is static or not
        FIELD_SCHEMAFUNC    //!< schema function pointer
    };

    typedef std::function<Schema()> SchemaFunction;

    /** Contains the necessary information to describe one object field. */
    typedef std::tuple< std::string,
                        std::string,
                        size_t,
                        size_t,
                        bool,
                        SchemaFunction> Field;

    /** All the fields in object */
    const std::vector< Field > fields;
};



}

#endif
