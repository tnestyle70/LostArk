#pragma once

// 뷰/투영 행렬 구분 D3DTS, DirectX 수학 타입과 namespace 매크로를 사용한다.
#include "Engine_Defines.h"
// Shader의 호출 중 공유 참조와 GameInstance로 넘기는 단독 소유권을 선언한다.
#include <memory>

NS_BEGIN(Engine)

// GameInstance가 소유하는 현재 카메라의 행렬 저장소다.
// 카메라가 행렬을 설정하고 Refresh_CameraState가 역행렬과 월드 카메라 위치를 갱신한다.
// 반환 포인터는 내부 저장소를 차용하므로 객체 수명 안에서 사용하며 다음 갱신의 영향을 받는다.
class CPipeLine final
{
private:
	CPipeLine();
public:
	~CPipeLine();

public:
	// 계산 타입을 저장 타입으로 복사한다. 역행렬과 카메라 위치는 다음 Update에서 함께 갱신한다.
	void Set_Transform(D3DTS eType, fmatrix_t TransformMatrix);
	// 마지막 Update에서 구한 월드 카메라 위치다. 별도 할당이나 소유권 이전은 없다.
	const float4_t* Get_CamPosition();
	// eType은 END보다 작은 유효 enum이어야 한다. 이 경계에서 배열 범위를 다시 검사하지 않는다.
	const float4x4_t* Get_Transform(D3DTS eType);
	const float4x4_t* Get_InverseTransform(D3DTS eType);

public:
	// 지정한 shader 상수에 현재 카메라 값을 전달한다. 유효한 shader와 상수 이름은 호출자가 제공한다.
	HRESULT Bind_CamPos_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	// 원본 행렬을 이름으로 바인딩하며 shader의 실패 결과를 그대로 전달한다.
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	// 마지막 Update에서 계산한 역행렬을 전달한다. 이 함수가 역행렬을 다시 계산하지 않는다.
	HRESULT Bind_Inverse_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);

public:
	// GameInstance::Refresh_CameraState가 Frustum 갱신 전에 호출한다. 내부 잠금 없이 렌더 흐름에서 사용한다.
	void Update();

private:
	// 행벡터 규약의 View/Projection과 역행렬을 저장 타입으로 소유한다. 계산 때 XMLoad/XMStore를 사용한다.
	// 종류별 인덱스는 D3DTS와 일치하며 카메라 위치는 inverse View의 마지막 행이다.
	float4x4_t				m_TransformMatrices[ETOUI(D3DTS::END)] = {};
	float4x4_t				m_TransformInverseMatrices[ETOUI(D3DTS::END)] = {};
	float4_t				m_vCamPosition = {};

public:
	// GameInstance에 유일 소유권을 넘긴다. 유효한 카메라 행렬 설정은 이후 카메라 갱신이 담당한다.
	static unique_ptr<CPipeLine> Create();
};

NS_END