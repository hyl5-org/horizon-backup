#pragma once


#include <filesystem>

#include "runtime/function/rhi/rhi_utils.h"

namespace Horizon::Backend {

class Shader {
  public:
    Shader(ShaderType type) noexcept;
    virtual ~Shader() noexcept = default;

    Shader(const Shader &rhs) noexcept = delete;
    Shader &operator=(const Shader &rhs) noexcept = delete;
    Shader(Shader &&rhs) noexcept = delete;
    Shader &operator=(Shader &&rhs) noexcept = delete;

    ShaderType GetType() const noexcept;

  protected:
    const ShaderType m_type{};


};

} // namespace Horizon::Backend