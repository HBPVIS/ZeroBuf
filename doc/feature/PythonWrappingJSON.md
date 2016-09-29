Python wrapping using JSON {#pythonjson}
============

In order to provide support for accessing and modifiying ZeroBuf objects from
within python, this document describes the approach of generating a python class
per ZeroBuf object that can be serialized using JSON. This allows exchanging and
updating ZeroBuf objects between different applications and languages.

## Requirements

* Code-generation of python module named 'namespace' containing a python class
  per schema.
* Python class supports serialization through JSON and provides the same
  semantics as the C++ generated version.

## New dependencies

* None; python is already a dependency

## API

New zerobuf_generate_python() in new zerobufGeneratePython.cmake

One generated python class per fbs table, for instance:

    namespace mymodule;
    table Foo {
      field1: string;
      field2: bool;
    }

leads to:

    import json
    class Foo(object):
        def __init__(self):
            self._data = {}
            self._data['field1'] = 'bar'
            self._data['field2'] = False

        \@property
        def field1(self):
            return self._data['field1']

        @field1.setter
        def field1(self, value):
            self._data['field1'] = value

        \@property
        def field2(self):
            return self._data['field2']

        @field2.setter
        def field2(self, value):
            self._data['field2'] = value

        def to_JSON(self):
            return json.dumps(self._data)

        def from_JSON(self, data):
            self._data = json.loads(data)

        def __str__(self):
            return self.to_JSON()

## Examples

    import Foo from mymodule

    foo = Foo()
    foo.field1 = 'bla'
    data = json.loads(foo.to_JSON())
    data['field2'] = True
    foo.from_JSON(json.dumps(data))
    print(foo)

## Implementation

* New code generator 'zerobufPython.py' outputs python class, analogue to C++
*
