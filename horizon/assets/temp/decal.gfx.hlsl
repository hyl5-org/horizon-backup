#ifdef VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif
#include "include/common/descriptor.hlsl"
#include "include/common/bindless.hlsl"
#include "include/postprocess/postprocess.hlsl"
#include "include/shading/ibl.hlsl"
#include "include/shading/light_defination.hlsl"
#include "include/shading/lighting.hlsl"
#include "include/shading/material_params_defination.hlsl"
#include "include/translation/translation.hlsl"
#include "resource.hlsl"
// per material resources TODO: register for dx12



RES(Texture2D<float4>, decal_material_textures[], UPDATE_FREQ_BINDLESS);

struct DecalInstanceParameter
{
    float4x4  model;
    float4x4  decal_to_world;
    float4x4  world_to_decal;
    uint material_id;
};

RES(StructuredBuffer<DecalInstanceParameter>, decal_instance_parameter, UPDATE_FREQ_PER_FRAME);

RES(StructuredBuffer<MaterialDescription>, decal_material_descriptions[], UPDATE_FREQ_PER_FRAME);

RES(SamplerState, default_sampler, UPDATE_FREQ_PER_FRAME);

RES(Texture2D<float>, depth_tex, UPDATE_FREQ_PER_FRAME);

DECLARE_SCENE_CONSTANT

// PUSH_CONSTANT(ShadingModeID, b0)
// {
//     DATA(uint, shading_model_id;
// }

// per frame resources

// PUSH_CONSTANT(DrawRootConstant, b1){
//     DATA(uint, mesh_id_offset;
// };

const uint mesh_id_offset; 

struct VSInput
{
	float3 position: SV_Position;
	float3 normal: NORMAL;
	float2 uv0: TEXCOORD0;
	float2 uv1: TEXCOORD1;
	float3 tangent: TANGENT;
};

struct VSOutput
{
	float4 position: SV_Position;
    float3 world_pos: POSITION;
	float3 normal: NORMAL;
	float2 uv: TEXCOORD0;
	float3 tangent: TANGENT;
    nointerpolation uint instance_id;
    nointerpolation uint material_id;
    float4 curr_pos;
    float4 prev_pos;
};

struct PSOutput 
{
    float4 gbuffer0: SV_Target0;
    float4 gbuffer1: SV_Target1;
    float4 gbuffer2: SV_Target2;
    float4 gbuffer3: SV_Target3;
    float2 gbuffer4: SV_Target4;
    //uint2 vbuffer0, SV_Target0;
};

VSOutput VS_MAIN( VSInput vsin, uint instance_id : SV_InstanceID, uint vertex_id: SV_VertexID)
{
    
    VSOutput vsout;
    uint mesh_id;
    mesh_id = gl_DrawID;
    mesh_id += (mesh_id_offset);
    float4x4 decal_to_world = decal_instance_parameter[instance_id].decal_to_world;
    float4x4 model = decal_instance_parameter[instance_id].model;
    float4 pos = mul(decal_to_world, float4(vsin.position.xyz, 1.0));
    vsout.position = mul(camera_view_projection, mul(model, pos));
    vsout.instance_id = instance_id;
    vsout.material_id = decal_instance_parameter[instance_id].material_id;
    return vsout;
}

PSOutput PS_MAIN(VSOutput vsout, uint tri_id : SV_PrimitiveID) 
{
    
    PSOutput psout;

    // float2 uv = vsout.position.xy / float2((resolution.xy));
    // float3 world_pos = ReconstructWorldPos((camera_inverse_view_projection), SampleTex2D((depth_tex), default_sampler, uv).r, uv);

    // float4x4 world_to_decal = (decal_instance_parameter)[vsout.instance_id].world_to_decal;
    // float4 decal_pos = world_to_decal * float4(world_pos, 1.0);

    // decal_pos.xyz /= decal_pos.w;

    // if (decal_pos.x < -1.0 || decal_pos.x > 1.0 || decal_pos.y < -1.0 || decal_pos.y > 1.0)
    //     discard;

    // //float2 decal_tex_coord = ;
    // //decal_tex_coord.x    = 1.0 - decal_tex_coord.x;

    // //float2 decal_tc = decal_pos * mvp /w .xy;
    // float2 decal_tc = decal_pos.xy * 0.5 + 0.5;
    // uint material_id = vsout.material_id;

    // MaterialDescription material = (decal_material_descriptions)[0][material_id];
    // uint param_bitmask = material.param_bitmask;

    // // uint has_metallic_roughness = param_bitmask & HAS_METALLIC_ROUGHNESS_TEX;
    // // uint has_normal = param_bitmask & HAS_NORMAL_TEX;
    // // uint has_base_color = param_bitmask & HAS_BASE_COLOR_TEX;
    // // uint has_emissive = param_bitmask & HAS_EMISSIVE_TEX;

    // float3 albedo =
    //     pow(SampleTex2D((decal_material_textures)[material.base_color_texture_index], default_sampler, decal_tc).xyz,
    //         float3(2.2));
    // float alpha = 1.0;
    
    // // // uniform branching
    // // if (material.blend_state == BLEND_STATE_MASKED) {
    //     alpha =
    //         SampleLvlTex2D((decal_material_textures)[material.base_color_texture_index], default_sampler, decal_tc, 0).w;
    //     if (alpha<0.5) {
    //         discard;
    //     }
    // // }

    // // float3 normal_map =
    // //     has_normal != 0
    // //         ? SampleTex2D((material_textures)[material.normal_texture_index], default_sampler, vsout.uv).xyz
    // //         : float3(0.0, 0.0, 0.0); // normal map
    // // float2 mr =
    // //     has_metallic_roughness != 0
    // //         ? SampleTex2D((material_textures)[material.metallic_roughness_texture_index], default_sampler, vsout.uv)
    // //               .yz
    // //         : material.metallic_roughness;
    // // float3 emissive =
    // //     has_emissive != 0
    // //         ? pow(SampleTex2D((material_textures)[material.emissive_textue_index], default_sampler, vsout.uv).xyz,
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

    return psout;
}
