cmake_minimum_required(VERSION 3.10)
project(powerup_sim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(sim
    src/main.cpp
    external/glad/src/glad.c
)

target_include_directories(sim PRIVATE
    include
    external/glad/include
)

target_link_libraries(sim PRIVATE glfw OpenGL::GL)

include(CTest)
if(BUILD_TESTING)
    add_executable(scene_math_test tests/scene_math_test.cpp)
    target_include_directories(scene_math_test PRIVATE src)
    add_test(NAME scene_math COMMAND scene_math_test)
endif()
