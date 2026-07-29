#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"

NS_BEGIN(Client)

class CValtan final : public CContainerObject
{
public:
	typedef struct tagValtanDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		float3_t vPosition = {};
	} VALTAN_DESC;

	enum VALTAN_STATE
	{
		IDLE = 0x00000001,
	};

private:
	CValtan(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CValtan();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	uint32_t m_iState = { VALTAN_STATE::IDLE };

private:
	HRESULT Ready_PartObjects();

public:
	static unique_ptr<CValtan> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
