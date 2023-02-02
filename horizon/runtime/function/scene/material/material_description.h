/*****************************************************************//**
 * \file   material_description.h
 * \brief  
 * 
 * \author hylu
 * \date   January 2023
 *********************************************************************/

#pragma once

// standard libraries
#include <filesystem>

// third party libraries

// project headers
#include "runtime/function/rhi/buffer.h"
#include "runtime/function/rhi/texture.h"
#include "runtime/function/rhi/descriptor_set.h"
#include "runtime/function/rhi/rhi.h"
#include "runtime/function/resource/resource_loader/texture/texture_loader.h"

namespace Horizon {

enum MaterialParamFlags {
    HAS_BASE_COLOR_TEX = 0x01,
    HAS_NORMAL_TEX = 0x10,
    HAS_METALLIC_ROUGHNESS_TEX = 0x100,
    HAS_EMISSIVE_TEX = 0x1000,
    HAS_ALPHA_TEX = 0x10000
};

enum class BlendState { BLEND_STATE_OPAQUE, BLEND_STATE_MASKED, BLEND_STATE_TRANSPARENT };

// each correspond a drawcall
enum class ShadingModel { SHADING_MODEL_LIT, SHADING_MODEL_UNLIT, SHADING_MODEL_SUBSURFACE, SHADING_MODEL_TWO_SIDE, SHADING_MODEL_EYE, SHADING_MODEL_CLOTH };

enum class MaterialTextureType { BASE_COLOR, NORMAL, METALLIC_ROUGHTNESS, EMISSIVE, ALPHA_MASK };

class MaterialTextureDescription {
  public:
    MaterialTextureDescription() noexcept = default;
    MaterialTextureDescription(const std::filesystem::path& url) noexcept : url(url) {};

    ~MaterialTextureDescription() noexcept {}
    std::filesystem::path url{};
    TextureDataDesc texture_data_desc{};
};

struct MaterialParams {
    math::Vector3f base_color_factor;
    f32 roughness_factor;
    math::Vector3f emmissive_factor;
    f32 metallic_factor;
    u32 param_bitmask;
    u32 shading_model_id, two_side, pad;
};

class Material {
  public:
    Material(std::pmr::polymorphic_allocator<std::byte> allocator = {}) noexcept : material_textures(allocator){};
    ~Material() noexcept = default;

    Material(const Material &rhs) = delete;
    Material &operator=(const Material &rhs) noexcept = delete;
    Material(Material &&rhs) noexcept = delete;
    Material &operator=(Material &&rhs) noexcept = delete;

    ShadingModel GetShadingModelID() noexcept { return shading_model; }

  public:
    Container::HashMap<MaterialTextureType, MaterialTextureDescription> material_textures{};
    MaterialParams material_params{};
    ShadingModel shading_model{ShadingModel::SHADING_MODEL_LIT};
    BlendState blend_state{BlendState::BLEND_STATE_OPAQUE};
    //  Material* materials
};
} // namespace Horizon