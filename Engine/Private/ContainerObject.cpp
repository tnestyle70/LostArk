// 자식 Part의 공유 소유와 그룹 교체, 갱신 전달을 구현한다.
#include "ContainerObject.h"

// Part 후보를 기존 원형 저장소에서 복제한다. 새 소유 경로나 별도 원형 저장소를 만들지 않는다.
#include "GameInstance.h"
// Clone 결과의 동적 타입을 검사하고 자식의 Component/갱신 함수를 호출한다.
#include "PartObject.h"

CContainerObject::CContainerObject(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CContainerObject::~CContainerObject()
{
}

// Layer의 part 조회나 구체 캐릭터가 사용한다. part가 없으면 Component 조회를 시도하지 않는다.
shared_ptr<CComponent> CContainerObject::Get_Component(const wstring_t& strPartTag, const wstring_t& strComponentTag)
{
	auto		pPartObject = Find_PartObject(strPartTag);
	if (nullptr == pPartObject)
		return nullptr;

	return pPartObject->Get_Component(strComponentTag);	
}

// 구체 Container가 원형 준비를 덧붙이는 기본 성공 경로다. 이 기반에는 등록할 자식 원형이 없다.
HRESULT CContainerObject::Initialize_Prototype()
{
	return S_OK;
}

// 부모 GameObject의 Transform 초기화를 먼저 수행한다. Part 생성은 구체 Container 초기화가 담당한다.
HRESULT CContainerObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

// 부모의 프레임 콜백에서 등록된 모든 Part에 같은 초 단위 delta를 전달한다.
// 세 전달 루프는 map 순서이며 여기서는 자식의 phase mask를 다시 필터링하지 않는다.
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

// 기반 Container는 직접 그리지 않는다. 각 Part의 렌더 제출은 구체 Part/Container 구현을 따른다.
HRESULT CContainerObject::Render()
{
	return S_OK;
}

// 구체 캐릭터/보스 초기화가 단일 Part를 추가하는 경로다. 중복/타입 실패에서는 기존 map을 바꾸지 않는다.
HRESULT CContainerObject::Add_PartObject(
	uint32_t iPrototypeLevelIndex,
	const wstring_t& strPrototypeTag,
	const wstring_t& strPartObjectTag,
	void* pArg)
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

// Character/Valtan 등의 그룹 교체도 사용한다. 후보를 복제만 해 두어 실제 map 교체와 분리한다.
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

// Character/Valtan의 장비·외형 등 같은 접두사 그룹을 한 번에 교체한다.
// 기존 map 복사본에서 제거와 삽입을 끝낸 뒤 swap하므로 실패 시 현재 표시 그룹은 유지된다.
// CandidatePartObjects에서 이미 이동한 shared_ptr는 복원하지 않으며, 잡힌 모든 예외를 E_OUTOFMEMORY로 반환한다.
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
		// 기존 소유 참조를 지역 map에 복사한 뒤 대상 접두사만 제거해 나머지 그룹을 보존한다.
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
		// 모든 후보 삽입이 끝난 시점에 소유 map을 교체한다. 이전 그룹은 지역 map의 소멸 때 참조가 해제된다.
		m_PartObjects.swap(StagedPartObjects);
	}
	catch (...)
	{
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

// map의 소유 참조를 차용한다. 그룹 교체로 해당 항목이 사라진 뒤 포인터를 계속 사용하면 안 된다.
CPartObject* CContainerObject::Find_PartObject(const wstring_t& strPartObjectTag)
{
	auto	iter = m_PartObjects.find(strPartObjectTag);
	if(iter == m_PartObjects.end())
		return nullptr;

	return iter->second.get();
}
