#pragma once

#include "Sidekick/Renderer/Backend/GraphicsBackend.h"

#include <vector>

#include <webgpu/webgpu_cpp.h>

namespace Sidekick::Renderer::Backend {

class WgpuBackend final : public GraphicsBackend {
public:
  bool Initialize(const BackendBootstrapContext& context) override;
  void Shutdown() override;

  BufferHandle CreateBuffer(const BufferDesc& desc) override;
  ShaderHandle CreateShader(const ShaderDesc& desc) override;
  BindGroupLayoutHandle CreateBindGroupLayout(const BindGroupLayoutDesc& desc) override;
  BindGroupHandle CreateBindGroup(const BindGroupDesc& desc) override;
  PipelineHandle CreatePipeline(const PipelineDesc& desc) override;

  void UpdateBuffer(BufferHandle handle, std::uint64_t offset, const void* data, std::uint64_t size) override;

  bool BeginFrame(const FrameBeginDesc& begin_desc, const RenderTargetRefs& targets) override;
  void SetPipeline(PipelineHandle handle) override;
  void SetBindGroup(std::uint32_t index, BindGroupHandle handle) override;
  void SetVertexBuffer(std::uint32_t slot, BufferHandle handle, std::uint64_t offset) override;
  void SetIndexBuffer(BufferHandle handle, IndexType index_type, std::uint64_t offset) override;
  void DrawIndexed(std::uint32_t index_count, std::uint32_t instance_count, std::uint32_t first_index,
                   std::int32_t base_vertex, std::uint32_t first_instance) override;
  void EndFrameAndSubmit() override;

private:
  template <typename TObject> std::uint32_t AddResource(std::vector<TObject>& resources, TObject resource);

  wgpu::Buffer* GetBuffer(BufferHandle handle);
  wgpu::ShaderModule* GetShader(ShaderHandle handle);
  wgpu::BindGroupLayout* GetBindGroupLayout(BindGroupLayoutHandle handle);
  wgpu::BindGroup* GetBindGroup(BindGroupHandle handle);
  wgpu::RenderPipeline* GetPipeline(PipelineHandle handle);

  static wgpu::BufferUsage ToWgpuBufferUsage(BufferUsageFlags usage);
  static wgpu::ShaderStage ToWgpuShaderStage(ShaderStage stage);
  static wgpu::VertexFormat ToWgpuVertexFormat(VertexFormat format);
  static wgpu::TextureFormat ToWgpuTextureFormat(PixelFormat format);
  static wgpu::PrimitiveTopology ToWgpuPrimitiveTopology(PrimitiveTopology topology);
  static wgpu::CullMode ToWgpuCullMode(CullMode mode);
  static wgpu::FrontFace ToWgpuFrontFace(FrontFace face);
  static wgpu::CompareFunction ToWgpuCompareFunction(CompareFunction function);
  static wgpu::IndexFormat ToWgpuIndexFormat(IndexType index_type);

  wgpu::Device m_device;
  wgpu::Queue m_queue;

  std::vector<wgpu::Buffer> m_buffers;
  std::vector<wgpu::ShaderModule> m_shaders;
  std::vector<wgpu::BindGroupLayout> m_bind_group_layouts;
  std::vector<wgpu::BindGroup> m_bind_groups;
  std::vector<wgpu::RenderPipeline> m_pipelines;

  wgpu::CommandEncoder m_active_encoder;
  wgpu::RenderPassEncoder m_active_pass;
  bool m_in_frame = false;
};

} // namespace Sidekick::Renderer::Backend
