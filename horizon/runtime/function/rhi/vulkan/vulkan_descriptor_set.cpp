#include "vulkan_descriptor_set.h"

#include "runtime/function/rhi/vulkan/vulkan_descriptor_set_layout.h"

namespace Horizon::Backend {

VulkanDescriptorSet::VulkanDescriptorSet(const VulkanRendererContext &context, ResourceUpdateFrequency frequency,
                                         const Container::HashMap<Container::String, ShaderResource> &t_set_resources,
                                         VkDescriptorSet set) noexcept
    : DescriptorSet(frequency), m_context(context), m_set_resources(t_set_resources), m_set(set) {}

void VulkanDescriptorSet::SetResource(Buffer *buffer, const Container::String &resource_name) {
    auto res = m_set_resources.find(resource_name);
    if (res == m_set_resources.end()) {
        LOG_ERROR("resource {} is not declared in this descriptorset", resource_name);
        return;
    }

    VulkanBuffer *vk_buffer = reinterpret_cast<VulkanBuffer *>(buffer);

    VkDescriptorType descriptor_type = DescriptorSetLayout::find_descriptor_type(res->second.type, false);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstBinding = res->second.binding;
    write.descriptorType = descriptor_type;
    write.descriptorCount = 1;
    write.dstSet = m_set;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.pBufferInfo = vk_buffer->GetDescriptorBufferInfo(0, buffer->m_size); // TODO: offset

    writes.push_back(write);
}

void VulkanDescriptorSet::SetResource(Texture *texture, const Container::String &resource_name) {
    auto res = m_set_resources.find(resource_name);
    if (res == m_set_resources.end()) {
        LOG_ERROR("resource {} is not declared in this descriptorset", resource_name);
        return;
    }
    VulkanTexture *vk_texture = reinterpret_cast<VulkanTexture *>(texture);
    VkDescriptorType descriptor_type = DescriptorSetLayout::find_descriptor_type(res->second.type, false);
    VkWriteDescriptorSet write{};

    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstBinding = res->second.binding;
    write.descriptorType = descriptor_type;
    write.descriptorCount = 1;
    write.dstSet = m_set;
    write.dstArrayElement = 0;
    write.pImageInfo = vk_texture->GetDescriptorImageInfo(descriptor_type);
    writes.push_back(write);
}

void VulkanDescriptorSet::SetResource(Sampler *sampler, const Container::String &resource_name) {
    auto res = m_set_resources.find(resource_name);
    if (res == m_set_resources.end()) {
        LOG_ERROR("resource {} is not declared in this descriptorset", resource_name);
        return;
    }
    VulkanSampler *vk_sampler = reinterpret_cast<VulkanSampler *>(sampler);
    VkDescriptorType descriptor_type = DescriptorSetLayout::find_descriptor_type(res->second.type, false);
    VkWriteDescriptorSet write{};

    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstBinding = res->second.binding;
    write.descriptorType = descriptor_type; // sampler
    write.descriptorCount = 1;
    write.dstSet = m_set;
    write.dstArrayElement = 0;
    write.pImageInfo = vk_sampler->GetDescriptorImageInfo();
    writes.push_back(write);
}

void VulkanDescriptorSet::SetResource(Container::Array<Buffer *> &resource, const Container::String &resource_name) {

    auto res = m_set_resources.find(resource_name);
    if (res == m_set_resources.end()) {
        LOG_ERROR("resource {} is not declared in this descriptorset", resource_name);
        return;
    }
    VkDescriptorType descriptor_type = DescriptorSetLayout::find_descriptor_type(res->second.type, false);

    auto &buffer_descriptors = bindless_buffer_descriptors[resource_name];

    for (auto &buffer : resource) {
        auto vk_buffer = reinterpret_cast<VulkanBuffer *>(buffer);

        buffer_descriptors.push_back(*vk_buffer->GetDescriptorBufferInfo(0, buffer->m_size));
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = res->second.binding;
    write.dstArrayElement = 0;
    write.descriptorType = write.descriptorType = descriptor_type;

    write.descriptorCount = static_cast<uint32_t>(resource.size());
    write.pBufferInfo = buffer_descriptors.data();
    write.dstSet = m_set;
    writes.push_back(write);
}

void VulkanDescriptorSet::SetResource(Container::Array<Texture *> &resource, const Container::String &resource_name) {

    auto res = m_set_resources.find(resource_name);
    if (res == m_set_resources.end()) {
        LOG_ERROR("resource {} is not declared in this descriptorset", resource_name);
        return;
    }

    auto &texture_descriptors = bindless_image_descriptors[resource_name];
    VkDescriptorType descriptor_type = DescriptorSetLayout::find_descriptor_type(res->second.type, false);
    for (auto &texture : resource) {
        auto vk_texture = reinterpret_cast<VulkanTexture *>(texture);

        texture_descriptors.push_back(*vk_texture->GetDescriptorImageInfo(descriptor_type));
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = res->second.binding;
    write.dstArrayElement = 0;
    write.descriptorType = descriptor_type;

    write.descriptorCount = static_cast<uint32_t>(resource.size());
    write.pBufferInfo = 0;
    write.dstSet = m_set;
    write.pImageInfo = texture_descriptors.data();
    writes.push_back(write);
}

void VulkanDescriptorSet::SetResource(Container::Array<Sampler *> &resource, const Container::String &resource_name) {

    auto res = m_set_resources.find(resource_name);
    if (res == m_set_resources.end()) {
        LOG_ERROR("resource {} is not declared in this descriptorset", resource_name);
        return;
    }

    auto &sampler_descriptors = bindless_sampler_descriptors[resource_name];
    VkDescriptorType descriptor_type = DescriptorSetLayout::find_descriptor_type(res->second.type, false);
    for (auto &sampler : resource) {
        auto vk_sampler = reinterpret_cast<VulkanSampler *>(sampler);

        sampler_descriptors.push_back(*vk_sampler->GetDescriptorImageInfo());
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstBinding = res->second.binding;
    write.dstArrayElement = 0;
    write.descriptorType = descriptor_type;

    write.descriptorCount = static_cast<uint32_t>(resource.size());
    write.pBufferInfo = 0;
    write.dstSet = m_set;
    write.pImageInfo = sampler_descriptors.data();
    writes.push_back(write);
}

void VulkanDescriptorSet::Update() {
    vkUpdateDescriptorSets(m_context.device, static_cast<u32>(writes.size()), writes.data(), 0, nullptr);
}
} // namespace Horizon::Backend
