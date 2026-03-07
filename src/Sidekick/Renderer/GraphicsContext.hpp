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

class GraphicsBackend;

class GraphicsContext
{
public:
  GraphicsContext();
  ~GraphicsContext();

  GraphicsContext(const GraphicsContext&) = delete;
  GraphicsContext& operator=(const GraphicsContext&) = delete;

  GraphicsContext(GraphicsContext&& other) noexcept;
  GraphicsContext& operator=(GraphicsContext&& other) noexcept;

  bool Init(const GraphicsContextDescriptor& descriptor);

  void BeginFrame();
  void BeginRenderPass(const RenderPassDescriptor& descriptor);
  void EndRenderPass();
  void EndFrame();
  void Resize(uint32_t width, uint32_t height);

private:
  std::unique_ptr<GraphicsBackend> m_Backend;

  bool m_FrameActive{false};
  bool m_RenderPassActive{false};
};
} // namespace Sidekick
