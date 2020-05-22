cmake_minimum_required (VERSION 3.11)
project (menger)

configure_file(shaders/fragment-shader.glsl shaders/fragment-shader.glsl COPYONLY)
configure_file(shaders/fragment-shader-flat.glsl shaders/fragment-shader-flat.glsl COPYONLY)
configure_file(shaders/vertex-shader.glsl shaders/vertex-shader.glsl COPYONLY)
configure_file(shaders/vertex-shader-flat.glsl shaders/vertex-shader-flat.glsl COPYONLY)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(menger src/math.cpp src/math.h src/program.cpp)

target_link_libraries(menger
    PRIVATE OpenGL::GL
    PRIVATE GLEW::GLEW
    PRIVATE glfw)
