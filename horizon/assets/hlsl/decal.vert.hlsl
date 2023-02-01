#include "include/common/bindless.hlsl"
#ifdef VULKAN
#extension GL_EXT_nonuniform_qualifier : enable
#endif

CBUFFER(CameraParamsUb, UPDATE_FREQ_PER_FRAME)
{
    DATA(float4x4, vp, None);
    DATA(float4x4, prev_vp, None);
    DATA(float4, camera_position, None);
};

// PUSH_CONSTANT(DrawRootConstant, b1){
//     DATA(uint, mesh_id_offset, None);
// };

const uint mesh_id_offset; 

STRUCT(InstanceParameter)
{
    DATA(float4x4,  model, None);
    DATA(float4x4,  decal_to_world, None);
    DATA(float4x4,  world_to_decal, None);
    DATA(uint, material_id, None);
};

RES(Buffer(InstanceParameter), decal_instance_parameter, UPDATE_FREQ_PER_FRAME);

STRUCT(VSInput)
{
	DATA(float3, position, POSITION);
	DATA(float3, normal, NORMAL);
	DATA(float2, uv0, UV0);
	DATA(float2, uv1, UV1);
	DATA(float3, tangent, TANGENT);
};

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

VSOutput VS_MAIN( VSInput vsin, SV_InstanceID(uint) InstanceID, SV_VertexID(uint) vertex_id)
{
    
    VSOutput vsout;
    uint mesh_id;
#ifdef VULKAN
    mesh_id = gl_DrawID;
#endif
    mesh_id += Get(mesh_id_offset);
    float4x4 decal_to_world = Get(decal_instance_parameter)[InstanceID].decal_to_world;
    float4x4 model = Get(decal_instance_parameter)[InstanceID].model;
    float4 pos = decal_to_world * float4(vsin.position, 1.0);
    vsout.position = vp * model * pos;
    vsout.instance_id = InstanceID;
    vsout.material_id = Get(decal_instance_parameter)[InstanceID].material_id;
    RETURN(vsout);
}
