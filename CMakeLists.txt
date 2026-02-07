cmake_minimum_required(VERSION 3.31)

project(SidekickCpp)

include(FetchContent)

add_subdirectory(external)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)

add_executable(Sidekick src/main.cpp
    src/Sidekick/Graphics/Shader.hpp
    src/Sidekick/Graphics/Shader.cpp
    src/Sidekick/Graphics/Program.hpp
    src/Sidekick/Graphics/Program.cpp

    src/Sidekick/Core/Window.hpp
    src/Sidekick/Core/Window.cpp

    src/Sidekick/Core/Application.hpp
    src/Sidekick/Core/Application.cpp

    src/Sidekick/Core/Event.hpp

    src/Sidekick/Core/KeyCode.hpp

    src/Sidekick/Core/Input.hpp
    src/Sidekick/Core/Input.cpp
)
target_link_libraries(Sidekick PRIVATE glfw glad::glad glm::glm)

target_include_directories(Sidekick PRIVATE src)
target_compile_features(Sidekick PRIVATE cxx_std_23)
