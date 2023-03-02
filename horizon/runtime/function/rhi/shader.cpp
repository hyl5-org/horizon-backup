#include "shader.h"

#include <algorithm>
#include <utility>

namespace Horizon::Backend {

Shader::Shader(ShaderType type) noexcept : m_type(type) {}

ShaderType Shader::GetType() const noexcept { return m_type; }

} // namespace Horizon::Backend