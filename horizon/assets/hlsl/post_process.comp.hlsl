#include "include/postprocess/postprocess.hlsl"
#include "include/common/hash.hlsl"
RES(RWTexture2D<float4>, color_image, UPDATE_FREQ_PER_FRAME);
RES(RWTexture2D<float4>, out_color_image, UPDATE_FREQ_PER_FRAME);

CBUFFER(exposure_constants, UPDATE_FREQ_PER_FRAME)
{
    DATA(float4, exposure_ev100__, None);
};

[numthreads(8, 8, 1)]
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID) 
{
    
    
    float4 color  = LoadRWTex2D(Get(color_image), thread_id.xy);

    color.xyz *= Get(exposure_ev100__).x;

    color.xyz = TonemapACES(color.xyz);

    color.xyz = GammaCorrection(color.xyz);

    Write2D(Get(out_color_image), thread_id.xy, color);

    
}

