#include "ValtanPresentationAssetService.h"

// 보스 표현 ID·본체/무기/armor 재질 입력과 필수 clip·본을 조회한다.
#include "ActorCatalog.h"
// 선택적 armor 생성 실패 때 decoder 진단을 붙인다.
#include "BinaryAsset/ModelDecoderRegistry.h"
// Level에서 공유할 발탄 body GameObject 원형을 만든다.
#include "Body_Valtan.h"
// 기본 발탄에 게시된 선택적 저작 clip을 추가한다.
#include "BoneAnimationDocument.h"
// 원형 등록 성공 뒤 발탄 effect 도메인을 prewarm한다.
#include "EffectV2_Runtime.h"
// 완성한 원형 묶음을 Level의 Engine 저장소에 넘긴다.
#include "GameInstance.h"
// 본체·animation donor·무기·armor 모델을 만들고 골격/clip을 검사한다.
#include "Model.h"
// Resources 상대 animation donor 경로를 검증·해석한다.
#include "RuntimeAssetRoot.h"
// 필수 무기 본·armor tag와 보스 GameObject 원형을 사용한다.
#include "Valtan.h"

// 동일 서비스의 준비/조회/Level 기록 초기화를 직렬화한다.
#include <mutex>
// animation donor 파일 검사 결과의 오류 코드를 구분한다.
#include <system_error>
// Level별 준비 archetype 목록을 보관한다.
#include <unordered_map>
// 등록 완료 archetype을 중복 없이 기억한다.
#include <unordered_set>
// tag와 원형 pair를 만들고 소유권을 이동한다.
#include <utility>
// armor 입력과 등록 전 원형 묶음을 임시 소유한다.
#include <vector>

namespace
{
	std::mutex g_ValtanAssetMutex;
	std::unordered_map<uint32_t, std::unordered_set<std::string>> g_ReadyByLevel;

	// 발탄 client presentation 계약을 가진 정의만 허용해 기본/파생 tag를 구분한다.
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

// Loader·replication·아레나 준비 요청을 받는다. 기존 준비 기록을 검사하고 자원을 임시 생성·검증한 뒤 묶음 등록한다.
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
	// 이미 다른 발탄 계열을 준비한 Level이면 Body/Valtan GameObject 원형은 다시 만들지 않는다.
	const bool hasSharedObjectPrototypes = ready != g_ReadyByLevel.end();
	if (hasSharedObjectPrototypes && ready->second.contains(std::string(archetypeId)))
		return S_FALSE;

	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss(archetypeId);
	const wstring_t bodyTag = Get_BodyModelPrototypeTag(archetypeId);
	const wstring_t weaponTag = Get_WeaponModelPrototypeTag(archetypeId);
	if (nullptr == pActor || bodyTag.empty() || weaponTag.empty())
		return RejectAsset(archetypeId, "no supported boss catalog definition");

	Engine::MODEL_ASSET_LOAD_DESC bodyLoad;
	Engine::MODEL_ASSET_LOAD_DESC weaponLoad;
	std::string materialStatus;
	if (!CActorCatalog::Build_ModelLoadDescription(pActor->bodyModel, bodyLoad, materialStatus))
		return RejectAsset(archetypeId, "body material input failed: " + materialStatus);
	if (!CActorCatalog::Build_ModelLoadDescription(pActor->weaponModel, weaponLoad, materialStatus))
		return RejectAsset(archetypeId, "weapon material input failed: " + materialStatus);
	const std::filesystem::path animSetPath =
		CRuntimeAssetRoot::Resolve(pActor->animationSetId);
	if (animSetPath.empty())
		return RejectAsset(archetypeId, "invalid Resources-relative animation path");
	std::vector<std::pair<uint32_t, Engine::MODEL_ASSET_LOAD_DESC>> armorAssets;
	armorAssets.reserve(pActor->armorParts.size());
	for (const BOSS_ARMOR_PART_ENTRY& armorPart : pActor->armorParts)
	{
		Engine::MODEL_ASSET_LOAD_DESC armorLoad;
		if (!CActorCatalog::Build_ModelLoadDescription(armorPart.modelAssetId, armorLoad, materialStatus))
			return RejectAsset(archetypeId, "armor material input failed: " + materialStatus);
		armorAssets.emplace_back(armorPart.stateMask, std::move(armorLoad));
	}
	std::error_code animSetFileError;
	const bool_t hasAnimSetFile = std::filesystem::is_regular_file(
		animSetPath, animSetFileError);
	const bool_t isMissingAnimSetFile = !hasAnimSetFile &&
		(ERROR_FILE_NOT_FOUND == animSetFileError.value() ||
			ERROR_PATH_NOT_FOUND == animSetFileError.value());
	if (!hasAnimSetFile && (!isMissingAnimSetFile || "BOSS_VALTAN" != archetypeId))
		return RejectAsset(archetypeId, "animation donor unavailable: " + pActor->animationSetId);

// 파일 없음이 지정한 Win32 오류로 판정된 기본 BOSS_VALTAN만 Debug에서 내장 clip으로 계속할 수 있다. Release는 거부한다.
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
		pDevice, pContext, MODEL::ANIM, bodyLoad,
		XMMatrixScaling(bodyScale, bodyScale, bodyScale));
	if (nullptr == bodyModel || 0u == bodyModel->Get_NumMeshes() ||
		0u == bodyModel->Get_SkeletonHash() ||
		!bodyModel->Has_Bone(CValtan::WEAPON_SOCKET_BONE))
	{
		return RejectAsset(archetypeId, "body geometry/skeleton/weapon socket unavailable: " +
			pActor->bodyModel);
	}
	for (const auto& particle : pActor->defaultParticles)
		if (!bodyModel->Has_Bone(particle.boneName.c_str()))
			return RejectAsset(archetypeId, "default particle bone unavailable: " +
				particle.occurrenceId + " / " + particle.boneName);
	if (hasAnimSetFile)
	{
		const unique_ptr<CModel> animSetModel = CModel::Create(
			pDevice, pContext, MODEL::ANIM, animSetPath.string().c_str(),
			XMMatrixScaling(bodyScale, bodyScale, bodyScale));
		// donor는 골격 hash·본 수·clip 이름 충돌 검사를 거쳐 본체에 붙인다. 아직 Engine에 등록하지 않은 임시 자원이다.
		if (nullptr == animSetModel || !animSetModel->Has_Animations() ||
			FAILED(bodyModel->Attach_AnimationSet(*animSetModel)))
		{
			return RejectAsset(archetypeId, "animation donor does not match the body: " +
				pActor->animationSetId);
		}
	}
    // 게시 저작 clip은 선택 사항이다. 실패하면 진단 후 기존 native animation으로 계속한다.
    if (archetypeId == "BOSS_VALTAN")
    {
        std::string authoredStatus;
        if (!CBoneAnimationDocument::Load_PublishedValtan(*bodyModel, authoredStatus))
            OutputDebugStringA(("[ValtanPresentation] Optional authored clips retained native animation: " + authoredStatus + "\n").c_str());
    }
	if (!bodyModel->Has_Animations())
		return RejectAsset(archetypeId, "combined body has no animation clips");
	if ("BOSS_VALTAN" != archetypeId)
	{
		const auto& clips = pActor->presentationClips;
		// 여기서는 death clip을 필수 검사 목록에 포함하지 않는다.
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
	// 무기 pre-rotation degree를 먼저 적용하고 catalog의 preScale을 곱한다.
	const float3_t& weaponRotation = pActor->weaponModelPreRotationDegrees;
	staged.emplace_back(weaponTag,
		CModel::Create(pDevice, pContext, MODEL::NONANIM, weaponLoad,
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(weaponRotation.x),
				XMConvertToRadians(weaponRotation.y),
				XMConvertToRadians(weaponRotation.z)) *
			XMMatrixScaling(weaponScale, weaponScale, weaponScale)));
	for (const auto& [stateMask, armorLoad] : armorAssets)
	{
		unique_ptr<CModel> armorModel = CModel::Create(
			pDevice, pContext, MODEL::ANIM, armorLoad,
			XMMatrixScaling(bodyScale, bodyScale, bodyScale));
		// 개별 armor Model 생성 실패는 진단 후 건너뛰고 나머지 원형 준비를 계속한다.
		if (nullptr == armorModel)
		{
			OutputDebugStringA(("[Client][Valtan] armour plate rejected: " +
				armorLoad.meshPath.string() + " | " +
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
	// Engine의 전체 tag 사전 검사와 임시 node 등록 경로에 원형 묶음의 소유권을 넘긴다.
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return RejectAsset(archetypeId, "atomic prototype registration failed");

	// 묶음 등록 성공 후 준비 기록을 남긴다. 선택적으로 건너뛴 armor까지 존재한다는 뜻은 아니다.
	g_ReadyByLevel[iLevelIndex].insert(std::string(archetypeId));
	// 발탄 파생 표현도 BOSS_VALTAN의 패턴·이펙트 자원을 미리 준비한다.
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
