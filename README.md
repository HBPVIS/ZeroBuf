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
* Access to arrays using raw pointers, iterators, std::array,
  std::string and std::vector

# Extensions to flatbuffers grammar

* Arrays can have an optional fixed size specified as part of the type,
  e.g., ```matrix:[float:16]``` for a 16 value float array
