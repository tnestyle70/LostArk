#pragma once

#include "Client_Defines.h"
#include "ContainerObject.h"

NS_BEGIN(Client)

class CLanceMaster final : public CContainerObject
{
public:
	typedef struct tagLanceMasterDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		float3_t vPosition = {};
	} LANCEMASTER_DESC;

	enum LANCEMASTER_STATE
	{
		IDLE = 0x00000001,
	};

private:
	CLanceMaster(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CLanceMaster();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	uint32_t m_iState = { LANCEMASTER_STATE::IDLE };

private:
	HRESULT Ready_PartObjects();

public:
	static unique_ptr<CLanceMaster> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
