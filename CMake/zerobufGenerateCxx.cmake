# Copyright (c) 2015-2016 Human Brain Project
#                         Stefan.Eilemann@epfl.ch
#
# Function to generate zerobuf C++ headers for fbs schemas
# Outputs:
# * ${Name}_HEADERS: names of generated header files
# * ${Name}_SOURCES: names of generated source files

if(NOT PYTHON_EXECUTABLE)
  find_package(PythonInterp REQUIRED)
endif()
find_file(ZEROBUF_CXX zerobufCxx.py HINTS
  ${CMAKE_CURRENT_LIST_DIR}/../../../bin # from share/zerobuf/CMake (!Windows)
  ${CMAKE_CURRENT_LIST_DIR}/../../bin # from zerobuf/CMake (Windows)
  ${CMAKE_CURRENT_LIST_DIR}/../bin) # from build dir
if(NOT ZEROBUF_CXX)
  message(FATAL_ERROR "zerobufCxx.py not found")
endif()

function(zerobuf_generate_cxx Name OutputDir)
  set(${Name}_HEADERS)
  set(${Name}_SOURCES)
  foreach(FILE ${ARGN})
    get_filename_component(ZEROBUF_OUTPUT ${FILE} NAME_WE)
    set(ZEROBUF_HEADER "${OutputDir}/${ZEROBUF_OUTPUT}.h")
    if(MSVC)
      set(ZEROBUF_SOURCE_EXTENSION ipp)
    else()
      set(ZEROBUF_SOURCE_EXTENSION cpp)
    endif()
    set(ZEROBUF_SOURCE "${OutputDir}/${ZEROBUF_OUTPUT}.${ZEROBUF_SOURCE_EXTENSION}")
    list(APPEND ${Name}_HEADERS ${ZEROBUF_HEADER})
    list(APPEND ${Name}_SOURCES ${ZEROBUF_SOURCE})

    add_custom_command(
      COMMAND ${PYTHON_EXECUTABLE} ${ZEROBUF_CXX}
      ARGS -o "${OutputDir}" -e ${ZEROBUF_SOURCE_EXTENSION} ${ZEROBUF_EXTRA_ARGS} ${FILE}
      COMMENT "Building zerobuf C++ headers for ${FILE} in ${OutputDir}"
      DEPENDS ${FILE} ${ZEROBUF_CXX}
      OUTPUT ${ZEROBUF_HEADER} ${ZEROBUF_SOURCE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
  set(${Name}_HEADERS ${${Name}_HEADERS} PARENT_SCOPE)
  set(${Name}_SOURCES ${${Name}_SOURCES} PARENT_SCOPE)
endfunction()

macro(zerobuf_generate_qobject Name OutputDir)
  set(ZEROBUF_EXTRA_ARGS "-q")
  zerobuf_generate_cxx(${Name} ${OutputDir} ${ARGN})
endmacro()
