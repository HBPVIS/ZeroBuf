
/* Copyright (c) 2015, Human Brain Project
 *                     Daniel Nachbaur <danielnachbaur@epfl.ch>
 */


#include <testSchema.h>

#define SETVALUES(type, name) \
    const std::vector< type > name##Vector { type(1), type(1), type(2), type(3) }; \
    const type name##value( 42 ); \
    \
    object.set##name##dynamic( name##Vector ); \
    object.set##name##array( name##Vector ); \
    object.set##name##value( name##value );

#define TESTVALUES(type, name) \
    const std::vector< type > expected##name##Vector { type(1), type(1), type(2), type(3) }; \
    const type expected##name##value( 42 ); \
    \
    const std::vector< type >& name##Dynamic( object.get##name##dynamicVector( )); \
    const std::vector< type >& name##Array( object.get##name##arrayVector( )); \
    const type& name##Value( object.get##name##value( )); \
    \
    BOOST_CHECK_EQUAL_COLLECTIONS( expected##name##Vector.begin(), expected##name##Vector.end(),\
                                   name##Dynamic.begin(), name##Dynamic.end( )); \
    BOOST_CHECK_EQUAL_COLLECTIONS( expected##name##Vector.begin(), expected##name##Vector.end(),\
                                   name##Array.begin(), name##Array.end( )); \
    BOOST_CHECK_EQUAL( expected##name##value, name##Value );


test::TestSchema getTestObject()
{
    test::TestSchema object;
    SETVALUES(int32_t, Int);
    SETVALUES(uint32_t, Uint);
    SETVALUES(float, Float);
    SETVALUES(double, Double);
    SETVALUES(int8_t, Byte);
    SETVALUES(int16_t, Short);
    SETVALUES(uint8_t, Ubyte);
    SETVALUES(uint16_t, Ushort);
    SETVALUES(uint64_t, Ulong);
    SETVALUES(uint8_t, Uint8_t);
    SETVALUES(uint16_t, Uint16_t);
    SETVALUES(uint32_t, Uint32_t);
    SETVALUES(uint64_t, Uint64_t);
    SETVALUES(servus::uint128_t, Uint128_t);
    SETVALUES(int8_t, Int8_t);
    SETVALUES(int16_t, Int16_t);
    SETVALUES(int32_t, Int32_t);
    SETVALUES(int64_t, Int64_t);
    object.setBoolvalue( true );
    object.setStringvalue( "testmessage" );

    object.setEnumeration( test::TestEnum_SECOND );
    const std::vector<test::TestEnum> testEnums = { test::TestEnum_FIRST,
                                                    test::TestEnum_SECOND };
    object.setEnumerations( testEnums );

    int32_t intMagic = 42;
    uint32_t uintMagic = 43;

    // Write nested table using a stack object
    test::TestNested nested;
    nested.setIntvalue( intMagic );
    nested.setUintvalue( uintMagic );
    object.setNested( nested );

    // Writable copy of the table is acquired from parent schema
    test::TestNested mutableNested = object.getNested();
    mutableNested.setIntvalue( intMagic );
    mutableNested.setUintvalue( uintMagic );
    BOOST_CHECK_EQUAL( mutableNested.getIntvalue(), intMagic  );
    BOOST_CHECK_EQUAL( mutableNested.getUintvalue(), uintMagic  );
    BOOST_CHECK_EQUAL( object.getNested().getIntvalue(), intMagic  );
    BOOST_CHECK_EQUAL( object.getNested().getUintvalue(), uintMagic  );

    // Non writable copy of the table is acquired from parent schema
    const test::TestSchema& constObject = object;
    const test::TestNested& constNested = constObject.getNested();
    BOOST_CHECK_EQUAL( constNested.getIntvalue(), intMagic  );
    BOOST_CHECK_EQUAL( constNested.getUintvalue(), uintMagic  );

    // Copy of non writable object should write into it's own memory
    test::TestNested nestedCopy = constObject.getNested();
    BOOST_CHECK_EQUAL( nestedCopy.getIntvalue(), intMagic  );
    BOOST_CHECK_EQUAL( nestedCopy.getUintvalue(), uintMagic  );

    nestedCopy.setIntvalue( 2 * intMagic );
    nestedCopy.setUintvalue( 2 * uintMagic );
    BOOST_CHECK_EQUAL( nestedCopy.getIntvalue(), 2 * intMagic  );
    BOOST_CHECK_EQUAL( nestedCopy.getUintvalue(), 2 * uintMagic  );
    BOOST_CHECK_EQUAL( object.getNested().getIntvalue(), intMagic  );
    BOOST_CHECK_EQUAL( object.getNested().getUintvalue(), uintMagic  );

    // Writable copy of the table is acquired from parent schema
    std::vector< test::TestNested > nonConstTables = object.getNestedarrayVector();

    intMagic = 42;
    uintMagic = 43;

    for( std::vector< test::TestNested >::iterator it = nonConstTables.begin();
         it != nonConstTables.end(); ++it )
    {
        test::TestNested& inner = *it;
        inner.setIntvalue( intMagic++ );
        inner.setUintvalue( uintMagic++ );
    }

    // Non writable copy of the tables are acquired from parent schema
    std::vector< test::TestNested > constTables =
                        static_cast<const test::TestSchema&>(object).getNestedarrayVector();

    intMagic = 42;
    uintMagic = 43;
    for( std::vector< test::TestNested >::iterator it = constTables.begin();
         it != constTables.end(); ++it )
    {
        test::TestNested& inner = *it;
        BOOST_CHECK_EQUAL( inner.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( inner.getUintvalue(), uintMagic++  );
        BOOST_REQUIRE_THROW( inner.setIntvalue( intMagic ), std::runtime_error );
    }

    intMagic = 42;
    uintMagic = 43;

    // Writable nested tables
    std::vector< test::TestNested > nesteds;
    for( size_t i = 0; i < constTables.size(); ++i  )
    {
        test::TestNested inner;
        inner.setIntvalue( intMagic++ );
        inner.setUintvalue( uintMagic++ );
        nesteds.push_back( inner );
    }
    object.setNestedarray( nesteds );

    intMagic = 42;
    uintMagic = 43;

    // Setting dynamic tables
    nesteds.clear();
    for( size_t i = 0; i < 2; ++i  )
    {
        test::TestNested inner;
        inner.setIntvalue( intMagic++ );
        inner.setUintvalue( uintMagic++ );
        nesteds.push_back( inner );
    }
    object.setNesteddynamic( nesteds );

    return object;
}

void checkTestObject( const test::TestSchema& object )
{
    TESTVALUES(int32_t, Int);
    TESTVALUES(uint32_t, Uint);
    TESTVALUES(float, Float);
    TESTVALUES(double, Double);
    TESTVALUES(int8_t, Byte);
    TESTVALUES(int16_t, Short);
    TESTVALUES(uint8_t, Ubyte);
    TESTVALUES(uint16_t, Ushort);
    TESTVALUES(uint64_t, Ulong);
    TESTVALUES(uint8_t, Uint8_t);
    TESTVALUES(uint16_t, Uint16_t);
    TESTVALUES(uint32_t, Uint32_t);
    TESTVALUES(uint64_t, Uint64_t);
    TESTVALUES(servus::uint128_t, Uint128_t);
    TESTVALUES(int8_t, Int8_t);
    TESTVALUES(int16_t, Int16_t);
    TESTVALUES(int32_t, Int32_t);
    TESTVALUES(int64_t, Int64_t);
    BOOST_CHECK( object.getBoolvalue( ));
    BOOST_CHECK_EQUAL( object.getStringvalueString(), "testmessage" );

    const test::TestNested& nested = object.getNested( );
    BOOST_CHECK_EQUAL( nested.getIntvalue(), 42  );
    BOOST_CHECK_EQUAL( nested.getUintvalue(), 43  );

    std::vector< test::TestNested > tables = object.getNestedarrayVector();

    // Test retrieved tables
    int32_t intMagic = 42;
    uint32_t uintMagic = 43;
    for( std::vector< test::TestNested >::iterator it = tables.begin();
         it != tables.end(); ++it )
    {
        test::TestNested& inner = *it;
        BOOST_CHECK_EQUAL( inner.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( inner.getUintvalue(), uintMagic++  );
    }

    // Test retrieved dynamic tables vector
    intMagic = 42;
    uintMagic = 43;
    const auto& tablesDynamicVector = object.getNestedarrayVector();
    for( const test::TestNested& inner : tablesDynamicVector )
    {
        BOOST_CHECK_EQUAL( inner.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( inner.getUintvalue(), uintMagic++  );
    }

    // Test retrieved dynamic tables
    intMagic = 42;
    uintMagic = 43;
    ::zerobuf::ConstVector< test::TestNested > dynamicTables =
          object.getNesteddynamic();
    for( size_t i = 0; i < dynamicTables.size(); ++i )
    {
        const test::TestNested& inner = dynamicTables[i];
        BOOST_CHECK_EQUAL( inner.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( inner.getUintvalue(), uintMagic++  );
    }

    BOOST_REQUIRE_THROW( dynamicTables.data(), std::runtime_error );
}
