cmake_minimum_required(VERSION 3.30)

project(sidekick VERSION 0.1.0 LANGUAGES CXX)

option(SIDEKICK_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(SIDEKICK_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(SIDEKICK_ENABLE_CLANG_TIDY "Enable clang-tidy during compilation" ON)

set(SIDEKICK_FETCHCONTENT_BASE_DIR ".cache/fetchcontent" CACHE PATH "Persistent base dir for FetchContent sources")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(IS_ABSOLUTE "${SIDEKICK_FETCHCONTENT_BASE_DIR}")
  set(_sidekick_fetchcontent_base_dir "${SIDEKICK_FETCHCONTENT_BASE_DIR}")
else()
  set(_sidekick_fetchcontent_base_dir "${CMAKE_SOURCE_DIR}/${SIDEKICK_FETCHCONTENT_BASE_DIR}")
endif()

file(MAKE_DIRECTORY "${_sidekick_fetchcontent_base_dir}")
set(FETCHCONTENT_BASE_DIR "${_sidekick_fetchcontent_base_dir}" CACHE PATH "FetchContent base dir" FORCE)

include(FetchContent)

set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "" FORCE)
set(DAWN_USE_GLFW ON CACHE BOOL "" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TINT_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
)

FetchContent_Declare(
    dawn
    GIT_REPOSITORY https://github.com/google/dawn.git
    GIT_TAG v20260306.200308
    GIT_SHALLOW OFF
    GIT_SUBMODULES ""
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW ON
)

FetchContent_MakeAvailable(glfw dawn glm)

if(SIDEKICK_DAWN_TARGET STREQUAL "")
  message(FATAL_ERROR "Unable to find Dawn link target (expected webgpu_dawn or dawn::webgpu_dawn)")
endif()

set(SIDEKICK_SOURCES
    src/main.cpp

    src/Sidekick/Core/Application.hpp
    src/Sidekick/Core/Application.cpp
    src/Sidekick/Core/Event.hpp
    src/Sidekick/Core/Window.hpp
    src/Sidekick/Core/Window.cpp
    src/Sidekick/Renderer/GraphicsContext.hpp
    src/Sidekick/Renderer/GraphicsContext.cpp
)

add_executable(sidekick ${SIDEKICK_SOURCES})

target_compile_features(sidekick PRIVATE cxx_std_23)

target_link_libraries(sidekick PRIVATE
    glfw
    glm::glm
    dawn::webgpu_dawn
    webgpu_glfw
)
target_include_directories(sidekick PRIVATE src)

include(cmake/Warnings.cmake)
sidekick_set_project_warnings(sidekick ${SIDEKICK_WARNINGS_AS_ERRORS})

include(cmake/ClangTools.cmake)
sidekick_configure_clang_tools(sidekick "${SIDEKICK_SOURCES}")
