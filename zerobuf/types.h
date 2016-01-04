
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_TYPES_H
#define ZEROBUF_TYPES_H

#include <servus/types.h>
#include <memory>

/**
 * ZeroBuf is a replacement for FlatBuffers and protobuf.
 *
 * It provides direct get and set functionality on the defined data members; a
 * single, in-memory buffer storing all data members, which is directly
 * serializable; Usable, random access to the the data members; and zero copy of
 * the data used by the (C++) implementation from and to the network.
 */
namespace zerobuf
{
class Allocator;
class Zerobuf;
class NonMovingAllocator;
class NonMovingBaseAllocator;
struct Schema;
class Zerobuf;

typedef std::unique_ptr< Allocator > AllocatorPtr;
typedef std::unique_ptr< const Allocator > ConstAllocatorPtr;

template< class T > class Vector;

typedef std::vector< Schema > Schemas;

using servus::uint128_t;
}

namespace Json
{
class Value;
}

#endif
