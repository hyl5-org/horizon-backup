// compact a big index buffer of visible mesh to save cpu bind index buffer
// triangle culling cull single triangle to reduce vertex count into gpu.
// reduced IA, VS, stage

#include "include/platform/platform.hlsl"
#include "include/common/common_math.hlsl"
#include "include/indirectcommand/indirect_command.hlsl"
#include "include/geometry/geometry.hlsl"
#include "include/common/descriptor.hlsl"

#define ENABLE_CULL_BACKFACE			true
#define ENABLE_CULL_FRUSTUM				true
#define ENABLE_CULL_SMALL_PRIMITIVES	true

RES(RWStructuredBuffer<MeshInfo>, mesh_infos, UPDATE_FREQ_PER_FRAME);
RES(RWStructuredBuffer<uint>, mesh_index_offsets, UPDATE_FREQ_PER_FRAME);
RES(RWStructuredBuffer<uint>, visible_meshes, UPDATE_FREQ_PER_FRAME);

RES(StructuredBuffer<uint3>, index_buffers[], UPDATE_FREQ_BINDLESS);
RES(StructuredBuffer<uint3>, compacted_index_buffer, UPDATE_FREQ_PER_FRAME); // compact rest triangle into new index buffer

// uint GetMeshID(uint index_id) { return (mesh_id_list)[index_id]; }

[numthreads(WORK_GROUP_SIZE, 1, 1)]
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID, uint3 lane_id : SV_GroupThreadID) 
{
    

    // uint index_id = thread_id.x;
    // uint mesh_id = GetMeshID(thread_id.x);
    // uint index_offset = GetIndexOffset(mesh_id, index_id);
    // compacted_index_buffer[index_id] = (index_buffers)[mesh_id][index_offset];

    // if (index_id > (mesh_index_offsets)[current_mesh_id]) {
    //     AtomicAdd(current_mesh_id, 1);
    // }

    
}

