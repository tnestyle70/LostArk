#include "MonsterPresentationAssetService.h"

// 몬스터 지원 상태·모델 재질 입력·preScale/yaw를 조회한다.
#include "ActorCatalog.h"
// 완성한 Model/CNpc 원형 묶음을 Engine 저장소에 넘긴다.
#include "GameInstance.h"
// catalog 모델을 animated CModel 원형으로 생성한다.
#include "Model.h"
// 여러 archetype이 함께 쓰는 CNpc GameObject 원형을 준비한다.
#include "Npc.h"
// 기존 include 경계를 유지한다. 이 본문은 경로 해석을 ActorCatalog에 맡긴다.
#include "RuntimeAssetRoot.h"

// 기존 include를 보존한다. 이 본문에는 filesystem 직접 호출이 없다.
#include <filesystem>
// 준비 기록 검사부터 원형 등록까지 같은 서비스의 중복 준비를 막는다.
#include <mutex>
// Level별 archetype 준비 목록을 찾는다.
#include <unordered_map>
// 준비한 archetype과 공통 CNpc 원형 등록 Level을 기억한다.
#include <unordered_set>
// tag/원형 pair와 unique_ptr 소유권 이동을 사용한다.
#include <utility>
// 등록 전 임시 원형 묶음을 소유한다.
#include <vector>

namespace
{
	// 프로세스 수명 준비 기록이다. lock은 이 서비스 호출을 직렬화하며 Engine 전체의 thread 안전성을 대신하지 않는다.
	std::mutex g_MonsterAssetMutex;
	std::unordered_map<uint32_t, std::unordered_set<std::string>>
		g_ReadyArchetypesByLevel;
	std::unordered_set<uint32_t> g_ReadyObjectLevels;
	constexpr const wchar_t* MONSTER_OBJECT_PROTOTYPE =
		L"Prototype_GameObject_MonsterPresentation";
}

void Client::CMonsterPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_MonsterAssetMutex };
	g_ReadyArchetypesByLevel.erase(iLevelIndex);
	g_ReadyObjectLevels.erase(iLevelIndex);
}

// Loader 또는 replicated spawn이 호출한다. supported 정의만 모델·공통 객체 원형을 준비한다.
HRESULT Client::CMonsterPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext || archetypeId.empty() ||
		iLevelIndex >= ETOUI(LEVEL::END))
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_MonsterAssetMutex };
	auto& readyArchetypes = g_ReadyArchetypesByLevel[iLevelIndex];
	if (readyArchetypes.contains(std::string(archetypeId)))
		return S_FALSE;

	const MONSTER_ACTOR_ENTRY* actor = CActorCatalog::Find_Monster(archetypeId);
	if (nullptr == actor || actor->runtimeStatus != "supported")
		return E_FAIL;
    MODEL_ASSET_LOAD_DESC modelLoad;
    std::string materialStatus;
    if (!CActorCatalog::Build_ModelLoadDescription(actor->modelAssetId, modelLoad, materialStatus))
        return E_FAIL;

	const matrix_t preTransform =
		XMMatrixScaling(
			actor->modelScale, actor->modelScale, actor->modelScale) *
		XMMatrixRotationY(XMConvertToRadians(actor->modelYawDegrees));
	// 등록 전 자원은 임시 unique_ptr가 소유하므로 중간 준비 실패 시 정리된다.
	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.emplace_back(
		Get_ModelPrototypeTag(archetypeId),
		CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			modelLoad,
			preTransform));
	if (!g_ReadyObjectLevels.contains(iLevelIndex))
	{
		staged.emplace_back(
			MONSTER_OBJECT_PROTOTYPE,
			CNpc::Create(pDevice, pContext));
	}
	for (const auto& [tag, prototype] : staged)
	{
		(void)tag;
		if (nullptr == prototype)
			return E_FAIL;
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(
		iLevelIndex, std::move(staged))))
	{
		return E_FAIL;
	}

	// Engine 등록이 성공한 뒤에만 준비 완료 기록을 추가한다.
	readyArchetypes.insert(std::string(archetypeId));
	g_ReadyObjectLevels.insert(iLevelIndex);
	return S_OK;
}

std::wstring Client::CMonsterPresentationAssetService::Get_ModelPrototypeTag(
	const std::string_view archetypeId)
{
	std::wstring result = L"Prototype_Component_Model_Monster_";
	result.reserve(result.size() + archetypeId.size());
	for (const char character : archetypeId)
		result.push_back(static_cast<wchar_t>(
			static_cast<unsigned char>(character)));
	return result;
}

const wchar_t*
Client::CMonsterPresentationAssetService::Get_GameObjectPrototypeTag()
{
	return MONSTER_OBJECT_PROTOTYPE;
}
