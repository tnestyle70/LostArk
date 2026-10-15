#pragma once

// 장치/context와 vertex/index buffer의 COM 소유, 입력 바인딩·draw 경로를 상속한다.
#include "VIBuffer.h"

/* 네모의 형태를 구성해주기위한 정점과 인덱스를 보관한다. */

NS_BEGIN(Engine)

// CCell의 navigation 디버그 표시용 버퍼다. 위 기존 설명과 달리 실제 구현은 세 점을 0→1→2→0으로 잇는 삼각형 선분이다.
class ENGINE_DLL CVIBuffer_Cell final : public CVIBuffer
{
private:
	CVIBuffer_Cell(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer_Cell();

public:
	// CCell이 전달한 세 float3를 복사해 GPU 자원을 만든다. 입력 포인터/길이 검사는 호출자가 보장한다.
	virtual HRESULT Initialize_Prototype(const float3_t* pPoints);
	// 기본 clone 초기화는 pArg를 사용하지 않고 기존 자원을 유지한다.
	virtual HRESULT Initialize(void* pArg) override;



public:
	// 초기화 실패는 nullptr며 이미 준비된 COM 자원도 임시 객체와 함께 정리한다.
	static unique_ptr<CVIBuffer_Cell> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const float3_t* pPoints);
	// 기본 복사로 같은 GPU buffer를 공유하는 clone이다. 별도 점 배열이나 GPU 사본을 만들지 않는다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END