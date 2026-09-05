#include "imgui.h"

#include "CharacterPreviewPanel.h"

#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "CompositionAnimationResource.h"
#include "GameInstance.h"
#include "Level_ValtanArena.h"
#include "Level_KakulSaydonArena.h"
#include "Model.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "PlayableCharacterAssetService.h"
#include "PlayableCharacterPreviewContract.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string_view>
#include <utility>

namespace
{
	constexpr const char* SAYDON_HAMMER_ASSET = "Character/KoukuSaton/WP_MN_RPCT_06/WP_MN_RPCT_06.wmodel";
	constexpr const wchar_t* SAYDON_HAMMER_PROTOTYPE = L"Prototype_Component_Model_AnimationPreview_KoukuSaydon_WP_MN_RPCT_06";
	constexpr const char* SAYDON_HAMMER_SOCKET = "b_wp_1";

	std::string Resolve_SaydonHammerClip(const std::string_view bodyClip)
	{
		constexpr std::string_view prefix = "mn_rpct_06_sk.ao_";
		if (!bodyClip.starts_with(prefix)) return {};
		std::string suffix(bodyClip.substr(prefix.size()));
		if (suffix.starts_with("att_battle_1_")) suffix.insert(11u, "0");
		else if (suffix.starts_with("att_battle_3_")) suffix.insert(11u, "0");
		return "wprpct06_" + suffix;
	}

	bool_t Try_ResolveRaidPreviewPlacement(const uint32_t currentLevel,
		float3_t& position, std::string& status)
	{
		if (currentLevel == ETOUI(LEVEL::VALTAN_ARENA))
		{
			const auto* arena = Client::CLevel_ValtanArena::Get_Active();
			if (nullptr != arena) return arena->Try_Get_AuthoringPreviewPlacement(position, status);
		}
		else if (currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA))
		{
			const auto* arena = Client::CLevel_KakulSaydonArena::Get_Active();
			if (nullptr != arena) return arena->Try_Get_AuthoringPreviewPlacement(position, status);
		}
		status = "active raid arena placement owner is unavailable";
		return false;
	}
}

Client::CCharacterPreviewPanel::~CCharacterPreviewPanel()
{
	/* The owning tool may outlive the level the body was staged into, so the
	   layer removal is skipped and only the published target is cleared. */
	Release(false);
}

void Client::CCharacterPreviewPanel::On_LevelChanged()
{
	Release(false);
	m_iPreparedGenericPreviewLevelIndex = UINT32_MAX;
	m_PreparedGenericPreviewAssetIds.clear();
}

void Client::CCharacterPreviewPanel::Refresh_Level()
{
	const auto resetPreparedGenericAssets = [this]()
	{
		m_iPreparedGenericPreviewLevelIndex = UINT32_MAX;
		m_PreparedGenericPreviewAssetIds.clear();
	};
	const uint32_t currentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	if (UINT32_MAX != m_iPreparedGenericPreviewLevelIndex &&
		currentLevel != m_iPreparedGenericPreviewLevelIndex)
	{
		resetPreparedGenericAssets();
	}
	const shared_ptr<CGameObject> previewObject = m_pPreviewObject.lock();
	if (nullptr == previewObject)
	{
		if (nullptr != m_pPreviewAsset || UINT32_MAX != m_iPreviewLevelIndex)
		{
			CAnimationTargetService::Clear_Preview();
			resetPreparedGenericAssets();
		}
		m_pPreviewObject.reset();
		m_pPreviewAsset = nullptr;
		m_iPreviewLevelIndex = UINT32_MAX;
		return;
	}

	if (currentLevel == m_iPreviewLevelIndex)
		return;

	CAnimationTargetService::Clear_Preview();
	resetPreparedGenericAssets();
	m_pPreviewObject.reset();
	m_pPreviewAsset = nullptr;
	m_iPreviewLevelIndex = UINT32_MAX;
}

void Client::CCharacterPreviewPanel::Set_SessionLock(
	const CHARACTER_PREVIEW_LOCK_OWNER eOwner,
	const bool_t isLocked,
	string strReason)
{
	if (eOwner >= CHARACTER_PREVIEW_LOCK_OWNER::END)
		return;
	const size_t index = ETOI(eOwner);
	m_SessionLocks[index] = isLocked;
	m_SessionLockReasons[index] = isLocked ? std::move(strReason) : string{};
}

bool_t Client::CCharacterPreviewPanel::Select_TargetAsset(
	const string& strAnimationAssetName)
{
	if (strAnimationAssetName.empty())
	{
		m_Status = "Preview target asset name is empty.";
		return false;
	}
	const auto asset = std::find_if(
		ANIMATION_PREVIEW_ASSETS.begin(), ANIMATION_PREVIEW_ASSETS.end(),
		[&strAnimationAssetName](const ANIMATION_PREVIEW_ASSET& candidate)
		{
			return strAnimationAssetName == candidate.pAssetName;
		});
	if (asset == ANIMATION_PREVIEW_ASSETS.end())
	{
		m_Status = "Preview target is not admitted: " + strAnimationAssetName;
		return false;
	}
	const uint32_t currentLevel = CGameInstance::Get().Get_CurrentLevelID();
	if (nullptr != m_pPreviewAsset &&
		strAnimationAssetName == m_pPreviewAsset->pAssetName &&
		!m_pPreviewObject.expired() && m_iPreviewLevelIndex == currentLevel)
	{
		const bool_t raidCompositionPreview =
			(currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
			 currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA)) &&
			Is_CompositionAnimationTargetAsset(strAnimationAssetName);
		if (!raidCompositionPreview) return true;

		// Recenter an existing clone using the level's replication owner too;
		// AnimationTargetService's optional scene binding is not a raid roster.
		float3_t placement{};
		std::string placementSource;
		if (!Try_ResolveRaidPreviewPlacement(currentLevel, placement, placementSource))
		{
			m_Status = "Arena preview placement unavailable: " + placementSource + ".";
			return false;
		}
		const shared_ptr<CValtan> previewBoss =
			dynamic_pointer_cast<CValtan>(m_pPreviewObject.lock());
		if (nullptr != previewBoss && nullptr != previewBoss->Get_Transform())
			previewBoss->Get_Transform()->Set_State(STATE::POSITION,
				XMVectorSetW(XMLoadFloat3(&placement), 1.f));
		else
		{
			const shared_ptr<CPart_Body> body = dynamic_pointer_cast<CPart_Body>(m_pPreviewObject.lock());
			if (nullptr == body || nullptr == body->Get_Model())
			{
				m_Status = "Arena preview body is unavailable.";
				return false;
			}
			float4x4_t& root = m_PreviewParentMatrices[m_iPreviewParentMatrixIndex];
			XMStoreFloat4x4(&root, XMMatrixTranslation(placement.x, placement.y, placement.z));
			m_PreviewUnscaledParentMatrix = root;
			CAnimationTargetService::Bind_Preview(body->Get_Model(), asset->pAssetName, root);
		}
		m_Status = "Target=LOCAL ARENA PREVIEW | anchor=" + placementSource +
			" | collision=OFF | Server boss=UNCHANGED.";
		return true;
	}
	for (size_t i = 0u; i < m_SessionLocks.size(); ++i)
	{
		if (!m_SessionLocks[i])
			continue;
		m_Status = m_SessionLockReasons[i].empty() ?
			"Preview target change is locked by an unsaved authoring document." :
			m_SessionLockReasons[i];
		return false;
	}
	return Select_Asset(*asset);
}

bool_t Client::CCharacterPreviewPanel::Set_PreviewScaleMultiplier(
	const shared_ptr<Engine::CModel>& expectedModel, const f32_t multiplier)
{
	if (!std::isfinite(multiplier) || multiplier <= 0.f)
	{
		m_Status = "Preview scale is unavailable from BossCatalog; previous preview scale preserved.";
		return false;
	}
	const auto body = dynamic_pointer_cast<CPart_Body>(m_pPreviewObject.lock());
	if (nullptr == body || nullptr == expectedModel || nullptr == m_pPreviewAsset ||
		body->Get_Model() != expectedModel ||
		CAnimationTargetService::Resolve_Model() != expectedModel)
		return false;
	auto& root = m_PreviewParentMatrices[m_iPreviewParentMatrixIndex];
	XMStoreFloat4x4(&root, XMMatrixScaling(multiplier, multiplier, multiplier) *
		XMLoadFloat4x4(&m_PreviewUnscaledParentMatrix));
	CAnimationTargetService::Bind_Preview(expectedModel, m_pPreviewAsset->pAssetName, root);
	// ImGui can seek after the level update; refresh the existing part's cached
	// combined matrix immediately without advancing its animation clock.
	body->Update(0.f);
	if (const auto weapon = m_pPreviewWeaponObject.lock()) weapon->Update(0.f);
	Synchronize_PreviewWeapon();
	return true;
}

void Client::CCharacterPreviewPanel::Synchronize_PreviewWeapon()
{
	if (nullptr == m_pPreviewAsset || std::string_view(m_pPreviewAsset->pAssetName) != "MN_RPCT_06") return;
	const auto body = dynamic_pointer_cast<CPart_Body>(m_pPreviewObject.lock());
	const auto weapon = dynamic_pointer_cast<CPart_Body>(m_pPreviewWeaponObject.lock());
	if (nullptr == body || nullptr == weapon || nullptr == body->Get_Model() || nullptr == weapon->Get_Model()) return;
	const auto bodyRoot = dynamic_pointer_cast<CTransform>(body->Get_Component(g_strTransformComTag));
	const auto bodyModel = body->Get_Model();
	const auto weaponModel = weapon->Get_Model();
	if (nullptr == bodyRoot || !bodyModel->Has_Bone(SAYDON_HAMMER_SOCKET)) return;
	// Source wp_1_20 is b_wp_1 with zero offset/rotation and unit scale.
	// Its bone already carries the body pretransform; the hammer adds no second
	// preview scale or yaw. Only the selected actor parent contains the 100x.
	XMStoreFloat4x4(&m_PreviewWeaponParentMatrices[m_iPreviewParentMatrixIndex],
		bodyModel->Get_BoneMatrix(SAYDON_HAMMER_SOCKET) *
		XMLoadFloat4x4(bodyRoot->Get_WorldMatrixPtr()) *
		XMLoadFloat4x4(&m_PreviewParentMatrices[m_iPreviewParentMatrixIndex]));
	weaponModel->Set_AnimPaused(true);
	const auto bodyIndex = bodyModel->Get_CurrentAnimIndex();
	const char* bodyClip = bodyModel->Get_AnimationName(bodyIndex);
	const std::string weaponClip = nullptr != bodyClip ? Resolve_SaydonHammerClip(bodyClip) : std::string{};
	std::uint32_t weaponIndex = 0u;
	for (; weaponIndex < weaponModel->Get_NumAnimations(); ++weaponIndex)
	{
		const char* name = weaponModel->Get_AnimationName(weaponIndex);
		if (nullptr != name && weaponClip == name) break;
	}
	f32_t bodyPosition = 0.f, bodyDuration = 0.f, weaponPosition = 0.f, weaponDuration = 0.f;
	const f32_t bodyTps = bodyModel->Get_AnimationTickPerSecond(bodyIndex);
	const f32_t weaponTps = weaponModel->Get_AnimationTickPerSecond(weaponIndex);
	const bool_t mapped = weaponIndex < weaponModel->Get_NumAnimations() &&
		bodyModel->Get_AnimationProgress(bodyIndex, bodyPosition, bodyDuration) &&
		weaponModel->Get_AnimationProgress(weaponIndex, weaponPosition, weaponDuration) &&
		std::isfinite(bodyTps) && bodyTps > 0.f && std::isfinite(weaponTps) && weaponTps > 0.f;
	if (mapped)
	{
		if (weaponModel->Get_CurrentAnimIndex() != weaponIndex || weaponModel->Is_AnimLoop())
			(void)weaponModel->Start_Animation(weaponIndex, false);
		// The family-3 source lengths differ. Preserve source seconds and hold
		// the hammer's last pose; do not invent an original start delay or retime.
		(void)weaponModel->Set_AnimTrackPosition(weaponIndex,
			std::clamp(bodyPosition / bodyTps * weaponTps, 0.f, weaponDuration));
	}
	weaponModel->Set_AnimPaused(true);
	weapon->Update(0.f);
	if (!mapped)
	{
		// No idle/1_01 counterpart exists. Restore the cloned skeleton rest pose
		// captured before its first animation, rather than playing clip zero.
		for (std::uint32_t i = 0u; i < m_PreviewWeaponRestPose.size(); ++i)
			(void)weaponModel->Set_BoneLocalMatrix(i, XMLoadFloat4x4(&m_PreviewWeaponRestPose[i]));
		weaponModel->Refresh_BoneCombinedMatrices();
	}
}

bool_t Client::CCharacterPreviewPanel::Declares_Weapon(
	const ANIMATION_PREVIEW_ASSET& asset)
{
	return nullptr != asset.pWeaponModelAssetId &&
		nullptr != asset.pWeaponPrototypeTag &&
		nullptr != asset.pWeaponSocketBone;
}

void Client::CCharacterPreviewPanel::Release(const bool_t removeFromLayer)
{
	const shared_ptr<CGameObject> previewObject = m_pPreviewObject.lock();
	const shared_ptr<CGameObject> previewWeapon =
		m_pPreviewWeaponObject.lock();
	/* The weapon rides the body's bone, so it is dropped whenever the body is,
	   including the early return where the body already expired on its own. */
	if (nullptr != previewWeapon &&
		removeFromLayer &&
		m_iPreviewLevelIndex == CGameInstance::Get().Get_CurrentLevelID())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iPreviewLevelIndex,
			TEXT("Layer_AnimationPreview"),
			previewWeapon);
	}
	m_pPreviewWeaponObject.reset();
	m_PreviewWeaponRestPose.clear();

	if (nullptr == previewObject)
	{
		if (nullptr != m_pPreviewAsset || UINT32_MAX != m_iPreviewLevelIndex)
			CAnimationTargetService::Clear_Preview();
		m_pPreviewObject.reset();
		m_pPreviewAsset = nullptr;
		m_iPreviewLevelIndex = UINT32_MAX;
		return;
	}

	CAnimationTargetService::Clear_Preview();
	if (removeFromLayer &&
		m_iPreviewLevelIndex ==
			CGameInstance::Get().Get_CurrentLevelID())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iPreviewLevelIndex,
			TEXT("Layer_AnimationPreview"),
			previewObject);
	}
	m_pPreviewObject.reset();
	m_pPreviewAsset = nullptr;
	m_iPreviewLevelIndex = UINT32_MAX;
}

bool_t Client::CCharacterPreviewPanel::Select_Asset(
	const ANIMATION_PREVIEW_ASSET& asset)
{
	const uint32_t currentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t bRaidCompositionPreview =
		(currentLevel == ETOUI(LEVEL::VALTAN_ARENA) ||
		 currentLevel == ETOUI(LEVEL::KAKULSAYDON_ARENA)) &&
		nullptr != asset.pAssetName && Is_CompositionAnimationTargetAsset(asset.pAssetName);
	if (currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT) && !bRaidCompositionPreview)
	{
		m_Status = "Character previews are admitted in Character Select or Development; raid arenas also admit the registered Composition animation bodies.";
		return false;
	}
	if (m_iPreparedGenericPreviewLevelIndex != currentLevel)
	{
		m_iPreparedGenericPreviewLevelIndex = currentLevel;
		m_PreparedGenericPreviewAssetIds.clear();
	}

	vector_t previewPosition = XMVectorSet(2.5f, 0.f, 0.f, 1.f);
	std::string strPlacementSource = "scene character / world-right";
	const shared_ptr<CCharacter> character =
		CAnimationTargetService::Resolve_SceneCharacter();
	if (bRaidCompositionPreview)
	{
		float3_t placement{};
		if (!Try_ResolveRaidPreviewPlacement(currentLevel, placement, strPlacementSource))
		{
			m_Status = "Arena preview placement unavailable: " + strPlacementSource + ".";
			return false;
		}
		previewPosition = XMVectorSetW(XMLoadFloat3(&placement), 1.f);
	}
	else if (nullptr != character && nullptr != character->Get_Transform())
	{
		previewPosition = XMVectorAdd(
			previewPosition,
			character->Get_Transform()->Get_State(STATE::POSITION));
		previewPosition = XMVectorSetW(previewPosition, 1.f);
	}
	const size_t stagedParentMatrixIndex =
		(m_iPreviewParentMatrixIndex + 1u) % m_PreviewParentMatrices.size();
	float4x4_t& stagedParentMatrix =
		m_PreviewParentMatrices[stagedParentMatrixIndex];
	XMStoreFloat4x4(
		&stagedParentMatrix,
		XMMatrixTranslationFromVector(previewPosition));

	shared_ptr<CGameObject> stagedObject;
	shared_ptr<CGameObject> stagedWeapon;
	shared_ptr<CCharacter> stagedCharacter;
	shared_ptr<CValtan> stagedValtan;
	shared_ptr<CPart_Body> stagedBody;
	std::vector<float4x4_t> stagedWeaponRestPose;
	if (asset.bPlayableClassBody)
	{
		PLAYABLE_CHARACTER_PREVIEW_COMPOSITION composition;
		if (!CPlayableCharacterPreviewContract::Stage(asset, composition) ||
			nullptr == composition.pSpec ||
			PLAYABLE_PREVIEW_OWNER_KIND::CHARACTER != composition.eOwnerKind)
		{
			m_Status = string("Playable preview composition is not admitted: ") +
				asset.pAssetName;
			return false;
		}
		if (!CPlayableCharacterAssetService::Is_Ready(
				currentLevel, composition.pSpec->eCharacterClass) &&
			FAILED(CPlayableCharacterAssetService::Ensure_Prototypes(
				m_pDevice,
				m_pContext,
				currentLevel,
				composition.pSpec->eCharacterClass)))
		{
			m_Status = string("Playable preview assets failed to prepare: ") +
				asset.pAssetName;
			return false;
		}

		CCharacter::CHARACTER_DESC desc{};
		desc.iPrototypeLevelIndex = currentLevel;
		desc.pSpec = composition.pSpec;
		desc.fSpeedPerSec = 6.f;
		desc.fRotationPerSec = 180.f;
		desc.vPosition = float3_t(
			XMVectorGetX(previewPosition),
			XMVectorGetY(previewPosition),
			XMVectorGetZ(previewPosition));
		desc.strNickName = "Model View";
		desc.isLocallyControlled = false;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
				currentLevel,
				TEXT("Prototype_GameObject_Character"),
				currentLevel,
				TEXT("Layer_AnimationPreview"),
				&desc,
				&stagedObject)))
		{
			m_Status = string("Playable preview parts failed to stage: ") +
				asset.pAssetName;
			return false;
		}
		stagedCharacter = dynamic_pointer_cast<CCharacter>(stagedObject);
		if (nullptr == stagedCharacter ||
			nullptr == stagedCharacter->Get_BodyModel() ||
			stagedCharacter->Get_Spec() != composition.pSpec)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				currentLevel,
				TEXT("Layer_AnimationPreview"),
				stagedObject);
			m_Status =
				"Playable preview did not expose its Character composition.";
			return false;
		}

		LostArk::Shared::PLAYER_STANCE_ID previewStance =
			composition.eFallbackStance;
		if (nullptr != character && nullptr != character->Get_Spec() &&
			character->Get_Spec()->eCharacterClass ==
				composition.pSpec->eCharacterClass)
		{
			LostArk::Shared::PLAYER_STANCE_ID sceneStance;
			if (character->Try_Get_NetworkStance(sceneStance))
				previewStance = sceneStance;
		}
		stagedCharacter->Apply_NetworkStance(previewStance);
	}
	else if (nullptr != asset.pBossArchetypeId)
	{
		if (string{ asset.pBossArchetypeId } != "BOSS_VALTAN")
		{
			m_Status = string("Boss preview composition is not admitted: ") +
				asset.pAssetName;
			return false;
		}
		const BOSS_ACTOR_ENTRY* pBoss =
			CActorCatalog::Find_Boss(asset.pBossArchetypeId);
		if (nullptr == pBoss ||
			pBoss->clientPresentationId != "boss.valtan.client.v1")
		{
			m_Status = nullptr == pBoss ? CActorCatalog::Get_Status() :
				"Boss preview presentation contract is not admitted.";
			return false;
		}
		if (!CValtanPresentationAssetService::Is_Ready(currentLevel) &&
			FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
				m_pDevice,
				m_pContext,
				currentLevel)))
		{
			m_Status = string("Boss preview assets failed to prepare: ") +
				asset.pAssetName;
			return false;
		}

		CValtan::VALTAN_DESC desc{};
		desc.iPrototypeLevelIndex = currentLevel;
		desc.vPosition = float3_t(
			XMVectorGetX(previewPosition),
			XMVectorGetY(previewPosition),
			XMVectorGetZ(previewPosition));
		desc.fScale = pBoss->presentationScale;
		desc.fCollisionRadius = 0.f;
		desc.isServerAuthoritative = false;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
				currentLevel,
				TEXT("Prototype_GameObject_Valtan"),
				currentLevel,
				TEXT("Layer_AnimationPreview"),
				&desc,
				&stagedObject)))
		{
			m_Status = "Valtan product composition failed to stage.";
			return false;
		}

		stagedValtan = dynamic_pointer_cast<CValtan>(stagedObject);
		float4x4_t presentationRoot{};
		if (nullptr == stagedValtan ||
			nullptr == stagedValtan->Get_BodyModel() ||
			!stagedValtan->Try_Get_PresentationRootMatrix(&presentationRoot))
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				currentLevel,
				TEXT("Layer_AnimationPreview"),
				stagedObject);
			m_Status =
				"Valtan preview did not expose its body/weapon presentation root.";
			return false;
		}
	}
	else
	{
		if ((currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) ||
			 bRaidCompositionPreview) &&
			!m_PreparedGenericPreviewAssetIds.contains(asset.pId))
		{
			const std::filesystem::path modelPath =
				CRuntimeAssetRoot::Resolve(asset.pModelAssetId);
			const matrix_t previewTransform =
				XMMatrixScaling(
					asset.fPreviewScale,
					asset.fPreviewScale,
					asset.fPreviewScale) *
				XMMatrixRotationY(
					XMConvertToRadians(asset.fPreviewYawDegrees));
			unique_ptr<CModel> model;
			if (!modelPath.empty() &&
				std::filesystem::is_regular_file(modelPath))
			{
				model = CModel::Create(
					m_pDevice,
					m_pContext,
					MODEL::ANIM,
					modelPath.string().c_str(),
					previewTransform);
			}
			if (nullptr == model)
			{
				m_Status = string("Preview asset failed to prepare: ") +
					asset.pModelAssetId;
				return false;
			}
			/* Merged before the prototype is published so the clip table this
			   level hands to the tools is complete on its first read. */
			if (nullptr != asset.pAnimationSetAssetId)
			{
				const std::filesystem::path donorPath =
					CRuntimeAssetRoot::Resolve(asset.pAnimationSetAssetId);
				unique_ptr<CModel> donor;
				if (!donorPath.empty() &&
					std::filesystem::is_regular_file(donorPath))
				{
					donor = CModel::Create(
						m_pDevice,
						m_pContext,
						MODEL::ANIM,
						donorPath.string().c_str(),
						previewTransform);
				}
				if (nullptr == donor ||
					FAILED(model->Attach_AnimationSet(*donor)))
				{
					m_Status = string("Preview animation set is not admitted: ") +
						asset.pAnimationSetAssetId;
					return false;
				}
			}
			if (FAILED(CGameInstance::Get().Add_Prototype(
					currentLevel,
					asset.pPrototypeTag,
					std::move(model))))
			{
				m_Status = string("Preview asset failed to prepare: ") +
					asset.pModelAssetId;
				return false;
			}

			/* The weapon is a static mesh riding one bone, so it takes no
			   animation and no preview yaw of its own: the socket bone matrix
			   already carries both. Only the unit ratio between the two
			   authored assets belongs here. */
			if (Declares_Weapon(asset))
			{
				const std::filesystem::path weaponPath =
					CRuntimeAssetRoot::Resolve(asset.pWeaponModelAssetId);
				unique_ptr<CModel> weaponModel;
				if (!weaponPath.empty() &&
					std::filesystem::is_regular_file(weaponPath) &&
					std::isfinite(asset.fWeaponScale) &&
					asset.fWeaponScale > 0.f)
				{
					weaponModel = CModel::Create(
						m_pDevice,
						m_pContext,
						MODEL::NONANIM,
						weaponPath.string().c_str(),
						XMMatrixScaling(
							asset.fWeaponScale,
							asset.fWeaponScale,
							asset.fWeaponScale));
				}
				if (nullptr == weaponModel ||
					FAILED(CGameInstance::Get().Add_Prototype(
						currentLevel,
						asset.pWeaponPrototypeTag,
						std::move(weaponModel))))
				{
					m_Status = string("Preview weapon failed to prepare: ") +
						asset.pWeaponModelAssetId;
					return false;
				}
			}
			m_PreparedGenericPreviewAssetIds.insert(asset.pId);
		}

		CPart_Body::PART_BODY_DESC desc{};
		desc.pParentMatrix = &stagedParentMatrix;
		desc.iPrototypeLevelIndex = currentLevel;
		desc.strModelTag = asset.pPrototypeTag;
		desc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		desc.pInitialAnimation = nullptr;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
				currentLevel,
				TEXT("Prototype_GameObject_Part_Body"),
				currentLevel,
				TEXT("Layer_AnimationPreview"),
				&desc,
				&stagedObject)))
		{
			m_Status = string("Preview asset is not admitted: ") +
				asset.pModelAssetId;
			return false;
		}

		stagedBody = dynamic_pointer_cast<CPart_Body>(stagedObject);
		if (nullptr == stagedBody || nullptr == stagedBody->Get_Model())
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				currentLevel,
				TEXT("Layer_AnimationPreview"),
				stagedObject);
			m_Status = "Preview body did not expose an animated CModel.";
			return false;
		}

		if (std::string_view(asset.pAssetName) == "MN_RPCT_06")
		{
			const auto rejectHammer = [&](const std::string& reason)
			{
				if (nullptr != stagedWeapon)
					CGameInstance::Get().Remove_GameObject_from_Layer(currentLevel, TEXT("Layer_AnimationPreview"), stagedWeapon);
				CGameInstance::Get().Remove_GameObject_from_Layer(currentLevel, TEXT("Layer_AnimationPreview"), stagedObject);
				m_Status = "Large Saydon hammer preview could not stage: " + reason;
				return false;
			};
			if (!stagedBody->Get_Model()->Has_Bone(SAYDON_HAMMER_SOCKET))
				return rejectHammer("right-hand bone b_wp_1 is missing");
			if (!m_PreparedGenericPreviewAssetIds.contains(SAYDON_HAMMER_ASSET))
			{
				const auto path = CRuntimeAssetRoot::Resolve(SAYDON_HAMMER_ASSET);
				unique_ptr<CModel> model;
				/* The hammer previews with the pre-scale and socket rotation the
				running exe admitted for the big Saydon boss, so the Workbench
				shows the same hammer the spawned boss holds. Missing catalog input
				preserves the previous preview instead of using raw asset units. */
				const BOSS_ACTOR_ENTRY* pBigSaydon =
					CActorCatalog::Find_Boss("BOSS_KAKULSAYDON_G2_BIG_SAYDON");
				if (nullptr == pBigSaydon || pBigSaydon->weaponModel != SAYDON_HAMMER_ASSET ||
					!std::isfinite(pBigSaydon->weaponModelPreScale) || pBigSaydon->weaponModelPreScale <= 0.f)
					return rejectHammer("BossCatalog big Saydon hammer is unavailable: " + CActorCatalog::Get_Status());
				const f32_t hammerScale = pBigSaydon->weaponModelPreScale;
				const float3_t hammerRotation = pBigSaydon->weaponModelPreRotationDegrees;
				if (!path.empty() && std::filesystem::is_regular_file(path))
					model = CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, path.string().c_str(),
						XMMatrixRotationRollPitchYaw(
							XMConvertToRadians(hammerRotation.x),
							XMConvertToRadians(hammerRotation.y),
							XMConvertToRadians(hammerRotation.z)) *
						XMMatrixScaling(hammerScale, hammerScale, hammerScale));
				if (nullptr == model || FAILED(CGameInstance::Get().Add_Prototype(currentLevel,
					SAYDON_HAMMER_PROTOTYPE, std::move(model))))
					return rejectHammer(SAYDON_HAMMER_ASSET);
				m_PreparedGenericPreviewAssetIds.insert(SAYDON_HAMMER_ASSET);
			}
			stagedBody->Update(0.f);
			XMStoreFloat4x4(&m_PreviewWeaponParentMatrices[stagedParentMatrixIndex],
				stagedBody->Get_Model()->Get_BoneMatrix(SAYDON_HAMMER_SOCKET) * XMLoadFloat4x4(&stagedParentMatrix));
			CPart_Body::PART_BODY_DESC weaponDesc{};
			weaponDesc.pParentMatrix = &m_PreviewWeaponParentMatrices[stagedParentMatrixIndex];
			weaponDesc.iPrototypeLevelIndex = currentLevel;
			weaponDesc.strModelTag = SAYDON_HAMMER_PROTOTYPE;
			weaponDesc.strShaderTag = TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
			if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(currentLevel,
				TEXT("Prototype_GameObject_Part_Body"), currentLevel, TEXT("Layer_AnimationPreview"), &weaponDesc, &stagedWeapon)))
				return rejectHammer("animated weapon part creation failed");
			const auto weapon = dynamic_pointer_cast<CPart_Body>(stagedWeapon);
			if (nullptr == weapon || nullptr == weapon->Get_Model())
				return rejectHammer("weapon model is unavailable");
			const auto weaponModel = weapon->Get_Model();
			weaponModel->Set_AnimPaused(true);
			matrix_t local;
			for (std::uint32_t i = 0u; weaponModel->Get_BoneLocalMatrix(i, local); ++i)
			{
				float4x4_t stored;
				XMStoreFloat4x4(&stored, local);
				stagedWeaponRestPose.push_back(stored);
			}
			if (stagedWeaponRestPose.empty()) return rejectHammer("weapon skeleton rest pose is unavailable");
		}
		else if (Declares_Weapon(asset))
		{
			const shared_ptr<CTransform> bodyRoot =
				dynamic_pointer_cast<CTransform>(
					stagedBody->Get_Component(g_strTransformComTag));
			if (nullptr == bodyRoot)
			{
				CGameInstance::Get().Remove_GameObject_from_Layer(
					currentLevel,
					TEXT("Layer_AnimationPreview"),
					stagedObject);
				m_Status = "Preview body did not expose its visual root.";
				return false;
			}

			CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
			weaponDesc.pParentMatrix = &stagedParentMatrix;
			weaponDesc.iPrototypeLevelIndex = currentLevel;
			weaponDesc.strModelTag = asset.pWeaponPrototypeTag;
			weaponDesc.strShaderTag =
				TEXT("Prototype_Component_Shader_VtxMeshBinary");
			weaponDesc.pSkeletonModel = stagedBody->Get_Model();
			weaponDesc.pSocketBoneName = asset.pWeaponSocketBone;
			weaponDesc.pSocketRootMatrix = bodyRoot->Get_WorldMatrixPtr();
			if (nullptr != asset.pWeaponMaterialProfileId)
				weaponDesc.strMaterialProfileId = asset.pWeaponMaterialProfileId;

			/* A missing socket bone is reported by CPart_Equipment rather than
			   guessed at, so a rig without the hand bone loses the whole
			   selection instead of previewing a weapon stuck at the origin. */
			if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
					currentLevel,
					TEXT("Prototype_GameObject_Part_Equipment"),
					currentLevel,
					TEXT("Layer_AnimationPreview"),
					&weaponDesc,
					&stagedWeapon)))
			{
				CGameInstance::Get().Remove_GameObject_from_Layer(
					currentLevel,
					TEXT("Layer_AnimationPreview"),
					stagedObject);
				m_Status = string("Preview weapon is not admitted on ") +
					asset.pWeaponSocketBone + ": " + asset.pWeaponModelAssetId;
				return false;
			}
		}
	}

	/* The new body is staged before the old one is dropped, so a failure above
	   leaves the previous target published and editable. */
	Release(true);
	m_iPreviewParentMatrixIndex = stagedParentMatrixIndex;
	m_PreviewUnscaledParentMatrix = stagedParentMatrix;
	m_pPreviewObject = stagedObject;
	/* Published after Release so the drop of the previous selection cannot take
	   the weapon that was just staged for this one. */
	m_pPreviewWeaponObject = stagedWeapon;
	m_PreviewWeaponRestPose = std::move(stagedWeaponRestPose);
	m_pPreviewAsset = &asset;
	m_iPreviewLevelIndex = currentLevel;
	if (nullptr != stagedCharacter)
	{
		CAnimationTargetService::Bind_Preview(stagedCharacter);
		m_Status = string("Previewing live Character parts for ") +
			asset.pLabel +
			" 2.5 m to the right of the scene character.";
	}
	else if (nullptr != stagedValtan)
	{
		CAnimationTargetService::Bind_Preview(
			stagedValtan,
			asset.pAssetName);
		m_Status = string("Target=ARENA CLONE | ") + asset.pLabel +
			" body+axe | anchor=" + strPlacementSource +
			" | collision=OFF | Server Valtan=UNCHANGED.";
	}
	else
	{
		CAnimationTargetService::Bind_Preview(
			stagedBody->Get_Model(),
			asset.pAssetName,
			stagedParentMatrix);
		m_Status = bRaidCompositionPreview ?
			string("Target=LOCAL ARENA REFERENCE | ") + asset.pLabel +
			" | anchor=" + strPlacementSource + " | collision=OFF | Server boss=UNCHANGED." :
			string("Previewing ") + asset.pLabel +
			(nullptr != stagedWeapon ? " and its socketed weapon" : "") +
			" 2.5 m to the right of the scene character.";
	}
	Synchronize_PreviewWeapon();
	return true;
}

void Client::CCharacterPreviewPanel::Render_Selector(
	const bool_t isLocked,
	const string& strLockReason,
	const bool_t includePreviewOnlyTargets)
{
	ImGui::SeparatorText("Target");
	bool_t isSessionLocked = false;
	const string* pSessionLockReason = nullptr;
	for (size_t i = 0u; i < m_SessionLocks.size(); ++i)
	{
		if (!m_SessionLocks[i])
			continue;
		isSessionLocked = true;
		if (nullptr == pSessionLockReason)
			pSessionLockReason = &m_SessionLockReasons[i];
	}
	const bool_t isSelectionLocked = isLocked || isSessionLocked;
	const string& effectiveLockReason = nullptr != pSessionLockReason ?
		*pSessionLockReason : strLockReason;
	ImGui::BeginDisabled(isSelectionLocked);
	const bool_t sceneSelected = m_pPreviewObject.expired();
	if (ImGui::Selectable("Scene Character", sceneSelected))
		Release(true);

	for (const ANIMATION_PREVIEW_ASSET& asset : ANIMATION_PREVIEW_ASSETS)
	{
		if (!includePreviewOnlyTargets && !asset.bPlayableClassBody)
			continue;
		ImGui::PushID(asset.pId);
		const bool_t isSelected =
			!m_pPreviewObject.expired() && m_pPreviewAsset == &asset;
		if (ImGui::Selectable(asset.pLabel, isSelected) && !isSelected)
			Select_Asset(asset);
		ImGui::PopID();
	}
	ImGui::EndDisabled();
	if (isSelectionLocked && !effectiveLockReason.empty())
		ImGui::TextDisabled("%s", effectiveLockReason.c_str());
	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Separator();
}
