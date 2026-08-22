#include "NpcPresentationAssetService.h"

#include "ActorCatalog.h"
#include "EffectV2_Runtime.h"
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
	std::map<uint32_t, std::set<std::string, std::less<>>> g_ReadyArchetypes;
	std::map<uint32_t, std::set<std::string, std::less<>>> g_ReadyAnimSets;
	std::unordered_set<uint32_t> g_NpcObjectReadyLevels;

	Engine::wstring_t Derive_ModelTag(const std::string& modelAssetId)
	{
		const std::filesystem::path assetPath(modelAssetId);
		const std::wstring stem = assetPath.stem().wstring();
		if (stem.empty())
			return {};
		return Engine::wstring_t(TEXT("Prototype_Component_Model_")) + stem;
	}

	Engine::wstring_t Derive_AnimSetTag(const std::string& animationSetId)
	{
		const std::filesystem::path assetPath(animationSetId);
		const std::wstring stem = assetPath.stem().wstring();
		if (stem.empty())
			return {};
		return Engine::wstring_t(TEXT("Prototype_Component_Model_AnimSet_")) +
			stem;
	}
}

void Client::CNpcPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_NpcAssetMutex };
	g_ReadyArchetypes.erase(iLevelIndex);
	g_ReadyAnimSets.erase(iLevelIndex);
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

	unique_ptr<CModel> bodyModel = CModel::Create(
		pDevice,
		pContext,
		MODEL::ANIM,
		modelPath.string().c_str(),
		preTransform);
	if (nullptr == bodyModel)
		return E_FAIL;

	wstring_t animSetTag;
	unique_ptr<CModel> newAnimSet;
	bool_t animSetAlreadyReady = false;
	if (!actor->animationSetId.empty())
	{
		animSetTag = Derive_AnimSetTag(actor->animationSetId);
		if (animSetTag.empty())
			return E_FAIL;
		const auto animSets = g_ReadyAnimSets.find(iLevelIndex);
		animSetAlreadyReady = animSets != g_ReadyAnimSets.end() &&
			animSets->second.contains(actor->animationSetId);
		shared_ptr<CModel> existingAnimSet;
		const CModel* pAnimSet = nullptr;
		if (animSetAlreadyReady)
		{
			existingAnimSet = dynamic_pointer_cast<CModel>(
				CGameInstance::Get().Clone_Prototype(
					iLevelIndex, animSetTag));
			pAnimSet = existingAnimSet.get();
		}
		else
		{
			const std::filesystem::path animSetPath =
				CRuntimeAssetRoot::Resolve(actor->animationSetId);
			if (animSetPath.empty())
				return E_FAIL;
			newAnimSet = CModel::Create(
				pDevice,
				pContext,
				MODEL::ANIM,
				animSetPath.string().c_str(),
				preTransform);
			pAnimSet = newAnimSet.get();
		}
		if (nullptr == pAnimSet ||
			FAILED(bodyModel->Attach_AnimationSet(*pAnimSet)))
		{
			return E_FAIL;
		}
	}

	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(3u);
	if (nullptr != newAnimSet)
		staged.emplace_back(animSetTag, std::move(newAnimSet));
	staged.emplace_back(modelTag, std::move(bodyModel));
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
	if (!actor->animationSetId.empty() && !animSetAlreadyReady)
		g_ReadyAnimSets[iLevelIndex].insert(actor->animationSetId);
	if (needsObjectPrototype)
		g_NpcObjectReadyLevels.insert(iLevelIndex);
	CEffectV2Runtime::Prewarm_Archetype(pDevice, pContext, std::string(archetypeId));
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
