# CMakeList.txt: projeto de CMake para SampleRender, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.
set(TARGET_NAME Render)

set(LIB_TYPE STATIC)
if(BUILD_SHARED_LIBS)
	set(LIB_TYPE SHARED)
endif()

file(GLOB_RECURSE RENDER_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE RENDER_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

add_library(${TARGET_NAME} ${LIB_TYPE} ${RENDER_HDRS} ${RENDER_SRCS})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_cxx_project_standards(${TARGET_NAME} 20 FALSE)

if(WIN32)
	trace_file(NAME "d3d12core" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/d3d12core.ps1" LOCATION "${PROJECT_SOURCE_DIR}" EXTENSION "zip")
	add_custom_target(copy_d3d12 ALL
			COMMAND ${CMAKE_COMMAND} -E copy
			"${PROJECT_SOURCE_DIR}/D3D12Core/build/native/bin/x64/D3D12Core.dll"
			${CMAKE_PREFIX_PATH}/bin/D3D12
			COMMENT "Copying D3D12Core dll into binary directory")
	add_dependencies(${TARGET_NAME} copy_d3d12)
endif()
if(BUILD_SHARED_LIBS)
	if(WIN32)
		target_compile_definitions(${TARGET_NAME} PRIVATE SAMPLE_RENDER_DLL_MACRO_EXPORT)
		target_compile_definitions(${TARGET_NAME} PUBLIC WIN32_LEAN_AND_MEAN)
		target_compile_definitions(${TARGET_NAME} INTERFACE SAMPLE_RENDER_DLL_MACRO_EXPORT)
	endif()
endif()


