#pragma once

#include "Engine_Defines.h"
#include <span>

NS_BEGIN(Engine)

// Current-frame CPU depth only. No GPU query, previous-frame visibility or
// scene ownership is retained. Uncertain input always remains visible.
class ENGINE_DLL COcclusionCuller final
{
public:
    enum class CULL_MODE : uint8_t { NONE, BACK, FRONT };
    explicit COcclusionCuller(bool_t forceSse2 = false);
    ~COcclusionCuller();
    COcclusionCuller(const COcclusionCuller&) = delete;
    COcclusionCuller& operator=(const COcclusionCuller&) = delete;

    bool_t Begin(const float4x4_t& view, const float4x4_t& projection,
        uint32_t viewportWidth, uint32_t viewportHeight);
    // Positions are x/y/z at byte offsets 0/4/8. Geometry is borrowed only for
    // this call, must be opaque and match the geometry actually drawn this frame.
    bool_t Rasterize(const float* positions, uint32_t vertexCount,
        uint32_t strideBytes, std::span<const uint32_t> indices,
        const float4x4_t& world, CULL_MODE cullMode);
    bool_t TestBounds(const float3_t& worldMin, const float3_t& worldMax) const;

    static constexpr uint32_t MAXIMUM_INPUT_TRIANGLES = 100000u;
    uint32_t GetInputTriangles() const;
    uint32_t GetRasterizedTriangles() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    // 0 SSE2, 1 SSE4.1, 2 AVX2; UINT32_MAX until initialized.
    uint32_t GetImplementation() const;

private:
    struct STATE;
    std::unique_ptr<STATE> m_State;
    bool_t m_ForceSse2 = false;
};

NS_END
