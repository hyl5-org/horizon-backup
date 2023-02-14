#ifndef __GEOMETRY__
#define __GEOMETRY__

struct AABB {
    float4 a;
    float4 b;
};

struct ExpandedAABB {
    float4 data[8]; // 8 point
};
struct Plane {
    float4 data;
};

struct FrustumPlanes {
    Plane planes[6]; // left, right, bottom, up, near, far
};

ExpandedAABB GetExpandedAABB(AABB aabb) {
    ExpandedAABB ret;
    return ret;
}

struct MeshInfo {
    float3 center;
    AABB aabb;
    float4x4 transform;
    uint index_count;
};

#endif