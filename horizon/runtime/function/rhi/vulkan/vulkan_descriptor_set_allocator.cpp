#include "vulkan_descriptor_set_allocator.h"

#include <algorithm>

#include "runtime/function/rhi/rhi_utils.h"
#include "runtime/function/rhi/resource_cache.h"
#include "runtime/function/rhi/vulkan/vulkan_pipeline.h"

namespace Horizon::Backend {

VulkanDescriptorSetAllocator::VulkanDescriptorSetAllocator(const VulkanRendererContext &context) noexcept
    : m_context(context) {
    // create empty layout
    if (m_empty_descriptor_set == VK_NULL_HANDLE || m_empty_descriptor_set_layout_hash_key == 0) {

        VkDescriptorSetLayoutCreateInfo set_layout_create_info{};
        set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        set_layout_create_info.flags = 0;
        set_layout_create_info.pNext = nullptr;
        set_layout_create_info.bindingCount = 0;
        set_layout_create_info.pBindings = nullptr;

        VkDescriptorSetLayout layout;
        CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_context.device, &set_layout_create_info, nullptr, &layout));
        u64 seed = 0;
        HashCombine(seed, set_layout_create_info);
        m_empty_descriptor_set_layout_hash_key = seed;
        m_descriptor_set_layout_map.emplace(m_empty_descriptor_set_layout_hash_key, layout);
    }
}

VulkanDescriptorSetAllocator::~VulkanDescriptorSetAllocator() noexcept {
    for (auto &layout : m_descriptor_set_layout_map) {
        vkDestroyDescriptorSetLayout(m_context.device, layout.second, nullptr);
    }

    // all descriptor sets allocated from the pool are implicitly freed and become invalid

    // for (auto &pool : m_descriptor_pools) {
    //     /*pool.clear();*/
    //     for (auto p : pool) {
    //         vkDestroyDescriptorPool(m_context.device, p, nullptr);
    //     }
    // }

    vkDestroyDescriptorPool(m_context.device, m_temp_descriptor_pool, nullptr);
}


void VulkanDescriptorSetAllocator::CreateDescriptorPool() {
    Container::FixedArray<VkDescriptorType, 5> types{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                          VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
    auto stack_memory = Memory::GetStackMemoryResource(1024);
    Container::Array<VkDescriptorPoolSize> pool_sizes(&stack_memory);
    for (auto type : types) {
        pool_sizes.push_back(VkDescriptorPoolSize{type, 2048});
    }

    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = nullptr;
    pool_create_info.flags = 0;

    pool_create_info.maxSets = 2048;
    pool_create_info.poolSizeCount = static_cast<u32>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();

    CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device, &pool_create_info, nullptr, &m_temp_descriptor_pool));

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

void VulkanDescriptorSetAllocator::CreateBindlessDescriptorPool() {
    Container::FixedArray<VkDescriptorType, 5> types{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                          VK_DESCRIPTOR_TYPE_SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                                          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};
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

    CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device, &pool_create_info, nullptr, &m_bindless_descriptor_pool));
}

void VulkanDescriptorSetAllocator::ResetDescriptorPool() {
    for (auto &set : allocated_sets) {
        if(set)
            delete set;
    }
    allocated_sets.clear();
    if (m_temp_descriptor_pool)
    vkResetDescriptorPool(m_context.device, m_temp_descriptor_pool, 0);
    if (m_bindless_descriptor_pool) {
        vkResetDescriptorPool(m_context.device, m_bindless_descriptor_pool, 0);
    }
    //for (u32 freq = 0; freq < DESCRIPTOR_SET_UPDATE_FREQUENCIES; freq++) {

    //    for (auto &[pipeline, resource] : pipeline_descriptor_set_resources) {
    //        //resource.m_used_set_counter[freq] = 0;
    //    }
    //    if (!m_descriptor_pools.empty()) {
    //        // reset all descriptorpool and free all descriptors
    //        for (auto &pool : m_descriptor_pools) {
    //            vkResetDescriptorPool(m_context.device, pool, 0);
    //        }
    //    }
    //}
}

VkDescriptorSetLayout VulkanDescriptorSetAllocator::GetVkDescriptorSetLayout(u64 key) const {
    return m_descriptor_set_layout_map.at(key);
}

} // namespace Horizon::Backend