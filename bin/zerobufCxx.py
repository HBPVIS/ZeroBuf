#!/usr/bin/env python

# TODO:
# * nested dynamic tables
# * endian swap method

# UUID is MD5 hash of namespace::namespace[<cxxtype>|<cxxtype>*|<cxxtype><size>]
# See @ref Binary for a description of the memory layout

import argparse
import hashlib
import os
import re
import sys
from collections import namedtuple

TypeDescription = namedtuple("TypeDescription", "size cxxtype")
DEFAULT_TYPES = {"int" : TypeDescription(4, "int32_t"),
                 "uint" : TypeDescription(4, "uint32_t"),
                 "float" : TypeDescription(4, "float"),
                 "double" : TypeDescription(8, "double"),
                 "byte" : TypeDescription(1, "uint8_t"),
                 "short" : TypeDescription(2, "int16_t"),
                 "ubyte" : TypeDescription(1, "uint8_t"),
                 "ushort" : TypeDescription(2, "uint16_t"),
                 "ulong" : TypeDescription(8, "uint64_t"),
                 "uint8_t" : TypeDescription(1, "uint8_t"),
                 "uint16_t" : TypeDescription(2, "uint16_t"),
                 "uint32_t" : TypeDescription(4, "uint32_t"),
                 "uint64_t" : TypeDescription(8, "uint64_t"),
                 "uint128_t" : TypeDescription(16, "::zerobuf::uint128_t"),
                 "int8_t" : TypeDescription(1, "int8_t"),
                 "int16_t" : TypeDescription(2, "int16_t"),
                 "int32_t" : TypeDescription(4, "int32_t"),
                 "int64_t" : TypeDescription(8, "int64_t"),
                 "bool" : TypeDescription(1, "bool"),
                 "string" : TypeDescription(1, "char*"),
                 }

NEXTLINE = "\n    "

def create_FBS_parser():
    from pyparsing import (oneOf, Group, ZeroOrMore, Word, alphanums, Keyword,
                           Suppress, Optional, OneOrMore, Literal, nums, Or,
                           alphas, cppStyleComment)

    fbsBaseType = oneOf(list(DEFAULT_TYPES.keys()))

    # namespace foo.bar
    fbsNamespaceName = Group( ZeroOrMore( Word( alphanums ) + Suppress( '.' )) +
                             Word( alphanums ))
    fbsNamespace = Group( Keyword( "namespace" ) + fbsNamespaceName +
                         Suppress( ';' ))

    # enum EventDirection : ubyte { Subscriber, Publisher, Both }
    fbsEnumValue = ( Word( alphanums+"_" ) + Suppress( Optional( ',' )))
    fbsEnum = Group( Keyword( "enum" ) + Word( alphanums ) + Suppress( ':' ) +
                    fbsBaseType + Suppress( '{' ) + OneOrMore( fbsEnumValue ) +
                    Suppress( '}' ))

    # value:[type] = defaultValue; entries in table
    # TODO: support more default values other than numbers and booleans
    fbsType = ( fbsBaseType ^ Word( alphanums ))
    fbsTableArray = ( ( Literal( '[' ) + fbsType + Literal( ']' )) ^
                     ( Literal( '[' ) + fbsType + Literal( ':' ) + Word( nums ) +
                      Literal( ']' )) )
    fbsTableValue = ((fbsType ^ fbsTableArray) +
                     ZeroOrMore(Suppress('=') + Or([Word("true"), Word("false"), Word(nums+"-. ,")])))
    fbsTableEntry = Group( Word( alphanums+"_" ) + Suppress( ':' ) + fbsTableValue +
                          Suppress( ';' ))
    fbsTableSpec = ZeroOrMore( fbsTableEntry )

    # table Foo { entries }
    fbsTable = Group( Keyword( "table" ) + Word( alphas, alphanums ) +
                    Suppress( '{' ) + fbsTableSpec + Suppress( '}' ))

    # root_type foo;
    fbsRootType = Group( Keyword( "root_type" ) + Word( alphanums ) +
                        Suppress( ";" ))

    # namespace, table(s), root_type
    fbsItem = Or([fbsEnum, fbsTable])
    fbsObject = ( Optional( fbsNamespace ) + OneOrMore( fbsItem ) +
                Optional( fbsRootType ))

    fbsComment = cppStyleComment
    fbsObject.ignore( fbsComment )

    #fbsTableArray.setDebug()
    #fbsTableValue.setDebug()
    #fbsTableEntry.setDebug()
    return fbsObject


class ValueType():
    """The value type of a C++ member"""

    def __init__(self, type, size, is_zerobuf_type=False, is_enum_type=False, is_byte_type=False):
        self.type = type
        self.size = size
        self.is_zerobuf_type = is_zerobuf_type
        self.is_enum_type = is_enum_type
        self.is_byte_type = is_byte_type
        self.is_string = (type == "char*")
        if self.is_string:
            self.type = "char"
            self.size = 1

    def get_data_type(self):
        return "uint32_t" if self.is_zerobuf_type or self.is_enum_type else self.type


class DoxygenDoc():
    """Doxygen documentation for C++ functions"""
    def __init__(self, brief="", params=[], ret=""):
        self.brief = brief
        self.params = params
        self.ret = ret

    def to_string(self):
        dox = "/**" + NEXTLINE
        for i in range(len(self.brief)):
            dox += " * " + self.brief[i] + NEXTLINE
        dox += " * " + NEXTLINE
        for i in range(len(self.params)):
            dox += " * @param " + self.params[i] + NEXTLINE

        if self.ret:
            dox += " * @return " + self.ret + NEXTLINE
        return dox + " **/"


class Function():
    """A C++ Function"""
    def __init__(self, ret_val, function, body, doxygen=None, static=False,
                 explicit=False, virtual=False, split=True):
        self.ret_val = ret_val
        self.function = function
        self.body = body
        self.doxygen = doxygen
        self.static = "static " if static else ""
        self.explicit = "explicit " if explicit else ""
        self.virtual = "virtual " if virtual else ""
        self.split_implementation = split

    def declaration(self):
        if self.ret_val:
            return "{0}{1} {2} {{ {3} }}".format( self.static, self.ret_val,
                                                  self.function, self.body )
        # ctor '[initializer list]{ body }'
        return "{0}{1}{2} {3}".format( self.virtual, self.explicit,
                                       self.function, self.body )

    def definition(self):
        if self.ret_val:
            return "{0}{1} {2};".format( self.static, self.ret_val,
                                         self.function, self.body )
        # ctor '[initializer list]{ body }'
        return "{0}{1}{2};".format( self.virtual, self.explicit, self.function )

    def write_declaration(self, file):
        if self.doxygen:
            file.write(NEXTLINE + self.doxygen.to_string())
        if self.split_implementation:
            file.write(NEXTLINE + self.definition())
        else:
            file.write(NEXTLINE + self.declaration())

    def write_implementation(self, file, classname, extra_op = ""):
        if not self.split_implementation:
            return

        impl_function = re.sub(r" final$", "", self.function) # remove ' final' keyword
        impl_function = re.sub(r" = [0-9\.f]+ ", " ", impl_function) # remove default params

        if self.ret_val: # '{}'-less body
            file.write("\n" + self.ret_val + " " + classname +
                       "::" + impl_function +
                       "\n{" +
                       NEXTLINE + self.body)
            if extra_op:
                file.write(NEXTLINE + extra_op)
            file.write("\n}\n")
        else:      # ctor '[initializer list]{ body }'
            file.write("\n" + classname + "::" + impl_function +
                       NEXTLINE + self.body + "\n")


class ClassMember(object):
    """A member of a C++ class"""

    def __init__(self, name, value_type):
        assert(isinstance(value_type, ValueType))
        self.cxxname = name
        self.cxxName = name[0].upper() + str(name[1:])
        self.value_type = value_type
        self.allocator_offset = 0
        self.qsignal_declaration = self.cxxname + "Changed();"
        self.qsignal = self.cxxname + "Changed();"

    def write_typedefs(self, file):
        """Derived classes may implement this function to declare typedefs"""
        return

    def write_accessors_declaration(self, file):
        self.write_typedefs(file)
        for function in self.accessor_functions():
            function.write_declaration(file)

    def write_qt_getters(self, file):
        self.write_typedefs(file)
        for function in self.const_getters():
            function.write_declaration(file)

    def write_qt_setters(self, file):
        for function in self.setters():
            function.write_declaration(file)

    def write_qt_signals(self, file):
        file.write(NEXTLINE + "void " + self.qsignal_declaration)

    def write_accessors_implementation(self, file, classname, generate_qobject):
        if generate_qobject:
            for function in self.const_getters():
                function.write_implementation(file, classname)
            for function in self.setters(qproperty=True):
                function.write_implementation(file, classname)
        else:
            for function in self.accessor_functions():
                function.write_implementation(file, classname)

    def get_unique_identifier(self):
        return self.value_type.type.encode('utf-8')

    def accessor_functions(self):
        return self.getters() + self.setters()

    def qualified_type(self, classname):
        return "{0}::{1}".format(classname, self.get_cxxtype())

    def check_value_changed(self, value_name):
        return "if( {0} == value )".format(value_name) + NEXTLINE +\
               "    return;" + NEXTLINE

    def emit_value_changed(self, qproperty):
        return NEXTLINE + "emit " + self.qsignal if qproperty else ""

    def const_ref_getter(self, classname=None):
        val_type = self.qualified_type(classname) if classname else self.get_cxxtype()
        return Function("const {0}&".format(val_type),
                        "get" + self.cxxName + "() const",
                        "return _{0};".format(self.cxxname))

    def ref_getter(self, classname=None):
        val_type = self.qualified_type(classname) if classname else self.get_cxxtype()
        return Function("{0}&".format(val_type),
                        "get" + self.cxxName + "()",
                        "return _{0};".format(self.cxxname),
                        DoxygenDoc(["Get a reference to the {0} dynamic member.".format(self.value_type.type),
                                    "WARNING: If the reference is used to modify the object, " +
                                    "notifyChanged() needs to be explicitly called afterwards."],
                                   [], "a reference to the {0} dynamic member.".format(self.value_type.type)))

    def ref_setter(self, qproperty=False):
        current_value = "_{0}".format(self.cxxname)
        return Function("void",
                        "set{0}( const {1}& value )".format(self.cxxName, self.get_cxxtype()),
                        (self.check_value_changed(current_value) if qproperty else "") +
                        "{0} = value;".format(current_value) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} member.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value the {0} value to be set in the current object".format(self.value_type.type)]))


class FixedSizeMember(ClassMember):
    """A member of a class which has a fixed size (such as a POD type)"""

    def __init__(self, name, type):
        super(FixedSizeMember, self).__init__(name, type)
        self.qsignal_declaration = self.cxxname + "Changed( {0} );".format(self.get_cxxtype())
        self.qsignal = self.cxxname + "Changed( get{0}( ));".format(self.cxxName)

    def value_getter(self):
        return Function(self.get_cxxtype(),
                        "get{0}() const".format(self.cxxName),
                        "return getAllocator().template getItem< {0} >( {1} );".\
                        format(self.get_cxxtype(), self.allocator_offset))

    def value_setter(self, qproperty=False):
        current_value = "getAllocator().template getItem< {0} >( {1} )".\
                        format(self.get_cxxtype(), self.allocator_offset)
        return Function("void",
                        "set{0}( {1} value )".format(self.cxxName, self.get_cxxtype()),
                        (self.check_value_changed(current_value) if qproperty else "") +
                        "{0} = value;".format(current_value) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} fixed size member.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value the {0} value to be set in the current object".format(self.value_type.type)]))

    def getters(self):
        if self.value_type.is_zerobuf_type:
            return [self.const_ref_getter(), self.ref_getter()]
        return [self.value_getter()]

    def const_getters(self):
        if self.value_type.is_zerobuf_type:
            return [self.const_ref_getter()]
        return self.getters()

    def setters(self, qproperty=False):
        if self.value_type.is_zerobuf_type:
            return [self.ref_setter(qproperty)]
        return [self.value_setter(qproperty)]

    def get_byte_size(self):
        return self.value_type.size

    def get_cxxtype(self):
        return self.value_type.type

    def get_initializer(self):
        return [self.cxxname, 1, self.value_type.type, self.allocator_offset, self.value_type.size]

    def get_declaration(self):
        return "{0} _{1};".format(self.value_type.type, self.cxxname)

    def from_json(self):
        if self.value_type.is_zerobuf_type:
            return '::zerobuf::fromJSON( ::zerobuf::getJSONField( json, "{0}" ), _{0} );'.\
                format(self.cxxname)
        else:
            return 'set{0}( {1}( ::zerobuf::fromJSON< {2} >( ::zerobuf::getJSONField( json, "{3}" ))));'.\
                format(self.cxxName, self.value_type.type, self.value_type.get_data_type(), self.cxxname)

    def to_json(self):
        if self.value_type.is_zerobuf_type:
            return '::zerobuf::toJSON( static_cast< const ::zerobuf::Zerobuf& >( _{0} ), ::zerobuf::getJSONField( json, "{0}" ));'.\
                format(self.cxxname)
        else:
            return '::zerobuf::toJSON( {0}( get{1}( )), ::zerobuf::getJSONField( json, "{2}" ));'.\
                format(self.value_type.get_data_type(), self.cxxName, self.cxxname)


class FixedSizeArray(ClassMember):
    """A member of a class which is a fixed size array"""

    def __init__(self, name, type, elem_count, classname):
        super(FixedSizeArray, self).__init__(name, type)
        self.nElems = elem_count
        self.classname = classname

        if self.nElems < 2:
            sys.exit( "Static array of size {0} for field {1} not supported".
                      format(self.nElems, self.cxxname))
        if self.value_type.size == 0:
            sys.exit( "Static array of {0} dynamic elements not implemented".
                      format(self.nElems))
        if self.value_type.is_zerobuf_type:
            if self.value_type.size == 0:
                sys.exit("Static arrays of empty ZeroBuf (field {0}) not supported".format(self.cxxname))

    def check_array_changed(self, dst_ptr):
        return "if( ::memcmp( {0}, {1}, {2} * sizeof( {3} )) == 0 )".\
               format(self.data_ptr(), dst_ptr, self.nElems, self.value_type.type) + NEXTLINE +\
               "    return;" + NEXTLINE

    def data_ptr(self):
        return "getAllocator().template getItemPtr< {0} >( {1} )".\
                format(self.value_type.type, self.allocator_offset)

    def size_getter(self):
        return Function("size_t", "get{0}Size() const".format(self.cxxName),
                        "return {0};".format(self.nElems))

    def ptr_getter(self):
        return Function(self.value_type.type + "*",
                        "get" + self.cxxName + "()",
                        "return getAllocator().template getItemPtr< {0} >( {1} );".\
                        format(self.value_type.type, self.allocator_offset),
                        DoxygenDoc(["Get a pointer to the {0} fixed size array object.".format(self.value_type.type),
                                    "WARNING: If the pointer is used to modify the object, " +
                                   "notifyChanged() needs to be explicitly called afterwards."],
                                   [], "a pointer to the {0} fixed size array object({0}*).".format(self.value_type.type)))

    def const_ptr_getter(self):
        return Function("const {0}*".format(self.value_type.type),
                        "get" + self.cxxName + "() const",
                        "return getAllocator().template getItemPtr< {0} >( {1} );".\
                        format(self.value_type.type, self.allocator_offset))

    def vector_getter(self):
        return Function(self.get_cxxtype(),
                        "get{0}Vector() const".format(self.cxxName),
                        "const {0}* ptr = getAllocator().template getItemPtr< {0} >( {1} );".\
                        format(self.value_type.type, self.allocator_offset) + NEXTLINE +
                        "return {0}( ptr, ptr + {1} );".format(self.get_cxxtype(), self.nElems))

    def c_array_setter(self, qproperty=False):
        src_ptr = "value"
        return Function("void",
                        "set{0}( {1} value[ {2} ] )".format(self.cxxName, self.value_type.type, self.nElems),
                        (self.check_array_changed(src_ptr) if qproperty else "") +
                        "::memcpy( {0}, {1}, {2} * sizeof( {3} ));".\
                        format(self.data_ptr(), src_ptr, self.nElems, self.value_type.type) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} fixed size array object from a {0}*.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value a {0}-length {1} array with the data to be set in the current object".format(self.nElems, self.value_type.type)]))

    def vector_setter(self, qproperty=False):
        src_ptr = "value.data()"
        return Function("void",
                        "set{0}( const std::vector< {1} >& value )".format(self.cxxName, self.value_type.type),
                        "if( {0} < value.size( ))".format(self.nElems) + NEXTLINE +
                        "    return;" + NEXTLINE +
                        (self.check_array_changed(src_ptr) if qproperty else "") +
                        "::memcpy( {0}, {1}, value.size() * sizeof( {3} ));".\
                        format(self.data_ptr(), src_ptr, self.nElems, self.value_type.type) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} fixed size array object from a {0} std::vector.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value a std::vector< {0} > with the data to be set in the current object".format(self.value_type.type)]))

    def getters(self):
        if self.value_type.is_zerobuf_type:
            return [self.const_ref_getter(self.classname),
                    self.ref_getter(self.classname),
                    self.size_getter()]
        return [self.ptr_getter(),
                self.const_ptr_getter(),
                self.vector_getter(),
                self.size_getter()]

    def const_getters(self):
        if self.value_type.is_zerobuf_type:
            return [self.const_ref_getter(self.classname),
                    self.size_getter()]
        return [self.const_ptr_getter(),
                self.vector_getter(),
                self.size_getter()]

    def setters(self, qproperty=False):
        if self.value_type.is_zerobuf_type:
            return [self.ref_setter(qproperty)]
        return [self.c_array_setter(qproperty),
                self.vector_setter(qproperty)]

    def accessor_functions(self):
        """Override ClassMember.accessor_functions for legacy ordering of functions"""
        if self.value_type.is_zerobuf_type:
            return [self.const_ref_getter(self.classname),
                    self.ref_getter(self.classname),
                    self.ref_setter(),
                    self.size_getter()]
        # Dynamic array of PODs
        return [self.ptr_getter(),
                self.const_ptr_getter(),
                self.vector_getter(),
                self.c_array_setter(),
                self.vector_setter(),
                self.size_getter()]

    def get_byte_size(self):
        return self.value_type.size * self.nElems

    def get_unique_identifier(self):
        return super(FixedSizeArray, self).get_unique_identifier() + str(self.nElems).encode('utf-8')

    def get_cxxtype(self):
        if self.value_type.is_zerobuf_type:
            # use the typedef for the std::array< T >
            return self.cxxName
        # static array of POD
        return "std::vector< {0} >".format(self.value_type.type)

    def write_typedefs(self, file):
        if self.value_type.is_zerobuf_type:
            file.write(NEXTLINE + "typedef std::array< {0}, {1} > {2};".
                       format(self.value_type.type, self.nElems, self.cxxName))

    def get_initializer(self):
        return [self.cxxname, self.nElems, self.value_type.type, self.allocator_offset, self.value_type.size]

    def get_declaration(self):
        return "{0} _{1};".format(self.cxxName, self.cxxname)

    def from_json(self):
        fromJSON = "{"
        fromJSON += NEXTLINE + '   const Json::Value& field = ::zerobuf::getJSONField( json, "{0}" );'.format(self.cxxname)

        if self.value_type.is_zerobuf_type and not self.value_type.is_enum_type:
            for i in range(0, self.nElems):
                fromJSON += NEXTLINE + "    ::zerobuf::fromJSON( ::zerobuf::getJSONField( field, {1} ), _{0}[{1}] );".\
                    format(self.cxxname, i)
        else:
            fromJSON += NEXTLINE + "    {0}* array = ({0}*)get{1}();".\
                format(self.value_type.get_data_type(), self.cxxName)

            if self.value_type.is_byte_type:
                fromJSON += NEXTLINE + "    const std::string& decoded = ::zerobuf::fromJSONBinary( field );"
                fromJSON += NEXTLINE + "    ::memcpy( array, decoded.data(), std::min( decoded.length(), size_t( {0}ull )));".format(self.nElems)
            else:
                for i in range(0, self.nElems):
                    fromJSON += NEXTLINE + "    array[{0}] = ::zerobuf::fromJSON< {1} >( ::zerobuf::getJSONField( field, {0} ));".\
                        format(i, self.value_type.get_data_type())

        fromJSON += NEXTLINE + "}"
        return fromJSON

    def to_json(self):
        toJSON = "{"
        toJSON += NEXTLINE + '    Json::Value& field = ::zerobuf::getJSONField( json, "{0}" );'.\
            format(self.cxxname)

        if self.value_type.is_zerobuf_type and not self.value_type.is_enum_type:
            for i in range(0, self.nElems):
                toJSON += NEXTLINE + "    ::zerobuf::toJSON( static_cast< const ::zerobuf::Zerobuf& >( _{0}[{1}] ), ::zerobuf::getJSONField( field, {1} ));".\
                    format(self.cxxname, i)
        else:
            toJSON += NEXTLINE + "    const {0}* array = (const {0}*)get{1}();".\
                format(self.value_type.get_data_type(), self.cxxName)

            if self.value_type.is_byte_type:
                toJSON += NEXTLINE + "    ::zerobuf::toJSONBinary( array, {0}, field );".format(self.nElems)
            else:
                for i in range(0, self.nElems):
                    toJSON += NEXTLINE + "    ::zerobuf::toJSON( array[{0}], ::zerobuf::getJSONField( field, {0} ));".format(i)
        toJSON += NEXTLINE + "}"
        return toJSON


class DynamicZeroBufMember(ClassMember):
    """A member of a class which has a dynamic size and is a ZeroBuf type"""

    def __init__(self, name, type, dynamic_type_index):
        super(DynamicZeroBufMember,self).__init__(name, type)
        self.dynamic_type_index = dynamic_type_index

    def getters(self):
        return [self.ref_getter(),
                self.const_ref_getter()]

    def const_getters(self):
        return [self.const_ref_getter()]

    def setters(self, qproperty=False):
        return [self.ref_setter(qproperty)]

    def get_byte_size(self):
        return 16 # 8b offset, 8b size

    def get_cxxtype(self):
        # dynamic Zerobuf member, use the ZeroBuf type
        return self.value_type.type

    def get_initializer(self):
        return [self.cxxname, 1, self.value_type.type, self.dynamic_type_index, 0]

    def get_declaration(self):
        return "{0} _{1};".format(self.value_type.type, self.cxxname)

    def from_json(self):
        return '::zerobuf::fromJSON( ::zerobuf::getJSONField( json, "{0}" ), _{0} );'.\
            format(self.cxxname)

    def to_json(self):
        return '::zerobuf::toJSON( static_cast< const ::zerobuf::Zerobuf& >( _{0} ), ::zerobuf::getJSONField( json, "{0}" ));'.\
            format(self.cxxname)


class DynamicMember(ClassMember):
    """A member of a class which has a dynamic size (vector or string type)"""

    def __init__(self, name, type, dynamic_type_index, classname):
        super(DynamicMember, self).__init__(name, type)
        self.dynamic_type_index = dynamic_type_index
        self.classname = classname

        if self.value_type.is_zerobuf_type: # Dynamic array of (static) Zerobufs
            if self.value_type.size == 0:
                sys.exit("Dynamic arrays of empty ZeroBuf (field {0}) not supported".format(self.cxxname))

        if self.value_type.is_string:
            self.qsignal_declaration = self.cxxname + "Changed( QString );"
            self.qsignal = self.cxxname + "Changed( QString::fromLatin1( get{0}().data( )));".format(self.cxxName)

    def vector_dynamic_getter(self):
        return Function(self.vector_type(),
                        "get{0}Vector() const".format(self.cxxName),
                        "const {0}& vec = get{0}();".format(self.cxxName) + NEXTLINE +
                        "{0} ret;".format(self.vector_type()) + NEXTLINE +
                        "ret.reserve( vec.size( ));" + NEXTLINE +
                        "for( size_t i = 0; i < vec.size(); ++i )" + NEXTLINE +
                        "    ret.push_back( vec[i] );" + NEXTLINE +
                        "return ret;")

    def vector_dynamic_setter(self, qproperty=False):
        current_value = "get{0}Vector()".format(self.cxxName)
        return Function("void",
                        "set{0}( const {1}& value )".format(self.cxxName, self.vector_type()),
                        (self.check_value_changed(current_value) if qproperty else "") +
                        "::zerobuf::Vector< {0} > dynamic( getAllocator(), {1} );".\
                        format(self.value_type.type, self.dynamic_type_index) + NEXTLINE +
                        "dynamic.clear();" + NEXTLINE +
                        "for( const " + self.value_type.type + "& data : value )" + NEXTLINE +
                        "    dynamic.push_back( data );" + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} dynamic object from a {0} std::vector.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value a std::vector< {0} > object with the data to be set in the current object".format(self.value_type.type)]))

    def check_c_array_changed(self):
        return "if( ::memcmp( _{0}.data(), value, size * sizeof( {1} )) == 0 )".\
               format(self.cxxname, self.value_type.type) + NEXTLINE +\
               "    return;" + NEXTLINE

    def c_pointer_setter(self, qproperty=False):
        return Function("void",
                        "set{0}( {1} const * value, size_t size )".\
                        format(self.cxxName, self.value_type.type),
                        (self.check_c_array_changed() if qproperty else "") +
                        "_copyZerobufArray( value, size * sizeof( {0} ), {1} );".\
                        format(self.value_type.type, self.dynamic_type_index) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} dynamic object from a {0}* and size.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value a pointer to the data to be set in the current object".format(self.value_type.type),
                                    "size the size of the data to be set"]))

    def vector_pod_getter(self):
        return Function(self.vector_type(),
                        "get{0}Vector() const".format(self.cxxName),
                        "return {0}( _{1}.data(), _{1}.data() + _{1}.size( ));".\
                        format(self.vector_type(), self.cxxname))

    def vector_pod_setter(self, qproperty=False):
        current_value = "get{0}Vector()".format(self.cxxName)
        return Function("void",
                        "set{0}( const {1}& value )".format(self.cxxName, self.vector_type()),
                        (self.check_value_changed(current_value) if qproperty else "") +
                        "_copyZerobufArray( value.data(), value.size() * sizeof( {0} ), {1} );".\
                        format(self.value_type.type, self.dynamic_type_index) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} dynamic object from a {0} std::vector.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                    ["value a std::vector< {0} > object with the data to be set in the current object".format(self.value_type.type)]))

    def string_getter(self):
        return Function("std::string",
                        "get{0}String() const".format(self.cxxName),
                        "const uint8_t* ptr = getAllocator().template getDynamic< const uint8_t >( {0} );".\
                        format(self.dynamic_type_index) + NEXTLINE +
                        "return std::string( ptr, ptr + getAllocator().template getItem< uint64_t >( {0} ));".\
                        format(self.allocator_offset + 8))

    def string_setter(self, qproperty=False):
        current_value = "get{0}String()".format(self.cxxName)
        return Function("void",
                        "set{0}( const std::string& value )".format(self.cxxName),
                        (self.check_value_changed(current_value) if qproperty else "") +
                        "_copyZerobufArray( value.c_str(), value.length(), {0} );".\
                        format(self.dynamic_type_index) + NEXTLINE +
                        "notifyChanged();" + self.emit_value_changed(qproperty),
                        DoxygenDoc(["Set the value of the {0} dynamic object from a std::string.".format(self.value_type.type),
                                    "notifyChanged() is internally called after the change has been done."],
                                   ["value a std::string with the data to be set in the current object"]))

    def getters(self):
        if self.value_type.is_zerobuf_type: # Dynamic array of (static) Zerobufs
            return [self.ref_getter(self.classname),
                    self.const_ref_getter(self.classname),
                    self.vector_dynamic_getter()]

        if self.value_type.is_string:
            return [self.ref_getter(self.classname),
                    self.const_ref_getter(self.classname),
                    self.string_getter()]

        # Dynamic array of PODs
        return [self.ref_getter(self.classname),
                self.const_ref_getter(self.classname),
                self.vector_pod_getter()]

    def const_getters(self):
        if self.value_type.is_zerobuf_type: # Dynamic array of (static) Zerobufs
            return [self.const_ref_getter(self.classname),
                    self.vector_dynamic_getter()]
        # Dynamic array of PODs
        return [self.const_ref_getter(self.classname),
                self.vector_pod_getter()]

    def setters(self, qproperty=False):
        if self.value_type.is_zerobuf_type: # Dynamic array of (static) Zerobufs
            return [self.vector_dynamic_setter(qproperty)]

        if self.value_type.is_string:
            return [self.c_pointer_setter(qproperty),
                    self.string_setter(qproperty)]

        # Dynamic array of PODs
        return [self.c_pointer_setter(qproperty),
                self.vector_pod_setter(qproperty)]

    def accessor_functions(self):
        """Override ClassMember.accessor_functions for legacy ordering of functions"""
        if self.value_type.is_zerobuf_type: # Dynamic array of (static) Zerobufs
            return self.getters() + self.setters()

        if self.value_type.is_string:
            return [self.ref_getter(self.classname),
                    self.const_ref_getter(self.classname),
                    self.c_pointer_setter(),
                    self.string_getter(),
                    self.string_setter()]

        # Dynamic array of PODs
        return [self.ref_getter(self.classname),
                self.const_ref_getter(self.classname),
                self.c_pointer_setter(),
                self.vector_pod_getter(),
                self.vector_pod_setter()]

    def get_byte_size(self):
        return 16 # 8b offset, 8b size

    def get_unique_identifier(self):
        return super(DynamicMember, self).get_unique_identifier() + b"Vector"

    def get_cxxtype(self):
        if self.value_type.is_string:
            return "std::string"
        # vector of any type
        return self.vector_type()

    def vector_type(self):
        return "std::vector< {0} >".format(self.value_type.type)

    def qualified_type(self, classname):
        return "{0}::{1}".format(classname, self.cxxName)

    def get_initializer(self):
        return [self.cxxname, 0, self.cxxName, self.dynamic_type_index, self.value_type.size]

    def get_declaration(self):
        return "{0} _{1};".format(self.cxxName, self.cxxname)

    def write_typedefs(self, file):
        file.write(NEXTLINE + "typedef ::zerobuf::Vector< {0} > {1};".
                   format(self.value_type.type, self.cxxName))

    def from_json(self):
        if self.value_type.is_string:
            return 'set{0}( ::zerobuf::fromJSON< std::string >( ::zerobuf::getJSONField( json, "{1}" )));'.format(self.cxxName, self.cxxname)
        elif self.value_type.is_byte_type:
            return '_{0}.fromJSONBinary( ::zerobuf::getJSONField( json, "{0}" ));'.format(self.cxxname)
        else:
            return '_{0}.fromJSON( ::zerobuf::getJSONField( json, "{0}" ));'.format(self.cxxname)

    def to_json(self):
        if self.value_type.is_string:
            return '::zerobuf::toJSON( get{0}String(), ::zerobuf::getJSONField( json, "{1}" ));'.format(self.cxxName, self.cxxname)
        elif self.value_type.is_byte_type:
            return '_{0}.toJSONBinary( ::zerobuf::getJSONField( json, "{0}" ));'.format(self.cxxname)
        else:
            return '_{0}.toJSON( ::zerobuf::getJSONField( json, "{0}" ));'.format(self.cxxname)


class FbsEnum():
    """An fbs enum which can be written as a C++ enum."""

    def __init__(self, item):
        self.name = item[1]
        self.type = item[2]
        self.values = item[3:]

    def write_declaration(self, file):
        file.write("enum " + self.name + "\n{")
        for enumValue in self.values:
            file.write(NEXTLINE + self.name + "_" + enumValue + ",")
        header.write("\n};\n\n")


class FbsTable():
    """An fbs Table (class) which can be written to a C++ implementation."""

    def __init__(self, item, namespace, fbsFile):
        self.name = item[1]
        self.attributes = item[2:]
        self.namespace = namespace
        self.offset = 0
        self.dynamic_members = []
        self.static_members = []
        self.all_members = []
        self.initializers = []
        self.default_value_setters = []
        self.md5 = hashlib.md5()
        self.generate_qobject = fbsFile.generate_qobject

        self.parse_members(fbsFile)
        self.compute_offsets()
        self.compute_md5()
        self.fill_initializer_list()

    def has_data(self):
        # return true if the table has data
        return self.offset != 0

    def is_dynamic(self, attrib, fbsFile):
        #  field is a sub-struct and field size is dynamic (==0)
        if attrib[1] in fbsFile.table_names and fbsFile.types[attrib[1]].size == 0:
            return True

        if attrib[1] == "string": # strings are dynamic
            return True

        if len(attrib) == 4: # name : [type] dynamic array
            return True

        return False

    def parse_members(self, fbsFile):
        dynamic_type_index = 0
        for attrib in self.attributes:
            name = attrib[0]
            fbs_type = attrib[1] if len(attrib) < 4 else attrib[2]
            cxxtype = fbsFile.types[fbs_type].cxxtype
            cxxtype_size = fbsFile.types[fbs_type].size
            is_zerobuf_type = cxxtype in fbsFile.table_names
            is_enum_type = cxxtype in fbsFile.enum_names
            is_byte_type = fbs_type == "byte" or fbs_type == "ubyte"
            value_type = ValueType(cxxtype, cxxtype_size, is_zerobuf_type, is_enum_type, is_byte_type)

            if self.is_dynamic(attrib, fbsFile):
                if len(attrib) == 2 and is_zerobuf_type:
                    member = DynamicZeroBufMember(name, value_type, dynamic_type_index)
                else:
                    member = DynamicMember(name, value_type, dynamic_type_index, self.name)
                dynamic_type_index += 1
                self.dynamic_members.append(member)
            else:
                if len(attrib) == 2 or len(attrib) == 3:
                    member = FixedSizeMember(name, value_type)
                    if len(attrib) == 3:
                        default_values = attrib[2]
                        setter = "set{0}({1}( {2} ));".format(member.cxxName, cxxtype, default_values)
                        self.default_value_setters.append(setter)
                else:
                    elem_count = int(attrib[4])
                    member = FixedSizeArray(name, value_type, elem_count, self.name)
                self.static_members.append(member)

            self.all_members.append(member)

    def compute_offsets(self):
        self.offset = 4 # 4b version header in host endianness
        for member in self.dynamic_members:
            member.allocator_offset = self.offset
            self.offset += member.get_byte_size()
        for member in self.static_members:
            member.allocator_offset = self.offset
            self.offset += member.get_byte_size()
        if self.offset == 4: # OPT: table has no data
            self.offset = 0

    def compute_md5(self):
        for namespace in self.namespace:
            self.md5.update(namespace.encode('utf-8') + b"::")
        self.md5.update(self.name.encode('utf-8'))
        for member in self.dynamic_members:
            self.md5.update(member.get_unique_identifier())
        for member in self.static_members:
            self.md5.update(member.get_unique_identifier())

    def get_virtual_destructor(self):
        return Function(None, "~" + self.name + "()", "{}", virtual=True)

    def empty_constructors(self):
        functions = []
        initList = ": ::zerobuf::Zerobuf( ::zerobuf::AllocatorPtr( ))"
        if self.generate_qobject:
            initList = ": QObject()\n    , ::zerobuf::Zerobuf( ::zerobuf::AllocatorPtr( ))"
        functions.append(Function(None, "{0}()".format(self.name), "{0}{{}}".format(initList)))
        functions.append(Function(None, "{0}( const {0}& )".format(self.name), "{0}{{}}".format(initList)))
        functions.append(self.get_virtual_destructor())
        functions.append(Function(self.name+"&", "operator = ( const " + self.name + "& )",
                                  "return *this;", split=False))
        return functions

    def special_member_functions(self):
        functions = []
        allocator_init = ": {0}( ::zerobuf::AllocatorPtr( new ::zerobuf::NonMovingAllocator( {1}, {2} )))\n". \
                                       format(self.name, self.offset, len(self.dynamic_members))
        # default ctor
        functions.append(Function(None, "{0}()".format(self.name),
                                  allocator_init + "{}"))
        # member initialization ctor
        memberArgs = []
        setters = []
        for member in self.all_members:
            valueName = member.cxxname + 'Value'
            memberArgs.append("const {0}& {1}".format(member.get_cxxtype(), valueName))
            setters.append("set{0}( {1} );".format(member.cxxName, valueName))
        functions.append(Function(None,
                                  "{0}( {1} )".format(self.name, ', '.join(memberArgs)),
                                  allocator_init +
                                  "{" + NEXTLINE +
                                  NEXTLINE.join(setters) +
                                  "\n}"))
        # copy ctor
        functions.append(Function(None,
                                  "{0}( const {0}& rhs )".format(self.name),
                                  allocator_init +
                                  "{\n" +
                                  "    *this = rhs;\n" +
                                  "}"))
        # move ctor
        functions.append(Function(None,
                                  "{0}( {0}&& rhs ){1}".format(self.name,\
                                  " throw()" if os.name == "nt" else " noexcept"),
                                  ": ::zerobuf::Zerobuf( std::move( rhs ))\n" +
                                  self.get_move_initializer()))
        # copy-from-baseclass ctor
        functions.append(Function(None,
                                  "{0}( const ::zerobuf::Zerobuf& rhs )".format(self.name),
                                  allocator_init +
                                  "{\n" +
                                  "    ::zerobuf::Zerobuf::operator = ( rhs );\n" +
                                  "}"))
        # Zerobuf object owns allocator!
        functions.append(Function(None,
                                  "{0}( ::zerobuf::AllocatorPtr allocator )".format(self.name),
                                  ": ::zerobuf::Zerobuf( std::move( allocator ))\n" +
                                  self.get_initializer_list() +
                                  "{" + (NEXTLINE + "if( !getAllocator().isMutable( ))\n        return;\n" +
                                  NEXTLINE if len(self.default_value_setters) > 0 else "") +
                                  NEXTLINE.join(self.default_value_setters) +
                                  "\n}",
                                  explicit = True))

        functions.append(self.get_virtual_destructor())

        # copy ctor and copy assignment operator
        functions.append(Function(self.name+"&",
                                  "operator = ( const " + self.name + "& rhs )",
                                  "::zerobuf::Zerobuf::operator = ( rhs ); return *this;",
                                  split=False))
        functions.append(Function("{0}&".format(self.name),
                                  "operator = ( {0}&& rhs )".format(self.name),
                                  "::zerobuf::Zerobuf::operator = ( std::move( rhs ));\n" +
                                  self.get_move_operator() +
                                  "    return *this;"))
        return functions

    def introspection_functions(self):
        digest = self.md5.hexdigest()
        high = digest[ 0 : len( digest ) - 16 ]
        low  = digest[ len( digest ) - 16: ]
        zerobufType = "::zerobuf::uint128_t( 0x{0}ull, 0x{1}ull )".format(high, low)
        zerobufName = "{0}{1}{2}".format("::".join(self.namespace),
                                         "::" if self.namespace else "",
                                         self.name)
        functions = []
        functions.append(Function("std::string", "getTypeName() const final",
                                  'return "{0}";'.format(zerobufName), split=False))
        functions.append(Function("std::string", "ZEROBUF_TYPE_NAME()",
                                  'return "{0}";'.format(zerobufName), static=True, split=False))
        functions.append(Function("::zerobuf::uint128_t", "getTypeIdentifier() const final",
                                  "return {0};".format(zerobufType), split=False))
        functions.append(Function("::zerobuf::uint128_t", "ZEROBUF_TYPE_IDENTIFIER()",
                                  "return {0};".format(zerobufType), static=True, split=False))
        functions.append(Function("size_t", "getZerobufStaticSize() const final",
                                  "return {0};".format(self.offset), split=False))
        functions.append(Function("size_t", "ZEROBUF_STATIC_SIZE()",
                                  "return {0};".format(self.offset), static=True, split=False))
        functions.append(Function("size_t", "getZerobufNumDynamics() const final",
                                  "return {0};".format(len(self.dynamic_members)), split=False))
        functions.append(Function("size_t", "ZEROBUF_NUM_DYNAMICS()",
                                  "return {0};".format(len(self.dynamic_members)), static=True, split=False))
        if self.has_data():
            functions.append(Function("Const{0}Ptr".format(self.name),
                                      "create( const void* data, const size_t size )",
                                      "return Const{0}Ptr( new {0}( ::zerobuf::AllocatorPtr( " \
                                      "new ::zerobuf::ConstAllocator( reinterpret_cast< const uint8_t* >( data ), size ))));"
                                      .format(self.name), static=True, split=False))
        return functions

    def json_functions(self):
        from_json = []
        to_json = []

        for member in self.dynamic_members:
            from_json.append(member.from_json())
            to_json.append(member.to_json())
        for member in self.static_members:
            from_json.append(member.from_json())
            to_json.append(member.to_json())

        if not from_json or not to_json:
            return []

        return [Function("void", "_parseJSON( const Json::Value& json ) final", NEXTLINE.join(from_json)),
                Function("void", "_createJSON( Json::Value& json ) const final", NEXTLINE.join(to_json))]

    def from_binary_function(self):
        member_emits = []
        for member in self.dynamic_members:
            member_emits.append(member.emit_value_changed(True))
        for member in self.static_members:
            member_emits.append(member.emit_value_changed(True))
        body = []
        body.append("const bool ret = ::zerobuf::Zerobuf::_fromBinary( data, size );")
        body.append(NEXTLINE.join(member_emits))
        body.append("return ret;")
        function = Function("bool", "_fromBinary( const void* data, const size_t size ) final",
            "{0}".format(NEXTLINE.join(body)))
        return function

    def write_declarations(self, functions, file):
        for function in functions:
            function.write_declaration(file)

    def write_json_declarations(self, file):
        functions = self.json_functions()
        if( len(functions) > 0 ):
            file.write("\n")
            self.write_declarations(functions, file)

    def write_declaration(self, file):
        if self.has_data():
            file.write("class {0};\n" \
                       "typedef std::unique_ptr< const {0} > Const{0}Ptr;\n\n".format(self.name))

        self.write_class_begin(file)

        if self.generate_qobject:
            self.write_qobject_members_declarations(file)
        else:
            self.write_members(file)

        # class functions
        if len(self.dynamic_members) > 0:
            self.compact_function().write_declaration(file)

        if self.has_data():
            self.write_declarations(self.special_member_functions(), file)
        else:
            self.write_declarations(self.empty_constructors(), file)

        file.write("\n")
        file.write(NEXTLINE + "// Introspection")
        self.write_declarations(self.introspection_functions(), file)
        self.write_json_declarations(file)
        self.write_class_end(file)

    def write_class_begin(self, file):
        parent_classes = ["public ::zerobuf::Zerobuf"]
        if self.generate_qobject:
            parent_classes.insert(0, "public QObject")
        parents = ", ".join(parent_classes)
        file.write("class {0} : {1}\n".format(self.name, parents))
        file.write("{\n")
        if self.generate_qobject:
            file.write("    Q_OBJECT\n\n")
        file.write("public:")

    def write_qobject_members_declarations(self, file):
        if len(self.all_members) == 0:
            return

        for member in self.dynamic_members:
            member.write_qt_getters(file)
        for member in self.static_members:
            member.write_qt_getters(file)
        file.write("\n\n")

        file.write("public slots:")
        for member in self.dynamic_members:
            member.write_qt_setters(file)
        for member in self.static_members:
            member.write_qt_setters(file)
        file.write("\n\n")

        file.write("signals:")
        for member in self.dynamic_members:
            member.write_qt_signals(file)
        for member in self.static_members:
            member.write_qt_signals(file)
        file.write("\n\n")
        file.write("public:")

    def write_members(self, file):
        for member in self.dynamic_members:
            member.write_accessors_declaration(file)
            file.write("\n")
        for member in self.static_members:
            member.write_accessors_declaration(file)
            file.write("\n")

    def write_class_end(self, file):
        member_declarations = []

        for member in self.dynamic_members:
            member_declarations.append(member.get_declaration())
        for member in self.static_members:
            if member.value_type.is_zerobuf_type:
                member_declarations.append(member.get_declaration())

        if len(member_declarations) > 0:
            file.write("\n\n")
            file.write("private:" + NEXTLINE)
            file.write(NEXTLINE.join(member_declarations))

        if self.generate_qobject:
            fun = self.from_binary_function()
            file.write("\n\nprivate:" + NEXTLINE)
            fun.write_declaration(file)
        file.write("\n};\n\n")

    def write_implementations(self, functions, file):
        for function in functions:
            function.write_implementation(file, self.name)

    def write_implementation(self, file):
        # members accessors
        for member in self.dynamic_members:
            member.write_accessors_implementation(file, self.name, self.generate_qobject)
        for member in self.static_members:
            member.write_accessors_implementation(file, self.name, self.generate_qobject)

        # class functions
        if len(self.dynamic_members) > 0:
            self.compact_function().write_implementation(file, self.name)

        if self.has_data():
            self.write_implementations(self.special_member_functions(), file)
        else:
            self.write_implementations(self.empty_constructors(), file)

        self.write_implementations(self.introspection_functions(), file)
        self.write_implementations(self.json_functions(), file)
        if self.generate_qobject:
            fun = self.from_binary_function()
            fun.write_implementation(file, self.name)

    def compact_function(self):
        # Recursive compaction
        compact = ''
        for dynamic_member in self.dynamic_members:
            compact += "    _{0}.compact( threshold );\n".format(dynamic_member.cxxname)
        compact += "    ::zerobuf::Zerobuf::compact( threshold );"
        compact = compact[4:]
        return Function("void", "compact( float threshold = 0.1f ) final", compact)

    def fill_initializer_list(self):
        for member in self.dynamic_members:
            self.initializers.append(member.get_initializer())
        for member in self.static_members:
            if member.value_type.is_zerobuf_type:
                self.initializers.append(member.get_initializer())

    def get_move_statics(self):
        movers = ''
        # [cxxname, nElems, cxxtype, offset|index, elemSize]
        for initializer in self.initializers:
            if initializer[1] == 1: # single member
                if initializer[4] == 0: # dynamic member
                    allocator = "::zerobuf::NonMovingSubAllocator( {{0}}, {0}, {1}::ZEROBUF_NUM_DYNAMICS(), {1}::ZEROBUF_STATIC_SIZE( ))".format(initializer[3], initializer[2])
                else:
                    allocator = "::zerobuf::StaticSubAllocator( {{0}}, {0}, {1} )".format(initializer[3], initializer[4])
                movers += "    _{0}.reset( ::zerobuf::AllocatorPtr( new {1}));\n" \
                    .format(initializer[0], allocator ).format( "getAllocator()" )
                movers += "    rhs._{0}.reset( ::zerobuf::AllocatorPtr( new {1}));\n" \
                    .format(initializer[0], allocator ).format( "rhs.getAllocator()" )
            elif initializer[1] != 0: # static array
                for i in range(0, initializer[1]):
                    movers += "    _{0}[{1}].reset( ::zerobuf::AllocatorPtr( " \
                              "new ::zerobuf::StaticSubAllocator( getAllocator(), {2}, {3} )));\n" \
                        .format(initializer[0], i, initializer[3], initializer[3] + i * initializer[4])
                    movers += "    rhs._{0}[{1}].reset( ::zerobuf::AllocatorPtr( " \
                              "new ::zerobuf::StaticSubAllocator( rhs.getAllocator(), {2}, {3} )));\n" \
                        .format(initializer[0], i, initializer[3], initializer[3] + i * initializer[4])
        return movers

    def get_move_operator(self):
        movers = ''
        # [cxxname, nElems, cxxtype, offset|index, elem_size]
        for initializer in self.initializers:
            if initializer[1] == 0: # dynamic array
                movers += "    _{0}.reset( getAllocator( ));\n".format(initializer[0])
                movers += "    rhs._{0}.reset( rhs.getAllocator( ));\n".format(initializer[0])
        movers += self.get_move_statics()
        return movers

    def get_move_initializer(self):
        initializers = ''
        # [cxxname, nElems, cxxtype, offset|index, elem_size]
        for initializer in self.initializers:
            if initializer[1] == 0: # dynamic array
                initializers += "    , _{0}( getAllocator(), {1} )\n".format(initializer[0], initializer[3])
        initializers += "{\n"
        initializers += self.get_move_operator()
        initializers += "}"
        return initializers

    def get_initializer_list(self):
        initializers = ''

        # [cxxname, nElems, cxxtype, offset, elem_size]
        for initializer in self.initializers:
            if initializer[1] == 0: # dynamic array
                initializers += "    , _{0}( getAllocator(), {1} )\n".format(initializer[0], initializer[3])
            elif initializer[1] == 1: # single member
                if initializer[4] == 0: # dynamic member
                    allocator = "::zerobuf::NonMovingSubAllocator( getAllocator(), {0}, {1}::ZEROBUF_NUM_DYNAMICS(), {1}::ZEROBUF_STATIC_SIZE( ))".format(initializer[3], initializer[2])
                else:
                    allocator = "::zerobuf::StaticSubAllocator( getAllocator(), {0}, {1} )".format(initializer[3], initializer[4])
                initializers += "    , _{0}( ::zerobuf::AllocatorPtr( new {1}))\n" \
                    .format(initializer[0], allocator)
            else: # static array
                initializers += "    , _{0}{1}".format(initializer[0], "{{")
                for i in range( 0, initializer[1] ):
                    initializers += "\n        {0}( ::zerobuf::AllocatorPtr( " \
                                    "new ::zerobuf::StaticSubAllocator( getAllocator(), {1}, {2} ))){3} " \
                        .format(initializer[2], initializer[3] + i * initializer[4], initializer[4], "}}\n" if i == initializer[1] - 1 else ",")
        return initializers


class FbsFile():
    """An fbs file which can be written to C++ header and implementation files."""

    def __init__(self, schema, generate_qobject):
        self.generate_qobject = generate_qobject
        self.namespace = []
        self.enums = []
        self.enum_names = set()
        self.tables = []
        self.table_names = set()
        # type lookup table: fbs type : ( size, C++ type )
        self.types = DEFAULT_TYPES.copy()
        self.parse(schema)

    def parse(self, schema):
        """
        [['namespace', ['tide', 'rest']],
        ['enum', 'CommandType', 'uint', 'OpenContent', 'OpenWebbrowser'],
        ['table', 'Command', ['key', 'string'], ['value', 'string']],
        ['root_type', 'Command']]
        """
        root_options = { "namespace" : self.set_namespace,
                         "enum" : self.add_enum,
                         "table" : self.add_table,
                         "root_type" : self.set_root_type,
                        }
        for item in schema:
            root_options[ item[0] ]( item )

    def set_namespace(self, item):
        self.namespace = item[1]

    def add_enum(self, item):
        enum = FbsEnum(item)
        self.types[ enum.name ] = TypeDescription(4, enum.name)
        self.enum_names.add(enum.name)
        self.enums.append(enum)

    def add_table(self, item):
        table = FbsTable(item, self.namespace, self)
        self.tables.append(table)
        self.table_names.add(table.name)
        # record size in type lookup table, 0 if dynamically sized
        self.types[ table.name ] = TypeDescription( table.offset if len(table.dynamic_members) == 0 else 0, table.name )

    def set_root_type(self, item):
        # Nothing to do with this statement
        return

    def write_namespace_opening(self, file):
        for namespace in self.namespace:
            file.write("namespace " + namespace + "\n{\n")

    def write_namespace_closing(self, file):
        for namespace in self.namespace:
            file.write("}\n")

    def write_declaration(self, header):
        """Write the C++ header file."""

        header.write("// Generated by zerobufCxx.py\n\n")
        header.write("#pragma once\n")
        if self.generate_qobject:
            header.write( "#include <QObject> // base class\n")
        header.write("#include <zerobuf/Zerobuf.h> // base class\n")
        header.write("#include <zerobuf/ConstAllocator.h> // static create\n")
        header.write("#include <zerobuf/Vector.h> // member\n")
        header.write("#include <array> // member\n")
        header.write("#include <memory> // std::unique_ptr\n")
        header.write("\n")

        self.write_namespace_opening(header)
        header.write("\n")

        for enum in self.enums:
            enum.write_declaration(header)

        for table in self.tables:
            table.write_declaration(header)

        self.write_namespace_closing(header)

    def write_implementation(self, impl):
        """Write the C++ implementation file."""

        impl.write("#include <zerobuf/NonMovingAllocator.h>\n")
        impl.write("#include <zerobuf/NonMovingSubAllocator.h>\n")
        impl.write("#include <zerobuf/StaticSubAllocator.h>\n")
        impl.write("#include <zerobuf/json.h>\n")
        impl.write("\n")

        self.write_namespace_opening(impl)

        for table in self.tables:
            table.write_implementation(impl)

        impl.write("\n")
        self.write_namespace_closing(impl)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description=("zerobufCxx.py: A zerobuf C++ code generator for "
                     "extended flatbuffers schemas"))
    parser.add_argument( "files", nargs = "+" )
    parser.add_argument( '-o', '--outputdir', action='store', default = "",
                         help = "Prefix directory for all generated files.")
    parser.add_argument( '-q', '--qobject', action='store_true',
                         help = "Generate a QObject with signals and slots.")
    parser.add_argument( '-e', '--extension', action='store', default = "cpp",
                         help = "Extension for generated source files. (default: cpp)")

    # Parse, interpret and validate arguments
    args = parser.parse_args()

    inline_implementation = args.extension == "ipp"

    fbsObject = create_FBS_parser()
    for _file in args.files:
        basename = os.path.splitext( _file )[0]
        headerbase = os.path.basename( basename )
        if args.outputdir:
            if args.outputdir == '-':
                header = sys.stdout
                impl = sys.stdout
            else:
                if not os.path.exists( args.outputdir ):
                    os.makedirs( args.outputdir )
                header = open( args.outputdir + "/" + headerbase + ".h", 'w' )
                impl = open( args.outputdir + "/" + headerbase + "." + args.extension, 'w' )
        else:
            header = open( basename + ".h" , 'w' )
            impl = open( basename + "." + args.extension, 'w' )

        impl.write( "// Generated by zerobufCxx.py\n\n" )
        if not inline_implementation:
            impl.write( '#include "{0}.h"\n\n'.format(headerbase) )

        schema = fbsObject.parseFile(_file)
        # import pprint
        # pprint.pprint( schema.asList( ))
        fbsFile = FbsFile(schema, args.qobject)
        fbsFile.write_declaration(header)
        fbsFile.write_implementation(impl)

        if inline_implementation:
            header.write( '#include "{0}.ipp"\n\n'.format(headerbase) )
