#include "MonsterPresentationAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"

#include <filesystem>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
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
	const std::filesystem::path modelPath =
		CRuntimeAssetRoot::Resolve(actor->modelAssetId);
	if (modelPath.empty())
		return E_FAIL;

	const matrix_t preTransform =
		XMMatrixScaling(
			actor->modelScale, actor->modelScale, actor->modelScale) *
		XMMatrixRotationY(XMConvertToRadians(actor->modelYawDegrees));
	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.emplace_back(
		Get_ModelPrototypeTag(archetypeId),
		CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			modelPath.string().c_str(),
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
