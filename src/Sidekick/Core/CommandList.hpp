#pragma once

#include "glm/ext/vector_float4.hpp"

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace Sidekick
{
using TextureHandle = uint32_t;

enum class LoadOp
{
  Load,
  Clear,
  DontCare
};

enum class StoreOp
{
  Store,
  DontCare
};

struct ColorAttachment
{
  TextureHandle texture;
  LoadOp load_op{LoadOp::Clear};
  StoreOp store_op{StoreOp::Store};
  glm::vec4 clear_color{0.f, 0.f, 0.f, 1.f};
};

struct DepthAttachment
{
  TextureHandle texture;
  LoadOp depth_load_op{LoadOp::Clear};
  StoreOp depth_store_op{StoreOp::Store};
  LoadOp stencil_load_op{LoadOp::DontCare};
  StoreOp stencil_store_op{StoreOp::DontCare};
  float clear_depth{1.f};
  uint8_t clear_stencil{0};
};

struct Viewport
{
  float x{0.f};
  float y{0.f};
  float width{0.f};
  float height{0.f};
  float min_depth{0.f};
  float max_depth{1.f};
};

struct Rect
{
  int32_t x{0};
  int32_t y{0};
  int32_t width{0};
  int32_t height{0};
};

struct RenderPassDescriptor
{
  std::string_view name;
  std::span<ColorAttachment> color_attachments;
  std::optional<DepthAttachment> depth_attachment;
  Viewport viewport;
  Rect scissor;
};

enum class CommandListType
{
  Graphics,
  Compute,
  Transfer
};

struct CommandListDescriptor
{
  std::string_view debug_name;
  CommandListType type{CommandListType::Graphics};
};

class CommandList
{
public:
  virtual ~CommandList() noexcept = default;

  virtual void BeginRenderPass(RenderPassDescriptor&& desc) = 0;
  virtual void EndRenderPass() = 0;
};
} // namespace Sidekick
