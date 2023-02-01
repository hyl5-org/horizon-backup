#ifdef VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

#include "include/common/bindless.hlsl"
#include "include/postprocess/postprocess.hlsl"
#include "include/shading/ibl.hlsl"
#include "include/shading/light_defination.hlsl"
#include "include/shading/lighting.hlsl"
#include "include/shading/material_params_defination.hlsl"
#include "include/translation/translation.hlsl"
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

RES(Texture2D(float4), decal_material_textures[], UPDATE_FREQ_BINDLESS);

STRUCT(InstanceParameter)
{
    DATA(float4x4,  model, None);
    DATA(float4x4,  decal_to_world, None);
    DATA(float4x4,  world_to_decal, None);
    DATA(uint, material_id, None);
};

RES(Buffer(InstanceParameter), decal_instance_parameter, UPDATE_FREQ_PER_FRAME);

RES(Buffer(MaterialDescription), decal_material_descriptions[], UPDATE_FREQ_PER_FRAME);

RES(SamplerState, default_sampler, UPDATE_FREQ_PER_FRAME);

RES(Texture2D(float), depth_tex, UPDATE_FREQ_PER_FRAME);

CBUFFER(SceneConstants, UPDATE_FREQ_PER_FRAME)
{
    DATA(float4x4, camera_view, None);
    DATA(float4x4, camera_projection, None);
    DATA(float4x4, camera_view_projection, None);
    DATA(float4x4, camera_inverse_view_projection, None);
    DATA(uint2, resolution, None);
    DATA(uint2, pad_0, None);
    DATA(float3, camera_pos, None);
    DATA(uint, pad_1, None);
    DATA(float, ibl_intensity, None);
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

    // float2 uv = vsout.position.xy / float2(Get(resolution.xy));
    // float3 world_pos = ReconstructWorldPos(Get(camera_inverse_view_projection), SampleTex2D(Get(depth_tex), default_sampler, uv).r, uv);

    // float4x4 world_to_decal = Get(decal_instance_parameter)[vsout.instance_id].world_to_decal;
    // float4 decal_pos = world_to_decal * float4(world_pos, 1.0);

    // decal_pos.xyz /= decal_pos.w;

    // if (decal_pos.x < -1.0 || decal_pos.x > 1.0 || decal_pos.y < -1.0 || decal_pos.y > 1.0)
    //     discard;

    // //float2 decal_tex_coord = ;
    // //decal_tex_coord.x    = 1.0 - decal_tex_coord.x;

    // //float2 decal_tc = decal_pos * mvp /w .xy;
    // float2 decal_tc = decal_pos.xy * 0.5 + 0.5;
    // uint material_id = vsout.material_id;

    // MaterialDescription material = Get(decal_material_descriptions)[0][material_id];
    // uint param_bitmask = material.param_bitmask;

    // // uint has_metallic_roughness = param_bitmask & HAS_METALLIC_ROUGHNESS;
    // // uint has_normal = param_bitmask & HAS_NORMAL;
    // // uint has_base_color = param_bitmask & HAS_BASE_COLOR;
    // // uint has_emissive = param_bitmask & HAS_EMISSIVE;

    // float3 albedo =
    //     pow(SampleTex2D(Get(decal_material_textures)[material.base_color_texture_index], default_sampler, decal_tc).xyz,
    //         float3(2.2));
    // float alpha = 1.0;
    
    // // // uniform branching
    // // if (material.blend_state == BLEND_STATE_MASKED) {
    //     alpha =
    //         SampleLvlTex2D(Get(decal_material_textures)[material.base_color_texture_index], default_sampler, decal_tc, 0).w;
    //     if (alpha<0.5) {
    //         discard;
    //     }
    // // }

    // // float3 normal_map =
    // //     has_normal != 0
    // //         ? SampleTex2D(Get(material_textures)[material.normal_texture_index], default_sampler, vsout.uv).xyz
    // //         : float3(0.0, 0.0, 0.0); // normal map
    // // float2 mr =
    // //     has_metallic_roughness != 0
    // //         ? SampleTex2D(Get(material_textures)[material.metallic_roughness_texture_index], default_sampler, vsout.uv)
    // //               .yz
    // //         : material.metallic_roughness;
    // // float3 emissive =
    // //     has_emissive != 0
    // //         ? pow(SampleTex2D(Get(material_textures)[material.emissive_textue_index], default_sampler, vsout.uv).xyz,
    // //               float3(2.2))
    // //         : material.emissive;

    // // normal_map = (2.0 * normal_map - 1.0); // [-1, 1]

    // // float3 gbuffer_normal;

    // // if (has_normal != 0) {
    // //     // construct TBN
    // //     float3 normal = normalize(vsout.normal);
    // //     float3 tangent = normalize(vsout.tangent);
    // //     float3 bitangent = normalize(cross(tangent, normal));
    // //     // Calculate pixel normal using the normal map and the tangent space vectors
    // //     float3x3 tbn = make_f3x3_rows(tangent, bitangent, normal);
    // //     gbuffer_normal = normalize(mul(normal_map, tbn));
    // // } else {
    // //     gbuffer_normal = normalize(vsout.normal);
    // // }
    // // psout.gbuffer0 = float4(gbuffer_normal.x, gbuffer_normal.y, gbuffer_normal.z, asfloat(0));
    // // psout.gbuffer1 = float4(albedo.x, albedo.y, albedo.z, 0.0);
    // // psout.gbuffer2 = float4(emissive.x, emissive.y, emissive.z, 0.0);
    // // psout.gbuffer3 = float4(mr.y, mr.x, alpha, 0.0);


    // //psout.gbuffer0.xyz = normalize(vsout.normal); // normal
    // psout.gbuffer1.xyz = albedo; // albedo

    RETURN(psout);
}
