#include "vulkan_descriptor_pool.h"


namespace Horizon::Backend {
    Horizon::Backend::DescriptorPool::DescriptorPool(const VulkanRendererContext &t_context) noexcept
    : m_context(t_context) {
    //Container::FixedArray<VkDescriptorType, 5> types{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    //                                      VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    //                                      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
    //auto stack_memory = Memory::GetStackMemoryResource(1024);
    //Container::Array<VkDescriptorPoolSize> pool_sizes(&stack_memory);
    //for (auto type : types) {
    //    pool_sizes.push_back(VkDescriptorPoolSize{type, 2048});
    //}

    //VkDescriptorPoolCreateInfo pool_create_info{};
    //pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //pool_create_info.pNext = nullptr;
    //pool_create_info.flags = 0;

    //pool_create_info.maxSets = 2048;
    //pool_create_info.poolSizeCount = static_cast<u32>(pool_sizes.size());
    //pool_create_info.pPoolSizes = pool_sizes.data();

    //CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device, &pool_create_info, nullptr, &m_pool));

    Container::FixedArray<VkDescriptorType, 5> types{
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
    auto stack_memory = Memory::GetStackMemoryResource(1024);
    Container::Array<VkDescriptorPoolSize> pool_sizes(&stack_memory);
    for (auto type : types) {
        pool_sizes.push_back(VkDescriptorPoolSize{type, k_max_bindless_resources});
    }

    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;

    pool_create_info.maxSets = static_cast<u32>(pool_sizes.size()) * k_max_bindless_resources;
    pool_create_info.poolSizeCount = static_cast<u32>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();

    CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device, &pool_create_info, nullptr, &m_bindless_pool));

    // for (u32 freq = 0; freq < DESCRIPTOR_SET_UPDATE_FREQUENCIES; freq++) {
    //     if (m_descriptor_pools[freq].empty()) {
    //         Container::Array<VkDescriptorPoolSize> poolSizes(
    //             descriptor_pool_size_descs[freq].required_descriptor_count_per_type.size());

    //        u32 i = 0;
    //        for (auto &[type, count] : descriptor_pool_size_descs[freq].required_descriptor_count_per_type) {
    //            poolSizes[i++] = VkDescriptorPoolSize{type, count * m_reserved_max_sets[freq]};
    //        }

    //        if (poolSizes.empty()) {
    //            continue;
    //        }

    //        VkDescriptorPoolCreateInfo pool_create_info{};
    //        pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    //        pool_create_info.pNext = nullptr;
    //        pool_create_info.flags = 0;
    //
    //        pool_create_info.maxSets = m_reserved_max_sets[freq] * pipeline_descriptor_set_resources.size();
    //        pool_create_info.poolSizeCount = static_cast<u32>(poolSizes.size());
    //        pool_create_info.pPoolSizes = poolSizes.data();

    //        VkDescriptorPool ds_pool{};
    //        CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device, &pool_create_info, nullptr, &ds_pool));
    //        m_descriptor_pools[freq].push_back(ds_pool);
    //    }
    //}
}

Horizon::Backend::DescriptorPool::~DescriptorPool() noexcept {

}

VkDescriptorSet Horizon::Backend::DescriptorPool::AllocateDescriptorSet(const VkDescriptorSetLayout &layout) const {
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = m_bindless_pool;
    allocate_info.pNext = nullptr;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &layout;
    VkDescriptorSet set;
    vkAllocateDescriptorSets(m_context.device, &allocate_info, &set);
    return set;
}

}