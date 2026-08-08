#pragma once

#include "Layer.h"

/* 사본객체들을 레벨별로 그룹(CLayer)지어서 보관한다. */
/* 보관하고 있는객체들의 반복적인 업데이트 호출을 담당한다. */

NS_BEGIN(Engine)

class CObject_Manager final
{
private:
	CObject_Manager();
public:
	~CObject_Manager();

public:
	shared_ptr<CGameObject> Get_GameObject(uint32_t iLevelIndex, const wstring_t& strLayerTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);


public:
	HRESULT Initialize(uint32_t iNumLevels);
	HRESULT Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg, shared_ptr<CGameObject>* pOutGameObject = nullptr);
	HRESULT Remove_GameObject_from_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag, const shared_ptr<CGameObject>& pGameObject);
	void Priority_Update(f32_t fTimeDelta);
	void Update(f32_t fTimeDelta);
	void Post_Physics_Update(f32_t fTimeDelta);
	void Late_Update(f32_t fTimeDelta);
	HRESULT Clear(uint32_t iClearLevelID);



private:
	uint32_t			m_iNumLevels = {};

	unique_ptr<map<const wstring_t, shared_ptr<CLayer>>[]>		m_pLayers = { nullptr };
	typedef map<const wstring_t, shared_ptr<CLayer>>			LAYERS;

private:
	class CLayer* Find_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag);



public:
	static unique_ptr<CObject_Manager> Create(uint32_t iNumLevels);

};

NS_END
