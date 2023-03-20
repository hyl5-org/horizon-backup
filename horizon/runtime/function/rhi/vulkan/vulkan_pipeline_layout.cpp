#include "vulkan_pipeline_layout.h"

namespace Horizon::Backend {

Horizon::Backend::PipelineLayout::PipelineLayout(const VulkanRendererContext &t_context,
                                                 const Container::Array<VulkanShader *> shader_modules) noexcept
    : m_context(t_context) {
    CreateLayout(shader_modules);
}

Horizon::Backend::PipelineLayout::~PipelineLayout() noexcept {
    vkDestroyPipelineLayout(m_context.device, m_pipeline_layout, nullptr);
}

const Container::Array<ShaderResource>
Horizon::Backend::PipelineLayout::GetResource(const ShaderResourceType type, VkShaderStageFlagBits stage) const {
    Container::Array<ShaderResource> found_resources;

    for (auto &it : shader_resources) {
        auto &shader_resource = it.second;
        if (shader_resource.type == type || type == ShaderResourceType::All) {
            if (shader_resource.stages == stage || stage == VK_SHADER_STAGE_ALL) {
                found_resources.push_back(shader_resource);
            }
        }
    }

    return found_resources;
}

void Horizon::Backend::PipelineLayout::CreateLayout(const Container::Array<VulkanShader *> shader_modules) {

    // gather shader resources
    for (auto *shader_module : shader_modules) {
        for (const auto &shader_resource : shader_module->GetResources()) {
            Container::String key = shader_resource.name;
            // Since 'Input' and 'Output' resources can have the same name, we modify the key string
            if (shader_resource.type == ShaderResourceType::Input ||
                shader_resource.type == ShaderResourceType::Output) {
                key = Container::String{std::to_string(shader_resource.stages)} + "_" + key;
            }

            auto it = shader_resources.find(key);

            if (it != shader_resources.end()) {
                // Append stage flags if resource already exists
                it->second.stages |= shader_resource.stages;
            } else {
                // Create a new entry in the map
                shader_resources.emplace(key, shader_resource);
            }
        }
    }

    // Sift through the map of name indexed shader resources
    // Separate them into their respective sets
    for (auto &it : shader_resources) {
        auto &shader_resource = it.second;

        // Find binding by set index in the map.
        auto it2 = shader_sets.find(shader_resource.set);

        if (it2 != shader_sets.end()) {
            // Add resource to the found set index
            it2->second.emplace(shader_resource.name, shader_resource);
        } else {
            // Create a new set index and with the first resource
            Container::HashMap<Container::String, ShaderResource> map;
            map.emplace(shader_resource.name, shader_resource);
            shader_sets.emplace(shader_resource.set, std::move(map));
        }
    }

    // Create a descriptor set layout for each shader set in the shader modules
    for (auto &shader_set_it : shader_sets) {
        DescriptorSetLayout *layout =
            Memory::Alloc<DescriptorSetLayout>(m_context, shader_set_it.first, shader_modules, shader_set_it.second);
        descriptor_set_layouts.emplace_back(layout);
    }

    // Collect all the descriptor set layout handles, maintaining set order
    Container::Array<VkDescriptorSetLayout> descriptor_set_layout_handles;
    for (uint32_t i = 0; i < descriptor_set_layouts.size(); ++i) {
        if (descriptor_set_layouts[i]) {
            descriptor_set_layout_handles.push_back(descriptor_set_layouts[i]->get());
        } else {
            descriptor_set_layout_handles.push_back(VK_NULL_HANDLE);
        }
    }

    // Collect all the push constant shader resources
    Container::Array<VkPushConstantRange> push_constant_ranges;
    for (auto &push_constant_resource : GetResource(ShaderResourceType::PushConstant)) {
        push_constant_ranges.push_back(
            {push_constant_resource.stages, push_constant_resource.offset, push_constant_resource.size});
    }

    VkPipelineLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    create_info.setLayoutCount = static_cast<u32>(descriptor_set_layout_handles.size());
    create_info.pSetLayouts = descriptor_set_layout_handles.data();
    create_info.pushConstantRangeCount = static_cast<u32>(push_constant_ranges.size());
    create_info.pPushConstantRanges = push_constant_ranges.data();

    // Create the Vulkan pipeline layout handle
    CHECK_VK_RESULT(vkCreatePipelineLayout(m_context.device, &create_info, nullptr, &m_pipeline_layout));
}

} // namespace Horizon::Backend