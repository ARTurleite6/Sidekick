#include "Sidekick/Graphics/Program.hpp"

#include <iostream>
#include <stdexcept>

#include "Sidekick/Graphics/Shader.hpp"

namespace Graphics
{
Program::Program(std::string_view vertex_source, std::string_view fragment_source)
{
    Shader vertex_shader{GL_VERTEX_SHADER, vertex_source};
    Shader fragment_shader{GL_FRAGMENT_SHADER, fragment_source};

    m_ProgramID = glCreateProgram();
    glAttachShader(m_ProgramID, vertex_shader.GetId());
    glAttachShader(m_ProgramID, fragment_shader.GetId());
    glLinkProgram(m_ProgramID);

    GLint success;
    glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info_log[512];
        glGetProgramInfoLog(m_ProgramID, 512, nullptr, info_log);
        std::cerr << "Program linking failed: " << info_log << std::endl;
        throw std::runtime_error("Program linking failed");
    }
}

Program::Program(Program&& other) noexcept : m_ProgramID{other.m_ProgramID}
{
    other.m_ProgramID = 0;
}

Program& Program::operator=(Program&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    Reset();

    m_ProgramID = other.m_ProgramID;
    other.m_ProgramID = 0;

    return *this;
}

Program::~Program() noexcept
{
    Reset();
}

void Program::Reset()
{
    glDeleteProgram(m_ProgramID);
    m_ProgramID = 0;
}

void Program::Bind() const
{
    glUseProgram(m_ProgramID);
}

GLuint Program::GetId() const
{
    return m_ProgramID;
}

} // namespace Graphics
