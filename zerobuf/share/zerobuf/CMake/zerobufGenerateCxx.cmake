# Copyright (c) 2015 Human Brain Project
#                    Stefan.Eilemann@epfl.ch
#
# Function to generate zerobuf C++ headers for fbs schemas
# Outputs:
# * ${Name}_HEADERS: names of generated header files

set(ZEROBUF_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..)

function(zerobuf_generate_cxx Name OutputDir)
  set(${Name}_HEADERS)
  foreach(FILE ${ARGN})
    get_filename_component(ZEROBUF_OUTPUT ${FILE} NAME_WE)
    set(ZEROBUF_HEADER "${OutputDir}/${ZEROBUF_OUTPUT}.h"
                        "${OutputDir}/${ZEROBUF_OUTPUT}.ipp")
    list(APPEND ${Name}_HEADERS ${ZEROBUF_HEADER})

    add_custom_command(
      COMMAND ${ZEROBUF_BASE_DIR}/bin/zerobufCxx.py
      ARGS -o "${OutputDir}" ${FILE}
      COMMENT "Building zerobuf C++ headers for ${FILE}"
      DEPENDS ${FILE} ${ZEROBUF_BASE_DIR}/bin/zerobufCxx.py
      OUTPUT ${ZEROBUF_HEADER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
  set(${Name}_HEADERS ${${Name}_HEADERS} PARENT_SCOPE)
endfunction()
