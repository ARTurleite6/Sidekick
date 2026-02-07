#pragma once

#include <glad/glad.h>

#include <string_view>

namespace Graphics
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
    void Reset();
    GLuint GetId() const;

  private:
    GLuint m_ShaderID{0};
};

} // namespace Graphics
