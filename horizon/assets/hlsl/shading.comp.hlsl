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
    float4x4 camera_view;
    float4x4 camera_projection;
    float4x4 camera_view_projection;
    float4x4 camera_inverse_view_projection;
    float4x4 camera_prev_view_projection;
    uint2 resolution;
    uint2 pad_0;
    float3 camera_pos;
    uint pad_1;
    float ibl_intensity;
};

CBUFFER(LightCountUb, UPDATE_FREQ_PER_FRAME)
{
    uint directional_light_count;
    uint local_light_count;
};

CBUFFER(DirectionalLightDataUb, UPDATE_FREQ_PER_FRAME)
{
    LightParams directional_light_data[MAX_DYNAMIC_LIGHT_COUNT];
};

CBUFFER(LocalLightDataUb, UPDATE_FREQ_PER_FRAME)
{
    LightParams local_light_data[MAX_DYNAMIC_LIGHT_COUNT];
};

RES(RWTexture2D<float4>, out_color, UPDATE_FREQ_PER_FRAME);

RES(RWTexture2D<float4>, ao_tex, UPDATE_FREQ_PER_FRAME);

CBUFFER(DiffuseIrradianceSH3, UPDATE_FREQ_PER_FRAME)
{
    float3 sh[9];
};

RES(TextureCube<float4>, specular_map, UPDATE_FREQ_PER_FRAME);
RES(Texture2D<float4>, specular_brdf_lut, UPDATE_FREQ_PER_FRAME);
RES(SamplerState, ibl_sampler, UPDATE_FREQ_PER_FRAME);

[numthreads(8, 8, 1)] void CS_MAIN(uint3 thread_id
                                   : SV_DispatchThreadID, uint3 groupID
                                   : SV_GroupID)
{
    uint2 _resolution = resolution.xy - uint2(1.0, 1.0);

    if (thread_id.x > _resolution.x || thread_id.y > _resolution.y)
    {
        return;
    }

    float2 uv = float2(thread_id.xy) / float2(_resolution);
    float4 gbuffer0 = gbuffer0_tex.SampleLevel(default_sampler, uv, 0);
    float4 gbuffer1 = gbuffer1_tex.SampleLevel(default_sampler, uv, 0);
    float4 gbuffer2 = gbuffer2_tex.SampleLevel(default_sampler, uv, 0);
    float4 gbuffer3 = gbuffer3_tex.SampleLevel(default_sampler, uv, 0);

    MaterialProperties mat;
    mat.material_id = groupID.z;
    // mat.normal
    mat.albedo = gbuffer1.xyz;

    mat.metallic = gbuffer3.x;

    // adjust roughness to reduce specular aliasing
    float roughness = max(0.045f, gbuffer3.y);

    float alpha = gbuffer3.z;

    mat.roughness = roughness;
    mat.roughness2 = Pow2(roughness);
    mat.f0 = lerp(float3(0.04, 0.04, 0.04), mat.albedo, mat.metallic);

    float3 world_pos = ReconstructWorldPos((camera_inverse_view_projection), depth_tex.SampleLevel(default_sampler, uv, 0).r, uv);
    float3 n = normalize(gbuffer0.xyz);
    float3 v = -normalize(world_pos - (camera_pos).xyz);
    float NoV = saturate(dot(n, v));
    float4 radiance = float4(0.0, 0.0, 0.0, 0.0);

    // direct lighting
    for (uint i = 0; i < directional_light_count; i++)
    {
        radiance += RadianceDirectionalLight(mat, directional_light_data[i], n, v, world_pos);
    }

    for (uint j = 0; j < local_light_count; j++)
    {
        radiance += RadianceLocalLight(mat, local_light_data[j], n, v, world_pos);
    }

    // indirect light
    float3 reflect_dir = normalize(2.0 * dot(n, v) * n - v);
    float3 specular = specular_map.SampleLevel(ibl_sampler, reflect_dir, mat.roughness * 8.0).xyz;
    float2 ibl_uv = float2(mat.roughness, NoV);
    float2 env = specular_brdf_lut.SampleLevel(ibl_sampler, ibl_uv, 0).xy;
    float3 ambient = IBL((sh), specular, env, n, NoV, mat) * ao_tex[thread_id.xy].r * (ibl_intensity).x;
    radiance.xyz += ambient;
    out_color[thread_id.xy] = radiance;
}
