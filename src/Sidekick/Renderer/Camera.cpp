#include "Sidekick/Renderer/Camera.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace Sidekick::Renderer {

Camera::Camera(float fov_degrees, float aspect, float near_plane, float far_plane)
    : m_fov(fov_degrees), m_aspect(aspect), m_near(near_plane), m_far(far_plane) {
  UpdateMatrices();
}

void Camera::SetPosition(const glm::vec3& position) {
  m_position = position;
  UpdateMatrices();
}

void Camera::SetRotation(float yaw_radians, float pitch_radians) {
  m_yaw = yaw_radians;
  m_pitch = pitch_radians;
  UpdateMatrices();
}

void Camera::SetPerspective(float fov_degrees, float aspect, float near_plane, float far_plane) {
  m_fov = fov_degrees;
  m_aspect = aspect;
  m_near = near_plane;
  m_far = far_plane;
  UpdateMatrices();
}

void Camera::SetAspect(float aspect) {
  m_aspect = aspect;
  UpdateMatrices();
}

const glm::vec3& Camera::GetPosition() const {
  return m_position;
}

float Camera::GetYaw() const {
  return m_yaw;
}

float Camera::GetPitch() const {
  return m_pitch;
}

const glm::mat4& Camera::GetView() const {
  return m_view;
}

const glm::mat4& Camera::GetProjection() const {
  return m_proj;
}

const glm::mat4& Camera::GetViewProjection() const {
  return m_view_proj;
}

void Camera::UpdateMatrices() {
  const glm::vec3 forward{
      cosf(m_pitch) * cosf(m_yaw),
      sinf(m_pitch),
      cosf(m_pitch) * sinf(m_yaw),
  };
  const glm::vec3 target = m_position + glm::normalize(forward);
  m_view = glm::lookAt(m_position, target, glm::vec3(0.0f, 1.0f, 0.0f));

  m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);

  m_view_proj = m_proj * m_view;
}

}  // namespace Sidekick::Renderer
