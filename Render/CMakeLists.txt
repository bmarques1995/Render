# CMakeList.txt: projeto de CMake para SampleRender, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.
set(TARGET_NAME Render)

set(LIB_TYPE STATIC)
if(BUILD_SHARED_LIBS)
	set(LIB_TYPE SHARED)
endif()

if(WIN32)
	trace_file(NAME "d3d12core" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/d3d12core.ps1" LOCATION "${PROJECT_SOURCE_DIR}" EXTENSION "zip")
endif()

file(GLOB_RECURSE RENDER_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE RENDER_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

trace_dependency(NAME spdlog INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/spdlog.ps1")
add_library(${TARGET_NAME} ${LIB_TYPE} ${RENDER_HDRS} ${RENDER_SRCS})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)
set_cxx_project_standards(${TARGET_NAME} 20 FALSE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(${TARGET_NAME} PUBLIC RENDER_DEBUG_MODE)
else()
	target_compile_definitions(${TARGET_NAME} PUBLIC RENDER_RELEASE_MODE)
endif()

if(WIN32)
	add_custom_target(copy_d3d12 ALL
			COMMAND ${CMAKE_COMMAND} -E copy
			"${PROJECT_SOURCE_DIR}/D3D12Core/build/native/bin/x64/D3D12Core.dll"
			${CMAKE_PREFIX_PATH}/bin/D3D12/D3D12Core.dll
			COMMENT "Copying D3D12Core dll into binary directory")
	add_custom_target(copy_d3d12_sdk ALL
			COMMAND ${CMAKE_COMMAND} -E copy
			"${PROJECT_SOURCE_DIR}/D3D12Core/build/native/bin/x64/d3d12SDKLayers.dll"
			${CMAKE_PREFIX_PATH}/bin/D3D12/d3d12SDKLayers.dll
			COMMENT "Copying D3D12Core dll into binary directory")
	add_dependencies(${TARGET_NAME} copy_d3d12)
	add_dependencies(${TARGET_NAME} copy_d3d12_sdk)
	target_compile_definitions(${TARGET_NAME} PUBLIC WIN32_LEAN_AND_MEAN RENDER_USES_WINDOWS NOMINMAX)

	find_library(D3D12_FOUND NAMES d3d12 REQUIRED)
	find_library(DXGI_FOUND NAMES dxgi REQUIRED)
	find_library(DXGUID_FOUND NAMES dxguid REQUIRED)

	target_link_libraries(${TARGET_NAME} PUBLIC d3d12 dxgi dxguid)
endif()
if(BUILD_SHARED_LIBS)
	if(WIN32)
		target_compile_definitions(${TARGET_NAME} PRIVATE SAMPLE_RENDER_DLL_MACRO_EXPORT)
		target_compile_definitions(${TARGET_NAME} INTERFACE SAMPLE_RENDER_DLL_MACRO_IMPORT)
	endif()
endif()
