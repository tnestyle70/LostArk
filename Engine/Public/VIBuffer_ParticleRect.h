#pragma once

#include "VIBuffer.h"

#include <span>

NS_BEGIN(Engine)

struct VTXEFFECT_PARTICLE final
{
	float4x4_t World{};
	float4_t Color = { 1.f, 1.f, 1.f, 1.f };

	static constexpr uint32_t iNumElements = 7u;
	static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
};

class ENGINE_DLL CVIBuffer_ParticleRect final : public CVIBuffer
{
private:
	CVIBuffer_ParticleRect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CVIBuffer_ParticleRect();

public:
	HRESULT Initialize_Prototype(uint32_t iCapacity);
	virtual HRESULT Initialize(void* pArg) override;
	HRESULT Update_Instances(std::span<const VTXEFFECT_PARTICLE> Instances);
	virtual HRESULT Bind_Resources() override;
	virtual HRESULT Render() override;
	uint32_t Get_Capacity() const { return m_iCapacity; }
	uint32_t Get_InstanceCount() const { return m_iNumInstances; }

private:
	ComPtr<ID3D11Buffer> m_pInstanceBuffer;
	uint32_t m_iCapacity = 0u;
	uint32_t m_iNumInstances = 0u;

public:
	static unique_ptr<CVIBuffer_ParticleRect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iCapacity);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END

