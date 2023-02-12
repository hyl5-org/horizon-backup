#include "include/common/descriptor.hlsl"
#include "include/common/luminance.hlsl"
RES(RWTexture2D<float4>, prev_color_tex, UPDATE_FREQ_PER_FRAME);
RES(RWTexture2D<float4>, curr_color_tex, UPDATE_FREQ_PER_FRAME);
RES(RWTexture2D<float2>, mv_tex, UPDATE_FREQ_PER_FRAME);
RES(RWTexture2D<float4>, out_color_tex, UPDATE_FREQ_PER_FRAME);

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

[numthreads(8, 8, 1)] void CS_MAIN(uint3 thread_id
                                   : SV_DispatchThreadID)
{
    float3 curr_color = curr_color_tex[thread_id.xy].xyz;

    float2 motion_vector = mv_tex[thread_id.xy].xy;

    uint2 previous_sample = thread_id.xy - uint2(motion_vector * resolution);
    float3 prev_color = prev_color_tex[previous_sample].xyz;

    // Apply clamping on the history color.
    float3 c0 = curr_color_tex[thread_id.xy + int2(1, 0)].xyz;
    float3 c1 = curr_color_tex[thread_id.xy + int2(-1, 0)].xyz;
    float3 c2 = curr_color_tex[thread_id.xy + int2(0, 1)].xyz;
    float3 c3 = curr_color_tex[thread_id.xy + int2(0, -1)].xyz;

    float3 c_min = min(curr_color, min(c0, min(c1, min(c2, c3))));
    float3 c_max = max(curr_color, max(c0, max(c1, max(c2, c3))));
    ;

    prev_color = clamp(prev_color, c_min, c_max);

    // Karis, Brian. "High-quality temporal supersampling, result blur
    float w0 = Luminance(prev_color) * (1.0 - 0.05);
    float w1 = Luminance(curr_color) * 0.05;
    float w = w1 / (w0 + w1);

    float4 final_color = float4((1.0 - w) * prev_color + w * curr_color, 1.0);

    out_color_tex[thread_id.xy] = final_color;
}
