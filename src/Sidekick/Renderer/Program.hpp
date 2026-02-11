#pragma once

#include <glad/glad.h>

#include <string_view>

namespace Sidekick
{
class Program
{
public:
  Program(std::string_view vertex_source, std::string_view fragment_source);
  Program(const Program&) = delete;
  Program(Program&& other) noexcept;
  Program& operator=(const Program&) = delete;
  Program& operator=(Program&& other) noexcept;
  ~Program() noexcept;

  inline void Bind() const noexcept { glUseProgram(m_Id); }
  inline GLuint GetId() const noexcept { return m_Id; }

private:
  GLuint m_Id;
};
} // namespace Sidekick
