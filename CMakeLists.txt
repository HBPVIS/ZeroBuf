# Copyright (c) HBP 2015 Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(ZeroBuf VERSION 0.3.0)
set(ZeroBuf_VERSION_ABI 2)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
  ${PROJECT_SOURCE_DIR}/zerobuf/share/zerobuf/CMake
  ${CMAKE_SOURCE_DIR}/CMake/common)
include(GitExternal)

set(ZEROBUF_DEB_DEPENDS python-pyparsing)

include(Common)

set(COMMON_PROJECT_DOMAIN eu.humanbrainproject)
set(ZEROBUF_DESCRIPTION
  "Zero-copy, zero-serialize, zero-hassle protocol buffers")
set(ZEROBUF_MAINTAINER "Human Brain Project <HBPVis@googlegroups.com>")
set(ZEROBUF_LICENSE LGPL)
set(ZEROBUF_DEPENDENT_LIBRARIES Servus)

common_find_package(Boost 1.49 COMPONENTS unit_test_framework)
common_find_package(Servus REQUIRED)
common_find_package(PythonInterp REQUIRED)
common_find_package_post()

set(LCOV_EXCLUDE "zerobuf/jsoncpp/*")

add_subdirectory(bin)
add_subdirectory(zerobuf)
add_subdirectory(tests)

configure_file(CMake/zerobufGenerateCxx.cmake
  ${PROJECT_BINARY_DIR}/${CMAKE_MODULE_INSTALL_PATH}/zerobufGenerateCxx.cmake
  COPYONLY)
install(FILES CMake/zerobufGenerateCxx.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev)

set(DOXYGEN_MAINPAGE_MD README.md)
set(DOXYGEN_EXTRA_INPUT ${PROJECT_SOURCE_DIR}/README.md
  ${PROJECT_SOURCE_DIR}/doc/)
include(CPackConfig)
include(DoxygenRule)
