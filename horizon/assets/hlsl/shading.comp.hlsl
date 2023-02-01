#include "include/common/descriptor.hlsl"
#include "include/shading/material_params_defination.hlsl"
#include "include/shading/light_defination.hlsl"
#include "include/shading/lighting.hlsl"
#include "include/shading/ibl.hlsl"
#include "include/postprocess/postprocess.hlsl"
#include "include/translation/translation.hlsl"
// per frame resources

RES(Texture2D<float4>, gbuffer0_tex, UPDATE_FREQ_PER_FRAME);
RES(Texture2D<float4>, gbuffer1_tex, UPDATE_FREQ_PER_FRAME);
RES(Texture2D<float4>, gbuffer2_tex, UPDATE_FREQ_PER_FRAME);
RES(Texture2D<float4>, gbuffer3_tex, UPDATE_FREQ_PER_FRAME);
RES(Texture2D<float4>, depth_tex, UPDATE_FREQ_PER_FRAME);

RES(SamplerState, default_sampler, UPDATE_FREQ_PER_FRAME);

CBUFFER(SceneConstants, UPDATE_FREQ_PER_FRAME)
{
    DATA(float4x4, camera_view, None);
    DATA(float4x4, camera_projection, None);
    DATA(float4x4, camera_view_projection, None);
    DATA(float4x4, camera_inverse_view_projection, None);
    DATA(uint2, resolution, None);
    DATA(uint2, pad0, None);
    DATA(float3, camera_pos, None);
    DATA(uint, pad1, None);
    DATA(float, ibl_intensity, None);
};

CBUFFER(LightCountUb, UPDATE_FREQ_PER_FRAME)
{
    DATA(uint, directional_light_count, None);
    DATA(uint, local_light_count, None);
};

CBUFFER(DirectionalLightDataUb, UPDATE_FREQ_PER_FRAME)
{
    DATA(LightParams, directional_light_data[MAX_DYNAMIC_LIGHT_COUNT], None);
};

CBUFFER(LocalLightDataUb, UPDATE_FREQ_PER_FRAME)
{
    DATA(LightParams, local_light_data[MAX_DYNAMIC_LIGHT_COUNT], None);
};

RES(RWTexture2D<float4>, out_color, UPDATE_FREQ_PER_FRAME);

RES(RWTexture2D<float4>, ao_tex, UPDATE_FREQ_PER_FRAME);

CBUFFER(DiffuseIrradianceSH3, UPDATE_FREQ_PER_FRAME)
{
    DATA(float3, sh[9], None);
};

RES(TexCube(float4), specular_map, UPDATE_FREQ_PER_FRAME);
RES(Texture2D(float4), specular_brdf_lut, UPDATE_FREQ_PER_FRAME);
RES(SamplerState, ibl_sampler, UPDATE_FREQ_PER_FRAME);

[numthreads(8, 8, 1)]
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID, SV_GroupID(uint3) groupID) 
{
    
    uint2 _resolution = Get(resolution).xy - uint2(1.0, 1.0);

    if (thread_id.x>_resolution.x || thread_id.y>_resolution.y) {
        
    }

    float2 uv = float2(thread_id.xy) / float2(_resolution);
    float4 gbuffer0 = SampleTex2D(Get(gbuffer0_tex), default_sampler, uv);
    float4 gbuffer1 = SampleTex2D(Get(gbuffer1_tex), default_sampler, uv);
    float4 gbuffer2 = SampleTex2D(Get(gbuffer2_tex), default_sampler, uv);
    float4 gbuffer3 = SampleTex2D(Get(gbuffer3_tex), default_sampler, uv);
    
    MaterialProperties mat;
    mat.material_id = groupID.z;
    //mat.normal
    mat.albedo = gbuffer1.xyz;

    mat.metallic = gbuffer3.x;

    // adjust roughness to reduce specular aliasing
    float roughness = max(0.045f, gbuffer3.y);

    float alpha = gbuffer3.z;

    mat.roughness = roughness;
    mat.roughness2 = Pow2(roughness);
    mat.f0 = lerp(float3(0.04, 0.04, 0.04), mat.albedo, mat.metallic);

    float3 world_pos = ReconstructWorldPos(Get(camera_inverse_view_projection),SampleTex2D(Get(depth_tex), default_sampler, uv).r, uv);
    float3 n =  normalize(gbuffer0.xyz);
    float3 v = -normalize(world_pos - Get(camera_pos).xyz);
    float NoV = saturate(dot(n, v));
    float4 radiance = float4(0.0, 0.0, 0.0, 0.0);

    // direct lighting
    for(uint i = 0; i < Get(directional_light_count); i++) {
        radiance += RadianceDirectionalLight(mat, Get(directional_light_data)[i], n, v, world_pos);
    }

    for(uint i = 0; i < Get(local_light_count); i++) {
        radiance += RadianceLocalLight(mat, Get(local_light_data)[i], n, v, world_pos);
    }
  
    // indirect light
    float3 reflect_dir = normalize(2.0 * dot(n, v) * n - v);
    float3 specular = SampleLvlTexCube(Get(specular_map), ibl_sampler, reflect_dir, mat.roughness * 8.0).xyz; // 256x256 at miplevel0
    float2 ibl_uv = float2(mat.roughness, NoV);
    float2 env = SampleTex2D(Get(specular_brdf_lut), ibl_sampler, ibl_uv).xy;
    float3 ambient = IBL(Get(sh), specular, env, n, NoV, mat) * LoadRWTex2D(Get(ao_tex), thread_id.xy).r * Get(ibl_intensity).x;
    radiance.xyz += ambient;
    Write2D(Get(out_color), thread_id.xy, radiance);
    
}
