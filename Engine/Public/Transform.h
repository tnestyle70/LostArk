#pragma once

#include "Component.h"

/* 월드 공간에서의 상태를 담당할 월드변환행렬을 보관한다. */
/* 이 행렬의 상태를 제어하는 다양한 기능을 보유한다. */


NS_BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc
	{
		f32_t		fSpeedPerSec = {};
		f32_t		fRotationPerSec = {};
	}TRANSFORM_DESC;
private:
	CTransform(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CTransform();

public:
	vector_t Get_State(STATE eState) {
		return XMLoadFloat4x4(&m_WorldMatrix).r[ETOUI(eState)];
	}

	const float4x4_t* Get_WorldMatrixPtr() {
		return &m_WorldMatrix;
	}

	const float3_t Get_Scaled();

	void Set_State(STATE eState, fvector_t vState) {
		XMStoreFloat4(reinterpret_cast<float4_t*>(&m_WorldMatrix.m[ETOUI(eState)][0]), vState);
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName);

public:
	void Scale(f32_t fScaleX = 1.f, f32_t fScaleY = 1.f, f32_t fScaleZ = 1.f);
	void Scaling(f32_t fScaleX = 1.f, f32_t fScaleY = 1.f, f32_t fScaleZ = 1.f);
	void Go_Straight(f32_t fTimeDelta, shared_ptr<class CNavigation> pNavigation = nullptr);
	void Go_Backward(f32_t fTimeDelta);
	void Go_Left(f32_t fTimeDelta);
	void Go_Right(f32_t fTimeDelta);

	void Rotation(fvector_t vAxis, f32_t fDegree);
	void Rotation(f32_t fDegreeX, f32_t fDegreeY, f32_t fDegreeZ);
	void Turn(fvector_t vAxis, f32_t fTimeDelta);

	void LookAt(fvector_t vAt);
	void Chase(fvector_t vGoal, f32_t fTimeDelta, f32_t fLimitDistance = 0.1f);

private:
	float4x4_t					m_WorldMatrix = {};
	f32_t						m_fSpeedPerSec = {};
	f32_t						m_fRotationPerSec = {};

public:
	static shared_ptr<CTransform> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END