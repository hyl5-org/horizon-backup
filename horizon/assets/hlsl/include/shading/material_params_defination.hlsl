#define HAS_BASE_COLOR 0x01
#define HAS_NORMAL 0x10
#define HAS_METALLIC_ROUGHNESS 0x100
#define HAS_EMISSIVE 0x1000
#define HAS_ALPHA 0x10000

struct MaterialProperties {
    float3 albedo;
    float3 normal;
    float3 f0;
    float metallic;  // 0
    float roughness; // 0.5
    float roughness2;
    float3 emissive;

    // disney

    float anisotropic;     // 0
    float sheen;           // 0
    float sheen_tint;      // 0.5
    float subsurface;      // 0
    float clearcoat;       // 0
    float clearcoat_gloss; // 0.5
    uint material_id;
};

struct MaterialDescription {
    uint base_color_texture_index;
    uint normal_texture_index;
    uint metallic_roughness_texture_index;
    uint emissive_textue_index;
    uint alpha_mask_texture_index;
    uint subsurface_scattering_texture_index;
    uint param_bitmask;
    uint blend_state;
    float3 base_color;
    float pad1;
    float3 emissive;
    float pad2;
    float2 metallic_roughness;
    float2 pad3;
};

#define BLEND_STATE_OPAQUE 0
#define BLEND_STATE_MASKED 1