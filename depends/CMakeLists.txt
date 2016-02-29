include(ExternalProject)

set(DEPENDS_BUILD_DIR "${BUNDLE_DIR}/build")

# Prevent external project from cleaning
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

ExternalProject_Add(cmocka
	PREFIX "${DEPENDS_BUILD_DIR}"
	URL "https://cmocka.org/files/1.0/cmocka-1.0.1.tar.xz"
	CMAKE_ARGS -DCMAKE_BUILD_TYPE=Debug -DWITH_STATIC_LIB=ON
		"-DCMAKE_INSTALL_PREFIX=${BUNDLE_DIR}"
	INSTALL_DIR "${BUNDLE_DIR}"
	EXCLUDE_FROM_ALL 1)

add_library(libcmocka STATIC IMPORTED GLOBAL)
set_target_properties(libcmocka
	PROPERTIES IMPORTED_LOCATION "${BUNDLE_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}cmocka${CMAKE_STATIC_LIBRARY_SUFFIX}")
add_dependencies(libcmocka cmocka)
