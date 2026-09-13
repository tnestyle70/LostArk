// The CPU has already selected visible instances. One conservative LOD serves
// the whole batch, preserving one draw per mesh and the original instance order.
cbuffer MeshLodSelection : register(b0)
{
    uint4 g_Ranges[3]; // index count, first index, unused, unused
    float4 g_ErrorsAndScale;
    float4 g_ViewBounds;
    float4 g_ProjectionNearError;
    uint4 g_InstanceAndRangeCount;
};

RWByteAddressBuffer g_DrawArguments : register(u0);

[numthreads(1, 1, 1)]
void SelectMeshLod(uint3 threadId : SV_DispatchThreadID)
{
    uint selected = 0u;
    float nearestZ = g_ViewBounds.z - g_ViewBounds.w;
    float2 largestXY = abs(g_ViewBounds.xy) + g_ViewBounds.w;
    if (nearestZ > g_ProjectionNearError.z)
    {
        [unroll]
        for (uint level = 1u; level < 3u; ++level)
        {
            float error = g_ErrorsAndScale[level] * g_ErrorsAndScale.w;
            float safeZ = nearestZ - error;
            if (level < g_InstanceAndRangeCount.y && safeZ > g_ProjectionNearError.z)
            {
                // Bounds dx,dz independently, including off-axis perspective
                // magnification. A near/intersecting batch always stays at LOD0.
                float2 pixelError = g_ProjectionNearError.xy * error *
                    (1.f + largestXY / nearestZ) / safeZ;
                if (all(isfinite(pixelError)) &&
                    max(pixelError.x, pixelError.y) <= g_ProjectionNearError.w * .9f)
                    selected = level;
            }
        }
    }
    g_DrawArguments.Store(0u, g_Ranges[selected].x);
    g_DrawArguments.Store(4u, g_InstanceAndRangeCount.x);
    g_DrawArguments.Store(8u, g_Ranges[selected].y);
    g_DrawArguments.Store(12u, 0u);
    g_DrawArguments.Store(16u, 0u);
}
