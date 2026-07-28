#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	typedef struct tagCameraDesc : public CGameObject::GAMEOBJECT_DESC
	{
		float3_t	vEye, vAt;
		f32_t		fFovy, fNear, fFar;
	}CAMERA_DESC;

protected:
	CCamera(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CCamera();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	f32_t				m_fFovy = {}, m_fAspect = {}, m_fNear = {}, m_fFar = {};
	
protected:
	void Update_PipeLine();


public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
};

NS_END