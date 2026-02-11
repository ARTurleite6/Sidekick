#include "Sidekick/Renderer/Program.hpp"

#include "Sidekick/Renderer/Shader.hpp"

namespace Sidekick
{
Program::Program(std::string_view vertex_source,
                 std::string_view fragment_source)
{
  Shader vertex_shader(GL_VERTEX_SHADER, vertex_source);
  Shader fragment_shader(GL_FRAGMENT_SHADER, fragment_source);

  m_Id = glCreateProgram();
  glAttachShader(m_Id, vertex_shader.GetId());
  glAttachShader(m_Id, fragment_shader.GetId());

  glLinkProgram(m_Id);

  glDetachShader(m_Id, vertex_shader.GetId());
  glDetachShader(m_Id, fragment_shader.GetId());
}
Program::~Program() noexcept
{
  if (m_Id != 0)
  {
    glDeleteProgram(m_Id);
  }
}
Program& Program::operator=(Program&& other) noexcept
{
  std::swap(m_Id, other.m_Id);
  return *this;
}
Program::Program(Program&& other) noexcept : m_Id{std::exchange(other.m_Id, 0)}
{
}
} // namespace Sidekick
