#include "vulkan_descriptor_set_layout.h"

#include <algorithm>

namespace Horizon::Backend {

Horizon::Backend::DescriptorSetLayout::DescriptorSetLayout(
    const VulkanRendererContext &t_context, u32 t_set_index, const Container::Array<VulkanShader *> &shaders,
    const Container::HashMap<Container::String, ShaderResource> &shader_resources) noexcept
    : m_context(t_context), m_set_index(t_set_index) {
    // NOTE: `shader_modules` is passed in mainly for hashing their handles in `request_resource`.
    //        This way, different pipelines (with different shaders / shader variants) will get
    //        different descriptor set layouts (incl. appropriate name -> binding lookups)

    for (auto &[name, resource] : shader_resources) {
        // Skip shader resources whitout a binding point
        if (resource.type == ShaderResourceType::Input || resource.type == ShaderResourceType::Output ||
            resource.type == ShaderResourceType::PushConstant ||
            resource.type == ShaderResourceType::SpecializationConstant) {
            continue;
        }

        // Convert from ShaderResourceType to VkDescriptorType.
        auto descriptor_type = find_descriptor_type(resource.type, resource.mode == ShaderResourceMode::Dynamic);

        if (resource.mode == ShaderResourceMode::UpdateAfterBind) {
            binding_flags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT);
        } else {
            // When creating a descriptor set layout, if we give a structure to create_info.pNext, each binding needs to have a binding flag
            // (pBindings[i] uses the flags in pBindingFlags[i])
            // Adding 0 ensures the bindings that dont use any flags are mapped correctly.
            binding_flags.push_back(0);
        }

        // Convert ShaderResource to VkDescriptorSetLayoutBinding
        VkDescriptorSetLayoutBinding layout_binding{};

        layout_binding.binding = resource.binding;
        layout_binding.descriptorCount = resource.array_size;
        layout_binding.descriptorType = descriptor_type;
        layout_binding.stageFlags = static_cast<VkShaderStageFlags>(resource.stages);

        bindings.push_back(layout_binding);

        // Store mapping between binding and the binding point
        bindings_lookup.emplace(resource.binding, layout_binding);

        binding_flags_lookup.emplace(resource.binding, binding_flags.back());

        resources_lookup.emplace(resource.name, resource.binding);
    }

    VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    create_info.flags = 0;
    create_info.bindingCount = static_cast<u32>(bindings.size());
    create_info.pBindings = bindings.data();

    // Handle update-after-bind extensions
    if (std::find_if(
            shader_resources.begin(), shader_resources.end(),
            [](auto &shader_resource) {
            return shader_resource.second.mode == ShaderResourceMode::UpdateAfterBind;
        }) != shader_resources.end()) {
        // Spec states you can't have ANY dynamic resources if you have one of the bindings set to update-after-bind
        if (std::find_if(
                shader_resources.begin(), shader_resources.end(),
                [](auto &shader_resource) {
                return shader_resource.second.mode == ShaderResourceMode::Dynamic;
            }) != shader_resources.end()) {
            LOG_ERROR("Cannot create descriptor set layout, dynamic resources are not allowed if at "
                                     "least one resource is update-after-bind.");
            return;
        }

        if (!validate_flags(bindings, binding_flags)) {
            LOG_ERROR("Invalid binding, couldn't create descriptor set layout.");
            return;
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags_create_info{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT};
        binding_flags_create_info.bindingCount = static_cast<u32>(binding_flags.size());
        binding_flags_create_info.pBindingFlags = binding_flags.data();

        create_info.pNext = &binding_flags_create_info;
        create_info.flags |= std::find(binding_flags.begin(), binding_flags.end(),
                                       VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT) != binding_flags.end()
                                 ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT
                                 : 0;
    }

    // Create the Vulkan descriptor set layout handle
    CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_context.device, &create_info, nullptr, &m_layout));
}

DescriptorSetLayout::~DescriptorSetLayout() noexcept {}

VkDescriptorType DescriptorSetLayout::find_descriptor_type(ShaderResourceType resource_type, bool dynamic) {
    switch (resource_type) {
    case ShaderResourceType::InputAttachment:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        break;
    case ShaderResourceType::Image:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        break;
    case ShaderResourceType::ImageSampler:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
    case ShaderResourceType::ImageStorage:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;
    case ShaderResourceType::Sampler:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
    case ShaderResourceType::BufferUniform:
        if (dynamic) {
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        } else {
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        break;
    case ShaderResourceType::BufferStorage:
        if (dynamic) {
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        } else {
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }
        break;
    default:
        throw std::runtime_error("No conversion possible for the shader resource type.");
        break;
    }
}

bool DescriptorSetLayout::validate_binding(const VkDescriptorSetLayoutBinding &binding,
                                           const Container::Array<VkDescriptorType> &blacklist) {
    return !(std::find_if(blacklist.begin(), blacklist.end(), [binding](const VkDescriptorType &type) {
                 return type == binding.descriptorType;
             }) != blacklist.end());
}

bool DescriptorSetLayout::validate_flags(const Container::Array<VkDescriptorSetLayoutBinding> &bindings,
                                         const Container::Array<VkDescriptorBindingFlagsEXT> &flags) {
    // Assume bindings are valid if there are no flags
    if (flags.empty()) {
        return true;
    }

    // Binding count has to equal flag count as its a 1:1 mapping
    if (bindings.size() != flags.size()) {
        LOG_ERROR("Binding count has to be equal to flag count.");
        return false;
    }

    return true;
}

} // namespace Horizon::Backend
