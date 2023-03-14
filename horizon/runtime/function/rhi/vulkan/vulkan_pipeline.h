#pragma once

#include "runtime/core/container/container.h"
#include "runtime/function/rhi/pipeline.h"
#include "runtime/function/rhi/shader.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set_allocator.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"

namespace Horizon::Backend {

class PipelineLayout {
    
};

class PipelineState{

};

class VulkanPipeline : public Pipeline {
  public:
    VulkanPipeline(const VulkanRendererContext &context, const GraphicsPipelineCreateInfo &create_info) noexcept;
    VulkanPipeline(const VulkanRendererContext &context, const ComputePipelineCreateInfo &create_info) noexcept;

    virtual ~VulkanPipeline() noexcept;
    VulkanPipeline(const VulkanPipeline &rhs) noexcept = delete;
    VulkanPipeline &operator=(const VulkanPipeline &rhs) noexcept = delete;
    VulkanPipeline(VulkanPipeline &&rhs) noexcept = delete;
    VulkanPipeline &operator=(VulkanPipeline &&rhs) noexcept = delete;

    //void SetPipelineState();
    void SetShader(Shader *shader);
  private:
    void Create();
    void CreateGraphicsPipeline();
    void CreateComputePipeline();
    void CreatePipelineLayout();

    // void ParseRootSignatureFromShader(
    //     VulkanShader *shader, ResourceUpdateFrequency frequency);  
    // void CreateRTPipeline() noexcept;
  public:
    const VulkanRendererContext &m_context{};
    VkPipeline m_pipeline{};
    VkPipelineLayout m_pipeline_layout{};


    VkViewport view_port{};
    VkRect2D scissor{};
};

} // namespace Horizon::Backend