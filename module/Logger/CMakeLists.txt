include(${PROJECT_SOURCE_DIR}/cmake/ExAddG3log.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/ExAddZlib.cmake)

file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE IPPS "${CMAKE_CURRENT_SOURCE_DIR}/*.ipp")

add_library(Logger SHARED ${SRCS} ${HEADERS} ${IPPS})
set_target_properties(Logger PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Logger PUBLIC g3log PRIVATE ZLIB::ZLIBSTATIC)
set_target_properties(Logger PROPERTIES DEBUG_POSTFIX "d")

include(GNUInstallDirs)
install(TARGETS Logger RUNTIME DESTINATION ImgSlicer/${CMAKE_INSTALL_BINDIR})
install(FILES $<TARGET_RUNTIME_DLLS:Logger> DESTINATION ImgSlicer/${CMAKE_INSTALL_BINDIR})
