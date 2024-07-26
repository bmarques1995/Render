set(TARGET_NAME ShaderManager)

set(LIB_TYPE STATIC)
if(BUILD_SHARED_LIBS)
	set(LIB_TYPE SHARED)
endif()

file(GLOB_RECURSE SHADER_MNG_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE SHADER_MNG_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

trace_installable_file(NAME "dxc_manifest" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/dxc.ps1" LOCATION "${CMAKE_PREFIX_PATH}/lib/dxc" EXTENSION "txt")
trace_installable_file(NAME "jsoncpp" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/jsoncpp.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "dll")
trace_installable_file(NAME "jsoncpp" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/jsoncpp.ps1" LOCATION "${CMAKE_PREFIX_PATH}/lib" EXTENSION "lib")

find_package(jsoncpp)

add_library(${TARGET_NAME} ${LIB_TYPE} ${SHADER_MNG_HDRS} ${SHADER_MNG_SRCS})
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> $<BUILD_INTERFACE:${CMAKE_PREFIX_PATH}/include/dxc>)
target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_PREFIX_PATH}/lib/dxcompiler.lib)
target_link_libraries(${TARGET_NAME} PUBLIC Utils jsoncpp_lib)
set_cxx_project_standards(${TARGET_NAME} 20 FALSE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${TARGET_NAME} PUBLIC SHADER_MNG_DEBUG_MODE)
else()
	target_compile_definitions(${TARGET_NAME} PUBLIC SHADER_MNG_RELEASE_MODE)
endif()

if(BUILD_SHARED_LIBS)
	if(WIN32)
		target_compile_definitions(${TARGET_NAME} PRIVATE SAMPLE_SHADER_MNG_DLL_MACRO_EXPORT)
		target_compile_definitions(${TARGET_NAME} INTERFACE SAMPLE_SHADER_MNG_DLL_MACRO_IMPORT)
		target_compile_definitions(${TARGET_NAME} PUBLIC SHADER_MNG_USES_WINDOWS)
	endif()
endif()