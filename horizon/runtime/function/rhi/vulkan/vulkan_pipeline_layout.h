#pragma once

#include "runtime/core/container/container.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set.h"
#include "runtime/function/rhi/vulkan/vulkan_descriptor_set_layout.h"
#include "runtime/function/rhi/vulkan/vulkan_shader.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"

namespace Horizon::Backend {

class PipelineLayout {
  public:
    PipelineLayout(const VulkanRendererContext &t_context,
                   const Container::Array<VulkanShader *> shader_modules) noexcept;
    ~PipelineLayout() noexcept;
    const VkPipelineLayout& get() const noexcept { return m_pipeline_layout; }

    const Container::Array<ShaderResource> GetResource(const ShaderResourceType type = ShaderResourceType::All,
                                                       VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;
    void CreateLayout(const Container::Array<VulkanShader *> shader_modules);

    const Container::HashMap<Container::String, ShaderResource> &GetShaderResources() const noexcept {
        return shader_resources;
    }
    const Container::Array<DescriptorSetLayout *> &GetDescriptorSetLayouts() const noexcept {
        return descriptor_set_layouts;
    }

    const Container::HashMap<Container::String, ShaderResource> &GetSetResources(u32 i) const noexcept {
        return shader_sets.at(i);
    }

  private:
    const VulkanRendererContext &m_context{};
    VkPipelineLayout m_pipeline_layout{};
    Container::HashMap<Container::String, ShaderResource> shader_resources;
    Container::Array<DescriptorSetLayout *> descriptor_set_layouts;
    Container::HashMap<u32, Container::HashMap<Container::String, ShaderResource>> shader_sets;
};

} // namespace Horizon::Backend
