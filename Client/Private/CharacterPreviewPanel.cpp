#include "imgui.h"

#include "CharacterPreviewPanel.h"

#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "ActorCatalog.h"
#include "Character.h"
#include "GameInstance.h"
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
#include <utility>

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
	if (nullptr != m_pPreviewAsset &&
		strAnimationAssetName == m_pPreviewAsset->pAssetName &&
		!m_pPreviewObject.expired())
	{
		return true;
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
	return Select_Asset(*asset);
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
	if (currentLevel != ETOUI(LEVEL::CHARACTER_SELECT) &&
		currentLevel != ETOUI(LEVEL::DEVELOPMENT))
	{
		m_Status =
			"Character previews are admitted in Character Select or Development.";
		return false;
	}
	if (m_iPreparedGenericPreviewLevelIndex != currentLevel)
	{
		m_iPreparedGenericPreviewLevelIndex = currentLevel;
		m_PreparedGenericPreviewAssetIds.clear();
	}

	vector_t previewPosition = XMVectorSet(2.5f, 0.f, 0.f, 1.f);
	const shared_ptr<CCharacter> character =
		CAnimationTargetService::Resolve_SceneCharacter();
	if (nullptr != character && nullptr != character->Get_Transform())
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
		if (currentLevel == ETOUI(LEVEL::CHARACTER_SELECT) &&
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

		if (Declares_Weapon(asset))
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
	m_pPreviewObject = stagedObject;
	/* Published after Release so the drop of the previous selection cannot take
	   the weapon that was just staged for this one. */
	m_pPreviewWeaponObject = stagedWeapon;
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
		m_Status = string("Previewing product Valtan body and socketed axe for ") +
			asset.pLabel +
			" 2.5 m to the right of the scene character.";
	}
	else
	{
		CAnimationTargetService::Bind_Preview(
			stagedBody->Get_Model(),
			asset.pAssetName,
			stagedParentMatrix);
		m_Status = string("Previewing ") + asset.pLabel +
			(nullptr != stagedWeapon ? " and its socketed weapon" : "") +
			" 2.5 m to the right of the scene character.";
	}
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
