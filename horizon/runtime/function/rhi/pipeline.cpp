#include "pipeline.h"

#include <algorithm>

#include <nlohmann/json.hpp>

#include "runtime/core/utils/functions.h"
#include "runtime/core/io/file_system.h"

namespace Horizon::Backend {

Pipeline::Pipeline() noexcept {}

Pipeline::~Pipeline() noexcept {}

PipelineType Pipeline::GetType() const noexcept { return m_create_info.type; }

} // namespace Horizon::Backend
