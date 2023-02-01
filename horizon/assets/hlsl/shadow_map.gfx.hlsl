// #include "include/common/descriptor.hlsl"
// #ifdef VULKAN
// #extension GL_EXT_nonuniform_qualifier : enable // how to hide this?
// #endif

// #include "include/common/bindless.hlsl"
// #include "include/postprocess/postprocess.hlsl"
// #include "include/shading/ibl.hlsl"
// #include "include/shading/light_defination.hlsl"
// #include "include/shading/lighting.hlsl"
// #include "include/shading/material_params_defination.hlsl"


// RES(Texture2D<float4>, material_textures[], UPDATE_FREQ_BINDLESS);
// RES(StructuredBuffer(MaterialDescription), material_descriptions[], UPDATE_FREQ_PER_FRAME);

// RES(SamplerState, default_sampler, UPDATE_FREQ_PER_FRAME);

// // PUSH_CONSTANT(ShadingModeID, b0)
// // {
// //     DATA(uint, shading_model_id;
// // }

// // per frame resources


// // PUSH_CONSTANT(ShadowMapDrawRootConstant, b1){
// //     DATA(uint, light_idx;
// //     DATA(uint, mesh_id_offset;
// // };

// struct InstanceParameter
// {
//     float4x4  model_matrix;
//     float4x4  decal_to_world;
//     float4x4  world_to_decal;
//     uint material_id;
// };

// RES(StructuredBuffer(InstanceParameter), instance_parameter, UPDATE_FREQ_PER_FRAME);

// STRUCT(ShadowMapData){
//     DATA(float4x4, shadow_map_view_projection;
// };

// RES(StructuredBuffer(ShadowMapData), shadow_map_data, UPDATE_FREQ_PER_FRAME);

// struct VSInput
// {
// 	float3 position: SV_Position;
// 	float3 normal: NORMAL;
// 	float2 uv0: TEXCOORD0;
// 	float2 uv1: TEXCOORD1;
// 	float3 tangent: TANGENT;
// };

// // // 52bytes vertex layout

// // STRUCT(PackedVsInput)
// // {
// // 	DATA(float, packed[VERETX_LAYOUT_STRIDE];
// // };

// // RES(StructuredBuffer(PackedVsInput), vertex_buffers[], UPDATE_FREQ_BINDLESS);

// struct VSOutput
// {
// 	float4 position: SV_Position;
//     float3 world_pos: POSITION;
// 	float3 normal: NORMAL;
// 	float2 uv: TEXCOORD0;
// 	float3 tangent: TANGENT;
//     nointerpolation uint instance_id;
// #ifdef VULKAN
//     nointerpolation uint material_id;
// #endif
//     float4 curr_pos;
//     float4 prev_pos;
// };


// struct PSOutput 
// {
//     float4 gbuffer0: SV_Target0;
//     float4 gbuffer1: SV_Target1;
//     float4 gbuffer2: SV_Target2;
//     float4 gbuffer3: SV_Target3;
//     float2 gbuffer4: SV_Target4;
//     //uint2 vbuffer0, SV_Target0;
// };


// VSOutput VS_MAIN( VSInput vsin, uint instance_id : SV_InstanceID, uint vertex_id: SV_VertexID)
// {
    
//     VSOutput vsout;
//     uint mesh_id;
// #ifdef VULKAN
//     mesh_id = gl_DrawID;
// #endif
//     mesh_id += (mesh_id_offset);
//     float4x4 model = (instance_parameter)[mesh_id].model_matrix;

//     // // float vbp[VERETX_LAYOUT_STRIDE] = (vertex_buffers)[0][mesh_id + vertex_id].packed;
//     // // vsin.position = GetVertexPositionFromPackedVertexBuffer(vbp);
//     // // vsin.normal = GetVertexNormalFromPackedVertexBuffer(vbp);
//     // // vsin.uv0 = GetVertexUv0FromPackedVertexBuffer(vbp);
//     // // vsin.uv1 = GetVertexUv1FromPackedVertexBuffer(vbp);
//     // // vsin.tangent = GetVertexTangentFromPackedVertexBuffer(vbp);
    
//     // vsout.position = vp * model * float4(vsin.position, 1.0);
//     // vsout.world_pos = (model * float4(vsin.position, 1.0)).xyz;
//     // //transpose(inverse(model)
//     // vsout.normal = normalize((model * float4(vsin.normal, 0.0)).xyz);
//     // vsout.uv = vsin.uv0;
//     // vsout.tangent = normalize((model * float4(vsin.tangent, 0.0)).xyz);
//     // vsout.instance_id = InstanceID;
//     // vsout.material_id = (instance_parameter)[mesh_id].material_id;
//     // vsout.prev_pos = prev_vp * model * float4(vsin.position, 1.0); // old
//     // vsout.curr_pos = vp * model * float4(vsin.position, 1.0); // new
//     RETURN(vsout);
// }


// PSOutput PS_MAIN(VSOutput vsout, SV_PrimitiveID(uint) tri_id) 
// {
    
//     PSOutput psout;

//     uint material_id = vsout.material_id;

//     MaterialDescription material = (material_descriptions)[0][material_id];
//     uint param_bitmask = material.param_bitmask;

//     uint has_metallic_roughness = param_bitmask & HAS_METALLIC_ROUGHNESS;
//     uint has_normal = param_bitmask & HAS_NORMAL;
//     uint has_base_color = param_bitmask & HAS_BASE_COLOR;
//     uint has_emissive = param_bitmask & HAS_EMISSIVE;

//     float3 albedo =
//         has_base_color != 0
//             ? pow(SampleTex2D((material_textures)[material.base_color_texture_index], default_sampler, vsout.uv).xyz,
//                   float3(2.2))
//             : material.base_color;
//     float alpha = 1.0;
//     // uniform branching
//     if (material.blend_state == BLEND_STATE_MASKED) {
//         alpha =
//             SampleLvlTex2D((material_textures)[material.base_color_texture_index], default_sampler, vsout.uv, 0).w;
//         if (alpha<0.5) {
//             discard;
//         }
//     }

//     float3 normal_map =
//         has_normal != 0
//             ? SampleTex2D((material_textures)[material.normal_texture_index], default_sampler, vsout.uv).xyz
//             : float3(0.0, 0.0, 0.0); // normal map
//     float2 mr =
//         has_metallic_roughness != 0
//             ? SampleTex2D((material_textures)[material.metallic_roughness_texture_index], default_sampler, vsout.uv)
//                   .yz
//             : material.metallic_roughness;
//     float3 emissive =
//         has_emissive != 0
//             ? pow(SampleTex2D((material_textures)[material.emissive_textue_index], default_sampler, vsout.uv).xyz,
//                   float3(2.2))
//             : material.emissive;

//     normal_map = normalize(2.0 * normal_map - 1.0); // [-1, 1]

//     float3 gbuffer_normal;

//     if (has_normal != 0) {
//         // construct TBN
//         float3 normal = normalize(vsout.normal);
//         float3 tangent = normalize(vsout.tangent);
//         float3 bitangent = normalize(cross(tangent, normal));
//         // Calculate pixel normal using the normal map and the tangent space vectors
//         float3x3 tbn = make_f3x3_cols(tangent, bitangent, normal);
//         gbuffer_normal = normalize(tbn * normal_map);
//     } else {
//         gbuffer_normal = normalize(vsout.normal);
//     }
//     psout.gbuffer0 = float4(gbuffer_normal.x, gbuffer_normal.y, gbuffer_normal.z, asfloat(0));
//     psout.gbuffer1 = float4(albedo.x, albedo.y, albedo.z, 0.0);
//     psout.gbuffer2 = float4(emissive.x, emissive.y, emissive.z, 0.0);
//     psout.gbuffer3 = float4(mr.y, mr.x, alpha, 0.0);

//     // Temporal Antialiasing In Uncharted 4, Ke XU
//     float4 prev_pos = vsout.prev_pos;
//     prev_pos.xyz /= prev_pos.w; // [-1, 1]
//     prev_pos.xy -= (taa_prev_curr_offset).xy;
//     float4 curr_pos = vsout.curr_pos;
//     curr_pos.xyz /= curr_pos.w;
//     curr_pos.xy -= (taa_prev_curr_offset).zw; 
//     psout.gbuffer4 = float2(curr_pos.xy - prev_pos.xy) * 0.5;// motionvector
//     //psout.vbuffer0 = uint2(vsout.instance_id, tri_id);
    
//     RETURN(psout);
// }
