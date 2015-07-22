ZeroBuf
=======

[TOC]

# Overview

ZeroBuf is a replacement for FlatBuffers, resolving the following
shortcomings:

* Direct get and set functionality on the defined data members
* A single, in-memory buffer storing all data members, which is directly
  serializable
* Usable, random access to the the data members
* Zero copy of the data used by the (C++) implementation from and to the network

# V1 Features

* Storage of (u)int[8,16,32,64,128]_t, float, double single elements, fixed
  size arrays and dynamic arrays
* Access to arrays using raw pointers, iterators, std::array and
  std::string/std::vector

# Implementation and Data Layout

* Header in producer endianness: version (4b)
* Data UUID (16b)
* fixed-elem PODs: stored in-order at start of array
* dynamic arrays/std::vector, std::string:
** offset (8b), size (8b) stored in place
** data at offset after all items at 4b boundary
* static arrays: data in place
* arrays: returned as ptr, iter, copied std::vector
* getter/setter generated with hard-coded offsets
* Nested classes are handled in the same way as dynamic arrays
* saved in an atomic ptr for concurrent reallocs

# Extensions to flatbuffers grammar

* arrays can have an optional fixed size specified as part of the type,
  e.g., ```matrix:[float:16]``` for a 16 value float array
