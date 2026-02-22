cmake_minimum_required(VERSION 3.30)

project(Sidekick LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW (for app windowing)
include(FetchContent)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Disable GLFW examples" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Disable GLFW tests" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Disable GLFW docs" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Disable GLFW install targets" FORCE)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

# GLM (math)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Dawn (third_party/dawn) - keep build minimal and fetch deps.
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "Use Dawn fetch_dawn_dependencies.py" FORCE)
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "Disable Dawn install targets" FORCE)
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "Disable Dawn samples" FORCE)
set(DAWN_BUILD_TESTS OFF CACHE BOOL "Disable Dawn tests" FORCE)
set(DAWN_BUILD_BENCHMARKS OFF CACHE BOOL "Disable Dawn benchmarks" FORCE)
set(DAWN_BUILD_FUZZERS OFF CACHE BOOL "Disable Dawn fuzzers" FORCE)
set(DAWN_BUILD_NODE_BINDINGS OFF CACHE BOOL "Disable Dawn NodeJS bindings" FORCE)
set(DAWN_ENABLE_SWIFTSHADER OFF CACHE BOOL "Disable SwiftShader" FORCE)
set(DAWN_USE_GLFW ON CACHE BOOL "Enable GLFW utilities" FORCE)
add_subdirectory(third_party/dawn)

add_executable(Sidekick
  src/main.cpp
  src/Sidekick/Core/Application.cpp
  src/Sidekick/Core/Application.h
  src/Sidekick/Core/Assert.h
  src/Sidekick/Core/Event.h
  src/Sidekick/Core/Input.cpp
  src/Sidekick/Core/Input.h
  src/Sidekick/Core/KeyCodes.h
  src/Sidekick/Core/Log.cpp
  src/Sidekick/Core/Log.h
  src/Sidekick/Core/MouseCodes.h
  src/Sidekick/Core/Window.cpp
  src/Sidekick/Core/Window.h
  src/Sidekick/Renderer/Camera.cpp
  src/Sidekick/Renderer/Camera.h
  src/Sidekick/Renderer/CameraController.cpp
  src/Sidekick/Renderer/CameraController.h
  src/Sidekick/Renderer/Backend/GraphicsBackend.cpp
  src/Sidekick/Renderer/Backend/GraphicsBackend.h
  src/Sidekick/Renderer/Backend/WgpuBackend.cpp
  src/Sidekick/Renderer/Backend/WgpuBackend.h
)
target_include_directories(Sidekick PRIVATE src)
target_link_libraries(Sidekick PRIVATE glfw glm::glm dawn::dawn_glfw dawn::dawn_native dawn::dawn_proc)
