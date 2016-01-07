ZeroBuf
=======

[TOC]

# Overview

ZeroBuf is a replacement for FlatBuffers, resolving the following
shortcomings:

* Direct get and set functionality on the defined data members
* A single memory buffer storing all data members, which is directly
  serializable
* Usable, random read and write access to the the data members
* Zero copy of the data used by the (C++) implementation from and to the network

# Features

* Storage of (u)int[8,16,32,64,128]_t, float, double single elements,
  static and dynamic sub-structures, fixed size and dynamic arrays of
  static-sized elements
* Access to arrays using raw pointers, iterators, std::array,
  std::string and std::vector

# Extensions to flatbuffers grammar

* Arrays can have an optional fixed size specified as part of the type,
  e.g., ```matrix:[float:16]``` for a 16 value float array
