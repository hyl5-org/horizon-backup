#pragma once

#include "runtime/core/container/container.h"
#include "runtime/function/rhi/vulkan/vulkan_shader.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set_layout.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set.h"

namespace Horizon::Backend {

class DescriptorPool {
  public:
    DescriptorPool(const VulkanRendererContext& t_context) noexcept;
    ~DescriptorPool() noexcept;
    VkDescriptorSet AllocateDescriptorSet(const VkDescriptorSetLayout &layout) const;
  private: const VulkanRendererContext &m_context;
    VkDescriptorPool m_pool{};
    VkDescriptorPool m_bindless_pool{};
    static constexpr u32 k_max_bindless_resources = 65536;
};
}

