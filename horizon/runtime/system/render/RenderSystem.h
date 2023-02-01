#pragma once

#include <vulkan/vulkan.h>

#include "runtime/core/memory/memory.h"
#include "runtime/core/window/window.h"

#include "runtime/function/rhi/rhi.h"
#include "runtime/function/scene/camera/camera.h"
#include "runtime/function/resource/resource_manager/resource_manager.h"
#include "runtime/function/scene/scene_manager/scene_manager.h"

namespace Horizon {

class RenderSystem {
  public:
    RenderSystem(Window *window, RenderBackend backend, bool offscreen = false) noexcept;

    ~RenderSystem() noexcept;

    RenderSystem(const RenderSystem &rhs) noexcept = delete;

    RenderSystem &operator=(const RenderSystem &rhs) noexcept = delete;

    RenderSystem(RenderSystem &&rhs) noexcept = delete;

    RenderSystem &operator=(RenderSystem &&rhs) noexcept = delete;
    
    SceneManager *GetSceneManager() noexcept { return m_scene_manager.get(); };

  public:
    Backend::RHI *GetRhi() noexcept { return m_rhi.get(); }

  private:
    Window *m_window{};
    Memory::UniquePtr<ResourceManager>m_resource_manager{};
    Memory::UniquePtr<SceneManager>m_scene_manager{};
    Memory::UniquePtr<Backend::RHI> m_rhi{};
};
} // namespace Horizon