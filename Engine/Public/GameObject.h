#pragma once

#include "GameInstance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CPrototype
{
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{

	}GAMEOBJECT_DESC;
protected:
	CGameObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CGameObject();

public:
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_DeferredOverlay();
	virtual HRESULT Render_Shadow();

protected:
	map<const wstring_t, shared_ptr<CComponent>>		m_Components;

protected:
	shared_ptr<class CTransform>			m_pTransformCom = { nullptr };


	template<typename T>
	HRESULT Add_Component(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, const wstring_t& strComponentTag, shared_ptr<T>& pOut, void* pArg = nullptr)
	{
		if (nullptr != Find_Component(strComponentTag))
			return E_FAIL;

		shared_ptr<CComponent> pComponent = dynamic_pointer_cast<CComponent>(CGameInstance::Get().Clone_Prototype(iPrototypeLevelIndex, strPrototypeTag, pArg));
		if (nullptr == pComponent)
			return E_FAIL;

		m_Components.emplace(strComponentTag, pComponent);

		pOut = dynamic_pointer_cast<T>(pComponent);

		return S_OK;
	}
protected:
	CComponent* Find_Component(const wstring_t& strComponentTag);

public:
	virtual shared_ptr<CPrototype> Clone(void* pArg) = 0;
	void Free();

};

NS_END
