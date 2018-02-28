cmake_minimum_required(VERSION 3.8)
project(GameEngine)

set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES main.c shader.c shader.h utils.c utils.h draw.c draw.h types.h camera.c camera.h texture.c texture.h geometry.c geometry.h framebuffer.c framebuffer.h simplex_noise.c simplex_noise.h gui.c gui.h)

include(FindPkgConfig)

find_package(OpenGL)
find_package(SDL2)
find_package(GLEW)
find_package(KAZMATH)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

string(REPLACE "  -mwindows" "" SDL2_LIBRARIES ${SDL2_LIBRARIES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} ${GLEW_LIBRARIES} ${KAZMATH_LIBRARIES} ${SDL2IMAGE_LIBRARIES})

get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()