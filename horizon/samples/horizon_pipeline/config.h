#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>
#include <random>
#include <shared_mutex>

#include <runtime/core/log/log.h>
#include <runtime/core/math/hmath.h>
#include <runtime/core/units/Units.h>
#include <runtime/core/utils/definations.h>
#include <runtime/core/utils/functions.h>
#include <runtime/core/renderdoc/RenderDoc.h>
#include <runtime/core/window/Window.h>

#include <runtime/function/rhi/RHI.h>
#include <runtime/function/rhi/rhi_utils.h>
#include <runtime/function/resource/resources/mesh/Mesh.h>
#include <runtime/function/resource/resource_loader/mesh/mesh_loader.h>
#include <runtime/function/scene/light/Light.h>

#include <runtime/interface/horizon_runtime.h>

#include <runtime/system/render/RenderSystem.h>

using namespace Horizon;
using namespace Horizon::Backend;

static constexpr u32 _width = 3200, _height = 1800;

static std::filesystem::path asset_path = "C:/FILES/horizon/horizon/assets";

// TODO generate the enum in compile time
enum class ShaderList {
    COMPACT_INDEX_BUFFER_CS,
    GEOMETRY_VS,
    GEOMETRY_PS,
    GPU_MESH_CULLING_CS,
    GPU_TRIANGLE_CULLING_CS,
    POST_PROCESS_CS,
    SSAO_CS,
    SSAO_BLUR_CS,
    TAA_CS,
    SHADING_CS
};
