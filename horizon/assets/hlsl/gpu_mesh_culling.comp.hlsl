#include "include/platform/platform.hlsl"
#include "include/common/common_math.hlsl"
#include "include/indirectcommand/indirect_command.hlsl"
#include "include/translation/translation.hlsl"

RES(RWBuffer(uint), draw_count, UPDATE_FREQ_PER_FRAME);
RES(RWBuffer(DrawIndexedInstancedCommand), indirect_draw_command, UPDATE_FREQ_PER_FRAME);
RES(RBuffer(MeshInfo), mesh_infos, UPDATE_FREQ_PER_FRAME);
RES(RWBuffer(uint), mesh_index_offsets, UPDATE_FREQ_PER_FRAME);
RES(RWBuffer(uint), visible_meshes, UPDATE_FREQ_PER_FRAME);
RES(RWBuffer(uint3), compact_index_buffer_dispatch_command, UPDATE_FREQ_PER_FRAME);
CBUFFER(FrustumCullingConstants, UPDATE_FREQ_PER_FRAME)
{
    DATA(FrustumPlanes, camera_frustum_planes, None);
    DATA(uint, mesh_count, None);
};


// cull aabb by frustum plane
bool FrustumCull(ExpandedAABB expanded_aabb, FrustumPlanes frustum_planes) {
    bool culled = false;
    for (uint i = 0; i < 6; i++) {
        if ((Dot(frustum_planes.planes[i].data, expanded_aabb.data[0]) < 0) || 
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[1]) < 0) ||
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[2]) < 0) ||
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[3]) < 0) ||
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[4]) < 0) ||
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[5]) < 0) ||
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[6]) < 0) ||
            (Dot(frustum_planes.planes[i].data, expanded_aabb.data[7]) < 0)) {
            return false;
        }
    }
    return true;
}

// each thread a mesh
NUM_THREADS(WORK_GROUP_SIZE, 1, 1)
void CS_MAIN( uint3 thread_id: SV_DispatchThreadID, SV_GroupThreadID(uint3) lane_id) 
{
    

    if (thread_id.x > mesh_count) {
        return;
    }

    uint mesh_id = thread_id.x;
    ExpandedAABB expanded_aabb = GetExpandedAABB(Get(mesh_infos)[mesh_id].aabb);
    expanded_aabb = ToWorld(expanded_aabb, Get(mesh_infos)[mesh_id].transform);
    // tranform aabb to world space
    bool culled = FrustumCull(expanded_aabb, Get(camera_frustum_planes));

    uint visible_mesh_count = WavePrefixCountBits(!culled);

    // all renderable in wavefront culled 
    if (visible_mesh_count == 0) {
        return;
    }

    uint global_draw_offset;
    uint global_index_offset;

    uint local_draw_offset = WavePrefixSum(uint(!culled));
    uint local_index_offset = WavePrefixSum(Get(mesh_infos)[mesh_id].index_count);

    if (lane_id.x == 0) {
        uint summed_index_count = WaveActiveSum(Get(mesh_infos)[mesh_id].index_count);
        AtomicAdd(Get(draw_count)[0], visible_mesh_count, global_draw_offset);
        AtomicAdd(Get(compact_index_buffer_dispatch_command)[0].x, summed_index_count, global_index_offset); // slightly larger than the actual index count
    }
    
    if (!culled) {
        // global plus local to save synchronization between wavefront
        uint draw_offset = global_draw_offset + local_draw_offset;
        uint index_offset = global_index_offset + local_index_offset;
        // compact the draw command buffer
        // Get(indirect_draw_command)[draw_offset].index_count = Get(mesh_infos)[mesh_id].index_count;
        // Get(indirect_draw_command)[draw_offset].instance_count = 1;
        // Get(indirect_draw_command)[draw_offset].first_index = index_offset;
        // Get(indirect_draw_command)[draw_offset].first_vertex = 0;
        // Get(indirect_draw_command)[draw_offset].first_instance = 0;

        Get(visible_meshes)[draw_offset] = mesh_id;
        Get(mesh_index_offsets)[draw_offset] = index_offset;
    }

    // if the last mesh, divide index buffer count by WORK_GROUP_SIZE
    if (thread_id.x == (mesh_count - 1)) {
        Get(compact_index_buffer_dispatch_command)[0].x = AlignUp(Get(compact_index_buffer_dispatch_command)[0].x, WORK_GROUP_SIZE);
    }
    
    
}
