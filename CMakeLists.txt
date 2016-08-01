cmake_minimum_required(VERSION 3.0.2)
project(embedlib)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g")

set(BUNDLE_DIR "${PROJECT_SOURCE_DIR}/bundle")
set(BUNDLE_INCLUDE_DIR "${BUNDLE_DIR}/include")
set(BUNDLE_LIB_DIR "${BUNDLE_DIR}/lib")

find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file("${PROJECT_SOURCE_DIR}/Doxyfile.in"
		"${PROJECT_BINARY_DIR}/Doxyfile")
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} "${PROJECT_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endif()

option(HAS_UNIT_TEST "Enable/disable unit tests." OFF)
if(HAS_UNIT_TEST)
	enable_testing()
endif()

# Setup dependency.
add_subdirectory(depends)
include_directories("${BUNDLE_INCLUDE_DIR}")

add_subdirectory(array_utils)
add_subdirectory(rbtree)
