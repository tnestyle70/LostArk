#include "NpcPresentationAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"

#include <filesystem>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	std::mutex g_NpcAssetMutex;
	/* level index -> archetypes whose model prototypes are committed there */
	std::map<uint32_t, std::set<std::string, std::less<>>> g_ReadyArchetypes;
	/* levels that already committed the shared CNpc GameObject prototype */
	std::unordered_set<uint32_t> g_NpcObjectReadyLevels;

	Engine::wstring_t Derive_ModelTag(const std::string& modelAssetId)
	{
		const std::filesystem::path assetPath(modelAssetId);
		const std::wstring stem = assetPath.stem().wstring();
		if (stem.empty())
			return {};
		return Engine::wstring_t(TEXT("Prototype_Component_Model_")) + stem;
	}
}

void Client::CNpcPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_NpcAssetMutex };
	g_ReadyArchetypes.erase(iLevelIndex);
	g_NpcObjectReadyLevels.erase(iLevelIndex);
}

HRESULT Client::CNpcPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) || archetypeId.empty())
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_NpcAssetMutex };
	const auto readyLevel = g_ReadyArchetypes.find(iLevelIndex);
	if (readyLevel != g_ReadyArchetypes.end() &&
		readyLevel->second.contains(archetypeId))
	{
		return S_FALSE;
	}

	const NPC_ACTOR_ENTRY* actor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == actor || actor->runtimeStatus != "supported" ||
		actor->clientPresentationId.empty())
	{
		return E_FAIL;
	}
	const wstring_t modelTag = Derive_ModelTag(actor->modelAssetId);
	if (modelTag.empty())
		return E_FAIL;
	const std::filesystem::path modelPath =
		CRuntimeAssetRoot::Resolve(actor->modelAssetId);
	if (modelPath.empty())
		return E_FAIL;

	const matrix_t preTransform =
		XMMatrixScaling(0.0001f, 0.0001f, 0.0001f) *
		XMMatrixRotationY(XMConvertToRadians(-90.f));
	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(2u);
	staged.emplace_back(
		modelTag,
		CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			modelPath.string().c_str(),
			preTransform));
	const bool_t needsObjectPrototype =
		!g_NpcObjectReadyLevels.contains(iLevelIndex);
	if (needsObjectPrototype)
	{
		staged.emplace_back(
			TEXT("Prototype_GameObject_Npc"),
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

	g_ReadyArchetypes[iLevelIndex].insert(std::string(archetypeId));
	if (needsObjectPrototype)
		g_NpcObjectReadyLevels.insert(iLevelIndex);
	return S_OK;
}

bool_t Client::CNpcPresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	std::scoped_lock lock{ g_NpcAssetMutex };
	const auto readyLevel = g_ReadyArchetypes.find(iLevelIndex);
	return readyLevel != g_ReadyArchetypes.end() &&
		readyLevel->second.contains(archetypeId);
}

Engine::wstring_t Client::CNpcPresentationAssetService::Get_ModelPrototypeTag(
	const std::string_view archetypeId)
{
	const NPC_ACTOR_ENTRY* actor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == actor)
		return {};
	return Derive_ModelTag(actor->modelAssetId);
}
