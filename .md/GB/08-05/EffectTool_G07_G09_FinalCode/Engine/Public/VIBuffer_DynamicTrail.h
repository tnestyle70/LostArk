#pragma once

#include "VIBuffer.h"

#include <span>

NS_BEGIN(Engine)

struct VTXEFFECT_TRAIL final
{
	float3_t vPosition{};
	float2_t vTexcoord{};
	float4_t vColor = { 1.f, 1.f, 1.f, 1.f };

	static constexpr uint32_t iNumElements = 3u;
	static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
};

class ENGINE_DLL CVIBuffer_DynamicTrail final : public CVIBuffer
{
private:
	CVIBuffer_DynamicTrail(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CVIBuffer_DynamicTrail();

public:
	HRESULT Initialize_Prototype(uint32_t iMaxPoints);
	virtual HRESULT Initialize(void* pArg) override;
	HRESULT Update_Geometry(
		std::span<const VTXEFFECT_TRAIL> Vertices,
		std::span<const uint32_t> Indices);
	virtual HRESULT Render() override;
	uint32_t Get_VertexCount() const { return m_iActiveVertices; }

private:
	uint32_t m_iMaxVertices = 0u;
	uint32_t m_iMaxIndices = 0u;
	uint32_t m_iActiveVertices = 0u;

public:
	static unique_ptr<CVIBuffer_DynamicTrail> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iMaxPoints);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END

