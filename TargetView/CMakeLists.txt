# CMakeList.txt: projeto de CMake para SampleRender, incluir origem e definir
# lógica específica a um projeto aqui.
#

# Adicione a origem ao executável deste projeto.
set(TARGET_NAME TargetView)

file(GLOB_RECURSE TARGET_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.hpp")
file(GLOB_RECURSE TARGET_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

add_executable(${TARGET_NAME} ${TARGET_HDRS} ${TARGET_SRCS})
target_link_libraries(${TARGET_NAME} PRIVATE Render)
set_cxx_project_standards(${TARGET_NAME} 20 FALSE)

# TODO: Adicione testes e instale destinos, se necessário.
