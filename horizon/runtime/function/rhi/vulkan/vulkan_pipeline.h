#pragma once

#include "runtime/core/container/container.h"
#include "runtime/function/rhi/pipeline.h"
#include "runtime/function/rhi/shader.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_pool.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set.h"
#include "runtime/function/rhi/vulkan/vulkan_pipeline_layout.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"

namespace Horizon::Backend {

class PipelineState {};

class VulkanPipeline : public Pipeline {
  public:
    VulkanPipeline(const VulkanRendererContext &context, const DescriptorPool &t_descriptor_pool,
                   const GraphicsPipelineCreateInfo &create_info) noexcept;
    VulkanPipeline(const VulkanRendererContext &context, const DescriptorPool &t_descriptor_pool,
                   const ComputePipelineCreateInfo &create_info) noexcept;

    virtual ~VulkanPipeline() noexcept;
    VulkanPipeline(const VulkanPipeline &rhs) noexcept = delete;
    VulkanPipeline &operator=(const VulkanPipeline &rhs) noexcept = delete;
    VulkanPipeline(VulkanPipeline &&rhs) noexcept = delete;
    VulkanPipeline &operator=(VulkanPipeline &&rhs) noexcept = delete;

    //void SetPipelineState();
    void SetShader(Shader *shader) noexcept override;
    DescriptorSet *GetDescriptorSet(ResourceUpdateFrequency frequency);
    const VkPipeline &get() const noexcept { return m_pipeline; }
    const VkPipelineLayout &GetLayout() const noexcept { return m_pipeline_layout->get(); }
    const VkViewport &GetViewPort() const noexcept { return view_port; }
    const VkRect2D &GetScissor() const noexcept { return scissor; }

  private:
    void Create();

    void CreatePipelineLayout();

    void CreateGraphicsPipeline();

    void CreateComputePipeline();
    
    // void CreateRTPipeline() noexcept;s
  private:
    bool b_created = false;
    const VulkanRendererContext &m_context{};
    const DescriptorPool &m_descriptor_pool;
    VkPipeline m_pipeline{};
    Memory::UniquePtr<PipelineLayout> m_pipeline_layout;
    VkViewport view_port{};
    VkRect2D scissor{};
};

} // namespace Horizon::Backend