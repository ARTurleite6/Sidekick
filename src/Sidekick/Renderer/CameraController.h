#pragma once

struct GLFWwindow;

namespace Sidekick::Renderer {

class Camera;

class CameraController {
 public:
  explicit CameraController(GLFWwindow* window);

  bool Update(Camera& camera, float delta_time);

  void SetMoveSpeed(float move_speed);
  void SetLookSensitivity(float look_sensitivity);

 private:
  GLFWwindow* m_window = nullptr;

  float m_move_speed = 3.5f;
  float m_look_sensitivity = 0.0025f;

  bool m_first_mouse = true;
};

}  // namespace Sidekick::Renderer
