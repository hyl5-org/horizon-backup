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
    SPIRVReflection spirv_reflection;
    std::vector<u32> arr(spirv_code_size / sizeof(u32));
    memcpy(arr.data(), p_spirv_code, spirv_code_size);
    spirv_reflection.reflect_shader_resources(ToVkShaderStageBit(type), static_cast<u32 *>(p_spirv_code),
                                              spirv_code_size / sizeof(u32), resources);
}

VulkanShader::~VulkanShader() noexcept { vkDestroyShaderModule(m_context.device, m_shader_module, nullptr); }

} // namespace Horizon::Backend
