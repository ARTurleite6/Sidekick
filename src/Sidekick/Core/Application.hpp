#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstdint>
#include <print>
#include <vector>

#include "Sidekick/Renderer/Mesh.hpp"
#include "Sidekick/Renderer/Program.hpp"

constexpr const char* STATIC_VERT = R"(#version 330 core

layout(location = 0) in vec2 aPos;

void main()
{
    // Hardcoded triangle vertices
    vec2 positions[3] = vec2[](
            vec2(-0.5, -0.5),
            vec2(0.5, -0.5),
            vec2(0.0, 0.5)
        );
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}
)";

constexpr const char* STATIC_FRAG = R"(#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

constexpr const char* SIMPLE_VERT = R"(#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;

out vec4 vColor;

uniform mat4 uMVP;

void main()
{
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

constexpr const char* SIMPLE_FRAG = R"(#version 330 core

in vec4 vColor;
out vec4 FragColor;

void main()
{
    FragColor = vColor;
}
)";

namespace Sidekick
{
inline void GlfwErrorCallback(int code, const char* message)
{
  std::println("[GLFW ERROR]: {}", message);
}

constexpr std::string_view GlDebugSourcetoString(GLuint source)
{
  switch (source)
  {
  case GL_DEBUG_SOURCE_API:
    return "API";
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    return "WINDOW SYSTEM";
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    return "SHADER COMPILER";
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    return "THIRD PARTY";
  case GL_DEBUG_SOURCE_APPLICATION:
    return "APPLICATION";
  case GL_DEBUG_SOURCE_OTHER:
    return "UNKNOWN";
  default:
    return "UNKNOWN";
  }
}

constexpr std::string_view GlDebugTypeToString(GLuint type)
{
  switch (type)
  {
  case GL_DEBUG_TYPE_ERROR:
    return "ERROR";
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    return "DEPRECATED BEHAVIOR";
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    return "UNDEFINED BEHAVIOR";
  case GL_DEBUG_TYPE_PORTABILITY:
    return "PORTABILITY";
  case GL_DEBUG_TYPE_PERFORMANCE:
    return "PERFORMANCE";
  case GL_DEBUG_TYPE_OTHER:
    return "OTHER";
  case GL_DEBUG_TYPE_MARKER:
    return "MARKER";
  default:
    return "UNKNOWN";
  }
}

constexpr std::string_view GlDebugSeverityToString(GLuint severity)
{
  switch (severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:
    return "HIGH";
  case GL_DEBUG_SEVERITY_MEDIUM:
    return "MEDIUM";
  case GL_DEBUG_SEVERITY_LOW:
    return "LOW";
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    return "NOTIFICATION";
  default:
    return "UNKNOWN";
  }
}

inline void GlDebugMessageCallback(GLuint source, GLuint type, GLuint id,
                                   GLuint severity, GLint length,
                                   const char* message, const void* user_param)
{
  // Only log medium/high severity messages
  if (severity != GL_DEBUG_SEVERITY_MEDIUM &&
      severity != GL_DEBUG_SEVERITY_HIGH)
  {
    return;
  }

  auto source_str = GlDebugSourcetoString(source);
  auto type_str = GlDebugTypeToString(type);
  auto severity_str = GlDebugSeverityToString(severity);

  std::println("[OpenGL] [{} - {} ({}): [{}] {}", severity_str, type_str, id,
               source_str, message);
}

class Application
{
public:
  Application()
  {
    glfwInit();

    glfwSetErrorCallback(GlfwErrorCallback);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    m_Window = glfwCreateWindow(800, 600, "Sidekick", nullptr, nullptr);

    glfwMakeContextCurrent(m_Window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glDebugMessageCallback(GlDebugMessageCallback, nullptr);

    glEnable(GL_DEPTH_TEST);
  }

  ~Application()
  {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
  }

  void Run()
  {
    Program program{SIMPLE_VERT, SIMPLE_FRAG};

    // Define cube vertex positions
    std::vector<glm::vec3> positions{
        {-0.7f, -0.7f, -0.7f}, // 0
        {0.7f, -0.7f, -0.7f},  // 1
        {0.7f, 0.7f, -0.7f},   // 2
        {-0.7f, 0.7f, -0.7f},  // 3
        {-0.7f, -0.7f, 0.7f},  // 4
        {0.7f, -0.7f, 0.7f},   // 5
        {0.7f, 0.7f, 0.7f},    // 6
        {-0.7f, 0.7f, 0.7f}    // 7
    };

    // Define cube vertex colors
    std::vector<glm::vec4> colors{
        {1.f, 0.f, 0.f, 1.f}, // Red
        {0.f, 1.f, 0.f, 1.f}, // Green
        {0.f, 0.f, 1.f, 1.f}, // Blue
        {1.f, 1.f, 0.f, 1.f}, // Yellow
        {1.f, 0.f, 1.f, 1.f}, // Magenta
        {0.f, 1.f, 1.f, 1.f}, // Cyan
        {1.f, 1.f, 1.f, 1.f}, // White
        {0.f, 0.f, 0.f, 1.f}  // Black
    };

    // Define cube indices
    std::vector<std::uint32_t> indices{// Front face
                                       4, 5, 6, 4, 6, 7,
                                       // Back face
                                       0, 1, 2, 0, 2, 3,
                                       // Left face
                                       0, 3, 7, 0, 7, 4,
                                       // Right face
                                       1, 5, 6, 1, 6, 2,
                                       // Bottom face
                                       0, 1, 5, 0, 5, 4,
                                       // Top face
                                       3, 2, 6, 3, 6, 7};

    Mesh mesh{positions, colors, indices};

    while (m_Running)
    {
      glfwPollEvents();

      if (glfwWindowShouldClose(m_Window))
      {
        m_Running = false;
        break;
      }

      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      program.Bind();

      glfwSwapBuffers(m_Window);
    }
  }

private:
  GLFWwindow* m_Window;
  bool m_Running{true};
};
} // namespace Sidekick
