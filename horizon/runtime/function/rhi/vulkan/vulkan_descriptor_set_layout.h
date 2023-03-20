#pragma once

#include "runtime/core/container/container.h"
#include "runtime/function/rhi/vulkan/vulkan_shader.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"

namespace Horizon::Backend {

class DescriptorSetLayout {
  public:
    DescriptorSetLayout(const VulkanRendererContext &t_context, u32 t_set_index,
                        const Container::Array<VulkanShader *> &shaders,
                        const Container::HashMap<Container::String, ShaderResource> &shader_resources) noexcept;
    ~DescriptorSetLayout() noexcept;

    static VkDescriptorType find_descriptor_type(ShaderResourceType resource_type, bool dynamic);

    bool validate_binding(const VkDescriptorSetLayoutBinding &binding, const Container::Array<VkDescriptorType> &blacklist);

    bool validate_flags(const Container::Array<VkDescriptorSetLayoutBinding> &bindings,
                               const Container::Array<VkDescriptorBindingFlagsEXT> &flags);

    VkDescriptorSetLayout get() const noexcept { return m_layout; }
    u32 GetIndex() const noexcept { return m_set_index; }
  private:
    const VulkanRendererContext &m_context{};
    VkDescriptorSetLayout m_layout{};

    u32 m_set_index;

    Container::Array<VkDescriptorSetLayoutBinding> bindings;

    Container::Array<VkDescriptorBindingFlagsEXT> binding_flags;

    Container::HashMap<uint32_t, VkDescriptorSetLayoutBinding> bindings_lookup;

    Container::HashMap<uint32_t, VkDescriptorBindingFlagsEXT> binding_flags_lookup;

    Container::HashMap<Container::String, uint32_t> resources_lookup;

    //Container::Array<VulkanShader *> shader_modules;
};
} // namespace Horizon::Backend