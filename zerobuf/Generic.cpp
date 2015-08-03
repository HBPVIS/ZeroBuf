
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel.Nachbaur@epfl.ch
 */

#include "Generic.h"

namespace zerobuf
{

template< class Alloc >
GenericBase< Alloc >::GenericBase( const Schema& schema )
    : Zerobuf( new Alloc( schema.staticSize, schema.numDynamic ))
    , _schema( schema )
{}

template< class Alloc >
servus::uint128_t GenericBase< Alloc >::getZerobufType() const
{
    return _schema.type;
}

template< class Alloc >
Schema GenericBase< Alloc >::getSchema() const
{
    return _schema;
}

template class GenericBase< NonMovingAllocator >;

}
