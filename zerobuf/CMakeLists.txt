# Copyright (c) HBP 2015 Stefan.Eilemann@epfl.ch

set(ZEROBUF_PUBLIC_HEADERS
  Allocator.h
  Generic.h
  Schema.h
  Vector.h
  Zerobuf.h
  jsoncpp/json/json-forwards.h
  types.h
  )

set(ZEROBUF_HEADERS
  detail/base64.h
  detail/JsonConverter.h
  DynamicSubAllocator.h
  NonMovingAllocator.h
  NonMovingBaseAllocator.h
  NonMovingSubAllocator.h
  StaticSubAllocator.h
  jsoncpp/json/json.h
  )

set(ZEROBUF_SOURCES
  detail/base64.cpp
  DynamicSubAllocator.cpp
  Generic.cpp
  NonMovingAllocator.cpp
  NonMovingBaseAllocator.cpp
  NonMovingSubAllocator.cpp
  Schema.cpp
  StaticSubAllocator.cpp
  Zerobuf.cpp
  jsoncpp/jsoncpp.cpp
  )

list(APPEND CPPCHECK_EXTRA_ARGS
  --suppress=*:${CMAKE_CURRENT_LIST_DIR}/jsoncpp/*)

if(MSVC)
  set_source_files_properties(jsoncpp/jsoncpp.cpp PROPERTIES COMPILE_FLAGS
    /wd4267)
  set_source_files_properties(ZeroBuf.cpp PROPERTIES COMPILE_FLAGS
    /wd4267)
else()
set_source_files_properties(jsoncpp/jsoncpp.cpp PROPERTIES COMPILE_FLAGS
  -Wno-shadow)
endif()

set(ZEROBUF_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT} PUBLIC Servus)
set(ZEROBUF_INCLUDE_NAME zerobuf)
set(ZEROBUF_OMIT_LIBRARY_HEADER ON)
common_library(ZeroBuf)

add_subdirectory(render)
