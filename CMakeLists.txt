﻿# CMakeList.txt: arquivo de projeto de CMake de nível superior, fazer configuração global
# e inclua subprojetos aqui.
#
cmake_minimum_required (VERSION 3.22)

project ("SampleRender")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(REGISTER_VULKAN_LAYERS "Set vulkan layers location on regedit" OFF)

# Habilite a Recarga Dinâmica para compiladores MSVC, se houver suporte.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_helpers)
include(target_destination)
include(dep_tracer)

if(WIN32)
	add_compile_options(/utf-8)
endif()

# Inclua subprojetos.
add_subdirectory ("Utils")
add_subdirectory ("ShaderManager")
add_subdirectory ("Render")
add_subdirectory ("TargetView")
