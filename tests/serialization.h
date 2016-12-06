
/* Copyright (c) 2015-2016, Human Brain Project
 *                          Daniel Nachbaur <danielnachbaur@epfl.ch>
 */

#include <testschema/testSchema.h>

#define SETVALUES(type, name) \
    const std::vector< type > name##Vector { type(1), type(1), type(2), type(3) }; \
    const type name##value( 42 ); \
    \
    object.set##name##dynamic( name##Vector ); \
    object.set##name##array( name##Vector ); \
    object.set##name##value( name##value );

#define TESTVALUES( type, name )                                        \
    const std::vector< type > expected##name##Vector { type(1),         \
            type(1), type(2), type(3) };                                \
    const type expected##name##value( 42 );                             \
                                                                        \
    const std::vector< type >& name##Dynamic(                           \
        object.get##name##dynamicVector( ));                            \
    const std::vector< type >& name##Array(                             \
        object.get##name##arrayVector( ));                              \
    const type& name##Value( object.get##name##value( ));               \
                                                                        \
    BOOST_CHECK_EQUAL( expected##name##Vector.size(),                   \
                       name##Dynamic.size( ));                          \
    BOOST_CHECK_EQUAL( expected##name##Vector.size(),                   \
                       name##Array.size( ));                            \
    BOOST_CHECK_EQUAL_COLLECTIONS( expected##name##Vector.begin(),      \
                                   expected##name##Vector.end(),        \
                                   name##Dynamic.begin(),               \
                                   name##Dynamic.end( ));               \
    BOOST_CHECK_EQUAL_COLLECTIONS( expected##name##Vector.begin(),      \
                                   expected##name##Vector.end(),        \
                                   name##Array.begin(),                 \
                                   name##Array.end( ));                 \
    BOOST_CHECK_EQUAL( expected##name##value, name##Value );


test::TestSchema getTestObject()
{
    test::TestSchema object;
    SETVALUES(int32_t, Int);
    SETVALUES(uint32_t, Uint);
    SETVALUES(float, Float);
    SETVALUES(double, Double);
    SETVALUES(::zerobuf::byte_t, Byte);
    SETVALUES(int16_t, Short);
    SETVALUES(::zerobuf::byte_t, Ubyte);
    SETVALUES(uint16_t, Ushort);
    SETVALUES(uint64_t, Ulong);
    SETVALUES(uint8_t, Uint8_t);
    SETVALUES(uint16_t, Uint16_t);
    SETVALUES(uint32_t, Uint32_t);
    SETVALUES(uint64_t, Uint64_t);
    SETVALUES(::zerobuf::uint128_t, Uint128_t);
    SETVALUES(int8_t, Int8_t);
    SETVALUES(int16_t, Int16_t);
    SETVALUES(int32_t, Int32_t);
    SETVALUES(int64_t, Int64_t);
    object.setBoolvalue( true );
    object.setStringvalue( "testmessage" );

    object.setEnumeration( test::TestEnum::SECOND );
    const std::vector<test::TestEnum> testEnums = { test::TestEnum::FIRST,
                                                    test::TestEnum::SECOND };
    object.setEnumarray( testEnums );

    object.getEnumdynamic().push_back( test::TestEnum::SECOND );
    object.getEnumdynamic().push_back( test::TestEnum::THIRD_UNDERSCORE );

    int32_t intMagic = 42;
    uint32_t uintMagic = 4200;

    // Write nested table using a stack object
    test::TestNested nested;
    nested.setIntvalue( intMagic );
    nested.setUintvalue( uintMagic );
    object.setNested( nested );

    // Writable nested tables
    std::array< test::TestNested, 4 > nesteds;
    for( test::TestNested& inner : nesteds )
    {
        inner.setIntvalue( intMagic++ );
        inner.setUintvalue( uintMagic++ );
    }
    object.setNestedarray( nesteds );

    // Dynamic tables
    intMagic = 42;
    uintMagic = 4200;
    std::vector< test::TestNested > nestedDyn;
    for( size_t i = 0; i < 5; ++i  )
        nestedDyn.push_back( test::TestNested( intMagic++, uintMagic++ ));
    object.setNesteddynamic( nestedDyn );

    for( size_t i = 0; i < 5; ++i  )
    {
        test::TestNested inner;
        inner.setIntvalue( intMagic++ );
        inner.setUintvalue( uintMagic++ );
        object.getNesteddynamic().push_back( inner );
    }

    object.getNestedMember().setName( "Hugo" );
    return object;
}

void checkTestObject( const test::TestNested& nested )
{
    BOOST_CHECK_EQUAL( nested.getIntvalue(), 42 );
    BOOST_CHECK_EQUAL( nested.getUintvalue(), 4200 );
}

void checkTestObject( const test::TestSchema& object )
{
    object.check();

    TESTVALUES(int32_t, Int);
    TESTVALUES(uint32_t, Uint);
    TESTVALUES(float, Float);
    TESTVALUES(double, Double);
    TESTVALUES(::zerobuf::byte_t, Byte);
    TESTVALUES(int16_t, Short);
    TESTVALUES(::zerobuf::byte_t, Ubyte);
    TESTVALUES(uint16_t, Ushort);
    TESTVALUES(uint64_t, Ulong);
    TESTVALUES(uint8_t, Uint8_t);
    TESTVALUES(uint16_t, Uint16_t);
    TESTVALUES(uint32_t, Uint32_t);
    TESTVALUES(uint64_t, Uint64_t);
    TESTVALUES(::zerobuf::uint128_t, Uint128_t);
    TESTVALUES(int8_t, Int8_t);
    TESTVALUES(int16_t, Int16_t);
    TESTVALUES(int32_t, Int32_t);
    TESTVALUES(int64_t, Int64_t);
    BOOST_CHECK( object.getBoolvalue( ));
    BOOST_CHECK_EQUAL( object.getStringvalueString(), "testmessage" );
    BOOST_CHECK_EQUAL( object.getEnumeration(), test::TestEnum::SECOND );

    const std::vector<test::TestEnum> testEnums = { test::TestEnum::FIRST,
                                                    test::TestEnum::SECOND };
    const std::vector<test::TestEnum>& result = object.getEnumarrayVector();
    BOOST_CHECK_EQUAL_COLLECTIONS( testEnums.begin(), testEnums.end(),
                                   result.begin(), result.end( ));

    const std::vector<test::TestEnum> testEnumDynamic =
        { test::TestEnum::SECOND, test::TestEnum::THIRD_UNDERSCORE };

    const std::vector<test::TestEnum>& result2 = object.getEnumdynamicVector();
    BOOST_CHECK_EQUAL_COLLECTIONS( testEnumDynamic.begin(), testEnumDynamic.end(),
                                   result2.begin(), result2.end( ));

    checkTestObject( object.getNested( ));

    // Test retrieved tables
    BOOST_CHECK_EQUAL( object.getNestedarraySize(), 4 );
    const auto& tables = object.getNestedarray();
    int32_t intMagic = 42;
    uint32_t uintMagic = 4200;
    for( const auto& inner : tables )
    {
        inner.check();
        BOOST_CHECK_EQUAL( inner.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( inner.getUintvalue(), uintMagic++  );
    }

    // Test retrieved dynamic tables
    intMagic = 42;
    uintMagic = 4200;
    const test::TestSchema::Nesteddynamic& dynamicTables =
        object.getNesteddynamic();
    for( size_t i = 0; i < dynamicTables.size(); ++i )
    {
        const test::TestNested& inner = dynamicTables[i];
        inner.check();
        BOOST_CHECK_EQUAL( inner.getIntvalue(), intMagic++ );
        BOOST_CHECK_EQUAL( inner.getUintvalue(), uintMagic++  );
    }

    object.getNestedMember().check();
    BOOST_CHECK_EQUAL( object.getNestedMember().getNameString(), "Hugo" );
    BOOST_CHECK_EQUAL( object.getNestedMember().toBinary().size, 28 );
}
