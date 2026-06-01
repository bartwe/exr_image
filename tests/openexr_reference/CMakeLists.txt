cmake_minimum_required(VERSION 3.16)
project(exri_openexr_reference CXX)

if(NOT DEFINED EXRI_ROOT_DIR)
  get_filename_component(EXRI_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()

add_executable(exri_openexr_reference openexr_reference.cpp)
set_target_properties(exri_openexr_reference PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO)

if(NOT DEFINED EXRI_USE_OPENEXR_PACKAGE)
  set(EXRI_USE_OPENEXR_PACKAGE ON)
endif()

if(EXRI_USE_OPENEXR_PACKAGE)
  find_package(OpenEXR REQUIRED CONFIG)
  target_link_libraries(exri_openexr_reference PRIVATE OpenEXR::OpenEXR)
else()
  if(NOT DEFINED EXRI_OPENEXR_INSTALL_DIR)
    message(FATAL_ERROR "EXRI_OPENEXR_INSTALL_DIR is required when EXRI_USE_OPENEXR_PACKAGE=OFF")
  endif()
  if(NOT DEFINED EXRI_OPENEXR_BUILD_DIR)
    message(FATAL_ERROR "EXRI_OPENEXR_BUILD_DIR is required when EXRI_USE_OPENEXR_PACKAGE=OFF")
  endif()

  find_package(Threads REQUIRED)
  find_library(EXRI_OPENEXR_LIBRARY NAMES OpenEXR-4_0 OpenEXR PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_OPENEXR_CORE_LIBRARY NAMES OpenEXRCore-4_0 OpenEXRCore PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_ILMTHREAD_LIBRARY NAMES IlmThread-4_0 IlmThread PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_IEX_LIBRARY NAMES Iex-4_0 Iex PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_IMATH_LIBRARY NAMES Imath-3_2 Imath PATHS "${EXRI_OPENEXR_INSTALL_DIR}/lib" REQUIRED NO_DEFAULT_PATH)
  find_library(EXRI_OPENJPH_LIBRARY NAMES openjph PATHS "${EXRI_OPENEXR_BUILD_DIR}/external/OpenJPH/src/core" REQUIRED NO_DEFAULT_PATH)

  target_include_directories(exri_openexr_reference PRIVATE "${EXRI_OPENEXR_INSTALL_DIR}/include")
  target_link_libraries(exri_openexr_reference PRIVATE
    "${EXRI_OPENEXR_LIBRARY}"
    "${EXRI_OPENEXR_CORE_LIBRARY}"
    "${EXRI_OPENJPH_LIBRARY}"
    "${EXRI_ILMTHREAD_LIBRARY}"
    "${EXRI_IEX_LIBRARY}"
    "${EXRI_IMATH_LIBRARY}"
    Threads::Threads
    m)
endif()
