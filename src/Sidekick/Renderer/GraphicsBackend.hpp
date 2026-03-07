#pragma once

#include <cstdint>
#include <memory>

struct GLFWwindow;

namespace Sidekick
{
struct Extent2D
{
  uint32_t Width{0};
  uint32_t Height{0};
};

struct GraphicsContextDescriptor
{
  GLFWwindow* NativeWindow{nullptr};
  Extent2D FramebufferExtent{};
};

struct ClearColor
{
  double R{0.0};
  double G{0.0};
  double B{0.0};
  double A{1.0};
};

enum class LoadOp : std::uint8_t
{
  Load,
  Clear,
};

enum class StoreOp : std::uint8_t
{
  Discard,
  Store,
};

struct ColorAttachmentDescriptor
{
  LoadOp LoadOperation{LoadOp::Clear};
  StoreOp StoreOperation{StoreOp::Store};
  ClearColor ClearValue{};
};

struct RenderPassDescriptor
{
  ColorAttachmentDescriptor ColorAttachment{};
};

class GraphicsBackend
{
public:
  static std::unique_ptr<GraphicsBackend> Create();

  virtual ~GraphicsBackend() = default;

  GraphicsBackend(const GraphicsBackend&) = delete;
  GraphicsBackend& operator=(const GraphicsBackend&) = delete;
  GraphicsBackend(GraphicsBackend&&) noexcept = default;
  GraphicsBackend& operator=(GraphicsBackend&&) noexcept = default;

  bool Init(const GraphicsContextDescriptor& descriptor);

  void BeginFrame();
  void BeginRenderPass(const RenderPassDescriptor& descriptor);
  void EndRenderPass();
  void EndFrame();
  void Resize(std::uint32_t width, std::uint32_t height);

protected:
  GraphicsBackend() = default;

  virtual bool OnInit(const GraphicsContextDescriptor& descriptor) = 0;

  virtual void OnBeginFrame() = 0;
  virtual void OnBeginRenderPass(const RenderPassDescriptor& descriptor) = 0;
  virtual void OnEndRenderPass() = 0;
  virtual void OnEndFrame() = 0;
  virtual void OnResize(std::uint32_t width, std::uint32_t height) = 0;

private:
  bool m_Initialized{false};
  bool m_FrameActive{false};
  bool m_RenderPassActive{false};
};
} // namespace Sidekick
