
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#ifndef ZEROBUF_GENERIC_H
#define ZEROBUF_GENERIC_H

#include <zerobuf/api.h>
#include <zerobuf/Zerobuf.h> // base class
#include <zerobuf/Schema.h> // member

namespace zerobuf
{

/**
 * A ZeroBuf object which holds values described by the given schemas.
 *
 * The main purpose is to create an object at runtime for JSON conversion using
 * toJSON() and fromJSON() without having the schema file nor the generated
 * class file available. Does not provide any semantic methods.
 */
class Generic : public Zerobuf
{
public:
    ZEROBUF_API explicit Generic( const Schemas& schemas );

    ZEROBUF_API uint128_t getZerobufType() const final;
    ZEROBUF_API size_t getZerobufStaticSize() const final;
    ZEROBUF_API size_t getZerobufNumDynamics() const final;
    ZEROBUF_API Schemas getSchemas() const final;

private:
    const Schemas _schemas;
};

}

#endif
