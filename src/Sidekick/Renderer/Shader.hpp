#pragma once

#include <glad/glad.h>

#include <string_view>

namespace Sidekick
{
class Shader
{
public:
  Shader(GLenum type, std::string_view source);
  Shader(const Shader&) = delete;
  Shader(Shader&& other) noexcept;
  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&& other) noexcept;
  ~Shader() noexcept;

  inline GLuint GetId() const noexcept { return m_Id; }

private:
  GLuint m_Id;
};
} // namespace Sidekick
