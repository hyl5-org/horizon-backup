// #include "include/postprocess/postprocess.hlsl"
// #include "include/common/hash.hlsl"
// #include "include/common/descriptor.hlsl"
// RES(RWTexture2D<float4>, color_image, UPDATE_FREQ_PER_FRAME);
// RES(RWTexture2D<float4>, out_color_image, UPDATE_FREQ_PER_FRAME);

// CBUFFER(exposure_constants, UPDATE_FREQ_PER_FRAME)
// {
//     float4 exposure_ev100__;
// };

[numthreads(8, 8, 1)]
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID) 
{
    // float4 color  = color_image[thread_id.xy];

    // color.xyz *= exposure_ev100__.x;

    // color.xyz = TonemapACES(color.xyz);

    // color.xyz = GammaCorrection(color.xyz);

    // out_color_image[thread_id.xy] =  color;
}

