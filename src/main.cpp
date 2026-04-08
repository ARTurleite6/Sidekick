#include "Sidekick/Core/CommandList.hpp"
#include "Sidekick/Core/GraphicsContext.hpp"
#include "Sidekick/Core/Platform.hpp"
#include "Sidekick/Core/Swapchain.hpp"
#include "Sidekick/Core/Window.hpp"

#include <optional>

int main()
{
  auto platform = Sidekick::Platform::Create();
  auto window = platform->CreateWindow({});
  auto gfx = Sidekick::GraphicsContext::Create();
  auto swapchain = gfx->CreateSwapchain({.window_handle = window->GetHandle(), .size_x = 1280, .size_y = 720});

  auto cmd_list = gfx->CreateCommandList({.debug_name = "CommandList", .type = Sidekick::CommandListType::Graphics});

  while (!window->ShouldClose())
  {
    platform->Update();

    Sidekick::ColorAttachment color_attachments[] = {{
        .texture = {}, // invalid = backbuffer
        .load_op = Sidekick::LoadOp::Clear,
        .store_op = Sidekick::StoreOp::Store,
        .clear_color = {1.f, 0.f, 0.f, 1.f}, // red
    }};

    cmd_list->BeginRenderPass({
        .name = "RedScreen",
        .color_attachments = color_attachments,
        .depth_attachment = std::nullopt,
        .viewport =
            {
                .x = 0,
                .y = 0,
                .width = 1280,
                .height = 720,
            },
    });

    cmd_list->EndRenderPass();

    swapchain->Present();
  }
}
