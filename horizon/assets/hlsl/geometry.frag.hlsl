#include "include/common/bindless.hlsl"
#include "include/postprocess/postprocess.hlsl"
#include "include/shading/ibl.hlsl"
#include "include/shading/light_defination.hlsl"
#include "include/shading/lighting.hlsl"
#include "include/shading/material_params_defination.hlsl"

#ifdef VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

// per material resources TODO: register for dx12

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

RES(Texture2D(float4), material_textures[], UPDATE_FREQ_BINDLESS);
RES(Buffer(MaterialDescription), material_descriptions[], UPDATE_FREQ_PER_FRAME);

RES(SamplerState, default_sampler, UPDATE_FREQ_PER_FRAME);

CBUFFER(TAAOffsets, UPDATE_FREQ_PER_FRAME)
{
    DATA(float4, taa_prev_curr_offset, None);
};

// PUSH_CONSTANT(ShadingModeID, b0)
// {
//     DATA(uint, shading_model_id, None);
// }

// per frame resources

STRUCT(VSOutput) 
{
    DATA(float4, position, SV_Position);
    DATA(float3, world_pos, POSITION);
    DATA(float3, normal, NORMAL);
    DATA(float2, uv, TEXCOORD0);
    DATA(float3, tangent, TANGENT);
    DATA(FLAT(uint), instance_id, None);
#ifdef VULKAN
    DATA(FLAT(uint), material_id, None);
#endif
    DATA(float4, prev_pos, None);
    DATA(float4, curr_pos, None);
};

STRUCT(PSOutput) {
    DATA(float4, gbuffer0, SV_Target0);
    DATA(float4, gbuffer1, SV_Target0);
    DATA(float4, gbuffer2, SV_Target0);
    DATA(float4, gbuffer3, SV_Target0);
    DATA(float2, gbuffer4, SV_Target0);
    //DATA(uint2, vbuffer0, SV_Target0);
};

PSOutput PS_MAIN(VSOutput vsout, SV_PrimitiveID(uint) tri_id) 
{
    
    PSOutput psout;

    uint material_id = vsout.material_id;

    MaterialDescription material = Get(material_descriptions)[0][material_id];
    uint param_bitmask = material.param_bitmask;

    uint has_metallic_roughness = param_bitmask & HAS_METALLIC_ROUGHNESS;
    uint has_normal = param_bitmask & HAS_NORMAL;
    uint has_base_color = param_bitmask & HAS_BASE_COLOR;
    uint has_emissive = param_bitmask & HAS_EMISSIVE;

    float3 albedo =
        has_base_color != 0
            ? pow(SampleTex2D(Get(material_textures)[material.base_color_texture_index], default_sampler, vsout.uv).xyz,
                  float3(2.2))
            : material.base_color;
    float alpha = 1.0;
    // uniform branching
    if (material.blend_state == BLEND_STATE_MASKED) {
        alpha =
            SampleLvlTex2D(Get(material_textures)[material.base_color_texture_index], default_sampler, vsout.uv, 0).w;
        if (alpha<0.5) {
            discard;
        }
    }

    float3 normal_map =
        has_normal != 0
            ? SampleTex2D(Get(material_textures)[material.normal_texture_index], default_sampler, vsout.uv).xyz
            : float3(0.0, 0.0, 0.0); // normal map
    float2 mr =
        has_metallic_roughness != 0
            ? SampleTex2D(Get(material_textures)[material.metallic_roughness_texture_index], default_sampler, vsout.uv)
                  .yz
            : material.metallic_roughness;
    float3 emissive =
        has_emissive != 0
            ? pow(SampleTex2D(Get(material_textures)[material.emissive_textue_index], default_sampler, vsout.uv).xyz,
                  float3(2.2))
            : material.emissive;

    normal_map = normalize(2.0 * normal_map - 1.0); // [-1, 1]

    float3 gbuffer_normal;

    if (has_normal != 0) {
        // construct TBN
        float3 normal = normalize(vsout.normal);
        float3 tangent = normalize(vsout.tangent);
        float3 bitangent = normalize(cross(tangent, normal));
        // Calculate pixel normal using the normal map and the tangent space vectors
        float3x3 tbn = make_f3x3_cols(tangent, bitangent, normal);
        gbuffer_normal = normalize(tbn * normal_map);
    } else {
        gbuffer_normal = normalize(vsout.normal);
    }
    psout.gbuffer0 = float4(gbuffer_normal.x, gbuffer_normal.y, gbuffer_normal.z, asfloat(0));
    psout.gbuffer1 = float4(albedo.x, albedo.y, albedo.z, 0.0);
    psout.gbuffer2 = float4(emissive.x, emissive.y, emissive.z, 0.0);
    psout.gbuffer3 = float4(mr.y, mr.x, alpha, 0.0);

    // Temporal Antialiasing In Uncharted 4, Ke XU
    float4 prev_pos = vsout.prev_pos;
    prev_pos.xyz /= prev_pos.w; // [-1, 1]
    prev_pos.xy -= Get(taa_prev_curr_offset).xy;
    float4 curr_pos = vsout.curr_pos;
    curr_pos.xyz /= curr_pos.w;
    curr_pos.xy -= Get(taa_prev_curr_offset).zw; 
    psout.gbuffer4 = float2(curr_pos.xy - prev_pos.xy) * 0.5;// motionvector
    //psout.vbuffer0 = uint2(vsout.instance_id, tri_id);
    
    RETURN(psout);
}
