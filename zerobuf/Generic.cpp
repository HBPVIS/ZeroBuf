
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#include "Generic.h"
#include <zerobuf/NonMovingAllocator.h>

namespace zerobuf
{

Generic::Generic( const Schemas& schemas )
    : Zerobuf( AllocatorPtr(
                   new NonMovingAllocator( schemas.front().staticSize,
                                           schemas.front().numDynamics )))
    , _schemas( schemas )
{}

uint128_t Generic::getZerobufType() const
{
    return _schemas.front().type;
}

size_t Generic::getZerobufStaticSize() const
{
    return _schemas.front().staticSize;
}

size_t Generic::getZerobufNumDynamics() const
{
    return _schemas.front().numDynamics;
}

Schemas Generic::getSchemas() const
{
    return _schemas;
}

}
