cmake_minimum_required(VERSION 3.16)
project(exri_bench C CXX)

if(NOT DEFINED EXRI_ROOT_DIR)
  get_filename_component(EXRI_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()

if(NOT DEFINED EXRI_TINYEXR_SOURCE_DIR)
  message(FATAL_ERROR "EXRI_TINYEXR_SOURCE_DIR is required")
endif()

add_executable(exri_bench bench.cpp
  "${EXRI_TINYEXR_SOURCE_DIR}/deps/miniz/miniz.c")

set_target_properties(exri_bench PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO)

target_include_directories(exri_bench PRIVATE
  "${EXRI_TINYEXR_SOURCE_DIR}"
  "${EXRI_TINYEXR_SOURCE_DIR}/deps/miniz")

target_compile_definitions(exri_bench PRIVATE
  TINYEXR_USE_MINIZ=1
  TINYEXR_USE_OPENMP=0
  TINYEXR_USE_THREAD=0
  TINYEXR_USE_ZFP=0)

if(NOT DEFINED EXRI_USE_OPENEXR_PACKAGE)
  set(EXRI_USE_OPENEXR_PACKAGE ON)
endif()

if(EXRI_USE_OPENEXR_PACKAGE)
  find_package(OpenEXR REQUIRED CONFIG)
  target_link_libraries(exri_bench PRIVATE OpenEXR::OpenEXR)
else()
  if(NOT DEFINED EXRI_OPENEXR_INSTALL_DIR)
    message(FATAL_ERROR "EXRI_OPENEXR_INSTALL_DIR is required when EXRI_USE_OPENEXR_PACKAGE=OFF")
  endif()
  if(NOT DEFINED EXRI_OPENEXR_BUILD_DIR)
    message(FATAL_ERROR "EXRI_OPENEXR_BUILD_DIR is required when EXRI_USE_OPENEXR_PACKAGE=OFF")
  endif()

  find_package(Threads REQUIRED)
  find_library(EXRI_OPENEXR_LIBRARY   NAMES OpenEXR-4_0   OpenEXR   PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_OPENEXR_CORE_LIB  NAMES OpenEXRCore-4_0 OpenEXRCore PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_ILMTHREAD_LIB     NAMES IlmThread-4_0 IlmThread   PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_IEX_LIB           NAMES Iex-4_0       Iex         PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_IMATH_LIB         NAMES Imath-3_2     Imath       PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_OPENJPH_LIB       NAMES openjph       PATHS "${EXRI_OPENEXR_BUILD_DIR}/external/OpenJPH/src/core" REQUIRED NO_DEFAULT_PATH)

  target_include_directories(exri_bench PRIVATE "${EXRI_OPENEXR_INSTALL_DIR}/include")
  target_link_libraries(exri_bench PRIVATE
    "${EXRI_OPENEXR_LIBRARY}"
    "${EXRI_OPENEXR_CORE_LIB}"
    "${EXRI_OPENJPH_LIB}"
    "${EXRI_ILMTHREAD_LIB}"
    "${EXRI_IEX_LIB}"
    "${EXRI_IMATH_LIB}"
    Threads::Threads
    m)
endif()

if(UNIX)
  target_link_libraries(exri_bench PRIVATE m)
endif()
