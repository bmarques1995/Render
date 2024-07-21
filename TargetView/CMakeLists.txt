# CMakeList.txt: projeto de CMake para SampleRender, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.
set(TARGET_NAME TargetView)

if(WIN32)
	file(READ "${PROJECT_SOURCE_DIR}/d3d12sdk_version.txt" SDK_VERSION)
	string(REGEX REPLACE "[^0-9]" "" SANITIZED_SDK_VERSION "${SDK_VERSION}")
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/D3D12ContextExport.cxx.in SOURCE_CONTENTS)
	string(REGEX REPLACE SDK_VERSION "${SANITIZED_SDK_VERSION}" OUTPUT_SOURCE ${SOURCE_CONTENTS})
	file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/src/D3D12ContextExport.cpp ${OUTPUT_SOURCE})
endif()

file(GLOB_RECURSE TARGET_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE TARGET_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

add_executable(${TARGET_NAME} ${TARGET_HDRS} ${TARGET_SRCS})
target_link_libraries(${TARGET_NAME} PUBLIC Render)
if(WIN32)
	set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
set_cxx_project_standards(${TARGET_NAME} 20 FALSE)

# TODO: Adicione testes e instale destinos, se necessário.
