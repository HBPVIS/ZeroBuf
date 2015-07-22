
/* Copyright (c) 2015, Human Brain Project
 *                     Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_TYPES_H
#define ZEROBUF_TYPES_H

#include <servus/types.h>
#include <servus/uint128_t.h>

/**
 * ZeroBuf is a replacement for FlatBuffers/protobuf.
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
}

#endif
