#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Sidekick
{
class Camera
{
public:
  Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov = 45.0f,
         float aspect = 4.0f / 3.0f, float near = 0.1f, float far = 100.0f)
      : m_Position(position), m_Target(target), m_Up(up), m_Fov(fov),
        m_Aspect(aspect), m_Near(near), m_Far(far)
  {
    UpdateView();
    UpdateProjection();
  }

  void SetPosition(const glm::vec3& pos)
  {
    m_Position = pos;
    UpdateView();
  }
  void SetTarget(const glm::vec3& target)
  {
    m_Target = target;
    UpdateView();
  }
  void SetUp(const glm::vec3& up)
  {
    m_Up = up;
    UpdateView();
  }
  void SetAspect(float aspect)
  {
    m_Aspect = aspect;
    UpdateProjection();
  }
  void SetFov(float fov)
  {
    m_Fov = fov;
    UpdateProjection();
  }

  const glm::mat4& GetView() const { return m_View; }
  const glm::mat4& GetProjection() const { return m_Projection; }
  const glm::vec3& GetPosition() const { return m_Position; }

private:
  void UpdateView() { m_View = glm::lookAt(m_Position, m_Target, m_Up); }
  void UpdateProjection()
  {
    m_Projection =
        glm::perspective(glm::radians(m_Fov), m_Aspect, m_Near, m_Far);
  }

  glm::vec3 m_Position;
  glm::vec3 m_Target;
  glm::vec3 m_Up;
  float m_Fov, m_Aspect, m_Near, m_Far;
  glm::mat4 m_View, m_Projection;
};
} // namespace Sidekick
