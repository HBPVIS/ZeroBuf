
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#ifndef ZEROBUF_GENERIC_H
#define ZEROBUF_GENERIC_H

#include <zerobuf/api.h>
#include <zerobuf/Zerobuf.h> // base class
#include <zerobuf/Schema.h> // member
#include <zerobuf/NonMovingAllocator.h>

namespace zerobuf
{

/**
 * A ZeroBuf object which can hold values that are described by the given
 * schema.
 *
 * The main purpose is to create an object at runtime where the values can be
 * set and accessed via JSON w/o having the schema file nor the generated class
 * file available. Hence it does not provide any semantic methods.
 */
template< class Alloc = NonMovingAllocator >
class GenericBase : public Zerobuf
{
public:
    ZEROBUF_API GenericBase( const Schema& schema );

    ZEROBUF_API servus::uint128_t getZerobufType() const final;
    ZEROBUF_API Schema getSchema() const final;

private:
    const Schema _schema;
};

typedef GenericBase< NonMovingAllocator > Generic;

}

#endif
