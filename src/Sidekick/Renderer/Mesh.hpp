#pragma once

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstdint>
#include <vector>

namespace Sidekick
{
class Mesh
{
public:
  Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec4> colors,
       std::vector<std::uint32_t> indices)
      : m_Vertices{std::move(vertices)}, m_Colors{std::move(colors)},
        m_Indices{std::move(indices)}
  {
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_PositionsVBO);
    glGenBuffers(1, &m_ColorsVBO);

    glBindBuffer(GL_ARRAY_BUFFER, m_PositionsVBO);
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(glm::vec3),
                 m_Vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_ColorsVBO);
    glBufferData(GL_ARRAY_BUFFER, m_Colors.size() * sizeof(glm::vec4),
                 m_Colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    if (!m_Indices.empty())
    {
      glGenBuffers(1, &m_IndicesEBO);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndicesEBO);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   m_Indices.size() * sizeof(std::uint32_t), m_Indices.data(),
                   GL_STATIC_DRAW);
    }
  }

  Mesh(const Mesh&) = delete;
  Mesh(Mesh&& other) noexcept
      : m_Vertices(std::move(other.m_Vertices)),
        m_Colors(std::move(other.m_Colors)),
        m_Indices(std::move(other.m_Indices)),
        m_VAO(std::exchange(other.m_VAO, 0)),
        m_PositionsVBO(std::exchange(other.m_PositionsVBO, 0)),
        m_ColorsVBO(std::exchange(other.m_ColorsVBO, 0)),
        m_IndicesEBO(std::exchange(other.m_IndicesEBO, 0))
  {
  }
  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&& other) noexcept
  {
    std::swap(m_Vertices, other.m_Vertices);
    std::swap(m_Colors, other.m_Colors);
    std::swap(m_Indices, other.m_Indices);
    std::swap(m_VAO, other.m_VAO);
    std::swap(m_PositionsVBO, other.m_PositionsVBO);
    std::swap(m_ColorsVBO, other.m_ColorsVBO);
    std::swap(m_IndicesEBO, other.m_IndicesEBO);
    return *this;
  }

  ~Mesh() noexcept
  {
    if (m_IndicesEBO != 0)
      glDeleteBuffers(1, &m_IndicesEBO);
    if (m_ColorsVBO != 0)
      glDeleteBuffers(1, &m_ColorsVBO);
    if (m_PositionsVBO != 0)
      glDeleteBuffers(1, &m_PositionsVBO);
    if (m_VAO != 0)
      glDeleteVertexArrays(1, &m_VAO);
  }

  void Render() const noexcept
  {
    glBindVertexArray(m_VAO);
    if (m_IndicesEBO != 0)
    {
      glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
    }
    else
    {
      glDrawArrays(GL_TRIANGLES, 0, m_Vertices.size());
    }
    glBindVertexArray(0);
  }

private:
  std::vector<glm::vec3> m_Vertices;
  std::vector<glm::vec4> m_Colors;
  std::vector<std::uint32_t> m_Indices;
  GLuint m_VAO{0};
  GLuint m_PositionsVBO{0}, m_ColorsVBO{0};
  GLuint m_IndicesEBO{0};
};
} // namespace Sidekick
