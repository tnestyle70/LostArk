#pragma once

// 장치/context 및 vertex/index COM 자원과 기본 indexed draw를 상속한다.
#include "VIBuffer.h"

// 업로드할 CPU 정점/index 배열을 호출 동안만 차용한다.
#include <span>

NS_BEGIN(Engine)

// Client ribbon/trail 생성 결과를 shader에 전달한다. 위치/UV/color/동적 material 입력의 slot 0 byte layout이다.
struct VTXEFFECT_TRAIL final
{
	float3_t vPosition{};
	float2_t vTexcoord{};
	float4_t vColor = { 1.f, 1.f, 1.f, 1.f };
	float4_t vDynamicParameter{};

	static constexpr uint32_t iNumElements = 4u;
	static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
			0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
			0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
};

// trail 하나를 위한 가변 정점/index GPU 버퍼다. geometry의 계산·좌표계 선택은 Client 이펙트 renderer가 맡는다.
class ENGINE_DLL CVIBuffer_DynamicTrail final : public CVIBuffer
{
private:
	CVIBuffer_DynamicTrail(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CVIBuffer_DynamicTrail();

public:
	// 렌더 점 용량 2~16384를 받아 점당 정점2개·구간당 index6개의 공간을 준비한다.
	HRESULT Initialize_Prototype(uint32_t iMaxPoints);
	// 기반 클래스의 clone 초기화만 호출한다. geometry 업로드는 별도 Update_Geometry에서 수행한다.
	virtual HRESULT Initialize(void* pArg) override;
	// 용량 초과는 기존 draw 상태를 유지한다. 빈 입력 또는 GPU Map 실패는 활성 개수를 0으로 만든다.
	// 정점 수치와 index 범위는 여기서 검증하지 않으며 입력 span을 저장하지 않는다.
	HRESULT Update_Geometry(
		std::span<const VTXEFFECT_TRAIL> Vertices,
		std::span<const uint32_t> Indices);
	// 활성 index가 없으면 S_FALSE다. 소비자는 shader와 Bind_Resources를 준비한 뒤 draw를 호출한다.
	virtual HRESULT Render() override;
	// 할당 용량이 아니라 마지막 업로드의 활성 정점 수다.
	uint32_t Get_VertexCount() const
	{
		return m_iActiveVertices;
	}

private:
	// 고정 할당 용량과 이번 draw의 활성 개수를 구분한다. 활성 index 수는 기반 m_iNumIndices가 보관한다.
	uint32_t m_iMaxVertices = 0u;
	uint32_t m_iMaxIndices = 0u;
	uint32_t m_iActiveVertices = 0u;

public:
	// 준비 실패한 부분 GPU 자원은 임시 객체와 함께 정리하고 성공한 buffer만 반환한다.
	static unique_ptr<CVIBuffer_DynamicTrail> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iMaxPoints);
	// 같은 용량의 GPU 자원을 새로 만들어 쓰기 가능한 buffer를 clone끼리 공유하지 않는다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END

