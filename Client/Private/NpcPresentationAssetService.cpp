#include "NpcPresentationAssetService.h"

#include "ActorCatalog.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "RuntimeAssetRoot.h"
#include "WorldSequenceObject.h"
#include "DeferredMaterialRenderUtils.h"
#include "Shader.h"

#include <filesystem>
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Client
{
struct SAYDON_WEAPON_REPLACEMENT
{
	std::weak_ptr<Engine::CModel> body;
	std::weak_ptr<CWorldSequenceObject> object;
};
struct SAYDON_HAT_REPLACEMENT
{
	std::weak_ptr<Engine::CModel> body;
	std::weak_ptr<CWorldSequenceObject> object;
};
}

namespace
{
	std::mutex g_NpcAssetMutex;
	std::map<uint32_t, std::set<std::string, std::less<>>> g_ReadyArchetypes;
	std::map<uint32_t, std::set<std::string, std::less<>>> g_ReadyAnimSets;
	std::unordered_set<uint32_t> g_NpcObjectReadyLevels;
	// WORLD sampling and weapon rendering both run on the presentation thread.
	std::vector<std::weak_ptr<const Client::SAYDON_WEAPON_REPLACEMENT>> g_SaydonWeaponReplacements;
	std::vector<std::weak_ptr<const Client::SAYDON_HAT_REPLACEMENT>> g_SaydonHatReplacements;

    std::string Resolve_SaydonWeaponClip(const std::string_view bodyClip)
    {
        if (bodyClip == "rpct00_att_battle_17_01")
            return "wp_mn_rpct_05_sk.ao_att_battle_17_01";
        constexpr std::string_view giantPrefix = "mn_rpct_06_sk.ao_";
        if (!bodyClip.starts_with(giantPrefix)) return {};
        std::string suffix(bodyClip.substr(giantPrefix.size()));
        if (suffix.starts_with("att_battle_1_") || suffix.starts_with("att_battle_3_")) suffix.insert(11u, "0");
        return "wprpct06_" + suffix;
    }

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

HRESULT Client::CNpcPresentationAssetService::Prepare_SaydonHat(
	ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
	const std::shared_ptr<Engine::CModel>& body, std::shared_ptr<Engine::CModel>& outHat)
{
	if (!body) return E_INVALIDARG;
	bool saydonMaterial = false;
	for (uint32_t mesh = 0; mesh < body->Get_NumMeshes(); ++mesh)
		saydonMaterial |= body->Get_MaterialName(mesh) == "mn_rpct_05_mi";
	if (!saydonMaterial || !body->Has_Bone("bip001-head") ||
		!body->Has_Bone("b_wp_1") || !body->Has_Bone("b_wp_2")) return S_FALSE;
	Engine::MODEL_ASSET_LOAD_DESC load;
	std::string status;
	if (!CActorCatalog::Build_ModelLoadDescription(
		"Character/KoukuSaton/WP_MN_RPCT_08/wp_mn_rpct_08_1_sk.wmodel", load, status))
	{
		OutputDebugStringA(("[SaydonHat] Native material preparation failed: " + status + "\n").c_str());
		return E_FAIL;
	}
	// This head-specific mesh includes the source wp_3_1 placement. Its WModel
	// import basis is 100; .01 removes that once, before the owner's 1.7 basis.
	auto prototype = Engine::CModel::Create(device, context, MODEL::ANIM, load,
		XMMatrixScaling(.01f, .01f, .01f));
	if (!prototype || prototype->Get_NumMeshes() != 1u) return E_FAIL;
	auto staged = std::dynamic_pointer_cast<Engine::CModel>(prototype->Clone(nullptr));
	if (!staged) return E_FAIL;
	staged->Set_AnimPaused(true);
	staged->Refresh_BoneCombinedMatrices();
	outHat = std::move(staged);
	return S_OK;
}

void Client::CNpcPresentationAssetService::Track_SaydonHatReplacement(
	std::shared_ptr<const SAYDON_HAT_REPLACEMENT>& registration,
	const std::shared_ptr<Engine::CModel>& body, const std::shared_ptr<CWorldSequenceObject>& object)
{
	if (!body || !object) { registration.reset(); return; }
	if (registration && registration->body.lock() == body && registration->object.lock() == object) return;
	registration.reset();
	std::erase_if(g_SaydonHatReplacements, [](const auto& value) { return value.expired(); });
	auto staged = std::make_shared<const SAYDON_HAT_REPLACEMENT>(SAYDON_HAT_REPLACEMENT{body, object});
	g_SaydonHatReplacements.push_back(staged);
	registration = std::move(staged);
}

bool_t Client::CNpcPresentationAssetService::Is_SaydonHatSuppressed(
	const std::shared_ptr<Engine::CModel>& body)
{
	std::erase_if(g_SaydonHatReplacements, [](const auto& value) { return value.expired(); });
	if (!body) return false;
	for (const auto& value : g_SaydonHatReplacements)
		if (const auto replacement = value.lock(); replacement && replacement->body.lock() == body)
			if (const auto object = replacement->object.lock(); object && object->Is_Visible()) return true;
	return false;
}

bool_t Client::CNpcPresentationAssetService::Try_GetSaydonHatWorld(
	const std::shared_ptr<Engine::CModel>& body, const float4x4_t& bodyWorld, float4x4_t& outWorld)
{
	if (!body || !body->Has_Bone("bip001-head") || Is_SaydonHatSuppressed(body)) return false;
	XMStoreFloat4x4(&outWorld, body->Get_BoneMatrix("bip001-head") * XMLoadFloat4x4(&bodyWorld));
	return true;
}

HRESULT Client::CNpcPresentationAssetService::Render_SaydonHat(
	const std::shared_ptr<Engine::CModel>& body, const std::shared_ptr<Engine::CModel>& hat,
	const std::shared_ptr<Engine::CShader>& shader, const float4x4_t& bodyWorld,
	const uint32_t pass, const bool_t nativeBinaryBasePass, const bool_t shadow)
{
	if (!hat || Is_SaydonHatSuppressed(body)) return S_OK;
	if (!shader) return E_FAIL;
	float4x4_t world;
	if (!Try_GetSaydonHatWorld(body, bodyWorld, world)) return E_FAIL;
	if (FAILED(shader->Bind_Matrix("g_WorldMatrix", &world))) return E_FAIL;
	HRESULT result = S_OK;
	for (uint32_t mesh = 0; mesh < hat->Get_NumMeshes(); ++mesh)
	{
		const HRESULT material = shadow ? hat->Bind_Material(shader, "g_DiffuseTexture", mesh, aiTextureType_DIFFUSE, 0) :
			Bind_DeferredMaterialInputs(*hat, shader, mesh, {}, nullptr, nullptr, nativeBinaryBasePass);
		if (FAILED(material) || FAILED(hat->Bind_BoneMatrices(shader, "g_BoneMatrices", mesh)) ||
			FAILED(shader->Begin(pass)) || FAILED(hat->Render(mesh))) { result = E_FAIL; break; }
	}
	// A following weapon/outline/part must still receive the body root.
	const HRESULT restored = shader->Bind_Matrix("g_WorldMatrix", &bodyWorld);
	return FAILED(result) ? result : restored;
}

void Client::CNpcPresentationAssetService::Track_SaydonWeaponReplacement(
	std::shared_ptr<const SAYDON_WEAPON_REPLACEMENT>& registration,
	const std::shared_ptr<Engine::CModel>& body, const std::shared_ptr<CWorldSequenceObject>& object)
{
	if (!body || !object)
	{
		registration.reset();
		return;
	}
	if (registration && registration->body.lock() == body && registration->object.lock() == object) return;
	registration.reset();
	std::erase_if(g_SaydonWeaponReplacements, [](const auto& value) { return value.expired(); });
	auto replacement = std::make_shared<const SAYDON_WEAPON_REPLACEMENT>(SAYDON_WEAPON_REPLACEMENT{body, object});
	g_SaydonWeaponReplacements.push_back(replacement);
	registration = std::move(replacement);
}

bool_t Client::CNpcPresentationAssetService::Is_SaydonHammerSuppressed(
	const std::shared_ptr<Engine::CModel>& body)
{
	std::erase_if(g_SaydonWeaponReplacements, [](const auto& value) { return value.expired(); });
	if (!body) return false;
	for (const auto& value : g_SaydonWeaponReplacements)
		if (const auto replacement = value.lock(); replacement && replacement->body.lock() == body)
			if (const auto object = replacement->object.lock(); object && object->Is_Visible()) return true;
	return false;
}

void Client::CNpcPresentationAssetService::Synchronize_SaydonHammerPose(
	const std::shared_ptr<Engine::CModel>& body, const std::shared_ptr<Engine::CModel>& weapon,
	const std::vector<float4x4_t>& restPose)
{
	if (!body || !weapon) return;
    if (const auto* transition = body->Get_AnimationTransitionPose())
    {
        const auto mapClip = [&](uint32_t bodyIndex, float bodyTicks, uint32_t& weaponIndex, float& weaponTicks)
        {
            weaponIndex = UINT32_MAX; weaponTicks = 0.f;
            const char* bodyName = body->Get_AnimationName(bodyIndex);
            if (!bodyName) return;
            const std::string clip = Resolve_SaydonWeaponClip(bodyName);
            if (clip.empty()) return;
            for (uint32_t i = 0; i < weapon->Get_NumAnimations(); ++i)
            {
                if (clip != weapon->Get_AnimationName(i)) continue;
                float cursor = 0.f, end = 0.f;
                const float bodyTps = body->Get_AnimationTickPerSecond(bodyIndex);
                if (bodyTps <= 0.f || !weapon->Get_AnimationProgress(i, cursor, end)) return;
                weaponIndex = i;
                weaponTicks = (std::min)(end, bodyTicks / bodyTps * weapon->Get_AnimationTickPerSecond(i));
                return;
            }
        };
        Engine::CModel::ANIMATION_TRANSITION_POSE pose = *transition;
        mapClip(transition->sourceIndex, transition->sourceTicks, pose.sourceIndex, pose.sourceTicks);
        mapClip(transition->targetIndex, transition->targetTicks, pose.targetIndex, pose.targetTicks);
        weapon->Set_AnimPaused(true);
        (void)weapon->Set_AnimationTransitionPose(pose);
        return;
    }
    weapon->Clear_AnimationTransitionPose();
	const auto bodyIndex = body->Get_CurrentAnimIndex();
	const char* bodyName = body->Get_AnimationName(bodyIndex);
	const std::string weaponClip = bodyName ? Resolve_SaydonWeaponClip(bodyName) : std::string{};
	uint32_t weaponIndex = 0u;
	for (; weaponIndex < weapon->Get_NumAnimations(); ++weaponIndex)
		if (const char* name = weapon->Get_AnimationName(weaponIndex); name && weaponClip == name) break;
	f32_t bodyPosition = 0.f, bodyDuration = 0.f, weaponPosition = 0.f, weaponDuration = 0.f;
	const f32_t bodyTps = body->Get_AnimationTickPerSecond(bodyIndex);
	const f32_t weaponTps = weapon->Get_AnimationTickPerSecond(weaponIndex);
	const bool mapped = weaponIndex < weapon->Get_NumAnimations() &&
		body->Get_AnimationProgress(bodyIndex, bodyPosition, bodyDuration) &&
		weapon->Get_AnimationProgress(weaponIndex, weaponPosition, weaponDuration) &&
		std::isfinite(bodyTps) && bodyTps > 0.f && std::isfinite(weaponTps) && weaponTps > 0.f;
	weapon->Set_AnimPaused(true);
	if (mapped)
	{
		if (weapon->Get_CurrentAnimIndex() != weaponIndex || weapon->Is_AnimLoop())
			(void)weapon->Start_Animation(weaponIndex, false);
		(void)weapon->Set_AnimTrackPosition(weaponIndex,
			std::clamp(bodyPosition / bodyTps * weaponTps, 0.f, weaponDuration));
		weapon->Set_AnimPaused(true);
		weapon->Update_Animation(0.f);
	}
	else
	{
		for (uint32_t i = 0u; i < restPose.size(); ++i)
			(void)weapon->Set_BoneLocalMatrix(i, XMLoadFloat4x4(&restPose[i]));
		weapon->Refresh_BoneCombinedMatrices();
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

HRESULT Client::CNpcPresentationAssetService::Ensure_ObjectPrototype(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const uint32_t iLevelIndex)
{
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;
	std::scoped_lock lock{ g_NpcAssetMutex };
	if (g_NpcObjectReadyLevels.contains(iLevelIndex))
		return S_FALSE;
	auto prototype = CNpc::Create(pDevice, pContext);
	if (nullptr == prototype || FAILED(CGameInstance::Get().Add_Prototype(
		iLevelIndex, TEXT("Prototype_GameObject_Npc"), std::move(prototype))))
	{
		return E_FAIL;
	}
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
