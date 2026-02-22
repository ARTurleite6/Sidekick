#include "Sidekick/Renderer/CameraController.h"

#include "Sidekick/Core/Input.h"
#include "Sidekick/Renderer/Camera.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace Sidekick::Renderer {

namespace {

glm::vec3 ForwardFromYawPitch(float yaw, float pitch) {
  const glm::vec3 forward{
      cosf(pitch) * cosf(yaw),
      sinf(pitch),
      cosf(pitch) * sinf(yaw),
  };
  return glm::normalize(forward);
}

}  // namespace

CameraController::CameraController(GLFWwindow* window) : m_window(window) {}

bool CameraController::Update(Camera& camera, float delta_time) {
  if (m_window == nullptr) {
    return false;
  }

  bool updated = false;

  const bool right_mouse = Sidekick::Core::Input::IsMouseButtonPressed(Sidekick::Core::MouseButton::Right);
  if (right_mouse) {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (m_first_mouse) {
      m_first_mouse = false;
    } else {
      const double delta_x = Sidekick::Core::Input::GetMouseDeltaX();
      const double delta_y = Sidekick::Core::Input::GetMouseDeltaY();

      float yaw = camera.GetYaw();
      float pitch = camera.GetPitch();

      yaw += static_cast<float>(delta_x) * m_look_sensitivity;
      pitch -= static_cast<float>(delta_y) * m_look_sensitivity;
      const float pitch_limit = glm::radians(89.0f);
      pitch = std::clamp(pitch, -pitch_limit, pitch_limit);

      if (yaw != camera.GetYaw() || pitch != camera.GetPitch()) {
        camera.SetRotation(yaw, pitch);
        updated = true;
      }
    }
  } else {
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_first_mouse = true;
  }

  const float yaw = camera.GetYaw();
  const float pitch = camera.GetPitch();

  glm::vec3 move_direction(0.0f);
  const glm::vec3 forward = ForwardFromYawPitch(yaw, pitch);
  const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
  const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::W)) {
    move_direction += forward;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::S)) {
    move_direction -= forward;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::D)) {
    move_direction += right;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::A)) {
    move_direction -= right;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::E)) {
    move_direction += up;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::Q)) {
    move_direction -= up;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::Space)) {
    move_direction += up;
  }
  if (Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::LeftShift) ||
      Sidekick::Core::Input::IsKeyPressed(Sidekick::Core::KeyCode::RightShift)) {
    move_direction -= up;
  }

  if (glm::length(move_direction) > 0.0f) {
    const glm::vec3 position = camera.GetPosition() +
                               glm::normalize(move_direction) * (m_move_speed * delta_time);
    camera.SetPosition(position);
    updated = true;
  }

  return updated;
}

void CameraController::SetMoveSpeed(float move_speed) {
  m_move_speed = move_speed;
}

void CameraController::SetLookSensitivity(float look_sensitivity) {
  m_look_sensitivity = look_sensitivity;
}

}  // namespace Sidekick::Renderer
