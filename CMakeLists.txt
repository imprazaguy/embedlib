cmake_minimum_required(VERSION 3.0.2)
project(embedlib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

set(BUNDLE_DIR "${PROJECT_SOURCE_DIR}/bundle")
set(BUNDLE_INCLUDE_DIR "${BUNDLE_DIR}/include")
set(BUNDLE_LIB_DIR "${BUNDLE_DIR}/lib")

# Setup dependency.
add_subdirectory(depends)
include_directories("${BUNDLE_INCLUDE_DIR}")

