#pragma once

#include "MeshLod.h"
#include "Engine_VertexTypes.h"
#include <array>
#include <span>

NS_BEGIN(Engine)

struct STATIC_MESH_LOD_PROGRAM;

// Additional immutable index ranges share the original vertex buffer. The
// original mesh's own index buffer remains the default and shadow path.
class CStaticMeshLod final
{
public:
    static HRESULT Create(ID3D11Device* device, std::span<const VTXMESH> vertices,
        std::span<const uint32_t> indices, std::shared_ptr<CStaticMeshLod>& result);
    HRESULT Prepare(ID3D11DeviceContext* context, const MESH_SCREEN_LOD_DESC& view,
        uint32_t instanceCount);
    ID3D11Buffer* Get_IndexBuffer() const { return m_IndexBuffer.Get(); }
    ID3D11Buffer* Get_DrawArguments() const { return m_DrawArguments.Get(); }

private:
    struct RANGE final { uint32_t count = 0u, first = 0u; f32_t error = 0.f; };
    std::array<RANGE, 3> m_Ranges{};
    uint32_t m_RangeCount = 0u;
    std::shared_ptr<STATIC_MESH_LOD_PROGRAM> m_Program;
    ComPtr<ID3D11Buffer> m_IndexBuffer, m_Constants, m_DrawArguments;
    ComPtr<ID3D11UnorderedAccessView> m_ArgumentsUAV;
};

NS_END
