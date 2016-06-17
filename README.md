ZeroBuf
=======

# Overview

ZeroBuf implements zero-copy, zero-serialize, zero-hassle protocol
buffers. It is a replacement for FlatBuffers, resolving the following
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
* Conversion to and from a JSON representation

# Extensions to flatbuffers grammar

* Arrays can have an optional fixed size specified as part of the type,
  e.g., ```matrix:[float:16]``` for a 16 value float array
* byte and ubyte data is base64 encoded in JSON, int8_t and uint8_t are
  represented as value arrays

This work has been partially funded by the European Union Seventh Framework Program (FP7/2007­2013) under grant agreement no. 604102 (HBP).
