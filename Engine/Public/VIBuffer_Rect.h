#pragma once

// 공통 GPU 버퍼 소유권과 IA 바인딩·draw 기능을 상속한다. Rect는 원본 정점/인덱스 생성과 복제만 구체화한다.
#include "VIBuffer.h"

/* 네모의 형태를 구성해주기위한 정점과 인덱스를 보관한다. */

// 보충 설명: 아래 Rect는 로컬 XY 평면의 1 x 1 사각형을 정점 4개와 삼각형 인덱스 6개로 표현한다.
// 화면/월드 위치와 크기는 이 원본 정점을 바꾸는 대신 소비 객체의 Transform과 shader가 결정한다.
NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Rect final : public CVIBuffer
{
private:
	CVIBuffer_Rect(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CVIBuffer_Rect();

public:
	// 한 번 만든 VB/IB를 clone들이 공유한다. 생성 도중 실패한 prototype은 Create의 지역 소유 객체와 함께 정리된다.
	virtual HRESULT Initialize_Prototype() override;
	// clone별 입력은 현재 사용하지 않으며 복사한 GPU 자원을 유지한다.
	virtual HRESULT Initialize(void* pArg) override;

public:
	// MainApp 등록 및 Renderer/Effect cache가 호출한다. 실패는 nullptr이며 생성된 prototype의 소유권을 반환한다.
	static unique_ptr<CVIBuffer_Rect> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	// GPU 버퍼를 깊은 복사하지 않는다. 인스턴스별 변환/표시 상태는 이 버퍼 밖의 객체가 관리한다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END