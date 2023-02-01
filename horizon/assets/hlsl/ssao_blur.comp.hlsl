#include "include/common/descriptor.hlsl"
#include "include/postprocess/postprocess.hlsl"
RES(RWTexture2D<float4>, ssao_blur_in, UPDATE_FREQ_PER_FRAME); // TODO(hylu): replace with float
RES(RWTexture2D<float4>, ssao_blur_out, UPDATE_FREQ_PER_FRAME);

[numthreads(8, 8, 1)]
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID) 
{
    float4 result = float4(0.0, 0.0, 0.0, 0.0);

    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            uint2 coord = thread_id.xy + uint2(x, y);
            result += ssao_blur_in[coord].r;
        }
    }
    result = result / (4.0 * 4.0);
    
    ssao_blur_out[thread_id.xy] = result;
}

