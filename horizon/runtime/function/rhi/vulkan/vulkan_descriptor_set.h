#pragma once

#include "runtime/core/utils/definations.h"

#include "runtime/function/rhi/rhi_utils.h"
#include "runtime/function/rhi/descriptor_set.h"
#include "runtime/function/rhi/vulkan/vulkan_buffer.h"
#include "runtime/function/rhi/vulkan/vulkan_sampler.h"
#include "runtime/function/rhi/vulkan/vulkan_texture.h"
#include "runtime/function/rhi/vulkan/vulkan_shader.h"

namespace Horizon::Backend {

class VulkanDescriptorSet : public DescriptorSet {
  public:
    VulkanDescriptorSet(const VulkanRendererContext &context, ResourceUpdateFrequency frequency,
                        const Container::HashMap<Container::String, ShaderResource>& t_set_resources, VkDescriptorSet set) noexcept;
    virtual ~VulkanDescriptorSet() noexcept = default; 

    VulkanDescriptorSet(const VulkanDescriptorSet &rhs) noexcept = delete;
    VulkanDescriptorSet &operator=(const VulkanDescriptorSet &rhs) noexcept = delete;
    VulkanDescriptorSet(VulkanDescriptorSet &&rhs) noexcept = delete;
    VulkanDescriptorSet &operator=(VulkanDescriptorSet &&rhs) noexcept = delete;

  public:
    void SetResource(Buffer *resource,  const Container::String& resource_name) override;
    void SetResource(Texture *resource, const Container::String& resource_name) override;
    void SetResource(Sampler *resource, const Container::String& resource_name) override;
    void SetResource(Container::Array<Buffer *>& resource, const Container::String &resource_name) override;
    void SetResource(Container::Array<Texture *>& resource, const Container::String &resource_name) override;
    void SetResource(Container::Array<Sampler *>& resource, const Container::String &resource_name) override;
    //void SetBindlessResource(Container::Array<Buffer *>& resource, const Container::String &resource_name) override;
    //void SetBindlessResource(Container::Array<Texture *>& resource, const Container::String &resource_name) override;
    //void SetBindlessResource(Container::Array<Sampler *>& resource, const Container::String &resource_name) override;

    void Update() override;

  public:
    const VulkanRendererContext &m_context{};
    const Container::HashMap<Container::String, ShaderResource> &m_set_resources{};
    Container::Array<VkWriteDescriptorSet> writes{};
    Container::HashMap<Container::String, Container::Array<VkDescriptorImageInfo>> bindless_image_descriptors;
    Container::HashMap<Container::String, Container::Array<VkDescriptorBufferInfo>> bindless_buffer_descriptors;
    Container::HashMap<Container::String, Container::Array<VkDescriptorImageInfo>> bindless_sampler_descriptors;

    VkDescriptorSet m_set{};
};
} // namespace Horizon::Backend
