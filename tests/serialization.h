
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
    const std::vector<test::TestEnum> testEnums = { test::TestEnum_FIRST, test::TestEnum_SECOND };
    object.setEnumerations( testEnums );

    // Writable copy of the table is acquired from parent schema
    test::TestNestedTable nonConstestedTable = object.getTable( );
    nonConstestedTable.setIntvalue( 42 );
    nonConstestedTable.setUintvalue( 43 );
    BOOST_CHECK_EQUAL( nonConstestedTable.getIntvalue(), 42  );
    BOOST_CHECK_EQUAL( nonConstestedTable.getUintvalue(), 43  );

    // Non writable copy of the table is acquired from parent schema
    test::TestNestedTable constNestedTable = static_cast<const test::TestSchema&>( object ).getTable();
    BOOST_CHECK_EQUAL( constNestedTable.getIntvalue(), 42  );
    BOOST_CHECK_EQUAL( constNestedTable.getUintvalue(), 43  );
    BOOST_REQUIRE_THROW( constNestedTable.setIntvalue( 42 ), std::runtime_error );
    BOOST_REQUIRE_THROW( constNestedTable.setUintvalue( 43 ), std::runtime_error );

    int32_t intMagic = 42;
    uint32_t uintMagic = 43;

    // Writable copy of nested table
    test::TestNestedTable nestedTable;
    nestedTable.setIntvalue( intMagic );
    nestedTable.setUintvalue( uintMagic );
    object.setTable( nestedTable );

    // Writable copy of the table is acquired from parent schema
    std::vector< test::TestNestedTable > nonConstTables = object.getTablesVector();

    intMagic = 42;
    uintMagic = 43;

    for( std::vector< test::TestNestedTable >::iterator it = nonConstTables.begin();
         it != nonConstTables.end(); ++it )
    {
        test::TestNestedTable& table = *it;
        table.setIntvalue( intMagic++ );
        table.setUintvalue( uintMagic++ );
    }

    // Non writable copy of the table is acquired from parent schema
    std::vector< test::TestNestedTable > constTables =
                                static_cast<const test::TestSchema&>(object).getTablesVector();

    intMagic = 42;
    uintMagic = 43;
    for( std::vector< test::TestNestedTable >::iterator it = constTables.begin();
         it != constTables.end(); ++it )
    {
        test::TestNestedTable& table = *it;
        BOOST_CHECK_EQUAL( table.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( table.getUintvalue(), uintMagic++  );
        BOOST_REQUIRE_THROW( table.setIntvalue( intMagic ), std::runtime_error );
    }

    intMagic = 42;
    uintMagic = 43;
    std::vector< test::TestNestedTable > tables;
    for( size_t i = 0; i < constTables.size(); ++i  )
    {
        test::TestNestedTable table;
        table.setIntvalue( intMagic++ );
        table.setUintvalue( uintMagic++ );
        tables.push_back( std::move( table ));
    }
    object.setTables( tables );

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

    const test::TestNestedTable& nestedTable = object.getTable( );
    BOOST_CHECK_EQUAL( nestedTable.getIntvalue(), 42  );
    BOOST_CHECK_EQUAL( nestedTable.getUintvalue(), 43  );

    std::vector< test::TestNestedTable > tables = object.getTablesVector();

    int32_t intMagic = 42;
    uint32_t uintMagic = 43;
    for( std::vector< test::TestNestedTable >::iterator it = tables.begin();
         it != tables.end(); ++it )
    {
        test::TestNestedTable& table = *it;
        BOOST_CHECK_EQUAL( table.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( table.getUintvalue(), uintMagic++  );
    }
}
