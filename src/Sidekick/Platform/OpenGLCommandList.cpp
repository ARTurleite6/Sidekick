#include "Sidekick/Platform/OpenGLCommandList.hpp"

#include "Sidekick/Core/CommandList.hpp"

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cassert>
#include <cstdint>

namespace Sidekick
{
OpenGLCommandList::OpenGLCommandList(CommandListDescriptor&& desc) : m_Name{desc.debug_name}, m_Type{desc.type} {}

void OpenGLCommandList::BeginRenderPass(RenderPassDescriptor&& desc)
{
  assert(!m_InRenderPass && "Already inside a render pass");

  if (!desc.name.empty())
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, static_cast<GLsizei>(desc.name.size()), desc.name.data());

  // TODO: implement the FBO caching in the future
  const bool is_swapchain_pass = desc.color_attachments.empty() && false; // check that the texture is invalid

  if (is_swapchain_pass)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_CurrentFBO = 0;
  }
  else
  {
    m_CurrentFBO = GetOrCreateFBO(desc);
    glBindFramebuffer(GL_FRAMEBUFFER, m_CurrentFBO);
  }

  ApplyLoadOps(desc);

  glViewport(static_cast<GLint>(desc.viewport.x), static_cast<GLint>(desc.viewport.y), static_cast<GLsizei>(desc.viewport.width), static_cast<GLsizei>(desc.viewport.height));

  glDepthRangef(desc.viewport.min_depth, desc.viewport.max_depth);

  if (desc.scissor.width > 0 && desc.scissor.height > 0)
  {
    glEnable(GL_SCISSOR_TEST);
    glScissor(desc.scissor.x, desc.scissor.y, desc.scissor.width, desc.scissor.height);
  }
  else
  {
    glDisable(GL_SCISSOR_TEST);
  }

  m_InRenderPass = true;
  m_CurrentPassDesc = desc; // store for endRenderPass StoreOp if needed
}

void OpenGLCommandList::EndRenderPass()
{
  assert(m_InRenderPass && "Not inside a render pass");

  {
    std::array<GLenum, 9> discard_attachments;
    uint32_t discard_count{0};

    for (uint32_t i = 0; i < m_CurrentPassDesc.color_attachments.size(); ++i)
    {
      const auto& color_attachment = m_CurrentPassDesc.color_attachments[i];
      if (color_attachment.store_op == StoreOp::DontCare)
      {
        discard_attachments[discard_count++] = (m_CurrentFBO == 0) ? GL_COLOR : GL_COLOR_ATTACHMENT0 + i;
      }
    }

    if (m_CurrentPassDesc.depth_attachment && m_CurrentPassDesc.depth_attachment->depth_store_op == StoreOp::DontCare)
    {
      discard_attachments[discard_count++] = (m_CurrentFBO == 0) ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
    }

    if (discard_count > 0)
      glInvalidateFramebuffer(GL_FRAMEBUFFER, discard_count, discard_attachments.data());
  }

  if (!m_CurrentPassDesc.name.empty())
    glPopDebugGroup();

  glDisable(GL_SCISSOR_TEST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  m_InRenderPass = false;
  m_CurrentFBO = 0;
  // TODO: m_CurrentPipeline reset
}

uint32_t OpenGLCommandList::GetOrCreateFBO(const RenderPassDescriptor& desc)
{
  // Implement this in the future;
  return -1;
}

void OpenGLCommandList::ApplyLoadOps(const RenderPassDescriptor& desc)
{
  for (uint32_t i = 0; i < desc.color_attachments.size(); i++)
  {
    const auto& att = desc.color_attachments[i];
    if (att.load_op == LoadOp::Clear)
    {
      // glClearBufferfv clears a single draw buffer — correct for MRT
      glClearBufferfv(GL_COLOR, i, glm::value_ptr(att.clear_color));
    }
    // LoadOp::Load  → do nothing, contents are preserved
    // LoadOp::DontCare → do nothing, contents are undefined (driver may skip)
  }

  if (desc.depth_attachment)
  {
    const auto& att = *desc.depth_attachment;
    if (att.depth_load_op == LoadOp::Clear && att.stencil_load_op == LoadOp::Clear)
    {
      glClearBufferfi(GL_DEPTH_STENCIL, 0, att.clear_depth, static_cast<GLint>(att.clear_stencil));
    }
    else if (att.depth_load_op == LoadOp::Clear)
    {
      glClearBufferfv(GL_DEPTH, 0, &att.clear_depth);
    }
    else if (att.stencil_load_op == LoadOp::Clear)
    {
      GLint s = att.clear_stencil;
      glClearBufferiv(GL_STENCIL, 0, &s);
    }
  }
}

} // namespace Sidekick
