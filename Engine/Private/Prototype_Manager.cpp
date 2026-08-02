#include "Prototype_Manager.h"

CPrototype_Manager::CPrototype_Manager()
{
}

CPrototype_Manager::~CPrototype_Manager()
{
}

HRESULT CPrototype_Manager::Initialize(uint32_t iNumLevels)
{
    if (nullptr != m_pPrototypes)
        return E_FAIL;

    m_pPrototypes = unique_ptr<PROTOTYPES[]>(new PROTOTYPES[iNumLevels]);

    m_iNumLevels = iNumLevels;

	return S_OK;
}

HRESULT CPrototype_Manager::Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, unique_ptr<CPrototype> pPrototype)
{
    if (nullptr == m_pPrototypes || 
        iLevelIndex >= m_iNumLevels || 
        nullptr == pPrototype ||
        nullptr != Find_Prototype(iLevelIndex, strPrototypeTag))
        return E_FAIL;

    m_pPrototypes[iLevelIndex].emplace(strPrototypeTag, move(pPrototype));

    return S_OK;
}

shared_ptr<CPrototype> CPrototype_Manager::Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg)
{
    if (nullptr == m_pPrototypes ||
        iLevelIndex >= m_iNumLevels)
        return nullptr;

    auto        pPrototype = Find_Prototype(iLevelIndex, strPrototypeTag);
    if (nullptr == pPrototype)
        return nullptr;

    return pPrototype->Clone(pArg);
}

HRESULT CPrototype_Manager::Clear(uint32_t iClearLevelID)
{
    if (iClearLevelID >= m_iNumLevels ||
        nullptr == m_pPrototypes)
        return E_FAIL;

    m_pPrototypes[iClearLevelID].clear();

    return S_OK;
}

CPrototype* CPrototype_Manager::Find_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag)
{
    auto    iter = m_pPrototypes[iLevelIndex].find(strPrototypeTag);

    if(iter == m_pPrototypes[iLevelIndex].end())
        return nullptr;

    return iter->second.get();
}

unique_ptr<CPrototype_Manager> CPrototype_Manager::Create(uint32_t iNumLevels)
{
    auto pInstance = unique_ptr<CPrototype_Manager>(new CPrototype_Manager());

    if (FAILED(pInstance->Initialize(iNumLevels)))
    {
        MSG_BOX("Failed to Created : CPrototype_Manager");
        return nullptr;
    }

    return pInstance;
}
