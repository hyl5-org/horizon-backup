#pragma once

#include "runtime/core/utils/definations.h"
#include "runtime/function/rhi/buffer.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"

namespace Horizon::Backend {

class VulkanBuffer : public Buffer {
  public:
    VulkanBuffer(const VulkanRendererContext &context, const BufferCreateInfo &buffer_create_info,
                 MemoryFlag memory_flag) noexcept;
    virtual ~VulkanBuffer() noexcept;
    VulkanBuffer(const VulkanBuffer &rhs) noexcept = delete;
    VulkanBuffer &operator=(const VulkanBuffer &rhs) noexcept = delete;
    VulkanBuffer(VulkanBuffer &&rhs) noexcept = delete;
    VulkanBuffer &operator=(VulkanBuffer &&rhs) noexcept = delete;

    VkDescriptorBufferInfo *GetDescriptorBufferInfo(u64 offset, u64 size) noexcept;

  public:
    const VulkanRendererContext &m_context{};
    VkBuffer m_buffer{};
    VmaAllocation m_memory{};
    VmaAllocationInfo m_allocation_info{};
    VkDescriptorBufferInfo buffer_info{};
    VulkanBuffer* m_stage_buffer{};
};

} // namespace Horizon::Backend