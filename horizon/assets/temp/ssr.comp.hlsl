// #include "include/common/descriptor.hlsl"
// #include "include/translation/translation.hlsl"
// #include "include/common/common_math.hlsl"
// #include "include/common/noise.hlsl"

// RES(Texture2D<float4>, depth_tex, UPDATE_FREQ_PER_FRAME);
// RES(Texture2D<float4>, normal_tex, UPDATE_FREQ_PER_FRAME);
// RES(Texture2D<float4>, ssao_noise_tex, UPDATE_FREQ_PER_FRAME);
// RES(SamplerState, default_sampler, UPDATE_FREQ_PER_FRAME);
// RES(RWTexture2D<float4>, ao_factor_tex, UPDATE_FREQ_PER_FRAME);


// CBUFFER(SSRConstant, UPDATE_FREQ_PER_FRAME)
// {
//     DATA(float4x4, camera_projection, None);
//     DATA(float4x4, camera_inv_projection, None); // we only care about view space
//     DATA(float4x4, camera_view, None);
//     DATA(uint2, resolution, None);
//     DATA(float2, noise_scale, None);
//     DATA(float4, kernels[SSAO_SAMPLE_COUNT], None); // tangent space kernels
// };


// [numthreads(8, 8, 1)]
// void CS_MAIN( uint3 thread_id: SV_DispatchThreadID) 
// {
//     

//     // uint2 _resolution = (resolution.xy) - uint2(1.0, 1.0);

//     // if (thread_id.x>_resolution.x || thread_id.y>_resolution.y) {
//     //     
//     // }

//     // float2 uv = float2(thread_id.xy) / float2(_resolution);


//     // float depth = SampleTex2D((depth_tex), default_sampler, uv).r;
//     // float3 normal = SampleTex2D((normal_tex), default_sampler, uv).xyz;
//     // float3 albedo = LoadRWTex2D((curr_color_tex), thread_id.xy + int2(1, 0)).xyz;
    
//     // float3 view_normal = ((camera_view) * float4(normal, 0.0)).xyz; // view space normal
//     // float3 view_pos = ReconstructWorldPos((camera_inv_projection), depth, uv);
//     // float3 view_dir = normalize(float3(0.0) - view_pos);
//     // float3 reflect_dir = reflect(-view_dir, view_normal);

//     // float3 ray_pos = view_pos;
//     // for (uint step = 0; step < SSR_MARCH_STEP; step++) {
//     //     ray_pos += step * step_length;
//     //     float4 clip_pos = projection * float4(ray_pos, 1.0);
//     //     clip_pos.xyz /= clip_pos.w;
//     //     float2 screen_pos = (clip_pos.xy * 0.5 + 0.5) * (screen_resolution);
//     // }

//     // Write2D((ao_factor_tex), thread_id.xy, out_color);

//     
// }

