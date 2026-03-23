include(FetchContent)

# Zlib via FetchContent
FetchContent_Declare(
	zlib
	GIT_REPOSITORY https://github.com/madler/zlib.git
	GIT_TAG v1.3.2
)

# Keep Zlib lean (no examples/tests) and default to static.
if (NOT DEFINED ZLIB_BUILD_EXAMPLES)
	set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Build Zlib examples")
endif()

if (NOT DEFINED ZLIB_BUILD_SHARED)
	set(ZLIB_BUILD_SHARED OFF CACHE BOOL "Build Zlib as shared library")
endif()

if (NOT DEFINED ZLIB_BUILD_TESTING)
	set(ZLIB_BUILD_TESTING OFF CACHE BOOL "Build Zlib tests")
endif()

if (NOT DEFINED ZLIB_INSTALL)
	set(ZLIB_INSTALL OFF CACHE BOOL "Install Zlib")
endif()

FetchContent_MakeAvailable(zlib)
