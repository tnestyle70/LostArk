#pragma once

#include "GameObject.h"
#include <array>

NS_BEGIN(Engine)

class CLayer
{
private:
	CLayer(uint32_t levelIndex, const wstring_t& layerTag);
public:
	~CLayer();

public:
	shared_ptr<CGameObject> Get_GameObject(uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(const wstring_t& strComponentTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);


public:
	HRESULT Add_GameObject(shared_ptr<CGameObject> pGameObject);
	HRESULT Remove_GameObject(const shared_ptr<CGameObject>& pGameObject);
	virtual void Priority_Update(f32_t fTimeDelta);
	virtual void Update(f32_t fTimeDelta);
	virtual void Post_Physics_Update(f32_t fTimeDelta);
	virtual void Late_Update(f32_t fTimeDelta);
	void Submit_FinalCamera();
private:
	list<shared_ptr<CGameObject>>		m_GameObjects;
	// Built once with the layer identity; no per-object scopes or per-frame formatting.
	std::array<std::string, 4> m_ProfileScopeNames;
	// Non-owning phase membership. m_GameObjects owns every pointer until removal.
	std::vector<CGameObject*> m_FinalCameraObjects;
	std::array<std::list<CGameObject*>, 4> m_PhaseObjects;
	std::string m_FinalCameraScopeName;

public:
	static shared_ptr<CLayer> Create(uint32_t levelIndex, const wstring_t& layerTag);
};

NS_END
