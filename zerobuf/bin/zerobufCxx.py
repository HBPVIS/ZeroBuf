#!/usr/bin/env python

# TODO:
# * nested dynamic tables
# * endian swap method
# * configurable allocator class name ?

# UUID is MD5 hash of namespace::namespace[<cxxtype>|<cxxtype>*|<cxxtype><size>]
# See @ref Binary for a description of the memory layout

import argparse
import hashlib
import re
from pyparsing import *
import sys
import os

fbsBaseType = oneOf( "int uint float double byte short ubyte ushort ulong uint8_t uint16_t " +
                     "uint32_t uint64_t uint128_t int8_t int16_t int32_t int64_t bool string" )

# namespace foo.bar
fbsNamespaceName = Group( ZeroOrMore( Word( alphanums ) + Suppress( '.' )) +
                          Word( alphanums ))
fbsNamespace = Group( Keyword( "namespace" ) + fbsNamespaceName +
                      Suppress( ';' ))

# enum EventDirection : ubyte { Subscriber, Publisher, Both }
fbsEnumValue = ( Word( alphanums ) + Suppress( Optional( ',' )))
fbsEnum = Group( Keyword( "enum" ) + Word( alphanums ) + Suppress( ':' ) +
                 fbsBaseType + Suppress( '{' ) + OneOrMore( fbsEnumValue ) +
                 Suppress( '}' ))

# value:[type] = defaultValue; entries in table
# TODO: support more default values other than numbers and booleans
fbsType = ( fbsBaseType ^ Word( alphanums ))
fbsTableArray = ( ( Literal( '[' ) + fbsType + Literal( ']' )) ^
                  ( Literal( '[' ) + fbsType + Literal( ':' ) + Word( nums ) +
                    Literal( ']' )) )
fbsTableValue = ((fbsType ^ fbsTableArray) + ZeroOrMore(Suppress('=') +
                Or([Word("true"), Word("false"), Word(nums+".")])))
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
fbsObject = ( Optional( fbsNamespace ) + ZeroOrMore( fbsEnum ) +
              OneOrMore( fbsTable ) + Optional( fbsRootType ))

fbsComment = cppStyleComment
fbsObject.ignore( fbsComment )

#fbsTableArray.setDebug()
#fbsTableValue.setDebug()
#fbsTableEntry.setDebug()

def isDynamic( spec ):
    #  field is a sub-struct and field size is dynamic
    if spec[1] in emit.tables and emit.types[spec[1]][0] == 0:
        return True

    if spec[1] == "string": # strings are dynamic
        return True

    if len(spec) == 4: # name : [type] dynamic array
        return True

    return False

def countDynamic( specs ):
    numDynamic = 0
    for spec in specs:
        if isDynamic( spec ):
            numDynamic += 1
    return numDynamic

def emitFunction( retVal, function, body, static=False, explicit=False ):
    implFunc = re.sub(r" final$", "", function) # remove ' final' keyword
    implFunc = re.sub(r" = [0-9\.f]+ ", " ", implFunc) # remove default params

    if retVal: # '{}'-less body
        header.write( "    {0}{1} {2};\n".
                      format( "static " if static else "", retVal, function ))
        impl.write( "\n" + retVal + " " + emit.table +
                    "::" + implFunc + "\n{\n    " + body +
                    "\n}\n" )
    else:      # ctor '[initializer list]{ body }'
        header.write( "    {0}{1};\n".
                      format( "explicit " if explicit else "", function ))
        impl.write( "\n" + emit.table +
                    "::" + implFunc + "\n    " + body + "\n\n" )

def emitDynamicMember(spec):
    cxxname = spec[0]
    cxxName = cxxname[0].upper() + cxxname[1:]
    isString = (spec[1] == "string")
    cxxtype = emit.types[spec[1]][1]

    emit.md5.update( cxxtype.encode('utf-8'))

    emitFunction("{0}&".format(cxxtype), "get{0}()".format(cxxName),
                 "notifyChanging();\n    return _{0};".format(cxxname))
    emitFunction("const {0}&".format(cxxtype), "get{0}() const".format(cxxName),
                 "return _{0};".format(cxxname))
    emitFunction("void", "set{0}( const {1}& value )".format(cxxName, cxxtype),
                 "notifyChanging();\n    _{0} = value;".format(cxxname))
    emit.initializers.append([cxxname, 1, cxxtype, emit.currentDyn, 0])
    emit.members.append("{0} _{1};".format(cxxtype, cxxname));

    emit.schema.append("std::make_tuple( \"{0}\", {1}::ZEROBUF_TYPE(), {2}, 0, 1 )".
                        format(cxxname, cxxtype, emit.offset))
    emit.nestedSchemas.add( "{0}::schema()".format(cxxtype))

    emit.offset += 16 # 8b offset, 8b size
    emit.currentDyn += 1
    header.write("\n")
    impl.write("\n")

def emitDynamic(spec):
    if(len(spec) == 2 and spec[1] in emit.tables): # dynamic Zerobuf member
       return emitDynamicMember(spec)

    cxxname = spec[0]
    cxxName = cxxname[0].upper() + cxxname[1:]
    isString = (spec[1] == "string")
    if isString:
        cxxtype = "char"
        elemSize = 1
    else:
        cxxtype = emit.types[ spec[2] ][1]
        elemSize = emit.types[ spec[2] ][0]

    emit.md5.update( cxxtype.encode('utf-8') + b"Vector" )

    header.write( "    typedef ::zerobuf::Vector< {0} > {1};\n".
                  format( cxxtype, cxxName ))

    emitFunction( "typename {0}::{1}&".format( emit.table, cxxName ),
                  "get" + cxxName + "()",
                  "notifyChanging();\n    " +
                  "return _{0};".format( cxxname ))
    emitFunction( "const typename {0}::{1}&".format( emit.table, cxxName ),
                  "get" + cxxName + "() const",
                  "return _{0};".format( cxxname ))
    emit.initializers.append([cxxname, 0, cxxName, emit.currentDyn, elemSize])
    emit.members.append( "{0} _{1};".format( cxxName, cxxname ));

    if cxxtype in emit.tables: # Dynamic array of (static) Zerobufs
        if elemSize == 0:
            sys.exit("Dynamic arrays of empty ZeroBuf (field {0}) not supported".format(spec[0]))

        emitFunction( "std::vector< " + cxxtype + " >",
                      "get" + cxxName + "Vector() const",
                      "const {0}& vec = get{0}();\n".format( cxxName ) +
                      "    std::vector< " + cxxtype + " > ret;\n" +
                      "    ret.reserve( vec.size( ));\n" +
                      "    for( size_t i = 0; i < vec.size(); ++i )\n" +
                      "        ret.push_back( vec[i] );\n" +
                      "    return ret;\n" )
        emitFunction( "void",
                      "set" + cxxName + "( const std::vector< " +
                      cxxtype + " >& values )",
                      "notifyChanging();\n    " +
                      "::zerobuf::Vector< {0} > dynamic( getAllocator(), {1} );\n".format(cxxtype, emit.currentDyn) +
                      "    dynamic.clear();\n" +
                      "    for( const " + cxxtype + "& data : values )\n" +
                      "        dynamic.push_back( data );" )

    else: # Dynamic array of PODs
        emitFunction( "void",
                      "set" + cxxName + "( " + cxxtype +
                      " const * value, size_t size )",
                      "notifyChanging();\n    " +
                      "_copyZerobufArray( value, size * sizeof( " + cxxtype +
                      " ), " + str( emit.currentDyn ) + " );" )
        emitFunction( "std::vector< " + cxxtype + " >",
                      "get" + cxxName + "Vector() const",
                      "return std::vector< {0} >( _{1}.data(), _{1}.data() + _{1}.size( ));".format(cxxtype, cxxname))
        emitFunction( "void",
                      "set" + cxxName + "( const std::vector< " +
                      cxxtype + " >& value )",
                      "notifyChanging();\n    " +
                      "_copyZerobufArray( value.data(), value.size() * sizeof( " +
                      cxxtype + " ), " + str( emit.currentDyn ) + " );" )
        # string
        emitFunction( "std::string",
                      "get" + cxxName + "String() const",
                      "const uint8_t* ptr = getAllocator().template getDynamic< " +
                      "const uint8_t >( " + str( emit.currentDyn ) + " );\n" +
                      "    return std::string( ptr, ptr + " +
                      "getAllocator().template getItem< uint64_t >( " +
                      str( emit.offset + 8 ) + " ));" )
        emitFunction( "void",
                      "set" + cxxName + "( const std::string& value )",
                      "notifyChanging();\n    " +
                      "_copyZerobufArray( value.c_str(), value.length(), " +
                      str( emit.currentDyn ) + " );" )
    # schema entry
    if cxxtype in emit.enums:
        elemSize = 4
        cxxtype = "uint32_t"

    if cxxtype in emit.tables:
        emit.nestedSchemas.add( "{0}::schema()".format( cxxtype ))
        zerobufType = "{0}::ZEROBUF_TYPE()".format( cxxtype )
        elemSize = "( {0}::ZEROBUF_NUM_DYNAMICS() == 0 ? {1} : 0 )".format( cxxtype, emit.types[ cxxtype ][ 0 ] )
    else:
        digest = hashlib.md5( "{0}".format( cxxtype ).encode('utf-8')).hexdigest()
        high = digest[ 0 : len( digest ) - 16 ]
        low  = digest[ len( digest ) - 16: ]
        zerobufType = "::zerobuf::uint128_t( 0x{0}ull, 0x{1}ull )".format( high, low )

    emit.schema.append( "std::make_tuple( \"{0}\", {1}, {2}, {3}, 0 )".
                        format( spec[0], zerobufType, emit.offset, elemSize ))

    emit.offset += 16 # 8b offset, 8b size
    emit.currentDyn += 1
    header.write( "\n" )
    impl.write( "\n" )

def emitStaticMember( spec ):
    cxxname = spec[0]
    cxxName = cxxname[0].upper() + cxxname[1:]
    cxxtype = emit.types[ spec[1] ][1]
    elemSize = emit.types[ spec[1] ][0]
    if len(spec) == 3:
        emit.defaultValues += "    set{0}({1});\n".format(cxxName,spec[2])

    emit.md5.update( cxxtype.encode('utf-8') )

    if cxxtype in emit.tables:
        emitFunction( "const {0}&".format( cxxtype ),
                      "get" + cxxName + "() const",
                      "return _{0};".format( cxxname ))
        emitFunction( "{0}&".format( cxxtype ), "get" + cxxName + "()",
                      "notifyChanging();\n    " +
                      "return _{0};".format( cxxname ))
        emitFunction( "void",
                      "set"  + cxxName + "( const " + cxxtype + "& value )",
                      "notifyChanging();\n    " +
                      "_{0} = value;".format( cxxname ))
        emit.members.append( "{0} _{1};".format( cxxtype, cxxname ));
        emit.initializers.append([cxxname, 1, cxxtype, emit.offset, elemSize])
    else:
        emitFunction( cxxtype, "get" + cxxName + "() const",
                      "return getAllocator().template getItem< " + cxxtype +
                      " >( " + str( emit.offset ) + " );" )
        emitFunction( "void",
                      "set"  + cxxName + "( " + cxxtype + " value )",
                      "notifyChanging();\n    " +
                      "getAllocator().template getItem< " + cxxtype + " >( " +
                      str( emit.offset ) + " ) = value;" )

    # schema entry
    if cxxtype in emit.enums:
        cxxtype = "uint32_t"
    if cxxtype in emit.tables:
        emit.nestedSchemas.add( "{0}::schema()".format( cxxtype ))
        zerobufType = "{0}::ZEROBUF_TYPE()".format( cxxtype )
    else:
        digest = hashlib.md5( "{0}".format( cxxtype ).encode('utf-8')).hexdigest()
        high = digest[ 0 : len( digest ) - 16 ]
        low  = digest[ len( digest ) - 16: ]
        zerobufType = "::zerobuf::uint128_t( 0x{0}ull, 0x{1}ull )".format( high,
                                                                        low )
    emit.schema.append( "std::make_tuple( \"{0}\", {1}, {2}, {3}, 1 )".
                        format( spec[0], zerobufType, emit.offset, elemSize ))

    emit.offset += elemSize

def emitStaticArray( spec ):
    cxxname = spec[0]
    cxxName = cxxname[0].upper() + cxxname[1:]
    cxxtype = emit.types[ spec[2] ][1]
    elemSize = int( emit.types[ spec[2] ][0] )
    nElems = int( spec[4] )
    nBytes = elemSize * nElems

    emit.md5.update( (cxxtype + str( nElems )).encode('utf-8') )
    if nElems < 2:
        sys.exit( "Static array of size {0} for field {1} not supported".format(nElems, cxxname))
    if elemSize == 0:
        sys.exit( "Static array of dynamic elements not implemented".
                  format( nElems ))

    if cxxtype in emit.tables:
        if elemSize == 0:
            sys.exit("Static arrays of empty ZeroBuf (field {0}) not supported".format(spec[0]))

        header.write( "    typedef std::array< {0}, {1} > {2};\n".
                      format( cxxtype, nElems, cxxName ))
        emitFunction( "const {0}::{1}&".format( emit.table, cxxName ),
                      "get" + cxxName + "() const",
                      "return _{0};".format( cxxname ))
        emitFunction( "{0}::{1}&".format( emit.table, cxxName ),
                      "get" + cxxName + "()",
                      "notifyChanging();\n    " +
                      "return _{0};".format( cxxname ))
        emitFunction( "void",
                      "set{0}( const {0}& value )".format( cxxName ),
                      "notifyChanging();\n    " +
                      "_{0} = value;".format( cxxname ))
        emit.members.append( "{0} _{1};".format( cxxName, cxxname ))
        emit.initializers.append([cxxname, nElems, cxxtype, emit.offset,
                                  elemSize])

    else:
        emitFunction( cxxtype + "*", "get" + cxxName + "()",
                      "notifyChanging();\n    " +
                      "return getAllocator().template getItemPtr< " + cxxtype +
                      " >( " + str( emit.offset ) + " );" )
        emitFunction( "const " + cxxtype + "*",
                      "get" + cxxName + "() const",
                      "return getAllocator().template getItemPtr< " + cxxtype +
                      " >( " + str( emit.offset ) + " );" )
        emitFunction( "std::vector< " + cxxtype + " >",
                      "get" + cxxName + "Vector() const",
                      "const " + cxxtype + "* ptr = getAllocator().template " +
                      "getItemPtr< " + cxxtype + " >( " + str( emit.offset ) +
                      " );\n    return std::vector< " + cxxtype +
                      " >( ptr, ptr + " + str( nElems ) + " );" )
        emitFunction( "void",
                      "set"  + cxxName + "( " + cxxtype + " value[ " +
                      spec[4] + " ] )",
                      "notifyChanging();\n    " +
                      "::memcpy( getAllocator().template getItemPtr< " +
                      cxxtype + " >( " + str( emit.offset ) + " ), value, " +
                      spec[4] + " * sizeof( " + cxxtype + " ));" )
        emitFunction( "void",
                      "set" + cxxName + "( const std::vector< " +
                      cxxtype + " >& value )",
                      "if( " + str( nElems ) + " >= value.size( ))\n" +
                      "    {\n" +
                      "        notifyChanging();" +
                      "        ::memcpy( getAllocator().template getItemPtr<" +
                      cxxtype + ">( " + str( emit.offset ) +
                      " ), value.data(), value.size() * sizeof( " + cxxtype +
                      "));\n" +
                      "    }" )
        emitFunction( "void",
                      "set" + cxxName + "( const std::string& value )",
                      "if( " + str( nBytes ) + " >= value.length( ))\n" +
                      "    {\n" +
                      "        notifyChanging();\n" +
                      "        ::memcpy( getAllocator().template getItemPtr<" +
                      cxxtype + ">( " + str( emit.offset ) +
                      " ), value.data(), value.length( ));\n" +
                      "    }" )
    # schema entry
    if cxxtype in emit.enums:
        cxxtype = "uint32_t"
    if cxxtype in emit.tables:
        emit.nestedSchemas.add( "{0}::schema()".format( cxxtype ))
        zerobufType = "{0}::ZEROBUF_TYPE()".format( cxxtype )
    else:
        digest = hashlib.md5( "{0}".format( cxxtype ).encode('utf-8')).hexdigest()
        high = digest[ 0 : len( digest ) - 16 ]
        low  = digest[ len( digest ) - 16: ]
        zerobufType = "::zerobuf::uint128_t( 0x{0}ull, 0x{1}ull )".format( high,
                                                                        low )
    emit.schema.append( "std::make_tuple( \"{0}\", {1}, {2}, {3}, {4} )".
                        format( spec[0], zerobufType, emit.offset, elemSize,
                                nElems ))
    emit.offset += nBytes

def emitStatic(spec):
    if len(spec) == 2 or len(spec) == 3:
        emitStaticMember(spec)
    else:
        emitStaticArray(spec)
    header.write( "\n" )
    impl.write( "\n" )

def move_statics(emit):
    movers = ''
    # [cxxname, nElems, cxxtype, emit.offset|index, elemSize]
    for initializer in emit.initializers:
        if initializer[1] == 1: # single member
            if initializer[4] == 0: # dynamic member
                allocator = "::zerobuf::NonMovingSubAllocator( {{0}}, {0}, {1}::ZEROBUF_NUM_DYNAMICS(), {1}::ZEROBUF_STATIC_SIZE( ))".format(initializer[3], initializer[2])
            else:
                allocator = "::zerobuf::StaticSubAllocator( {{0}}, {0}, {1} )".format(initializer[3], initializer[4])
            movers += "    _{0}.reset( ::zerobuf::AllocatorPtr( new {1}));\n"\
                .format(initializer[0], allocator ).format( "getAllocator()" )
            movers += "    rhs._{0}.reset( ::zerobuf::AllocatorPtr( new {1}));\n"\
                .format(initializer[0], allocator ).format( "rhs.getAllocator()" )
        elif initializer[1] != 0: # static array
            for i in range(0, initializer[1]):
                movers += "    _{0}[{1}].reset( ::zerobuf::AllocatorPtr( "\
                    "new ::zerobuf::StaticSubAllocator( getAllocator(), {2}, {3} )));\n"\
                    .format(initializer[0], i, initializer[3], initializer[3] + i * initializer[4])
                movers += "    rhs._{0}[{1}].reset( ::zerobuf::AllocatorPtr( "\
                    "new ::zerobuf::StaticSubAllocator( rhs.getAllocator(), {2}, {3} )));\n"\
                    .format(initializer[0], i, initializer[3], initializer[3] + i * initializer[4])
    return movers

def move_operator(emit):
    movers = ''
    # [cxxname, nElems, cxxtype, emit.offset|index, elemSize]
    for initializer in emit.initializers:
        if initializer[1] == 0: # dynamic array
            movers += "    _{0}.reset( getAllocator( ));\n".format(initializer[0])
            movers += "    rhs._{0}.reset( rhs.getAllocator( ));\n".format(initializer[0])
    movers += move_statics(emit)
    return movers

def move_initializer(emit):
    initializers = ''
    # [cxxname, nElems, cxxtype, emit.offset|index, elemSize]
    for initializer in emit.initializers:
        if initializer[1] == 0: # dynamic array
            initializers += "    , _{0}( getAllocator(), {1} )\n".format(initializer[0], initializer[3])
    initializers += "{\n"
    initializers += move_operator(emit)
    initializers += "}"
    return initializers


def initializer_list(emit):
    initializers = ''

    # [cxxname, nElems, cxxtype, emit.offset, elemSize]
    for initializer in emit.initializers:
        if initializer[1] == 0: # dynamic array
            initializers += "    , _{0}( getAllocator(), {1} )\n".format(initializer[0], initializer[3])
        elif initializer[1] == 1: # single member
            if initializer[4] == 0: # dynamic member
                allocator = "::zerobuf::NonMovingSubAllocator( getAllocator(), {0}, {1}::ZEROBUF_NUM_DYNAMICS(), {1}::ZEROBUF_STATIC_SIZE( ))".format(initializer[3], initializer[2])
            else:
                allocator = "::zerobuf::StaticSubAllocator( getAllocator(), {0}, {1} )".format(initializer[3], initializer[4])
            initializers += "    , _{0}( ::zerobuf::AllocatorPtr( new {1}))\n"\
                .format(initializer[0], allocator)
        else: # static array
            initializers += "    , _{0}{1}".format(initializer[0], "{{")
            for i in range( 0, initializer[1] ):
                initializers += "\n        {0}( ::zerobuf::AllocatorPtr( "\
                    "new ::zerobuf::StaticSubAllocator( getAllocator(), {1}, {2} ))){3} "\
                    .format(initializer[2], initializer[3] + i * initializer[4], initializer[4], "}}\n" if i == initializer[1] - 1 else ",")

    return initializers

def emit():
    emit.namespace = ""
    emit.offset = 0
    emit.numDynamic = 0
    emit.currentDyn = 0
    emit.enums = set()
    emit.tables = set()
    emit.schema = []
    emit.nestedSchemas = set()
    emit.types = { "int" : ( 4, "int32_t" ),
                   "uint" : ( 4, "uint32_t" ),
                   "float" : ( 4, "float" ),
                   "double" : ( 8, "double" ),
                   "byte" : ( 1, "int8_t" ),
                   "short" : ( 2, "int16_t" ),
                   "ubyte" : ( 1, "uint8_t" ),
                   "ushort" : ( 2, "uint16_t" ),
                   "ulong" : ( 8, "uint64_t" ),
                   "uint8_t" : ( 1, "uint8_t" ),
                   "uint16_t" : ( 2, "uint16_t" ),
                   "uint32_t" : ( 4, "uint32_t" ),
                   "uint64_t" : ( 8, "uint64_t" ),
                   "uint128_t" : ( 16, "::zerobuf::uint128_t" ),
                   "int8_t" : ( 1, "int8_t" ),
                   "int16_t" : ( 2, "int16_t" ),
                   "int32_t" : ( 4, "int32_t" ),
                   "int64_t" : ( 8, "int64_t" ),
                   "bool" : ( 1, "bool" ),
                   "string" : ( 1, "char*" )
    }

    def namespace():
        for namespace in emit.namespace:
            header.write( "}\n" )
            impl.write( "}\n" )
        emit.namespace = item[1]
        for namespace in emit.namespace:
            header.write( "namespace " + namespace + "\n{\n" )
            impl.write( "namespace " + namespace + "\n{\n" )

    def enum():
        emit.types[ item[1] ] = ( 4, item[1] )
        header.write( "enum " + item[1] + "\n{\n" )
        for enumValue in item[3:]:
            header.write( "    " + item[1] + "_" + enumValue + ",\n" )
        header.write( "};\n\n" )
        emit.enums.add( item[1] )

    def table():
        emit.offset = 4 # 4b version header in host endianness
        emit.numDynamic = countDynamic( item[2:] )
        emit.currentDyn = 0
        emit.defaultValues = ''
        emit.members = []
        emit.initializers = []
        emit.schema = []
        emit.nestedSchemas = set()
        emit.table = item[1]
        emit.md5 = hashlib.md5()
        for namespace in emit.namespace:
            emit.md5.update( namespace.encode('utf-8') + b"::" )
        emit.md5.update( item[1].encode('utf-8') )

        # class header
        header.write( "class " + item[1] + " : public ::zerobuf::Zerobuf\n" +
                      "{\npublic:\n" )

        # member access
        for member in item[2:]:
            if isDynamic( member ):
                emitDynamic( member )
        for member in item[2:]:
            if not isDynamic( member ):
                emitStatic( member )

        # Recursive compaction
        if emit.numDynamic > 0:
            compact = ''
            for member in item[2:]:
                if isDynamic( member ):
                    compact += "    _{0}.compact( threshold );\n".format(member[0])
            compact += "    ::zerobuf::Zerobuf::compact( threshold );"
            compact = compact[4:]
            emitFunction("void", "compact( const float threshold = 0.1f ) final",
                         compact)

        # ctors, dtor and assignment operator
        if emit.offset == 4: # OPT: table has no data
            emit.offset = 0
            emitFunction(None, "{0}()".format( item[1] ),
                         ": ::zerobuf::Zerobuf( ::zerobuf::AllocatorPtr( )){}")
            emitFunction(None,
                         "{0}( const {0}& )".format( item[1] ),
                         ": ::zerobuf::Zerobuf( ::zerobuf::AllocatorPtr( )){}")
            header.write("    virtual ~" + item[1] + "() {}\n\n")
            header.write("    " + item[1] + "& operator = ( const " +
                         item[1] + "& ) { return *this; }\n\n")
        else:
            # default ctor
            emitFunction(None, "{0}()".format(item[1]),
                         ": {0}( ::zerobuf::AllocatorPtr( new ::zerobuf::NonMovingAllocator( {1}, {2} )))\n{{}}".format(item[1], emit.offset, emit.numDynamic))

            # member initialization ctor
            memberArgs = []
            initializers = ''
            for member in item[2:]:
                cxxname = member[0]
                cxxName = cxxname[0].upper() + cxxname[1:]
                if isDynamic( member ):
                    isString = (member[1] == "string")
                    if isString:
                        cxxtype = "std::string"
                    elif(len(member) == 2 and member[1] in emit.tables):
                        # dynamic Zerobuf member
                        cxxtype = emit.types[member[1]][1]
                    else:
                        cxxtype = "std::vector< {0} >".format(emit.types[member[2]][1])
                else:
                    if len(member) == 2 or len(member) == 3:
                        cxxtype = emit.types[member[1]][1] # static member
                    else:
                        if cxxtype in emit.tables:
                            cxxtype = cxxName # static array of zerobuf
                        else:
                            cxxtype = "std::vector< {0} >".format(emit.types[member[2]][1]) # static array of POD

                valueName = cxxname + 'Value'
                memberArgs.append("const {0}& {1}".format(cxxtype, valueName))
                initializers += "    set{0}( {1} );\n".format(cxxName, valueName)

            emitFunction( None,
                          "{0}( {1} )".format(item[1], ', '.join(memberArgs)),
                          ": {0}( ::zerobuf::AllocatorPtr( new ::zerobuf::NonMovingAllocator( {1}, {2} )))\n"
                          "{{\n{3}}}".format( item[1], emit.offset, emit.numDynamic, initializers ))

            # copy ctor
            emitFunction(None,
                         "{0}( const {0}& rhs )".format(item[1]),
                         ": {0}( ::zerobuf::AllocatorPtr( new ::zerobuf::NonMovingAllocator( {1}, {2} )))\n".format(item[1], emit.offset, emit.numDynamic) +
                         "{\n    *this = rhs;\n}")

            # move ctor
            emitFunction(None,
                         "{0}( {0}&& rhs ) throw()".format(item[1]),
                         ": ::zerobuf::Zerobuf( std::move( rhs ))\n" +
                         move_initializer(emit))

            # copy-from-baseclass ctor
            emitFunction(None,
                         "{0}( const ::zerobuf::Zerobuf& rhs )".format(item[1]),
                         ": {0}( ::zerobuf::AllocatorPtr( new ::zerobuf::NonMovingAllocator( {1}, {2} )))\n".format(item[1], emit.offset, emit.numDynamic) +
                         "{\n" +
                         "    ::zerobuf::Zerobuf::operator = ( rhs );\n" +
                         "}")

            # Zerobuf object owns allocator!
            emitFunction(None,
                         "{0}( ::zerobuf::AllocatorPtr allocator )".format( item[1] ),
                         ": ::zerobuf::Zerobuf( std::move( allocator ))\n{0}".format(initializer_list(emit)) +
                         "{{\n{3}}}".format(item[1], emit.offset, emit.numDynamic, emit.defaultValues),
                         explicit = True)

            header.write( "    virtual ~" + item[1] + "() {}\n\n" )
            header.write( "    " + item[1] + "& operator = ( const " + item[1] + "& rhs )\n"+
                          "        { ::zerobuf::Zerobuf::operator = ( rhs ); return *this; }\n\n" )
            emitFunction("{0}&".format(item[1]),
                         "operator = ( {0}&& rhs )".format(item[1]),
                         "::zerobuf::Zerobuf::operator = ( std::move( rhs ));\n" +
                         move_operator(emit) +
                         "    return *this;")

        # introspection
        digest = emit.md5.hexdigest()
        high = digest[ 0 : len( digest ) - 16 ]
        low  = digest[ len( digest ) - 16: ]
        zerobufType = "::zerobuf::uint128_t( 0x{0}ull, 0x{1}ull )".format( high,
                                                                           low )
        header.write( "    // Introspection\n" )
        header.write( "    static ::zerobuf::uint128_t ZEROBUF_TYPE() {{ return {0}; }}\n".format( zerobufType ))
        header.write( "    ::zerobuf::uint128_t getZerobufType() const final {{ return {0}; }}\n".format( zerobufType ))
        header.write( "    size_t getZerobufStaticSize() const final {{ return {0}; }}\n".format( emit.offset ))
        header.write( "    static size_t ZEROBUF_STATIC_SIZE() {{ return {0}; }}\n".format( emit.offset ))
        header.write( "    size_t getZerobufNumDynamics() const final {{ return {0}; }}\n".format( emit.numDynamic ))
        header.write( "    static size_t ZEROBUF_NUM_DYNAMICS() {{ return {0}; }}\n".format( emit.numDynamic ))
        header.write( "\n" )

        # schema
        schema = "{{ {0}, {1},\n        {2},\n        {{\n         {3}\n         }} }}".format( emit.offset, emit.currentDyn, zerobufType, ',\n         '.join( emit.schema ))
        schemas = "return ::zerobuf::Schemas{{ ::zerobuf::Schemas{{ {0}::schema() {1} }}}}".format(
            emit.table, (', ' + ', '.join( emit.nestedSchemas) if len(emit.nestedSchemas) > 0 else '' ))
        emitFunction( "::zerobuf::Schema", "schema()",
                      "return {0};".format( schema ), True )
        emitFunction( "::zerobuf::Schemas", "schemas()",
                      "{0};".format( schemas ), True )
        emitFunction( "::zerobuf::Schemas", "getSchemas() const final",
                      "return schemas();" )
        header.write( "\n" )

        # closing statements
        header.write( "private:\n    {0}\n".
                      format( '\n    '.join( emit.members )))
        header.write( "};\n\n" )

        # record size of myself in type lookup table, 0 if dynamically sized
        emit.types[ item[1] ] = ( emit.offset if emit.numDynamic == 0 else 0,
                                  item[1] )
        emit.tables.add( item[1] )

    def root_type():
        header.write( "" )

    rootOptions = { "namespace" : namespace,
                    "enum" : enum,
                    "table" : table,
                    "root_type" : root_type,
                    }

    header.write( "// Generated by zerobufCxx.py\n\n" )
    header.write( "#pragma once\n" )
    header.write( "#include <zerobuf/Zerobuf.h> // base class\n" )
    header.write( "#include <zerobuf/Vector.h> // member\n" )
    header.write( "#include <array> // member\n" )
    header.write( "\n" )
    impl.write( "// Generated by zerobufCxx.py\n\n" )
    impl.write( "#include \"" + headerbase + ".h\"\n\n" )
    impl.write( "#include <zerobuf/Schema.h>\n" )
    impl.write( "#include <zerobuf/NonMovingSubAllocator.h>\n" )
    impl.write( "#include <zerobuf/StaticSubAllocator.h>\n" )
    impl.write( "\n" )

    for item in schema:
        rootOptions[ item[0] ]()

    for namespace in emit.namespace:
        header.write( "}\n" )
        impl.write( "}\n" )


if __name__ == "__main__":
    if len(sys.argv) < 2 :
        sys.exit("ERROR - " + sys.argv[0] + " - too few input arguments!")

    parser = argparse.ArgumentParser( description =
                                      "zerobufCxx.py: A zerobuf C++ code generator for extended flatbuffers schemas" )
    parser.add_argument( "files", nargs = "*" )
    parser.add_argument( '-o', '--outputdir', action='store', default = "",
                         help = "Prefix directory for all generated files.")

    # Parse, interpret and validate arguments
    args = parser.parse_args()
    if len(args.files) == 0 :
        sys.exit("ERROR - " + sys.argv[0] + " - no input .fbs files given!")

    for file in args.files:
        basename = os.path.splitext( file )[0]
        headerbase = os.path.basename( basename )
        if args.outputdir:
            if args.outputdir == '-':
                header = sys.stdout
                impl = sys.stdout
            else:
                header = open( args.outputdir + "/" + headerbase + ".h", 'w' )
                impl = open( args.outputdir + "/" + headerbase + ".cpp", 'w' )
        else:
            header = open( basename + ".h" , 'w' )
            impl = open( basename + ".cpp" , 'w' )

        schema = fbsObject.parseFile( file )
        # import pprint
        # pprint.pprint( schema.asList( ))
        emit()
