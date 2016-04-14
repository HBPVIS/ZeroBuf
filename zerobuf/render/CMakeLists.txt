# Copyright (c) HBP 2015-2016 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

set(ZEROBUFRENDER_INCLUDE_NAME zerobuf/render)

include(zerobufGenerateCxx)
zerobuf_generate_cxx(ZEROBUFRENDER
  ${PROJECT_BINARY_DIR}/include/${ZEROBUFRENDER_INCLUDE_NAME}
  camera.fbs frame.fbs lookOut.fbs lookupTable1D.fbs imageJPEG.fbs)

set(ZEROBUFRENDER_PUBLIC_HEADERS ${ZEROBUFRENDER_HEADERS})
set(ZEROBUFRENDER_LINK_LIBRARIES PUBLIC ZeroBuf)

set(ZEROBUFRENDER_INCLUDE_NAME zerobuf/render)
set(ZEROBUFRENDER_NAMESPACE zerobufrender)
common_library(ZeroBufRender)
