#include "Sidekick/Graphics/Shader.hpp"

#include <iostream>
#include <stdexcept>

namespace Graphics
{

Shader::Shader(GLenum type, std::string_view source)
{
    m_ShaderID = glCreateShader(type);
    const char* src = source.data();
    glShaderSource(m_ShaderID, 1, &src, nullptr);
    glCompileShader(m_ShaderID);

    GLint success;
    glGetShaderiv(m_ShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetShaderInfoLog(m_ShaderID, 512, nullptr, info_log);
        std::cerr << "Shader compilation failed: " << info_log << std::endl;
        throw std::runtime_error("Shader compilation failed");
    }
}

Shader::Shader(Shader&& other) noexcept : m_ShaderID{std::exchange(other.m_ShaderID, 0)}
{
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    std::swap(m_ShaderID, other.m_ShaderID);
    return *this;
}

Shader::~Shader() noexcept
{
    Reset();
}

void Shader::Reset()
{
    glDeleteShader(m_ShaderID);
    m_ShaderID = 0;
}

GLuint Shader::GetId() const
{
    return m_ShaderID;
}

} // namespace Graphics
