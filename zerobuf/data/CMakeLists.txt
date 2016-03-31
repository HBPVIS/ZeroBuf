# Copyright (c) HBP 2016 Jafet Villafranca <jafet.villafrancadiaz@epfl.ch>

set(ZEROBUFDATA_INCLUDE_NAME zerobuf/data)

include(zerobufGenerateCxx)
zerobuf_generate_cxx(ZEROBUFDATA
  ${OUTPUT_INCLUDE_DIR}/${ZEROBUFDATA_INCLUDE_NAME} frame.fbs)
zerobuf_generate_cxx(ZEROBUFDATADETAIL
  ${OUTPUT_INCLUDE_DIR}/${ZEROBUFDATA_INCLUDE_NAME}/detail progress.fbs)

set(ZEROBUFDATA_PUBLIC_HEADERS ${ZEROBUFDATA_HEADERS}
  ${ZEROBUFDATADETAIL_HEADERS} Progress.h)
list(APPEND ZEROBUFDATA_SOURCES ${ZEROBUFDATADETAIL_SOURCES} Progress.cpp)
set(ZEROBUFDATA_LINK_LIBRARIES PUBLIC ZeroBuf)

set(ZEROBUFDATA_INCLUDE_NAME zerobuf/data)
set(ZEROBUFDATA_NAMESPACE zerobufdata)
common_library(ZeroBufData)
