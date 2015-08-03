# Copyright (c) HBP 2015 Daniel Nachbaur <daniel.nachbaur@epfl.ch>
# All rights reserved. Do not distribute without further notice.

set(ZEROBUFRENDER_INCLUDE_NAME zerobuf/render)

include(zerobufGenerateCxx)
zerobuf_generate_cxx(ZEROBUFRENDER
  ${OUTPUT_INCLUDE_DIR}/${ZEROBUFRENDER_INCLUDE_NAME} camera.fbs)

set(ZEROBUFRENDER_PUBLIC_HEADERS ${ZEROBUFRENDER_HEADERS})
set(ZEROBUFRENDER_LINK_LIBRARIES ZeroBuf)

set(ZEROBUFRENDER_INCLUDE_NAME zerobuf/render)
set(ZEROBUFRENDER_NAMESPACE zerobufrender)
common_library(ZeroBufRender)
