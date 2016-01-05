# Copyright (c) HBP 2015 Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(ZeroBuf)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${PROJECT_SOURCE_DIR}/zerobuf/share/zerobuf/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "2")
set(VERSION_PATCH "0")
set(VERSION_ABI 1)

include(Common)

set(COMMON_PROJECT_DOMAIN eu.humanbrainproject)
set(ZEROBUF_DESCRIPTION
  "Zero-copy, zero-serialize, zero-hassle protocol buffers")
set(ZEROBUF_MAINTAINER "Human Brain Project <HBPVis@googlegroups.com>")
set(ZEROBUF_LICENSE LGPL)
set(ZEROBUF_DEPENDENT_LIBRARIES Servus)

common_package(Boost 1.49 COMPONENTS unit_test_framework)
common_package(Servus REQUIRED)
common_package_post()

set(LCOV_EXCLUDE "zerobuf/jsoncpp/*")
add_subdirectory(zerobuf)
add_subdirectory(tests)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md
  ${PROJECT_SOURCE_DIR}/doc/)
include(CPackConfig)
include(DoxygenRule)
