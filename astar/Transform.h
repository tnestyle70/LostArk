#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc
	{
		_float		fSpeedPerSec;
		_float		fRotationPerSec;
	}TRANSFORM_DESC;

private:
	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);	
	virtual ~CTransform() = default;

public:
	_vector Get_State(STATE eState) const {
		return XMLoadFloat4x4(&m_WorldMatrix).r[ENUM_TO_UINT(eState)];
	}

	_float3 Get_Scaled() const;

	_matrix Get_WorldMatrixInverse() {
		return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));
	}

	const _float4x4* Get_WorldMatrixPtr() const {
		return &m_WorldMatrix;
	}

	void Set_State(STATE eState, const _float4& vState) {
		memcpy(&m_WorldMatrix.m[ENUM_TO_UINT(eState)], &vState, sizeof vState);
	}
	void Set_State(STATE eState, _fvector vState) {
		XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[ENUM_TO_UINT(eState)]), vState);
	}
	void Set_Scale(_float fScaleX, _float fScaleY, _float fScaleZ);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName);

public:
	void Go_Straight(_float fTimeDelta, class CNavigation* pNavigation = nullptr );
	void Go_Astar(_float fTimeDelta, class CNavigation* pNavigation, _int iCellIndex);
	void Go_Left(_float fTimeDelta);
	void Go_Right(_float fTimeDelta);
	void Go_Backward(_float fTimeDelta);
	void Rotation(_fvector vAxis, _float fRadian);
	void Rotation(_float fRadianX, _float fRadianY, _float fRadianZ);
	void Turn(_fvector vAxis, _float fTimeDelta);
	void LookAt(_fvector vPoint);
	_bool Chase(_fvector vPoint, _float fTimeDelta, _float fMinDistance = 0.f);

private:
	_float			m_fSpeedPerSec = { };
	_float			m_fRotationPerSec = { };
	_float4x4		m_WorldMatrix = {};

private:
	list<_vector>	m_Route;

public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END