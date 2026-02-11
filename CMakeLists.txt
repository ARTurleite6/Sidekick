cmake_minimum_required(VERSION 3.20)

project(Sidekick LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)

SET(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

add_subdirectory(external)

add_executable(Sidekick src/main.cpp

    src/Sidekick/Core/Application.hpp
    src/Sidekick/Core/Application.cpp

    src/Sidekick/Renderer/Shader.hpp
    src/Sidekick/Renderer/Shader.cpp

    src/Sidekick/Renderer/Program.hpp
    src/Sidekick/Renderer/Program.cpp
)

target_include_directories(Sidekick PRIVATE src)
target_link_libraries(Sidekick PRIVATE glfw glad::glad glm::glm)
target_precompile_headers(Sidekick PRIVATE src/sk_pch.hpp)
target_compile_features(Sidekick PRIVATE cxx_std_23)
