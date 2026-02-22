#include "Sidekick/Renderer/Backend/WgpuBackend.h"

#include "Sidekick/Core/Log.h"

#include <array>
#include <limits>
#include <vector>

namespace Sidekick::Renderer::Backend {

template <typename TObject> std::uint32_t WgpuBackend::AddResource(std::vector<TObject>& resources, TObject resource) {
  resources.push_back(std::move(resource));
  return static_cast<std::uint32_t>(resources.size());
}

bool WgpuBackend::Initialize(const BackendBootstrapContext& context) {
  if (context.native_device == nullptr || context.native_queue == nullptr) {
    SK_ERROR("WgpuBackend initialization failed: missing device or queue.");
    return false;
  }

  m_device = *static_cast<const wgpu::Device*>(context.native_device);
  m_queue = *static_cast<const wgpu::Queue*>(context.native_queue);
  return static_cast<bool>(m_device) && static_cast<bool>(m_queue);
}

void WgpuBackend::Shutdown() {
  m_active_pass = nullptr;
  m_active_encoder = nullptr;
  m_in_frame = false;

  m_pipelines.clear();
  m_bind_groups.clear();
  m_bind_group_layouts.clear();
  m_shaders.clear();
  m_buffers.clear();

  m_queue = nullptr;
  m_device = nullptr;
}

BufferHandle WgpuBackend::CreateBuffer(const BufferDesc& desc) {
  if (!m_device || desc.size == 0) {
    return {};
  }

  wgpu::BufferDescriptor buffer_desc = {};
  buffer_desc.size = desc.size;
  buffer_desc.usage = ToWgpuBufferUsage(desc.usage);

  wgpu::Buffer buffer = m_device.CreateBuffer(&buffer_desc);
  if (!buffer) {
    return {};
  }

  if (desc.initial_data != nullptr && desc.initial_data_size > 0) {
    m_queue.WriteBuffer(buffer, 0, desc.initial_data, desc.initial_data_size);
  }

  return BufferHandle{.id = AddResource(m_buffers, std::move(buffer))};
}

ShaderHandle WgpuBackend::CreateShader(const ShaderDesc& desc) {
  if (!m_device || desc.source_wgsl == nullptr) {
    return {};
  }

  wgpu::ShaderSourceWGSL wgsl_desc = {};
  wgsl_desc.code = desc.source_wgsl;

  wgpu::ShaderModuleDescriptor shader_desc = {};
  shader_desc.nextInChain = &wgsl_desc;
  wgpu::ShaderModule shader = m_device.CreateShaderModule(&shader_desc);
  if (!shader) {
    return {};
  }

  return ShaderHandle{.id = AddResource(m_shaders, std::move(shader))};
}

BindGroupLayoutHandle WgpuBackend::CreateBindGroupLayout(const BindGroupLayoutDesc& desc) {
  if (!m_device || desc.entry_count == 0 || desc.entries == nullptr) {
    return {};
  }

  std::vector<wgpu::BindGroupLayoutEntry> layout_entries(desc.entry_count);
  for (std::uint32_t index = 0; index < desc.entry_count; ++index) {
    layout_entries[index].binding = desc.entries[index].binding;
    layout_entries[index].visibility = ToWgpuShaderStage(desc.entries[index].visibility);
    layout_entries[index].buffer.type = wgpu::BufferBindingType::Uniform;
    layout_entries[index].buffer.minBindingSize = desc.entries[index].min_binding_size;
  }

  wgpu::BindGroupLayoutDescriptor layout_desc = {};
  layout_desc.entryCount = desc.entry_count;
  layout_desc.entries = layout_entries.data();

  wgpu::BindGroupLayout layout = m_device.CreateBindGroupLayout(&layout_desc);
  if (!layout) {
    return {};
  }

  return BindGroupLayoutHandle{.id = AddResource(m_bind_group_layouts, std::move(layout))};
}

BindGroupHandle WgpuBackend::CreateBindGroup(const BindGroupDesc& desc) {
  if (!m_device || desc.entry_count == 0 || desc.entries == nullptr) {
    return {};
  }

  wgpu::BindGroupLayout* layout = GetBindGroupLayout(desc.layout);
  if (layout == nullptr) {
    SK_ERROR("CreateBindGroup failed: invalid bind group layout handle.");
    return {};
  }

  std::vector<wgpu::BindGroupEntry> entries(desc.entry_count);
  for (std::uint32_t index = 0; index < desc.entry_count; ++index) {
    wgpu::Buffer* buffer = GetBuffer(desc.entries[index].buffer);
    if (buffer == nullptr) {
      SK_ERROR("CreateBindGroup failed: invalid buffer handle in entry {}.", index);
      return {};
    }
    entries[index].binding = desc.entries[index].binding;
    entries[index].buffer = *buffer;
    entries[index].offset = desc.entries[index].offset;
    entries[index].size = desc.entries[index].size;
  }

  wgpu::BindGroupDescriptor bind_group_desc = {};
  bind_group_desc.layout = *layout;
  bind_group_desc.entryCount = desc.entry_count;
  bind_group_desc.entries = entries.data();

  wgpu::BindGroup bind_group = m_device.CreateBindGroup(&bind_group_desc);
  if (!bind_group) {
    return {};
  }

  return BindGroupHandle{.id = AddResource(m_bind_groups, std::move(bind_group))};
}

PipelineHandle WgpuBackend::CreatePipeline(const PipelineDesc& desc) {
  wgpu::ShaderModule* shader = GetShader(desc.shader);
  if (!m_device || shader == nullptr || desc.vertex_entry == nullptr || desc.fragment_entry == nullptr ||
      desc.color_target_count == 0 || desc.color_targets == nullptr) {
    return {};
  }

  std::vector<wgpu::BindGroupLayout> bind_group_layouts(desc.bind_group_layout_count);
  for (std::uint32_t index = 0; index < desc.bind_group_layout_count; ++index) {
    wgpu::BindGroupLayout* layout = GetBindGroupLayout(desc.bind_group_layouts[index]);
    if (layout == nullptr) {
      SK_ERROR("CreatePipeline failed: invalid bind group layout handle.");
      return {};
    }
    bind_group_layouts[index] = *layout;
  }

  wgpu::PipelineLayoutDescriptor pipeline_layout_desc = {};
  pipeline_layout_desc.bindGroupLayoutCount = desc.bind_group_layout_count;
  pipeline_layout_desc.bindGroupLayouts = bind_group_layouts.data();
  wgpu::PipelineLayout pipeline_layout = m_device.CreatePipelineLayout(&pipeline_layout_desc);

  std::vector<std::array<wgpu::VertexAttribute, 8>> attribute_storage(desc.vertex_buffer_count);
  std::vector<wgpu::VertexBufferLayout> vertex_layouts(desc.vertex_buffer_count);
  for (std::uint32_t index = 0; index < desc.vertex_buffer_count; ++index) {
    const VertexBufferLayoutDesc& input_layout = desc.vertex_buffers[index];
    if (input_layout.attribute_count > attribute_storage[index].size()) {
      SK_ERROR("CreatePipeline failed: vertex attribute count too high.");
      return {};
    }

    for (std::uint32_t attr_index = 0; attr_index < input_layout.attribute_count; ++attr_index) {
      attribute_storage[index][attr_index].format = ToWgpuVertexFormat(input_layout.attributes[attr_index].format);
      attribute_storage[index][attr_index].offset = input_layout.attributes[attr_index].offset;
      attribute_storage[index][attr_index].shaderLocation = input_layout.attributes[attr_index].shader_location;
    }

    vertex_layouts[index].arrayStride = input_layout.array_stride;
    vertex_layouts[index].stepMode = wgpu::VertexStepMode::Vertex;
    vertex_layouts[index].attributeCount = input_layout.attribute_count;
    vertex_layouts[index].attributes = attribute_storage[index].data();
  }

  std::vector<wgpu::ColorTargetState> color_targets(desc.color_target_count);
  for (std::uint32_t index = 0; index < desc.color_target_count; ++index) {
    color_targets[index].format = ToWgpuTextureFormat(desc.color_targets[index].format);
  }

  wgpu::FragmentState fragment_state = {};
  fragment_state.module = *shader;
  fragment_state.entryPoint = desc.fragment_entry;
  fragment_state.targetCount = desc.color_target_count;
  fragment_state.targets = color_targets.data();

  wgpu::DepthStencilState depth_stencil_state = {};
  if (desc.has_depth_stencil) {
    depth_stencil_state.format = ToWgpuTextureFormat(desc.depth_stencil.format);
    depth_stencil_state.depthWriteEnabled = desc.depth_stencil.depth_write_enabled;
    depth_stencil_state.depthCompare = ToWgpuCompareFunction(desc.depth_stencil.depth_compare);
  }

  wgpu::RenderPipelineDescriptor pipeline_desc = {};
  pipeline_desc.layout = pipeline_layout;
  pipeline_desc.vertex.module = *shader;
  pipeline_desc.vertex.entryPoint = desc.vertex_entry;
  pipeline_desc.vertex.bufferCount = desc.vertex_buffer_count;
  pipeline_desc.vertex.buffers = vertex_layouts.data();
  pipeline_desc.primitive.topology = ToWgpuPrimitiveTopology(desc.topology);
  pipeline_desc.primitive.cullMode = ToWgpuCullMode(desc.cull_mode);
  pipeline_desc.primitive.frontFace = ToWgpuFrontFace(desc.front_face);
  pipeline_desc.fragment = &fragment_state;
  pipeline_desc.depthStencil = desc.has_depth_stencil ? &depth_stencil_state : nullptr;
  pipeline_desc.multisample.count = 1;

  wgpu::RenderPipeline pipeline = m_device.CreateRenderPipeline(&pipeline_desc);
  if (!pipeline) {
    return {};
  }

  return PipelineHandle{.id = AddResource(m_pipelines, std::move(pipeline))};
}

void WgpuBackend::UpdateBuffer(BufferHandle handle, std::uint64_t offset, const void* data, std::uint64_t size) {
  wgpu::Buffer* buffer = GetBuffer(handle);
  if (buffer == nullptr || data == nullptr || size == 0) {
    return;
  }
  m_queue.WriteBuffer(*buffer, offset, data, size);
}

bool WgpuBackend::BeginFrame(const FrameBeginDesc& begin_desc, const RenderTargetRefs& targets) {
  if (!m_device || targets.color_view == nullptr || targets.depth_view == nullptr) {
    return false;
  }
  if (m_in_frame) {
    SK_ERROR("BeginFrame called while a frame is already active.");
    return false;
  }

  const wgpu::TextureView* color_view = static_cast<const wgpu::TextureView*>(targets.color_view);
  const wgpu::TextureView* depth_view = static_cast<const wgpu::TextureView*>(targets.depth_view);
  if (!(*color_view) || !(*depth_view)) {
    return false;
  }

  m_active_encoder = m_device.CreateCommandEncoder();

  wgpu::RenderPassColorAttachment color_attachment = {};
  color_attachment.view = *color_view;
  color_attachment.loadOp = wgpu::LoadOp::Clear;
  color_attachment.storeOp = wgpu::StoreOp::Store;
  color_attachment.clearValue = {
      .r = begin_desc.clear_color.r,
      .g = begin_desc.clear_color.g,
      .b = begin_desc.clear_color.b,
      .a = begin_desc.clear_color.a,
  };

  wgpu::RenderPassDepthStencilAttachment depth_attachment = {};
  depth_attachment.view = *depth_view;
  depth_attachment.depthLoadOp = wgpu::LoadOp::Clear;
  depth_attachment.depthStoreOp = wgpu::StoreOp::Store;
  depth_attachment.depthClearValue = begin_desc.clear_depth;

  wgpu::RenderPassDescriptor pass_desc = {};
  pass_desc.colorAttachmentCount = 1;
  pass_desc.colorAttachments = &color_attachment;
  pass_desc.depthStencilAttachment = &depth_attachment;

  m_active_pass = m_active_encoder.BeginRenderPass(&pass_desc);
  m_in_frame = static_cast<bool>(m_active_pass);
  return m_in_frame;
}

void WgpuBackend::SetPipeline(PipelineHandle handle) {
  if (!m_in_frame) {
    return;
  }
  wgpu::RenderPipeline* pipeline = GetPipeline(handle);
  if (pipeline == nullptr) {
    SK_ERROR("SetPipeline failed: invalid pipeline handle.");
    return;
  }
  m_active_pass.SetPipeline(*pipeline);
}

void WgpuBackend::SetBindGroup(std::uint32_t index, BindGroupHandle handle) {
  if (!m_in_frame) {
    return;
  }
  wgpu::BindGroup* bind_group = GetBindGroup(handle);
  if (bind_group == nullptr) {
    SK_ERROR("SetBindGroup failed: invalid bind group handle.");
    return;
  }
  m_active_pass.SetBindGroup(index, *bind_group);
}

void WgpuBackend::SetVertexBuffer(std::uint32_t slot, BufferHandle handle, std::uint64_t offset) {
  if (!m_in_frame) {
    return;
  }
  wgpu::Buffer* buffer = GetBuffer(handle);
  if (buffer == nullptr) {
    SK_ERROR("SetVertexBuffer failed: invalid buffer handle.");
    return;
  }
  m_active_pass.SetVertexBuffer(slot, *buffer, offset, std::numeric_limits<std::uint64_t>::max());
}

void WgpuBackend::SetIndexBuffer(BufferHandle handle, IndexType index_type, std::uint64_t offset) {
  if (!m_in_frame) {
    return;
  }
  wgpu::Buffer* buffer = GetBuffer(handle);
  if (buffer == nullptr) {
    SK_ERROR("SetIndexBuffer failed: invalid buffer handle.");
    return;
  }
  m_active_pass.SetIndexBuffer(*buffer, ToWgpuIndexFormat(index_type), offset,
                               std::numeric_limits<std::uint64_t>::max());
}

void WgpuBackend::DrawIndexed(std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index,
                              std::int32_t base_vertex, std::uint32_t first_instance) {
  if (!m_in_frame) {
    return;
  }
  m_active_pass.DrawIndexed(index_count, instance_count, first_index, base_vertex, first_instance);
}

void WgpuBackend::EndFrameAndSubmit() {
  if (!m_in_frame) {
    return;
  }

  m_active_pass.End();
  m_active_pass = nullptr;

  wgpu::CommandBuffer commands = m_active_encoder.Finish();
  m_active_encoder = nullptr;
  m_in_frame = false;

  if (commands) {
    m_queue.Submit(1, &commands);
  }
}

wgpu::Buffer* WgpuBackend::GetBuffer(BufferHandle handle) {
  if (handle.id == 0 || handle.id > m_buffers.size()) {
    return nullptr;
  }
  return &m_buffers[handle.id - 1];
}

wgpu::ShaderModule* WgpuBackend::GetShader(ShaderHandle handle) {
  if (handle.id == 0 || handle.id > m_shaders.size()) {
    return nullptr;
  }
  return &m_shaders[handle.id - 1];
}

wgpu::BindGroupLayout* WgpuBackend::GetBindGroupLayout(BindGroupLayoutHandle handle) {
  if (handle.id == 0 || handle.id > m_bind_group_layouts.size()) {
    return nullptr;
  }
  return &m_bind_group_layouts[handle.id - 1];
}

wgpu::BindGroup* WgpuBackend::GetBindGroup(BindGroupHandle handle) {
  if (handle.id == 0 || handle.id > m_bind_groups.size()) {
    return nullptr;
  }
  return &m_bind_groups[handle.id - 1];
}

wgpu::RenderPipeline* WgpuBackend::GetPipeline(PipelineHandle handle) {
  if (handle.id == 0 || handle.id > m_pipelines.size()) {
    return nullptr;
  }
  return &m_pipelines[handle.id - 1];
}

wgpu::BufferUsage WgpuBackend::ToWgpuBufferUsage(BufferUsageFlags usage) {
  wgpu::BufferUsage result = wgpu::BufferUsage::None;
  if (HasFlag(usage, BufferUsageFlags::CopyDst)) {
    result = result | wgpu::BufferUsage::CopyDst;
  }
  if (HasFlag(usage, BufferUsageFlags::Vertex)) {
    result = result | wgpu::BufferUsage::Vertex;
  }
  if (HasFlag(usage, BufferUsageFlags::Index)) {
    result = result | wgpu::BufferUsage::Index;
  }
  if (HasFlag(usage, BufferUsageFlags::Uniform)) {
    result = result | wgpu::BufferUsage::Uniform;
  }
  return result;
}

wgpu::ShaderStage WgpuBackend::ToWgpuShaderStage(ShaderStage stage) {
  switch (stage) {
  case ShaderStage::Vertex:
    return wgpu::ShaderStage::Vertex;
  case ShaderStage::Fragment:
    return wgpu::ShaderStage::Fragment;
  default:
    return wgpu::ShaderStage::Vertex;
  }
}

wgpu::VertexFormat WgpuBackend::ToWgpuVertexFormat(VertexFormat format) {
  switch (format) {
  case VertexFormat::Float32x3:
    return wgpu::VertexFormat::Float32x3;
  default:
    return wgpu::VertexFormat::Float32x3;
  }
}

wgpu::TextureFormat WgpuBackend::ToWgpuTextureFormat(PixelFormat format) {
  switch (format) {
  case PixelFormat::BGRA8UnormSrgb:
    return wgpu::TextureFormat::BGRA8UnormSrgb;
  case PixelFormat::RGBA8UnormSrgb:
    return wgpu::TextureFormat::RGBA8UnormSrgb;
  case PixelFormat::Depth24Plus:
    return wgpu::TextureFormat::Depth24Plus;
  default:
    return wgpu::TextureFormat::Undefined;
  }
}

wgpu::PrimitiveTopology WgpuBackend::ToWgpuPrimitiveTopology(PrimitiveTopology topology) {
  switch (topology) {
  case PrimitiveTopology::TriangleList:
    return wgpu::PrimitiveTopology::TriangleList;
  default:
    return wgpu::PrimitiveTopology::TriangleList;
  }
}

wgpu::CullMode WgpuBackend::ToWgpuCullMode(CullMode mode) {
  switch (mode) {
  case CullMode::None:
    return wgpu::CullMode::None;
  case CullMode::Back:
    return wgpu::CullMode::Back;
  default:
    return wgpu::CullMode::None;
  }
}

wgpu::FrontFace WgpuBackend::ToWgpuFrontFace(FrontFace face) {
  switch (face) {
  case FrontFace::CCW:
    return wgpu::FrontFace::CCW;
  case FrontFace::CW:
    return wgpu::FrontFace::CW;
  default:
    return wgpu::FrontFace::CCW;
  }
}

wgpu::CompareFunction WgpuBackend::ToWgpuCompareFunction(CompareFunction function) {
  switch (function) {
  case CompareFunction::Less:
    return wgpu::CompareFunction::Less;
  default:
    return wgpu::CompareFunction::Less;
  }
}

wgpu::IndexFormat WgpuBackend::ToWgpuIndexFormat(IndexType index_type) {
  switch (index_type) {
  case IndexType::Uint16:
    return wgpu::IndexFormat::Uint16;
  case IndexType::Uint32:
    return wgpu::IndexFormat::Uint32;
  default:
    return wgpu::IndexFormat::Uint16;
  }
}

} // namespace Sidekick::Renderer::Backend
