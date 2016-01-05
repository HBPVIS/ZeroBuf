# Zerobuf Binary Encoding {#Binary}

Nomenclature: A Zerobuf object is the instantiation of a class compiled
from a schema file by the Zerobuf compiler. A zerobuf (lower case) is
the memory buffer of a Zerobuf object. It is a single memory allocation,
i.e., a pointer with a size, and can be serialized as is without any
additional transformation.

Zerobuf objects save all their member data in a single memory buffer in
host endianness. It is the receiving node's responsibility to correct
the endianness in a received zerobuf. The zerobuf binary format doubles
as the serialization protocol and needs to be implemented by all
language bindings correctly. Implementations may differ in their memory
allocation strategy.

The zerobuf binary format is as follows, and will be explained below:

    [version][dynamic storage headers][static storage][dynamic storage]
      dynamic storage headers: 8b offset, 8b size
        for all dynamic objects in order of spec
          implemented: dynamic arrays of builtins and static objects, strings
          not implemented: arrays of dynamic objects, dynamic sub-classes
      static storage: 1,2,4,8,16b (* static array size)
        for all static arrays and variables in order of spec
        Builtin types are: int (4b), uint (4b), float (4b), double (8b),
          byte (1b), short (2b), ubyte (1b), ushort (2b), ulong (8b),
          uint8_t (1b), uint16_t (2b), uint32_t (4b), uint64_t (8b),
          uint128_t (16b), int8_t (1b), int16_t (2b), int32_t (4b), int64_t
          (8b), bool (1b)
      dynamic storage layout is an implementation detail of the Allocator

A zerobuf always contains a static storage area, and optionally a
dynamic storage area for objects with dynamically-sized members. The
static storage section is at the beginning of the zerobuf. It contains a
version, followed by the headers for dynamic members and the storage for
static data. The number of dynamic storage headers and the layout of the
static storage section follows directly from the schema. The schema is
defined by the .fbs input file and available as the Schema structure
from each Zerobuf object.

The version is used to detect zerobuf format changes and the producer
endianness.

The dynamic storage headers contain an offset and size to point to
memory allocation in the dynamic storage section. The offset is in bytes
from the beginning of the zerobuf. The size is in bytes. The Allocator
may create holes during allocations in the dynamic storage section. A
dynamic allocation is either a dynamic array or a (recursive) zerobuf
managed by a sub allocator. Array allocations only store their data in
the allocations. A dynamic Zerobuf sub-object stores a full zerobuf, as
specified here. Strings ([string] in .fbs) are handled as a dynamic char
array.

The static storage section contains all members whose allocation size
can be determined at schema compile time. This includes builtin types
and static-sized Zerobuf objects, as well as static-sized arrays of the
aforementioned. 128-bit integers are exposed through servus::uint128_t
in the generated C++ API. All other types are exposed by their
corresponding C++ type.
