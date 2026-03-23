include(FetchContent)

FetchContent_Declare(
	g3log
	GIT_REPOSITORY https://github.com/KjellKod/g3log.git
	GIT_TAG 2.6
)

if (NOT DEFINED ADD_G3LOG_UNIT_TEST)
	set(ADD_G3LOG_UNIT_TEST OFF CACHE BOOL "g3log unit tests")
endif()

if (NOT DEFINED INSTALL_G3LOG)
	set(INSTALL_G3LOG OFF CACHE BOOL "Install g3log targets")
endif()

if (NOT DEFINED ADD_FATAL_EXAMPLE)
	set(ADD_FATAL_EXAMPLE OFF CACHE BOOL "Build g3log example targets")
endif()

if (NOT DEFINED USE_DYNAMIC_LOGGING_LEVELS)
	set(USE_DYNAMIC_LOGGING_LEVELS ON CACHE BOOL "Enable g3log log filtering")
endif()

FetchContent_MakeAvailable(g3log)