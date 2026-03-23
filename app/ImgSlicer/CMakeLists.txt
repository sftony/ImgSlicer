set(SELECTED_QT_VERSION "${SELECTED_QT_VERSION}" CACHE STRING "Set to 5 or 6 to specify Qt version")
set_property(CACHE SELECTED_QT_VERSION PROPERTY STRINGS 5 6)

if("${SELECTED_QT_VERSION}" STREQUAL "")
	message(FATAL_ERROR "Please set SELECTED_QT_VERSION to 5 or 6")
endif()

if ("${SELECTED_QT_VERSION}" STREQUAL "6")
	find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools Core5Compat)
elseif("${SELECTED_QT_VERSION}" STREQUAL "5")
	find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
else()
	message(FATAL_ERROR "Invalid SELECTED_QT_VERSION: must be 5 or 6")
endif()

if(Qt5_FOUND OR Qt6_FOUND)

	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	file(GLOB SRCS "*.cpp")
	file(GLOB HEADERS "*.h")
	set(UI_FORMS ImgSlicer.ui)
	add_executable(ImgSlicer WIN32 ${SRCS} ${HEADERS} ${UI_FORMS})
	set_target_properties(ImgSlicer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:ImgSlicer>)
	target_include_directories(ImgSlicer PUBLIC ${CMAKE_SOURCE_DIR}/module ${PROJECT_BINARY_DIR})

	file(GLOB TS_FILES "${CMAKE_SOURCE_DIR}/translations/*.ts")
	set(LIBS Logger)
	if(Qt5_FOUND)
		qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
		list(PREPEND LIBS Qt::Widgets)
	else()
		qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
		list(PREPEND LIBS Qt::Widgets)
	endif()
	target_sources(ImgSlicer PRIVATE ${QM_FILES})
	target_link_libraries(ImgSlicer PRIVATE ${LIBS})
	
	add_custom_command(TARGET ImgSlicer
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:ImgSlicer>"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${QM_FILES}" "$<TARGET_FILE_DIR:ImgSlicer>"
		VERBATIM)

	if(Qt5_FOUND)
		set(QT_BIN_DIR "${_qt5Core_install_prefix}/bin")
	else()
		get_target_property(_qt6Core_location Qt6::Core LOCATION)
		get_filename_component(_qt6Core_install_prefix "${_qt6Core_location}" DIRECTORY)
		set(QT_BIN_DIR "${_qt6Core_install_prefix}")
	endif()

	if (WIN32)
		if (MSVC)
			string(REGEX MATCH ".*Community/VC/" VCPATH "${CMAKE_CXX_COMPILER}")
			add_custom_command(TARGET ImgSlicer
				POST_BUILD
				COMMAND set VCINSTALLDIR=${VCPATH} &&
				"${QT_BIN_DIR}/windeployqt" ARGS "$<IF:$<CONFIG:debug>,--debug,--release>" --force --compiler-runtime --no-opengl-sw "$<TARGET_FILE:ImgSlicer>"
				WORKING_DIRECTORY "${QT_BIN_DIR}")
		else()
			add_custom_command(TARGET ImgSlicer
				POST_BUILD
				"${QT_BIN_DIR}/windeployqt" ARGS "$<IF:$<CONFIG:debug>,--debug,--release>" --force --compiler-runtime --no-opengl-sw "$<TARGET_FILE:ImgSlicer>"
				WORKING_DIRECTORY "${QT_BIN_DIR}")
		endif()
	endif()

	include(GNUInstallDirs)
	install(TARGETS ImgSlicer
		RUNTIME DESTINATION ImgSlicer/${CMAKE_INSTALL_BINDIR})
	install(FILES ${QM_FILES} DESTINATION ImgSlicer/${CMAKE_INSTALL_BINDIR})
	if(WIN32)
		if(MSVC)
			install(CODE "
				set(ENV{VCINSTALLDIR} \"${VCPATH}\")
				execute_process(
					COMMAND \"${QT_BIN_DIR}/windeployqt\" \"$<IF:$<CONFIG:Debug>,--debug,--release>\"
					--force --compiler-runtime --no-opengl-sw \"${CMAKE_INSTALL_PREFIX}/ImgSlicer/${CMAKE_INSTALL_BINDIR}/ImgSlicer.exe\"
					WORKING_DIRECTORY \"${QT_BIN_DIR}\"
				)
			")
		else()
			install(CODE "
				execute_process(
					COMMAND \"${QT_BIN_DIR}/windeployqt\" \"$<IF:$<CONFIG:Debug>,--debug,--release>\"
					--force --compiler-runtime --no-opengl-sw \"${CMAKE_INSTALL_PREFIX}/ImgSlicer/${CMAKE_INSTALL_BINDIR}/ImgSlicer.exe\"
					WORKING_DIRECTORY \"${QT_BIN_DIR}\"
				)
			")
		endif()
	endif()
endif()
