
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Stefan.Eilemann@epfl.ch
 */

#ifndef ZEROBUF_TYPES_H
#define ZEROBUF_TYPES_H

#include <memory>
#include <servus/serializable.h> // nested Data class
#include <servus/types.h>

/**
 * Zero-copy, zero-serialize, zero-hassle protocol buffers.
 *
 * Zerobuf uses a schema language (currently FlatBuffers .fbs) generating C++
 * classes which provide direct get and set access on the defined data members;
 * a single, in-memory buffer storing all data members, which is directly
 * serializable; Easy to use, random access to the the data members; and zero
 * copy of the data used by the (C++) implementation from and to the network.
 */
namespace zerobuf
{
class Allocator;
class NonMovingAllocator;
class NonMovingBaseAllocator;
class Zerobuf;
template <class T>
class Vector;

typedef std::unique_ptr<Allocator> AllocatorPtr;
typedef std::unique_ptr<const Allocator> ConstAllocatorPtr;

using servus::uint128_t;
typedef uint8_t byte_t; //!< alias type for base64 encoded fields

typedef servus::Serializable::Data Data;

template <typename T>
std::string enum_to_string(const T&);
template <typename T>
T string_to_enum(const std::string&);
}

namespace Json
{
class Value;
}

#endif
