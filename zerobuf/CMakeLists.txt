# Copyright (c) HBP 2015 Stefan.Eilemann@epfl.ch

set(ZEROBUF_PUBLIC_HEADERS
  Allocator.h
  BaseVector.h
  ConstVector.h
  Generic.h
  NonMovingAllocator.h
  NonMovingBaseAllocator.h
  NonMovingSubAllocator.h
  Vector.h
  Schema.h
  Types.h
  jsoncpp/json/json-forwards.h
  )

set(ZEROBUF_HEADERS
  detail/JsonSerialization.h
  jsoncpp/json/json.h
  )

set(ZEROBUF_SOURCES
  Generic.cpp
  NonMovingAllocator.cpp
  NonMovingBaseAllocator.cpp
  NonMovingSubAllocator.cpp
  Zerobuf.cpp
  jsoncpp/jsoncpp.cpp
  )

set_source_files_properties(jsoncpp/jsoncpp.cpp PROPERTIES COMPILE_FLAGS
  -Wno-shadow)

set(ZEROBUF_LINK_LIBRARIES ${SERVUS_LIBRARIES})
set(ZEROBUF_INCLUDE_NAME zerobuf)
set(ZEROBUF_OMIT_LIBRARY_HEADER ON)
common_library(ZeroBuf)

configure_file(${CMAKE_CURRENT_LIST_DIR}/share/zerobuf/CMake/zerobufGenerateCxx.cmake ${PROJECT_BINARY_DIR}/${CMAKE_MODULE_INSTALL_PATH}/zerobufGenerateCxx.cmake COPYONLY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/bin/zerobufCxx.py ${PROJECT_BINARY_DIR}/bin/zerobufCxx.py COPYONLY)
install(FILES bin/zerobufCxx.py DESTINATION bin COMPONENT bin)
install(FILES share/zerobuf/CMake/zerobufGenerateCxx.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev)
