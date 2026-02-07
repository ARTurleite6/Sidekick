#pragma once

#include <glad/glad.h>

#include <string_view>

namespace Graphics
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
    void Reset();
    void Bind() const;
    GLuint GetId() const;

  private:
    GLuint m_ProgramID{0};
};

} // namespace Graphics
