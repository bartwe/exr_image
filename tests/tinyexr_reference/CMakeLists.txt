cmake_minimum_required(VERSION 3.16)
project(exri_tinyexr_reference C CXX)

if(NOT DEFINED EXRI_ROOT_DIR)
  get_filename_component(EXRI_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()

if(NOT DEFINED EXRI_TINYEXR_SOURCE_DIR)
  message(FATAL_ERROR "EXRI_TINYEXR_SOURCE_DIR is required")
endif()

add_executable(exri_tinyexr_reference
  tinyexr_reference.cpp
  "${EXRI_TINYEXR_SOURCE_DIR}/deps/miniz/miniz.c")

set_target_properties(exri_tinyexr_reference PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO)

target_include_directories(exri_tinyexr_reference PRIVATE
  "${EXRI_ROOT_DIR}"
  "${EXRI_TINYEXR_SOURCE_DIR}"
  "${EXRI_TINYEXR_SOURCE_DIR}/deps/miniz")

target_compile_definitions(exri_tinyexr_reference PRIVATE
  TINYEXR_USE_MINIZ=1
  TINYEXR_USE_OPENMP=0
  TINYEXR_USE_THREAD=0
  TINYEXR_USE_ZFP=0)

if(UNIX)
  target_link_libraries(exri_tinyexr_reference PRIVATE m)
endif()
