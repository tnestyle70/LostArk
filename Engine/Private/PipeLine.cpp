// 현재 카메라 행렬 저장소의 공개 계약을 구현한다.
#include "PipeLine.h"

// 현재 행렬과 카메라 위치를 전달할 CShader의 실제 메서드 선언을 직접 읽는다.
#include "Shader.h"
// inverse View의 위치 행을 저장 타입으로 복사한다.
#include <cstring>

NS_BEGIN(Engine)

CPipeLine::CPipeLine()
{
}

CPipeLine::~CPipeLine()
{
}

// 카메라가 계산한 행렬을 CPU 저장소에 복사한다. 이 단계는 GPU 상수를 변경하지 않는다.
void CPipeLine::Set_Transform(D3DTS eType, fmatrix_t TransformMatrix)
{
	XMStoreFloat4x4(&m_TransformMatrices[ETOUI(eType)], TransformMatrix);
}

// 호출자는 내부 위치 값을 읽는 동안 CPipeLine의 수명과 갱신 순서를 보장한다.
const float4_t* CPipeLine::Get_CamPosition()
{
	return &m_vCamPosition;
}

// 현재 원본 행렬을 읽기 전용으로 차용한다. 입력 enum의 유효성은 호출자 전제다.
const float4x4_t* CPipeLine::Get_Transform(D3DTS eType)
{
	return &m_TransformMatrices[ETOUI(eType)];
}

// 마지막 카메라 상태 갱신에서 만든 역행렬을 차용한다.
const float4x4_t* CPipeLine::Get_InverseTransform(D3DTS eType)
{
	return &m_TransformInverseMatrices[ETOUI(eType)];
}

// shader의 원시 상수 복사 API에 월드 위치 4성분을 전달하고 그 결과를 반환한다.
HRESULT CPipeLine::Bind_CamPos_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return pShader->Bind_RawValue(pConstantName, &m_vCamPosition, sizeof(float4_t));
}

// 저장된 행렬을 지정 shader 상수로 옮긴다. 현재 행렬 저장소는 바꾸지 않는다.
HRESULT CPipeLine::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return pShader->Bind_Matrix(pConstantName, &m_TransformMatrices[ETOUI(eType)]);
}

// 역행렬 상수의 소비자도 같은 카메라 갱신 결과를 사용하도록 저장값을 바인딩한다.
HRESULT CPipeLine::Bind_Inverse_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return pShader->Bind_Matrix(pConstantName, &m_TransformInverseMatrices[ETOUI(eType)]);
}

// GameInstance::Refresh_CameraState가 Frustum 갱신 전에 호출한다. View는 world → camera, Projection은 camera → clip이다.
// 카메라가 유효한 가역 행렬을 설정해야 하며, 이 함수는 특이 행렬의 실패를 따로 보고하지 않는다.
void CPipeLine::Update()
{
	for (size_t i = 0; i < ETOUI(D3DTS::END); i++)
	{
		XMStoreFloat4x4(&m_TransformInverseMatrices[i],
			XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformMatrices[i])));
	}

	// 뷰의 역행렬은 카메라 로컬에서 월드로의 변환이다. 마지막 행의 위치와 w를 월드 카메라 값으로 복사한다.
	std::memcpy(&m_vCamPosition, &m_TransformInverseMatrices[ETOUI(D3DTS::VIEW)]._41, sizeof(float4_t));
}

// 생성된 저장소의 소유권을 GameInstance에 넘긴다. 할당 예외는 별도로 삼키지 않는다.
unique_ptr<CPipeLine> CPipeLine::Create()
{
	return unique_ptr<CPipeLine>(new CPipeLine());
}

NS_END
