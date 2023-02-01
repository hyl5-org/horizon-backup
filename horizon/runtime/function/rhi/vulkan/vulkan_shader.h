#pragma once

#include "runtime/function/rhi/rhi_utils.h"
#include "runtime/function/rhi/shader.h"
#include "runtime/function/rhi/vulkan/vulkan_utils.h"

#include <d3d12shader.h>
#include <dxc/dxcapi.h>

namespace Horizon::Backend {
class VulkanShader : public Shader {
  public:
    VulkanShader(const VulkanRendererContext &context, ShaderType type,
                        Container::Array<u8>& spirv_code, const std::filesystem::path& rsd_path) noexcept;
    virtual ~VulkanShader() noexcept;
    VulkanShader(const VulkanShader &rhs) noexcept = delete;
    VulkanShader &operator=(const VulkanShader &rhs) noexcept = delete;
    VulkanShader(VulkanShader &&rhs) noexcept = delete;
    VulkanShader &operator=(VulkanShader &&rhs) noexcept = delete;

  public:
    const VulkanRendererContext &m_context{};
    VkShaderModule m_shader_module{};
};

} // namespace Horizon::Backend