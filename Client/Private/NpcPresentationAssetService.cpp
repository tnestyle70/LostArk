#include "NpcPresentationAssetService.h"

#include "ActorCatalog.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"

#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	std::mutex g_NpcAssetMutex;
	std::unordered_set<uint32_t> g_ReadyLevels;
	constexpr const char* SUPPORTED_ARCHETYPE = "NPC_BEDA";
}

void Client::CNpcPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_NpcAssetMutex };
	g_ReadyLevels.erase(iLevelIndex);
}

HRESULT Client::CNpcPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) ||
		archetypeId != SUPPORTED_ARCHETYPE)
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_NpcAssetMutex };
	if (g_ReadyLevels.contains(iLevelIndex))
		return S_FALSE;

	const NPC_ACTOR_ENTRY* actor = CActorCatalog::Find_Npc(archetypeId);
	if (nullptr == actor || actor->runtimeStatus != "supported" ||
		actor->clientPresentationId != "npc.beda.client.v1")
	{
		return E_FAIL;
	}
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
		TEXT("Prototype_Component_Model_Npc_Beda"),
		CModel::Create(
			pDevice,
			pContext,
			MODEL::ANIM,
			modelPath.string().c_str(),
			preTransform));
	staged.emplace_back(
		TEXT("Prototype_GameObject_Npc"),
		CNpc::Create(pDevice, pContext));
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

	g_ReadyLevels.insert(iLevelIndex);
	return S_OK;
}

bool_t Client::CNpcPresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (archetypeId != SUPPORTED_ARCHETYPE)
		return false;
	std::scoped_lock lock{ g_NpcAssetMutex };
	return g_ReadyLevels.contains(iLevelIndex);
}
