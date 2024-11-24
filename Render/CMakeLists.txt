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

if(WIN32)

trace_file(NAME "d3d12core" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/d3d12core.ps1" LOCATION "${PROJECT_SOURCE_DIR}" EXTENSION "zip")

trace_dependency(NAME VulkanHeaders INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_headers.ps1")
trace_dependency(NAME VulkanLoader INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_loader.ps1")
trace_dependency(NAME Spirv-Headers INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/spirv_headers.ps1")
trace_dependency(NAME SPIRV-Tools INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/spirv_tools.ps1")
trace_dependency(NAME Eigen3 INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/eigen.ps1")
trace_dependency(NAME SDL3 INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/sdl3.ps1")

trace_installable_file(NAME "VkLayer_khronos_shader_object" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_extensions.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "dll")
trace_installable_file(NAME "VkLayer_khronos_shader_object" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_extensions.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "json")
trace_installable_file(NAME "VkLayer_khronos_synchronization2" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_extensions.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "dll")
trace_installable_file(NAME "VkLayer_khronos_synchronization2" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_extensions.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "json")
trace_installable_file(NAME "VkLayer_khronos_validation" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_validation.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "dll")
trace_installable_file(NAME "VkLayer_khronos_validation" INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vulkan_validation.ps1" LOCATION "${CMAKE_PREFIX_PATH}/bin" EXTENSION "json")

trace_dependency(NAME VulkanMemoryAllocator INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/vma.ps1")
trace_dependency(NAME D3D12MemoryAllocator INSTALL_SCRIPT "${PROJECT_SOURCE_DIR}/installers/d3d12ma.ps1")

if(REGISTER_VULKAN_LAYERS)

	file(GLOB LAYER_FILES "${CMAKE_INSTALL_PREFIX}/bin/VkLayer_khronos_*.json")

	list(JOIN LAYER_FILES "|" LAYER_FILES_STR)

	execute_process(COMMAND powershell -File "${PROJECT_SOURCE_DIR}/installers/vulkan_regedit.ps1" ${LAYER_FILES_STR})
	endif()

endif()

find_package(JsonCpp)

add_library(${TARGET_NAME} ${LIB_TYPE} ${RENDER_HDRS} ${RENDER_SRCS})
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Core>)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Window>)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics>)
if(WIN32)
target_include_directories(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Graphics/D3D12>)
target_compile_definitions(${TARGET_NAME} PRIVATE VK_PROTOTYPES VK_USE_PLATFORM_WIN32_KHR)
endif()
target_include_directories(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Graphics/VK>)
target_include_directories(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Window/SDL3>)
target_link_libraries(${TARGET_NAME} PUBLIC ShaderManager jsoncpp_lib Eigen3::Eigen PRIVATE ${CMAKE_PREFIX_PATH}/lib/dxcompiler.lib)
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
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_target(copy_d3d12_sdk ALL
				COMMAND ${CMAKE_COMMAND} -E copy
				"${PROJECT_SOURCE_DIR}/D3D12Core/build/native/bin/x64/d3d12SDKLayers.dll"
				${CMAKE_PREFIX_PATH}/bin/D3D12/d3d12SDKLayers.dll
				COMMENT "Copying d3d12SDKLayers dll into binary directory")
		add_dependencies(${TARGET_NAME} copy_d3d12_sdk)
	endif()
	add_dependencies(${TARGET_NAME} copy_d3d12)
	
	target_compile_definitions(${TARGET_NAME} PUBLIC WIN32_LEAN_AND_MEAN RENDER_USES_WINDOWS NOMINMAX VK_USE_PLATFORM_WIN32_KHR)

	find_library(D3D12_FOUND NAMES d3d12 REQUIRED)
	find_library(DXGI_FOUND NAMES dxgi REQUIRED)
	find_library(DXGUID_FOUND NAMES dxguid REQUIRED)

	target_link_libraries(${TARGET_NAME} PUBLIC d3d12 dxgi dxguid GPUOpen::D3D12MemoryAllocator)
endif()
target_link_libraries(${TARGET_NAME} PUBLIC Vulkan::Headers Vulkan::Loader Eigen3::Eigen SDL3::SDL3-shared GPUOpen::VulkanMemoryAllocator)
if(BUILD_SHARED_LIBS)
	if(WIN32)
		target_compile_definitions(${TARGET_NAME} PRIVATE SAMPLE_RENDER_DLL_MACRO_EXPORT)
		target_compile_definitions(${TARGET_NAME} INTERFACE SAMPLE_RENDER_DLL_MACRO_IMPORT)
	endif()
endif()
