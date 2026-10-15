#pragma once

// quad vertex/index와 장치/context의 COM 소유 및 엔진 prototype 경로를 상속한다.
#include "VIBuffer.h"

// Client가 계산한 particle instance 목록을 업로드 호출 동안 차용한다.
#include <span>

NS_BEGIN(Engine)

// sprite particle 한 개의 slot1 record다. slot0 quad 위치/UV와 결합하며 World/color/material 입력을 instance마다 읽는다.
struct VTXEFFECT_PARTICLE final
{
	float4x4_t World{};
	float4_t Color = { 1.f, 1.f, 1.f, 1.f };
	float4_t DynamicParameter{};
	// 현재/다음 SubUV frame의 변환이다. ParticleData는 Client가 정규화 수명과 frame 보간값을 전달한다.
	float4_t UVTransform = { 1.f, 1.f, 0.f, 0.f };
	float4_t UVTransformNext = { 1.f, 1.f, 0.f, 0.f };
	float2_t ParticleData{};

	static constexpr uint32_t iNumElements = 11u;
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
			1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "DYNAMIC", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "UVTRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "UVTRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
			1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "PARTICLEDATA", 0, DXGI_FORMAT_R32G32_FLOAT,
			1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
};

// 불변 quad와 동적 instance buffer를 소유한다. 같은 객체의 업로드/그리기는 immediate context 렌더 흐름에서 직렬화한다.
class ENGINE_DLL CVIBuffer_ParticleRect final : public CVIBuffer
{
private:
	CVIBuffer_ParticleRect(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CVIBuffer_ParticleRect();

public:
	// 1~2048 instance 용량으로 자원을 만든다. 후보 생성 실패의 부분 자원은 factory가 정리한다.
	HRESULT Initialize_Prototype(uint32_t iCapacity);
	// 기반 초기화만 수행하며 instance 데이터는 별도 업로드한다.
	virtual HRESULT Initialize(void* pArg) override;
	// 용량 초과는 기존 draw 상태를 유지한다. 빈 입력/Map 실패는 draw 개수를 0으로 만들며 span은 보관하지 않는다.
	HRESULT Update_Instances(std::span<const VTXEFFECT_PARTICLE> Instances);
	// quad slot0과 마지막 업로드 byte offset의 instance slot1, index buffer를 바인딩한다.
	virtual HRESULT Bind_Resources() override;
	// 빈 목록은 S_FALSE다. 이 구현은 Bind_Resources까지 수행한 뒤 instance draw를 제출한다. shader 선택은 호출자가 맡는다.
	virtual HRESULT Render() override;
	// 할당된 instance 상한과 현재 그릴 instance 개수를 구분해 조회한다.
	uint32_t Get_Capacity() const
	{
		return m_iCapacity;
	}
	uint32_t Get_InstanceCount() const
	{
		return m_iNumInstances;
	}

private:
	// dynamic instance 자원의 강한 COM 참조다. offset/cursor는 이 자원과 같은 객체에 보관한다.
	ComPtr<ID3D11Buffer> m_pInstanceBuffer;
	uint32_t m_iCapacity = 0u;
	uint32_t m_iNumInstances = 0u;
	// 아직 쓰지 않은 instance 구간에 이어 쓰고 공간이 끝나면 WRITE_DISCARD로 새 구간을 시작한다.
	uint32_t m_iNextInstanceIndex = 0u;
	uint32_t m_iInstanceByteOffset = 0u;

public:
	// quad와 instance 자원이 모두 준비된 객체만 소유자에게 넘긴다.
	static unique_ptr<CVIBuffer_ParticleRect> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		uint32_t iCapacity);
	// 같은 capacity의 자원을 새로 만든다. 원본 업로드 내용·cursor·활성 개수는 복사하지 않는다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END

