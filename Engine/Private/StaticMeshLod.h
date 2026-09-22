#pragma once

#include "MeshLod.h"
#include "Engine_VertexTypes.h"
#include <array>
#include <span>

NS_BEGIN(Engine)

// Additional immutable index ranges share the original vertex buffer. The
// original mesh's own index buffer remains the default and shadow path.
class CStaticMeshLod final
{
public:
    struct SELECTION final
    {
        uint32_t indexCount = 0u;
        uint32_t firstIndex = 0u;
        uint32_t level = 0u;
    };

    static HRESULT Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
        std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result);
    HRESULT Select_Range(const MESH_SCREEN_LOD_DESC& view, SELECTION& result) const;
    ID3D11Buffer* Get_IndexBuffer() const { return m_IndexBuffer.Get(); }

private:
    struct RANGE final { uint32_t count = 0u, first = 0u; f32_t error = 0.f; };
    std::array<RANGE, 3> m_Ranges{};
    uint32_t m_RangeCount = 0u;
    ComPtr<ID3D11Buffer> m_IndexBuffer;
};

NS_END
