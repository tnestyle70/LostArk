#include "ContainerObject.h"

#include "GameInstance.h"
#include "PartObject.h"

CContainerObject::CContainerObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CContainerObject::~CContainerObject()
{
}

shared_ptr<CComponent> CContainerObject::Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag)
{
	auto		pPartObject = Find_PartObject(strPartTag);
	if (nullptr == pPartObject)
		return nullptr;

	return pPartObject->Get_Component(strComponentTag);	
}

HRESULT CContainerObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CContainerObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CContainerObject::Priority_Update(f32_t fTimeDelta)
{
	for (auto Pair : m_PartObjects)
	{
		Pair.second->Priority_Update(fTimeDelta);
	}
}

void CContainerObject::Update(f32_t fTimeDelta)
{
	for (auto Pair : m_PartObjects)
	{
		Pair.second->Update(fTimeDelta);
	}
}

void CContainerObject::Late_Update(f32_t fTimeDelta)
{
	for (auto Pair : m_PartObjects)
	{
		Pair.second->Late_Update(fTimeDelta);
	}
}

HRESULT CContainerObject::Render()
{
	return S_OK;
}

HRESULT CContainerObject::Add_PartObject(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, const wstring_t& strPartObjectTag, void* pArg)
{
	if (nullptr != Find_PartObject(strPartObjectTag))
		return E_FAIL;

	shared_ptr<CPartObject> pCloneObject;
	if (FAILED(Clone_PartObject(
		iPrototypeLevelIndex, strPrototypeTag, pArg, pCloneObject)))
		return E_FAIL;

	m_PartObjects.emplace(strPartObjectTag, pCloneObject);

	return S_OK;
}

HRESULT CContainerObject::Clone_PartObject(
	const uint32_t iPrototypeLevelIndex,
	const wstring_t& strPrototypeTag,
	void* pArg,
	shared_ptr<CPartObject>& pOutPartObject)
{
	pOutPartObject.reset();
	if (strPrototypeTag.empty())
		return E_INVALIDARG;

	pOutPartObject = dynamic_pointer_cast<CPartObject>(
		CGameInstance::Get().Clone_Prototype(
			iPrototypeLevelIndex, strPrototypeTag, pArg));
	return nullptr != pOutPartObject ? S_OK : E_FAIL;
}

HRESULT CContainerObject::Replace_PartObjectGroup(
	const wstring_t& strPartObjectTagPrefix,
	PART_OBJECT_MAP&& CandidatePartObjects)
{
	if (strPartObjectTagPrefix.empty())
		return E_INVALIDARG;
	for (const auto& [tag, part] : CandidatePartObjects)
	{
		if (!tag.starts_with(strPartObjectTagPrefix) || nullptr == part)
			return E_INVALIDARG;
	}

	try
	{
		PART_OBJECT_MAP StagedPartObjects = m_PartObjects;
		for (auto iter = StagedPartObjects.begin();
			iter != StagedPartObjects.end();)
		{
			if (iter->first.starts_with(strPartObjectTagPrefix))
				iter = StagedPartObjects.erase(iter);
			else
				++iter;
		}

		for (auto& [tag, part] : CandidatePartObjects)
		{
			if (!StagedPartObjects.emplace(tag, std::move(part)).second)
				return E_FAIL;
		}
		m_PartObjects.swap(StagedPartObjects);
	}
	catch (...)
	{
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

CPartObject* CContainerObject::Find_PartObject(const wstring_t& strPartObjectTag)
{
	auto	iter = m_PartObjects.find(strPartObjectTag);
	if(iter == m_PartObjects.end())
		return nullptr;

	return iter->second.get();
}
