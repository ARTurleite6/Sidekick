#pragma once

#include <cstdint>
#include <memory>

namespace Sidekick::Renderer::Backend {

enum class GraphicsBackendType : std::uint8_t {
  Wgpu = 0,
};

enum class PixelFormat : std::uint8_t {
  Undefined = 0,
  BGRA8UnormSrgb,
  RGBA8UnormSrgb,
  Depth24Plus,
};

enum class ShaderStage : std::uint8_t {
  Vertex = 0,
  Fragment,
};

enum class VertexFormat : std::uint8_t {
  Float32x3 = 0,
};

enum class PrimitiveTopology : std::uint8_t {
  TriangleList = 0,
};

enum class CullMode : std::uint8_t {
  None = 0,
  Back,
};

enum class FrontFace : std::uint8_t {
  CCW = 0,
  CW,
};

enum class CompareFunction : std::uint8_t {
  Less = 0,
};

enum class IndexType : std::uint8_t {
  Uint16 = 0,
  Uint32,
};

enum class BufferUsageFlags : std::uint32_t {
  None = 0,
  CopyDst = 1u << 0u,
  Vertex = 1u << 1u,
  Index = 1u << 2u,
  Uniform = 1u << 3u,
};

constexpr BufferUsageFlags operator|(BufferUsageFlags lhs, BufferUsageFlags rhs) {
  return static_cast<BufferUsageFlags>(static_cast<std::uint32_t>(lhs) | static_cast<std::uint32_t>(rhs));
}

constexpr bool HasFlag(BufferUsageFlags flags, BufferUsageFlags flag) {
  return (static_cast<std::uint32_t>(flags) & static_cast<std::uint32_t>(flag)) != 0u;
}

struct BufferHandle {
  std::uint32_t id = 0;
};

struct ShaderHandle {
  std::uint32_t id = 0;
};

struct BindGroupLayoutHandle {
  std::uint32_t id = 0;
};

struct BindGroupHandle {
  std::uint32_t id = 0;
};

struct PipelineHandle {
  std::uint32_t id = 0;
};

struct BackendBootstrapContext {
  GraphicsBackendType type = GraphicsBackendType::Wgpu;
  const void* native_device = nullptr;
  const void* native_queue = nullptr;
  PixelFormat color_format = PixelFormat::Undefined;
  PixelFormat depth_format = PixelFormat::Depth24Plus;
};

struct BufferDesc {
  std::uint64_t size = 0;
  BufferUsageFlags usage = BufferUsageFlags::None;
  const void* initial_data = nullptr;
  std::uint64_t initial_data_size = 0;
};

struct ShaderDesc {
  const char* source_wgsl = nullptr;
};

struct BindGroupLayoutEntryDesc {
  std::uint32_t binding = 0;
  ShaderStage visibility = ShaderStage::Vertex;
  std::uint64_t min_binding_size = 0;
};

struct BindGroupLayoutDesc {
  const BindGroupLayoutEntryDesc* entries = nullptr;
  std::uint32_t entry_count = 0;
};

struct BindGroupEntryDesc {
  std::uint32_t binding = 0;
  BufferHandle buffer = {};
  std::uint64_t offset = 0;
  std::uint64_t size = 0;
};

struct BindGroupDesc {
  BindGroupLayoutHandle layout = {};
  const BindGroupEntryDesc* entries = nullptr;
  std::uint32_t entry_count = 0;
};

struct VertexAttributeDesc {
  VertexFormat format = VertexFormat::Float32x3;
  std::uint64_t offset = 0;
  std::uint32_t shader_location = 0;
};

struct VertexBufferLayoutDesc {
  std::uint64_t array_stride = 0;
  const VertexAttributeDesc* attributes = nullptr;
  std::uint32_t attribute_count = 0;
};

struct DepthStencilDesc {
  PixelFormat format = PixelFormat::Depth24Plus;
  bool depth_write_enabled = true;
  CompareFunction depth_compare = CompareFunction::Less;
};

struct ColorTargetDesc {
  PixelFormat format = PixelFormat::Undefined;
};

struct PipelineDesc {
  ShaderHandle shader = {};
  const char* vertex_entry = nullptr;
  const char* fragment_entry = nullptr;
  const BindGroupLayoutHandle* bind_group_layouts = nullptr;
  std::uint32_t bind_group_layout_count = 0;
  const VertexBufferLayoutDesc* vertex_buffers = nullptr;
  std::uint32_t vertex_buffer_count = 0;
  const ColorTargetDesc* color_targets = nullptr;
  std::uint32_t color_target_count = 0;
  bool has_depth_stencil = false;
  DepthStencilDesc depth_stencil = {};
  PrimitiveTopology topology = PrimitiveTopology::TriangleList;
  CullMode cull_mode = CullMode::Back;
  FrontFace front_face = FrontFace::CCW;
};

struct RenderTargetRefs {
  const void* color_view = nullptr;
  const void* depth_view = nullptr;
};

struct ClearColor {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;
};

struct FrameBeginDesc {
  ClearColor clear_color = {};
  float clear_depth = 1.0f;
};

class GraphicsBackend {
public:
  virtual ~GraphicsBackend() = default;

  virtual bool Initialize(const BackendBootstrapContext& context) = 0;
  virtual void Shutdown() = 0;

  virtual BufferHandle CreateBuffer(const BufferDesc& desc) = 0;
  virtual ShaderHandle CreateShader(const ShaderDesc& desc) = 0;
  virtual BindGroupLayoutHandle CreateBindGroupLayout(const BindGroupLayoutDesc& desc) = 0;
  virtual BindGroupHandle CreateBindGroup(const BindGroupDesc& desc) = 0;
  virtual PipelineHandle CreatePipeline(const PipelineDesc& desc) = 0;

  virtual void UpdateBuffer(BufferHandle handle, std::uint64_t offset, const void* data, std::uint64_t size) = 0;

  virtual bool BeginFrame(const FrameBeginDesc& begin_desc, const RenderTargetRefs& targets) = 0;
  virtual void SetPipeline(PipelineHandle handle) = 0;
  virtual void SetBindGroup(std::uint32_t index, BindGroupHandle handle) = 0;
  virtual void SetVertexBuffer(std::uint32_t slot, BufferHandle handle, std::uint64_t offset) = 0;
  virtual void SetIndexBuffer(BufferHandle handle, IndexType index_type, std::uint64_t offset) = 0;
  virtual void DrawIndexed(std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index,
                           std::int32_t base_vertex, std::uint32_t first_instance) = 0;
  virtual void EndFrameAndSubmit() = 0;
};

std::unique_ptr<GraphicsBackend> CreateGraphicsBackend(GraphicsBackendType type);

} // namespace Sidekick::Renderer::Backend
