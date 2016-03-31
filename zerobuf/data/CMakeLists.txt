# Copyright (c) HBP 2016 Jafet Villafranca <jafet.villafrancadiaz@epfl.ch>

set(ZEROBUFDATA_INCLUDE_NAME zerobuf/data)

include(zerobufGenerateCxx)
zerobuf_generate_cxx(ZEROBUFDATA
  ${OUTPUT_INCLUDE_DIR}/${ZEROBUFDATA_INCLUDE_NAME} frame.fbs)

set(ZEROBUFDATA_PUBLIC_HEADERS ${ZEROBUFDATA_HEADERS})
set(ZEROBUFDATA_LINK_LIBRARIES PUBLIC ZeroBuf)

set(ZEROBUFDATA_INCLUDE_NAME zerobuf/data)
set(ZEROBUFDATA_NAMESPACE zerobufdata)
common_library(ZeroBufData)
