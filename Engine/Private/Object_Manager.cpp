#include "Object_Manager.h"
#include "GameInstance.h"

CObject_Manager::CObject_Manager()
{
}

CObject_Manager::~CObject_Manager()
{
}

shared_ptr<CComponent> CObject_Manager::Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_Component(strComponentTag, iIndex);	
}

shared_ptr<CComponent> CObject_Manager::Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_Component(strPartTag, strComponentTag, iIndex);
}

HRESULT CObject_Manager::Initialize(uint32_t iNumLevels)
{
	if (nullptr != m_pLayers)
		return E_FAIL;

	m_pLayers = unique_ptr<LAYERS[]>(new LAYERS[iNumLevels]);

	m_iNumLevels = iNumLevels;

	return S_OK;
}

HRESULT CObject_Manager::Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg)
{
	auto	pGameObject = CGameInstance::Get().Clone_Prototype(iPrototypeLevelIndex, strPrototypeTag, pArg);
	if (nullptr == pGameObject)
		return E_FAIL;

	auto		pLayer = Find_Layer(iLayerLevelIndex, strLayerTag);
	
	if (nullptr == pLayer)
	{
		/* 레이어를 새로 생성하여 추가한다. */
		auto	pNewLayer = CLayer::Create();

		if (FAILED(pNewLayer->Add_GameObject(dynamic_pointer_cast<CGameObject>(pGameObject))))
			return E_FAIL;

		m_pLayers[iLayerLevelIndex].emplace(strLayerTag, pNewLayer);
	}
	else
	{
		/* 기존 레이어에 그냥 추가한다. */
		pLayer->Add_GameObject(dynamic_pointer_cast<CGameObject>(pGameObject));
	}

	return S_OK;
}

void CObject_Manager::Priority_Update(f32_t fTimeDelta)
{
	for (uint32_t i = 0; i < m_iNumLevels; ++i)
	{
		for (auto& Pair : m_pLayers[i])
		{
			if (nullptr != Pair.second)
				Pair.second->Priority_Update(fTimeDelta);
		}
	}
}

void CObject_Manager::Update(f32_t fTimeDelta)
{
	for (uint32_t i = 0; i < m_iNumLevels; ++i)
	{
		for (auto& Pair : m_pLayers[i])
		{
			if (nullptr != Pair.second)
				Pair.second->Update(fTimeDelta);
		}
	}
}

void CObject_Manager::Late_Update(f32_t fTimeDelta)
{
	for (uint32_t i = 0; i < m_iNumLevels; ++i)
	{
		for (auto& Pair : m_pLayers[i])
		{
			if (nullptr != Pair.second)
				Pair.second->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CObject_Manager::Clear(uint32_t iClearLevelID)
{
	if (iClearLevelID >= m_iNumLevels ||
		nullptr == m_pLayers)
		return E_FAIL;

	m_pLayers[iClearLevelID].clear();

	return S_OK;
}

CLayer* CObject_Manager::Find_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag)
{
	auto    iter = m_pLayers[iLevelIndex].find(strLayerTag);

	if (iter == m_pLayers[iLevelIndex].end())
		return nullptr;

	return iter->second.get();
}

unique_ptr<CObject_Manager> CObject_Manager::Create(uint32_t iNumLevels)
{
	auto pInstance = unique_ptr<CObject_Manager>(new CObject_Manager());

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX("Failed to Created : CPrototype_Manager");
		return nullptr;
	}

	return pInstance;
}
