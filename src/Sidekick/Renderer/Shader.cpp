#include "Sidekick/Renderer/Shader.hpp"

#include <stdexcept>

namespace Sidekick
{
Shader::Shader(GLenum type, std::string_view source)
{
  m_Id = glCreateShader(type);
  const auto* source_data = source.data();
  glShaderSource(m_Id, 1, &source_data, nullptr);
  glCompileShader(m_Id);
  GLint success;
  glGetShaderiv(m_Id, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    GLchar infoLog[512];
    glGetShaderInfoLog(m_Id, 512, nullptr, infoLog);
    throw std::runtime_error("Vertex Shader compilation failed");
  }
}
Shader::Shader(Shader&& other) noexcept : m_Id{std::exchange(other.m_Id, 0)} {}
Shader& Shader::operator=(Shader&& other) noexcept
{
  std::swap(m_Id, other.m_Id);
  return *this;
}
Shader::~Shader() noexcept
{
  if (m_Id != 0)
  {
    glDeleteShader(m_Id);
  }
}
} // namespace Sidekick
