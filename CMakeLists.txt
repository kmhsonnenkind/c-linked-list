# MIT License
#
# Copyright (c) 2021 Martin Kloesch
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

################################################################################
# Project configuration and settings
################################################################################
cmake_minimum_required(VERSION 3.12)

# Main project configuration
project(linkedlist
  VERSION 0.0.1
  DESCRIPTION "C library for generic thread-safe linked lists"
  HOMEPAGE_URL "https://github.com/kmhsonnenkind/c-linked-list"
  LANGUAGES C CXX)

# Build options
option(BUILD_DOCUMENTATION "Build API documentation using doxygen" OFF)
include(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(CODE_COVERAGE "Measure code coverage information" OFF "BUILD_TESTING" OFF)

# Compiler settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# Internal variables
if(BUILD_TESTING AND CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(_CODE_COVERAGE 1)
else()
  set(_CODE_COVERAGE 0)
endif()
if(WIN32 OR MSVC)
  set(_MUTEX_IMPLEMENTATION src/win32-mutex.c)
elseif(UNIX)
  set(_MUTEX_IMPLEMENTATION src/pthread-mutex.c)
else()
  message(FATAL_ERROR "No mutex implementation available for platform")
endif()


################################################################################
# Libraries
################################################################################
# Main linked list library
add_library(linkedlist include/linkedlist.h include/linkedlist-mutex.h src/linkedlist.c ${_MUTEX_IMPLEMENTATION})
add_library(kmhsonnenkind::linkedlist ALIAS linkedlist)
target_include_directories(linkedlist
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# Special version of linked list library for measuring code coverage
if(_CODE_COVERAGE)
  add_library(linkedlist-coverage include/linkedlist.h src/linkedlist.c ${_MUTEX_IMPLEMENTATION})
  target_include_directories(linkedlist-coverage
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>)
  target_compile_options(linkedlist-coverage PUBLIC -O0 -g --coverage -fPIC)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(linkedlist-coverage PUBLIC --coverage)
  else()
    target_link_libraries(linkedlist-coverage PUBLIC --coverage)
  endif()
endif()


################################################################################
# Examples
################################################################################
add_executable(linkedlist-example-int examples/linkedlist-example-int.c)
target_link_libraries(linkedlist-example-int PRIVATE kmhsonnenkind::linkedlist)

add_executable(linkedlist-example-custom examples/linkedlist-example-custom.c)
target_link_libraries(linkedlist-example-custom PRIVATE kmhsonnenkind::linkedlist)


################################################################################
# Tests
################################################################################
include(CTest)
if(BUILD_TESTING)
  # Catch2 tests for native C library
  add_executable(linkedlist-tests tests/linkedlist-tests-main.cpp tests/linkedlist-tests.cpp tests/linkedlist-mutex-tests.cpp)
  target_include_directories(linkedlist-tests PRIVATE tests)
  if(_CODE_COVERAGE)
    target_link_libraries(linkedlist-tests PRIVATE linkedlist-coverage)
  else()
    target_link_libraries(linkedlist-tests PRIVATE linkedlist)
  endif()
  add_test(NAME linkedlist-tests COMMAND linkedlist-tests)

  # Custom ctest overrides
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/CTestCustom.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake)
endif()


################################################################################
# Documentation
################################################################################
if(BUILD_DOCUMENTATION)
  # Doxygen documentation of main library
  find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot)
  if(DOXYGEN_DOT_EXECUTABLE)
    set(DOXYGEN_HAVE_DOT YES)
  else()
    set(DOXYGEN_HAVE_DOT NO)
  endif()
  set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/docs)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/docs/html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/README.md
      ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      ${CMAKE_CURRENT_SOURCE_DIR}/data/Doxyfile.in
      linkedlist
    COMMENT "Building doxygen documentation for 'linkedlist' library")
  add_custom_target(documentation ALL
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/docs/html/index.html)
endif()


################################################################################
# Installation
################################################################################
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Config file to be findable by cmake
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/data/linkedlist-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/linkedlist-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/linkedlist)

# Version file for semantic version checks
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/linkedlist-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

# Native linkedlist library
install(
  TARGETS linkedlist
  EXPORT linkedlist-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(
  FILES include/linkedlist.h include/linkedlist-mutex.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# CMake files for find_package()
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/linkedlist-config-version.cmake
    ${PROJECT_BINARY_DIR}/linkedlist-config.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/linkedlist)

# CMake targets for stuff like target_link_libraries()
install(
  EXPORT linkedlist-targets
  FILE linkedlist-targets.cmake
  NAMESPACE kmhsonnenkind::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/linkedlist)
export(
  EXPORT linkedlist-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/linkedlist-targets.cmake
  NAMESPACE kmhsonnenkind::)
export(PACKAGE linkedlist)
