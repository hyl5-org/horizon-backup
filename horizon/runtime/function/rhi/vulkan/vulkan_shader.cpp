/*****************************************************************/ /**
 * \file   vulkan_shader.cpp
 * \brief 
 * 
 * \author hylu
 * \date   January 2023
 *********************************************************************/

#include "vulkan_shader.h"

#include <filesystem>

//#ifndef SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
//#define SPIRV_REFLECT_USE_SYSTEM_SPIRV_H
//#include <spirv_reflect.h>
//#endif // !SPIRV_REFLECT_USE_SYSTEM_SPIRV_H

#include "runtime/function/rhi/shader_compiler.h"
#include "runtime/function/rhi/vulkan/spirv_reflection.h"

namespace Horizon::Backend {

VulkanShader::VulkanShader(const VulkanRendererContext &context, ShaderType type,
                           const Container::Array<u8> &shader_binary_code) noexcept
    : Shader(type), m_context(context) {
    shader_binary_code;
    ShaderBinaryHeader header{};
    std::memcpy(&header, shader_binary_code.data(), sizeof(ShaderBinaryHeader));
    if (header.header != hsb_header || header.shader_blob_offset != sizeof(ShaderBinaryHeader) ||
        header.shader_blob_size == 0) {
        LOG_ERROR("corrupted hsb file");
        return;
    }
    void *p_spirv_code = const_cast<u8 *>(shader_binary_code.data()) + header.shader_blob_offset;
    u32 spirv_code_size = header.shader_blob_size;
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.pCode = static_cast<u32 *>(p_spirv_code);
    shader_module_create_info.codeSize = spirv_code_size;
    CHECK_VK_RESULT(vkCreateShaderModule(m_context.device, &shader_module_create_info, nullptr, &m_shader_module));

    // reflect descriptor from shader
    {
        SPIRVReflection spirv_reflection;
        std::vector<u32> arr(spirv_code_size/4);
        memcpy(arr.data(), p_spirv_code, spirv_code_size);

        spirv_reflection.reflect_shader_resources(ToVkShaderStageBit(type), std::move(arr), resources);
        //spirv_reflection.reflect_shader_resources(ToVkShaderStageBit(type), arr, resources);

        //SpvReflectShaderModule module;
        //if (spvReflectCreateShaderModule(spirv_code_size, p_spirv_code, &module) != SPV_REFLECT_RESULT_SUCCESS) {
        //    LOG_ERROR("failed to reflect spirv code");
        //    return;
        //}

        //{
        //    u32 count = 0;
        //    SpvReflectResult result = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
        //    Container::Array<SpvReflectDescriptorSet *> reflected_sets(count);
        //    result = spvReflectEnumerateDescriptorSets(&module, &count, reflected_sets.data());
        //    assert(result == SPV_REFLECT_RESULT_SUCCESS);

        //    for (const auto &reflected_set : reflected_sets) {
        //        for (u32 binding; binding < reflected_set->binding_count; binding++) {
        //            const auto &reflected_binding = *reflected_set->bindings[binding];
        //            descriptor_bindings[reflected_set->set].emplace(reflected_binding.name,
        //                                                            VkDescriptorSetLayoutBinding{});
        //            auto &layout_binding_info = descriptor_bindings[reflected_set->set][reflected_binding.name];
        //            layout_binding_info.binding = reflected_binding.binding;
        //            layout_binding_info.descriptorType =
        //                static_cast<VkDescriptorType>(reflected_binding.descriptor_type);
        //            layout_binding_info.descriptorCount = 1;
        //            for (uint32_t i_dim = 0; i_dim < reflected_binding.array.dims_count; ++i_dim) {
        //                layout_binding_info.descriptorCount *= reflected_binding.array.dims[i_dim];
        //            }
        //            layout_binding_info.stageFlags |= static_cast<VkShaderStageFlagBits>(module.shader_stage);
        //        }
        //    }
        //}

        //spvReflectDestroyShaderModule(&module);
    }
}

VulkanShader::~VulkanShader() noexcept { vkDestroyShaderModule(m_context.device, m_shader_module, nullptr); }

} // namespace Horizon::Backend
