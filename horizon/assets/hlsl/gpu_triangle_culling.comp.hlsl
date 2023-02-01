// compact a big index buffer of visible mesh to save cpu bind index buffer
// counter to https://github.com/ConfettiFX/The-Forge/blob/06dcb5cea31e30f9509309b1d7557167397a71a4/Examples_3/Unit_Tests/src/15a_VisibilityBufferOIT/Shaders/FSL/triangle_filtering.comp.fsl
#include "include/platform/platform.hlsl"
#include "include/common/common_math.hlsl"
#include "include/indirectcommand/indirect_command.hlsl"
#include "include/geometry/geometry.hlsl"

#define ENABLE_CULL_BACKFACE			1
#define ENABLE_CULL_FRUSTUM				1
#define ENABLE_CULL_SMALL_PRIMITIVES	1

RES(RBuffer(MeshInfo), mesh_infos, UPDATE_FREQ_PER_FRAME);
RES(RWBuffer(uint), mesh_index_offsets, UPDATE_FREQ_PER_FRAME);
RES(RWBuffer(uint), visible_meshes, UPDATE_FREQ_PER_FRAME);

RES(Buffer(uint3), index_buffers[], UPDATE_FREQ_BINDLESS);
RES(Buffer(uint3), compacted_index_buffer, UPDATE_FREQ_PER_FRAME); // compact rest triangle into new index buffer

// uint GetMeshID(uint index_id) { return Get(mesh_id_list)[index_id]; }

NUM_THREADS(WORK_GROUP_SIZE, 1, 1)
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID, SV_GroupThreadID(uint3) lane_id) 
{
    

    // uint index_id = thread_id.x;
    // uint mesh_id = GetMeshID(thread_id.x);
    // uint index_offset = GetIndexOffset(mesh_id, index_id);
    // compacted_index_buffer[index_id] = Get(index_buffers)[mesh_id][index_offset];

    // if (index_id > Get(mesh_index_offsets)[current_mesh_id]) {
    //     AtomicAdd(current_mesh_id, 1);
    // }

    
}

// triangle culling cull single triangle to reduce vertex count into gpu.
reduced IA, VS, stage
