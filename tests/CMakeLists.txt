# Copyright (c) HBP 2015 Daniel.Nachbaur@epfl.ch
# Change this number when adding tests to force a CMake run: 0

set(TEST_LIBRARIES ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} zerobuf)

include(CommonCTest)
