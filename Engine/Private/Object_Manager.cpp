// Level/Layer 소유 저장소와 조회·등록·제거·프레임 전달을 구현한다.
#include "Object_Manager.h"
// 요청한 원형을 기존 Prototype_Manager 경계로 복제하고 그 결과만 Layer에 등록한다.
#include "GameInstance.h"

CObject_Manager::CObject_Manager()
{
}

CObject_Manager::~CObject_Manager()
{
}

// CGameInstance의 조회 요청을 Level과 Layer로 좁힌 뒤 현재 목록 순서로 객체를 찾는다.
shared_ptr<CGameObject> CObject_Manager::Get_GameObject(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_GameObject(iIndex);
}

// 일반 GameObject의 Component 조회다. 반환된 shared_ptr는 조회된 Component의 수명을 공유한다.
shared_ptr<CComponent> CObject_Manager::Get_Component(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	const wstring_t& strComponentTag,
	uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);

	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_Component(strComponentTag, iIndex);	
}

// part tag를 받는 경로는 해당 객체가 CContainerObject라는 호출자 전제를 따른다.
shared_ptr<CComponent> CObject_Manager::Get_Component(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	const wstring_t& strPartTag,
	const wstring_t& strComponentTag,
	uint32_t iIndex)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);

	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_Component(strPartTag, strComponentTag, iIndex);
}

// Level별 map 배열을 한 번 준비한다. 이미 초기화된 관리자의 기존 저장소는 덮어쓰지 않는다.
HRESULT CObject_Manager::Initialize(uint32_t iNumLevels)
{
	if (nullptr != m_pLayers)
		return E_FAIL;

	m_pLayers = unique_ptr<LAYERS[]>(new LAYERS[iNumLevels]);

	m_iNumLevels = iNumLevels;

	return S_OK;
}

// CGameInstance를 통해 원형을 복제하고 GameObject 타입인지 확인한 뒤 Layer에 넣는다.
// 새 Layer는 첫 객체 등록 후 map에 공개한다. 할당 예외를 포함한 전체 작업의 rollback은 구현하지 않는다.
HRESULT CObject_Manager::Add_GameObject_to_Layer(
	uint32_t iPrototypeLevelIndex,
	const wstring_t& strPrototypeTag,
	uint32_t iLayerLevelIndex,
	const wstring_t& strLayerTag,
	void* pArg,
	shared_ptr<CGameObject>* pOutGameObject)
{
	if (iLayerLevelIndex >= m_iNumLevels)
		return E_FAIL;

	auto	pGameObject = CGameInstance::Get().Clone_Prototype(iPrototypeLevelIndex, strPrototypeTag, pArg);
	if (nullptr == pGameObject)
		return E_FAIL;

	// 원형이 Component 등 다른 계열일 수도 있어 실제 GameObject 타입을 확인한다. 실패한 후보는 지역 소유 참조가 정리한다.
	auto pClonedGameObject = dynamic_pointer_cast<CGameObject>(pGameObject);
	if (nullptr == pClonedGameObject)
		return E_FAIL;

	auto		pLayer = Find_Layer(iLayerLevelIndex, strLayerTag);
	
	if (nullptr == pLayer)
	{
		/* 레이어를 새로 생성하여 추가한다. */
		// 아직 없는 tag이면 이 객체를 담을 Layer를 먼저 준비한다.
		auto	pNewLayer = CLayer::Create(iLayerLevelIndex, strLayerTag);

		if (FAILED(pNewLayer->Add_GameObject(pClonedGameObject)))
			return E_FAIL;

		m_pLayers[iLayerLevelIndex].emplace(strLayerTag, pNewLayer);
	}
	else
	{
		/* 기존 레이어에 그냥 추가한다. */
		// 기존 Layer의 소유 목록과 갱신 단계 목록에 함께 등록한다.
		if (FAILED(pLayer->Add_GameObject(pClonedGameObject)))
			return E_FAIL;
	}

	// 실제 Layer 등록까지 성공한 뒤에만 선택적 출력에 공유 소유권을 전달한다.
	if (nullptr != pOutGameObject)
		*pOutGameObject = pClonedGameObject;

	return S_OK;
}

// Layer가 비소유 단계/카메라 목록을 먼저 해제한 뒤 객체 소유 참조를 제거하도록 위임한다.
HRESULT CObject_Manager::Remove_GameObject_from_Layer(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	const shared_ptr<CGameObject>& pGameObject)
{
	if (iLevelIndex >= m_iNumLevels)
		return E_FAIL;

	CLayer* pLayer = Find_Layer(iLevelIndex, strLayerTag);
	if (nullptr == pLayer)
		return E_FAIL;

	return pLayer->Remove_GameObject(pGameObject);
}

// CGameInstance의 입력 처리 뒤, 카메라 갱신 전에 모든 Level 슬롯의 Priority 단계 객체를 갱신한다.
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

// 일반 객체 갱신 단계다. 어느 Level 슬롯을 순회할지는 현재 구현대로 전체 등록 슬롯을 사용한다.
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

// 물리 갱신이 끝난 뒤 객체가 결과를 소비하는 단계다.
void CObject_Manager::Post_Physics_Update(f32_t fTimeDelta)
{
	for (uint32_t i = 0; i < m_iNumLevels; ++i)
	{
		for (auto& Pair : m_pLayers[i])
		{
			if (nullptr != Pair.second)
				Pair.second->Post_Physics_Update(fTimeDelta);
		}
	}
}

// 구체 Level의 갱신 후 호출된다. 실제로 참여할 객체는 Layer에 등록된 단계 마스크가 결정한다.
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

// 최종 카메라 상태가 준비된 뒤 CGameInstance가 별도로 호출하는 렌더 제출 경로다.
void CObject_Manager::Submit_FinalCamera()
{
	for (uint32_t i = 0; i < m_iNumLevels; ++i)
		for (auto& pair : m_pLayers[i])
			if (pair.second)
				pair.second->Submit_FinalCamera();
}

// Level 전환의 Clear_Resources가 호출한다. Layer를 해제하며 외부 객체 참조까지 강제로 끊지 않는다.
HRESULT CObject_Manager::Clear(uint32_t iClearLevelID)
{
	if (iClearLevelID >= m_iNumLevels ||
		nullptr == m_pLayers)
		return E_FAIL;

	m_pLayers[iClearLevelID].clear();

	return S_OK;
}

// 반환 포인터는 map이 가진 shared_ptr의 차용이다. 해당 Level을 Clear한 뒤 계속 보관하면 안 된다.
CLayer* CObject_Manager::Find_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag)
{
	if (iLevelIndex >= m_iNumLevels || nullptr == m_pLayers)
		return nullptr;

	auto    iter = m_pLayers[iLevelIndex].find(strLayerTag);

	if (iter == m_pLayers[iLevelIndex].end())
		return nullptr;

	return iter->second.get();
}

// 초기화에 성공한 관리자만 CGameInstance에 넘긴다. 실패한 임시 객체는 unique_ptr가 정리한다.
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
