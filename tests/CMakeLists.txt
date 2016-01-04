# Copyright (c) HBP 2015 Daniel.Nachbaur@epfl.ch
# Change this number when adding tests to force a CMake run: 2

if(NOT BOOST_FOUND)
  return()
endif()

include(zerobufGenerateCxx)
zerobuf_generate_cxx(TESTSCHEMA ${CMAKE_CURRENT_BINARY_DIR} testSchema.fbs)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(TESTSCHEMA_LIBRARY_TYPE STATIC)
set(TESTSCHEMA_INCLUDE_NAME testschema)
set(TESTSCHEMA_LINK_LIBRARIES ZeroBuf)
set(TESTSCHEMA_OMIT_EXPORT ON)
common_library(testschema)

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ZeroBuf testschema ZeroBufRender)
include(CommonCTest)
