cmake_minimum_required(VERSION 3.30)

project(Sidekick)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY git@github.com:glfw/glfw.git
  GIT_TAG 3.4
)

FetchContent_Declare(
  glad
  GIT_REPOSITORY git@github.com:Dav1dde/glad.git
  GIT_TAG v2.0.8
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY git@github.com:g-truc/glm.git
  GIT_TAG 1.0.3
)

FetchContent_MakeAvailable(glfw glad glm)

add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)

glad_add_library(glad_gl_core_46 REPRODUCIBLE API gl:core=4.6)

add_executable(Sidekick 
  src/main.cpp
  src/Sidekick/Core/Platform.hpp
  src/Sidekick/Core/Platform.cpp

  src/Sidekick/Core/GraphicsContext.hpp
  src/Sidekick/Core/GraphicsContext.cpp

  src/Sidekick/Core/CommandList.hpp

  src/Sidekick/Core/Window.hpp

  src/Sidekick/Platform/GlfwPlatform.hpp
  src/Sidekick/Platform/GlfwPlatform.cpp

  src/Sidekick/Platform/GlfwWindow.hpp
  src/Sidekick/Platform/GlfwWindow.cpp

  src/Sidekick/Platform/OpenGLGraphicsContext.hpp
  src/Sidekick/Platform/OpenGLGraphicsContext.cpp

  src/Sidekick/Platform/OpenGLSwapchain.hpp
  src/Sidekick/Platform/OpenGLSwapchain.cpp

  src/Sidekick/Platform/OpenGLCommandList.hpp
  src/Sidekick/Platform/OpenGLCommandList.cpp
)

target_include_directories(Sidekick PRIVATE src)
target_link_libraries(Sidekick PRIVATE glfw glad_gl_core_46 glm::glm)
