# Copyright (c) HBP 2015 Stefan.Eilemann@epfl.ch

set(ZEROBUF_PUBLIC_HEADERS
  ${COMMON_INCLUDES}
  Allocator.h
  BaseVector.h
  ConstVector.h
  NonMovingAllocator.h
  NonMovingBaseAllocator.h
  NonMovingSubAllocator.h
  Vector.h
  jsoncpp/json/json-forwards.h
  jsoncpp/json/json.h
  types.h
  )

set(ZEROBUF_HEADERS
  )

set(ZEROBUF_SOURCES
  ${COMMON_SOURCES}
  NonMovingAllocator.cpp
  NonMovingBaseAllocator.cpp
  NonMovingSubAllocator.cpp
  Zerobuf.cpp
  jsoncpp/jsoncpp.cpp
  )

set_source_files_properties(jsoncpp/jsoncpp.cpp PROPERTIES COMPILE_FLAGS
  -Wno-shadow)

set(ZEROBUF_LINK_LIBRARIES ${SERVUS_LIBRARIES})
set(PROJECT_INCLUDE_NAME zerobuf)
set(ZEROBUF_OMIT_PROJECT_HEADER ON)
common_library(zerobuf)

install(FILES bin/zerobufCxx.py DESTINATION bin COMPONENT bin)
install(FILES share/zerobuf/CMake/zerobufGenerateCxx.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev)
