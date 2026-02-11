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
       std::vector<std::uint32_t> indices);
  Mesh(const Mesh&) = delete;
  Mesh(Mesh&& other) noexcept;
  Mesh& operator=(const Mesh&) = delete;
  Mesh& operator=(Mesh&& other) noexcept;
  ~Mesh() noexcept;

  void Draw() const noexcept;

private:
  std::vector<glm::vec3> m_Vertices;
  std::vector<glm::vec4> m_Colors;
  std::vector<std::uint32_t> m_Indices;
  GLuint m_VAO{0};
  GLuint m_PositionsVBO{0}, m_ColorsVBO{0};
  GLuint m_IndicesEBO{0};
};
} // namespace Sidekick
