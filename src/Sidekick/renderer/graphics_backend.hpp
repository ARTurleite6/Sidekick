#pragma once

#include <cstdint>
#include <memory>

struct GLFWwindow;

namespace sidekick
{
struct extent_2d
{
  uint32_t width{0};
  uint32_t height{0};
};

struct graphics_context_descriptor
{
  GLFWwindow* native_window{nullptr};
  extent_2d framebuffer_extent{};
};

struct clear_color
{
  double r{0.0};
  double g{0.0};
  double b{0.0};
  double a{1.0};
};

enum class load_op : std::uint8_t
{
  load,
  clear,
};

enum class store_op : std::uint8_t
{
  discard,
  store,
};

struct color_attachment_descriptor
{
  load_op load_operation{load_op::clear};
  store_op store_operation{store_op::store};
  clear_color clear_value{};
};

struct render_pass_descriptor
{
  color_attachment_descriptor color_attachment{};
};

class graphics_backend
{
public:
  static std::unique_ptr<graphics_backend> create();

  virtual ~graphics_backend() = default;

  graphics_backend(const graphics_backend&) = delete;
  graphics_backend& operator=(const graphics_backend&) = delete;
  graphics_backend(graphics_backend&&) noexcept = default;
  graphics_backend& operator=(graphics_backend&&) noexcept = default;

  bool init(const graphics_context_descriptor& descriptor);

  [[nodiscard]] bool begin_frame();
  void begin_render_pass(const render_pass_descriptor& descriptor);
  void end_render_pass();
  void end_frame();
  [[nodiscard]] bool resize(std::uint32_t width, std::uint32_t height);

protected:
  graphics_backend() = default;

  virtual bool on_init(const graphics_context_descriptor& descriptor) = 0;
  virtual bool on_begin_frame() = 0;
  virtual void on_begin_render_pass(const render_pass_descriptor& descriptor) = 0;
  virtual void on_end_render_pass() = 0;
  virtual void on_end_frame() = 0;
  virtual bool on_resize(std::uint32_t width, std::uint32_t height) = 0;

private:
  bool m_initialized{false};
  bool m_frame_active{false};
  bool m_render_pass_active{false};
};
} // namespace sidekick
