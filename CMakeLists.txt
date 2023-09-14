cmake_minimum_required(VERSION 3.25)
project(SR1CargaDeModelos)

set(CMAKE_PREFIX_PATH "C:/Libs/SDL2" ${CMAKE_PREFIX_PATH})
set(SDL_MAIN_HANDLED ON)
find_package(SDL2 REQUIRED)
set(EXECUTABLE_NAME "SR1CargaDeModelos")

set(CMAKE_CXX_STANDARD 23)

add_executable(SR1CargaDeModelos main.cpp
        fragment.h
        uniform.h
        color.h
        line.h
        triangle.h
        vertexArray.h
        loadObj.h
        point.h
        framebuffer.h
        shaders.h
        fastNoise.h
)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-Wl,-subsystem,console")
target_include_directories(SR1CargaDeModelos PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(SR1CargaDeModelos PRIVATE ${SDL2_LIBRARIES})
