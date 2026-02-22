#pragma once

#include <glm/glm.hpp>

namespace Sidekick::Renderer {

class Camera {
 public:
  Camera(float fov_degrees, float aspect, float near_plane, float far_plane);

  void SetPosition(const glm::vec3& position);
  void SetRotation(float yaw_radians, float pitch_radians);
  void SetPerspective(float fov_degrees, float aspect, float near_plane, float far_plane);
  void SetAspect(float aspect);

  const glm::vec3& GetPosition() const;
  float GetYaw() const;
  float GetPitch() const;

  const glm::mat4& GetView() const;
  const glm::mat4& GetProjection() const;
  const glm::mat4& GetViewProjection() const;

 private:
  void UpdateMatrices();

  glm::vec3 m_position{0.0f, 0.0f, 3.0f};
  float m_yaw = 0.0f;
  float m_pitch = 0.0f;

  float m_fov = 45.0f;
  float m_aspect = 16.0f / 9.0f;
  float m_near = 0.1f;
  float m_far = 100.0f;

  glm::mat4 m_view{1.0f};
  glm::mat4 m_proj{1.0f};
  glm::mat4 m_view_proj{1.0f};
};

}  // namespace Sidekick::Renderer
