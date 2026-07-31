#include "Layer.h"

#include "GameInstance.h"
#include "ContainerObject.h"

CLayer::CLayer()
{
}

CLayer::~CLayer()
{
}

shared_ptr<CGameObject> CLayer::Get_GameObject(uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto iter = m_GameObjects.begin();
	for (uint32_t index = 0; index < iIndex; ++index)
		++iter;

	return iter != m_GameObjects.end() ? *iter : nullptr;
}

shared_ptr<CComponent> CLayer::Get_Component(const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (uint32_t i = 0; i < iIndex; i++)
		++iter;

	if(iter == m_GameObjects.end())
		return nullptr;

	return (*iter)->Get_Component(strComponentTag);
}

shared_ptr<CComponent> CLayer::Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	if (iIndex >= m_GameObjects.size())
		return nullptr;

	auto	iter = m_GameObjects.begin();

	for (uint32_t i = 0; i < iIndex; i++)
		++iter;

	if (iter == m_GameObjects.end())
		return nullptr;

	return static_pointer_cast<CContainerObject>(*iter)->Get_Component(strPartTag, strComponentTag);
}

HRESULT CLayer::Add_GameObject(shared_ptr<CGameObject> pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	m_GameObjects.push_back(pGameObject);

	return S_OK;
}

HRESULT CLayer::Remove_GameObject(const shared_ptr<CGameObject>& pGameObject)
{
	if (nullptr == pGameObject)
		return E_FAIL;

	auto iter = find(m_GameObjects.begin(), m_GameObjects.end(), pGameObject);
	if (iter == m_GameObjects.end())
		return E_FAIL;

	m_GameObjects.erase(iter);
	return S_OK;
}

void CLayer::Priority_Update(f32_t fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Priority_Update(fTimeDelta);
	}
}

void CLayer::Update(f32_t fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Update(fTimeDelta);
	}
}

void CLayer::Late_Update(f32_t fTimeDelta)
{
	for (auto& pGameObject : m_GameObjects)
	{
		if (nullptr != pGameObject)
			pGameObject->Late_Update(fTimeDelta);
	}
}

shared_ptr<CLayer> CLayer::Create()
{
	return shared_ptr<CLayer>(new CLayer());
}
