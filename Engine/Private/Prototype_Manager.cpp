// Level별 원형 소유 배열과 tag 조회·단일/일괄 등록·Clone 전달을 구현한다.
#include "Prototype_Manager.h"

CPrototype_Manager::CPrototype_Manager()
{
}

CPrototype_Manager::~CPrototype_Manager()
{
}

// Create가 빈 Level 슬롯 배열을 준비한다. 이미 준비됐으면 E_FAIL이며 기존 저장소를 유지한다. 0개 요청을 별도로 거부하지 않는다.
HRESULT CPrototype_Manager::Initialize(uint32_t iNumLevels)
{
    if (nullptr != m_pPrototypes)
        return E_FAIL;

    m_pPrototypes = unique_ptr<PROTOTYPES[]>(new PROTOTYPES[iNumLevels]);

    m_iNumLevels = iNumLevels;

	return S_OK;
}

// CGameInstance가 넘긴 원형을 한 Level의 tag에 등록한다. 입력 소유권을 값으로 받으므로 검증 실패 시 새 원형도 해제된다.
HRESULT CPrototype_Manager::Add_Prototype(
	uint32_t iLevelIndex,
	const wstring_t& strPrototypeTag,
	unique_ptr<CPrototype> pPrototype)
{
    if (nullptr == m_pPrototypes || 
        iLevelIndex >= m_iNumLevels || 
        nullptr == pPrototype ||
        nullptr != Find_Prototype(iLevelIndex, strPrototypeTag))
        return E_FAIL;

    // 이 함수는 unique_ptr를 값으로 받는다. 위 검증에서 실패한 입력 원형도 반환 때 정리된다.
    // 성공하면 map으로 소유권을 옮기므로 호출자가 같은 원형을 다시 해제하지 않는다.
    m_pPrototypes[iLevelIndex].emplace(strPrototypeTag, move(pPrototype));

	return S_OK;
}

// 로더의 묶음 등록은 저장소/Level 검사 → 전체 tag 검사 → 임시 원형 소유 → 노드 병합 순서로 진행한다.
HRESULT CPrototype_Manager::Add_Prototypes(
	const uint32_t iLevelIndex,
	vector<pair<wstring_t, unique_ptr<CPrototype>>>&& prototypes)
{
	if (nullptr == m_pPrototypes ||
		iLevelIndex >= m_iNumLevels ||
		prototypes.empty())
	{
		return E_INVALIDARG;
	}

	// 먼저 값이 빈 map으로 tag 중복만 검사한다. 이 검증 중에는 입력 원형을 이동하지 않는다.
	PROTOTYPES staged;
	for (const auto& [tag, prototype] : prototypes)
	{
		if (tag.empty() || nullptr == prototype ||
			nullptr != Find_Prototype(iLevelIndex, tag) ||
			staged.contains(tag))
		{
			return E_FAIL;
		}
		staged.emplace(tag, nullptr);
	}
	// 검증이 끝나면 실제 원형을 임시 map으로 이동한다. 이후 할당 예외가 나면
	// 임시 map은 자신이 받은 원형을 정리하지만, 이미 이동한 입력 항목을 복원하지는 않는다.
	staged.clear();
	for (auto& [tag, prototype] : prototypes)
	{
		staged.emplace(std::move(tag), std::move(prototype));
	}

	// 노드는 이미 임시 map에 할당되어 있다. 기존 map에 없는 key만 검증했으므로
	// merge는 노드를 넘겨 등록하며 원형을 다시 복제하지 않는다.
	m_pPrototypes[iLevelIndex].merge(staged);
	return staged.empty() ? S_OK : E_FAIL;
}

// CGameInstance가 조회한 원형의 구체 Clone에 pArg를 전달한다. 잘못된 Level/없는 tag는 nullptr이며 이 관리자가 입력을 보관하지 않는다.
shared_ptr<CPrototype> CPrototype_Manager::Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg)
{
    if (nullptr == m_pPrototypes ||
        iLevelIndex >= m_iNumLevels)
        return nullptr;

    auto        pPrototype = Find_Prototype(iLevelIndex, strPrototypeTag);
    if (nullptr == pPrototype)
        return nullptr;

    // 구체 타입의 Clone이 복제·초기화를 수행한다. nullptr 실패도 그대로 호출자에게 전달한다.
    return pPrototype->Clone(pArg);
}

// Level 자원 정리 경로가 해당 슬롯의 원형 참조만 해제한다. 범위/준비 상태 오류는 E_FAIL이고 다른 슬롯은 변경하지 않는다.
HRESULT CPrototype_Manager::Clear(uint32_t iClearLevelID)
{
    if (iClearLevelID >= m_iNumLevels ||
        nullptr == m_pPrototypes)
        return E_FAIL;

    // 등록 원형만 해제한다. Clone이 공유한 자원은 남아 있는 소유 참조의 수명을 따른다.
    m_pPrototypes[iClearLevelID].clear();

    return S_OK;
}

// 공개 진입점이 배열과 Level 범위를 확인한 뒤 사용한다. 없는 tag는 nullptr이고 반환한 원형의 소유권은 map에 남는다.
CPrototype* CPrototype_Manager::Find_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag)
{
    auto    iter = m_pPrototypes[iLevelIndex].find(strPrototypeTag);

    if(iter == m_pPrototypes[iLevelIndex].end())
        return nullptr;

    return iter->second.get();
}

// CGameInstance가 소유할 저장소를 생성·초기화한다. HRESULT 실패 경로에서는 지역 unique_ptr가 후보를 정리한다.
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
