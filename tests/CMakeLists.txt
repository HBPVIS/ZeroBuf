# Copyright (c) HBP 2015 Daniel.Nachbaur@epfl.ch
# Change this number when adding tests to force a CMake run: 0

if(NOT BOOST_FOUND)
  return()
endif()

include(zerobufGenerateCxx)
zerobuf_generate_cxx(TESTJSON ${OUTPUT_INCLUDE_DIR} test_json.fbs)
set(TESTJSON_LIBRARY_TYPE STATIC)
set(TESTJSON_INCLUDE_NAME testjson)
common_library(testjson)

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ZeroBuf testjson)
include(CommonCTest)
