#include "ValtanPresentationAssetService.h"

#include "ActorCatalog.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Body_Valtan.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "Model.h"
#include "RuntimeAssetRoot.h"
#include "Valtan.h"

#include <mutex>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	std::mutex g_ValtanAssetMutex;
	std::unordered_map<uint32_t, std::unordered_set<std::string>> g_ReadyByLevel;

	wstring_t ModelPrototypeTag(
		const wchar_t* baseTag, const std::string_view archetypeId)
	{
		const Client::BOSS_ACTOR_ENTRY* actor =
			Client::CActorCatalog::Find_Boss(archetypeId);
		if (nullptr == actor || actor->clientPresentationId != "boss.valtan.client.v1")
			return {};
		if ("BOSS_VALTAN" == archetypeId)
			return baseTag;
		return wstring_t(baseTag) + L"_" +
			wstring_t(archetypeId.begin(), archetypeId.end());
	}

	HRESULT RejectAsset(const std::string_view archetypeId, const std::string& reason)
	{
		OutputDebugStringA(("[ValtanPresentation] " + std::string(archetypeId) +
			" admission rejected: " + reason + "\n").c_str());
		return E_FAIL;
	}

	bool HasClip(const Engine::CModel& model, const std::string& clip)
	{
		for (uint32_t index = 0u; index < model.Get_NumAnimations(); ++index)
		{
			const char* name = model.Get_AnimationName(index);
			if (nullptr != name && clip == name)
				return true;
		}
		return false;
	}
}

void Client::CValtanPresentationAssetService::Begin_LevelLoad(
	const uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_ValtanAssetMutex };
	g_ReadyByLevel.erase(iLevelIndex);
}

wstring_t Client::CValtanPresentationAssetService::Get_BodyModelPrototypeTag(
	const std::string_view archetypeId)
{
	return ModelPrototypeTag(TEXT("Prototype_Component_Model_Valtan"), archetypeId);
}

wstring_t Client::CValtanPresentationAssetService::Get_WeaponModelPrototypeTag(
	const std::string_view archetypeId)
{
	return ModelPrototypeTag(TEXT("Prototype_Component_Model_ValtanWeapon"), archetypeId);
}

HRESULT Client::CValtanPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END))
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_ValtanAssetMutex };
	const auto ready = g_ReadyByLevel.find(iLevelIndex);
	const bool hasSharedObjectPrototypes = ready != g_ReadyByLevel.end();
	if (hasSharedObjectPrototypes && ready->second.contains(std::string(archetypeId)))
		return S_FALSE;

	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss(archetypeId);
	const wstring_t bodyTag = Get_BodyModelPrototypeTag(archetypeId);
	const wstring_t weaponTag = Get_WeaponModelPrototypeTag(archetypeId);
	if (nullptr == pActor || bodyTag.empty() || weaponTag.empty())
		return RejectAsset(archetypeId, "no supported boss catalog definition");

	const std::filesystem::path bodyPath =
		CRuntimeAssetRoot::Resolve(pActor->bodyModel);
	const std::filesystem::path weaponPath =
		CRuntimeAssetRoot::Resolve(pActor->weaponModel);
	const std::filesystem::path animSetPath =
		CRuntimeAssetRoot::Resolve(pActor->animationSetId);
	if (bodyPath.empty() || weaponPath.empty() || animSetPath.empty())
		return RejectAsset(archetypeId, "invalid Resources-relative model path");
	std::vector<std::pair<uint32_t, std::filesystem::path>> armorAssets;
	armorAssets.reserve(pActor->armorParts.size());
	for (const BOSS_ARMOR_PART_ENTRY& armorPart : pActor->armorParts)
	{
		std::filesystem::path armorPath =
			CRuntimeAssetRoot::Resolve(armorPart.modelAssetId);
		if (armorPath.empty())
			return RejectAsset(archetypeId, "invalid armor model path");
		armorAssets.emplace_back(armorPart.stateMask, std::move(armorPath));
	}
	std::error_code animSetFileError;
	const bool_t hasAnimSetFile = std::filesystem::is_regular_file(
		animSetPath, animSetFileError);
	const bool_t isMissingAnimSetFile = !hasAnimSetFile &&
		(ERROR_FILE_NOT_FOUND == animSetFileError.value() ||
			ERROR_PATH_NOT_FOUND == animSetFileError.value());
	if (!hasAnimSetFile && (!isMissingAnimSetFile || "BOSS_VALTAN" != archetypeId))
		return RejectAsset(archetypeId, "animation donor unavailable: " + pActor->animationSetId);

#ifndef _DEBUG
	if (isMissingAnimSetFile)
		return RejectAsset(archetypeId, "animation donor unavailable: " + pActor->animationSetId);
#else
	if (!hasAnimSetFile)
	{
		::OutputDebugStringA(
			"[ValtanPresentation] The authored 146-clip animation set is missing. "
			"Debug will continue with the clips embedded in the Valtan body model; "
			"Release remains fail-closed. Missing asset: "
			"Character/Valtan/AnimSets/MN_RPBF_01_AnimSet.wmodel\n");
	}
#endif

	const float bodyScale = pActor->bodyModelPreScale;
	const float weaponScale = pActor->weaponModelPreScale;
	unique_ptr<CModel> bodyModel = CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyPath.string().c_str(),
		XMMatrixScaling(bodyScale, bodyScale, bodyScale));
	if (nullptr == bodyModel || 0u == bodyModel->Get_NumMeshes() ||
		0u == bodyModel->Get_SkeletonHash() ||
		!bodyModel->Has_Bone(CValtan::WEAPON_SOCKET_BONE))
	{
		return RejectAsset(archetypeId, "body geometry/skeleton/weapon socket unavailable: " +
			pActor->bodyModel);
	}
	if (hasAnimSetFile)
	{
		const unique_ptr<CModel> animSetModel = CModel::Create(
			pDevice, pContext, MODEL::ANIM, animSetPath.string().c_str(),
			XMMatrixScaling(bodyScale, bodyScale, bodyScale));
		/* CModel compares both skeleton hash and bone count and rejects duplicate
		clip names before attaching. All admission stays local until Add_Prototypes. */
		if (nullptr == animSetModel || !animSetModel->Has_Animations() ||
			FAILED(bodyModel->Attach_AnimationSet(*animSetModel)))
		{
			return RejectAsset(archetypeId, "animation donor does not match the body: " +
				pActor->animationSetId);
		}
	}
	if (!bodyModel->Has_Animations())
		return RejectAsset(archetypeId, "combined body has no animation clips");
	if ("BOSS_VALTAN" != archetypeId)
	{
		const auto& clips = pActor->presentationClips;
		// Death is optional: an unavailable death clip retires the corpse immediately.
		for (const std::string* clip : { &clips.idle, &clips.chase, &clips.patternWindup,
			&clips.patternActive, &clips.patternRecovery })
		{
			if (!HasClip(*bodyModel, *clip))
				return RejectAsset(archetypeId, "combined body is missing catalog clip " + *clip);
		}
	}

	std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
	staged.reserve(4u + armorAssets.size());
	staged.emplace_back(bodyTag, std::move(bodyModel));
	staged.emplace_back(weaponTag,
		CModel::Create(pDevice, pContext, MODEL::NONANIM, weaponPath.string().c_str(),
			XMMatrixScaling(weaponScale, weaponScale, weaponScale)));
	for (const auto& [stateMask, armorPath] : armorAssets)
	{
		unique_ptr<CModel> armorModel = CModel::Create(
			pDevice, pContext, MODEL::ANIM, armorPath.string().c_str(),
			XMMatrixScaling(bodyScale, bodyScale, bodyScale));
		/* Keep the established isolated-plate failure policy for ordinary Valtan. */
		if (nullptr == armorModel)
		{
			OutputDebugStringA(("[Client][Valtan] armour plate rejected: " +
				armorPath.string() + " | " +
				CModelDecoderRegistry::Get().Get_LastReport().error + "\n").c_str());
			continue;
		}
		staged.emplace_back(CValtan::Build_ArmorModelPrototypeTag(stateMask, archetypeId),
			std::move(armorModel));
	}
	if (!hasSharedObjectPrototypes)
	{
		staged.emplace_back(TEXT("Prototype_GameObject_Body_Valtan"),
			CBody_Valtan::Create(pDevice, pContext));
		staged.emplace_back(TEXT("Prototype_GameObject_Valtan"),
			CValtan::Create(pDevice, pContext));
	}
	for (const auto& [tag, prototype] : staged)
	{
		(void)tag;
		if (nullptr == prototype)
			return RejectAsset(archetypeId, "a required prototype could not be created");
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return RejectAsset(archetypeId, "atomic prototype registration failed");

	g_ReadyByLevel[iLevelIndex].insert(std::string(archetypeId));
	// The two rigs intentionally share the admitted Valtan pattern/effect domain.
	CEffectV2Runtime::Prewarm_Archetype(pDevice, pContext, "BOSS_VALTAN");
	return S_OK;
}

bool_t Client::CValtanPresentationAssetService::Is_Ready(
	const uint32_t iLevelIndex, const std::string_view archetypeId)
{
	std::scoped_lock lock{ g_ValtanAssetMutex };
	const auto ready = g_ReadyByLevel.find(iLevelIndex);
	return ready != g_ReadyByLevel.end() &&
		ready->second.contains(std::string(archetypeId));
}
