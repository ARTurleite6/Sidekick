#pragma once

#include "Sidekick/Core/CommandList.hpp"

#include <cstdint>
#include <string>

namespace Sidekick
{
class OpenGLCommandList final : public CommandList
{
public:
  explicit OpenGLCommandList(CommandListDescriptor&& desc);

  void BeginRenderPass(RenderPassDescriptor&& desc) override;

  void EndRenderPass() override;

private:
  uint32_t GetOrCreateFBO(const RenderPassDescriptor& desc);
  void ApplyLoadOps(const RenderPassDescriptor& desc);

  std::string m_Name;
  CommandListType m_Type;

  bool m_InRenderPass{false};

  uint32_t m_CurrentFBO{0};
  RenderPassDescriptor m_CurrentPassDesc{};
};
} // namespace Sidekick
