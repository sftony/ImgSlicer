cmake_minimum_required(VERSION 3.31)

project(ImgSlicer VERSION 1.0.0 LANGUAGES CXX)
configure_file("${PROJECT_SOURCE_DIR}/cmake/ImgSlicer_config.h.in"
                "${PROJECT_BINARY_DIR}/ImgSlicer_config.h")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)
endif()

add_subdirectory(app)
add_subdirectory(module)
