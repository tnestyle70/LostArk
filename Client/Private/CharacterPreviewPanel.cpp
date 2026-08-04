#include "imgui.h"

#include "CharacterPreviewPanel.h"

#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "GameInstance.h"
#include "Model.h"
#include "Part_Body.h"
#include "Transform.h"

Client::CCharacterPreviewPanel::~CCharacterPreviewPanel()
{
	/* The owning tool may outlive the level the body was staged into, so the
	   layer removal is skipped and only the published target is cleared. */
	Release(false);
}

void Client::CCharacterPreviewPanel::Refresh_Level()
{
	const shared_ptr<CPart_Body> previewBody = m_pPreviewBody.lock();
	if (nullptr == previewBody)
	{
		m_pPreviewBody.reset();
		m_pPreviewAsset = nullptr;
		m_iPreviewLevelIndex = UINT32_MAX;
		return;
	}

	const uint32_t currentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	if (currentLevel == m_iPreviewLevelIndex)
		return;

	CAnimationTargetService::Unbind_Preview(previewBody->Get_Model());
	m_pPreviewBody.reset();
	m_pPreviewAsset = nullptr;
	m_iPreviewLevelIndex = UINT32_MAX;
}

void Client::CCharacterPreviewPanel::Release(const bool_t removeFromLayer)
{
	const shared_ptr<CPart_Body> previewBody = m_pPreviewBody.lock();
	if (nullptr == previewBody)
	{
		m_pPreviewBody.reset();
		m_pPreviewAsset = nullptr;
		m_iPreviewLevelIndex = UINT32_MAX;
		return;
	}

	CAnimationTargetService::Unbind_Preview(previewBody->Get_Model());
	if (removeFromLayer &&
		m_iPreviewLevelIndex ==
			CGameInstance::Get().Get_CurrentLevelID())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			m_iPreviewLevelIndex,
			TEXT("Layer_AnimationPreview"),
			previewBody);
	}
	m_pPreviewBody.reset();
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

	vector_t previewPosition = XMVectorSet(2.5f, 0.f, 0.f, 1.f);
	const shared_ptr<CCharacter> character =
		CAnimationTargetService::Resolve_Character();
	if (nullptr != character && nullptr != character->Get_Transform())
	{
		previewPosition = XMVectorAdd(
			previewPosition,
			character->Get_Transform()->Get_State(STATE::POSITION));
		previewPosition = XMVectorSetW(previewPosition, 1.f);
	}
	XMStoreFloat4x4(
		&m_PreviewParentMatrix,
		XMMatrixTranslationFromVector(previewPosition));

	CPart_Body::PART_BODY_DESC desc{};
	desc.pParentMatrix = &m_PreviewParentMatrix;
	desc.iPrototypeLevelIndex = currentLevel;
	desc.strModelTag = asset.pPrototypeTag;
	desc.strShaderTag =
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
	desc.pInitialAnimation = nullptr;

	shared_ptr<CGameObject> stagedObject;
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

	const shared_ptr<CPart_Body> stagedBody =
		dynamic_pointer_cast<CPart_Body>(stagedObject);
	if (nullptr == stagedBody || nullptr == stagedBody->Get_Model())
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			currentLevel,
			TEXT("Layer_AnimationPreview"),
			stagedObject);
		m_Status = "Preview body did not expose an animated CModel.";
		return false;
	}

	/* The new body is staged before the old one is dropped, so a failure above
	   leaves the previous target published and editable. */
	Release(true);
	m_pPreviewBody = stagedBody;
	m_pPreviewAsset = &asset;
	m_iPreviewLevelIndex = currentLevel;
	CAnimationTargetService::Bind_Preview(
		stagedBody->Get_Model(),
		asset.pAssetName,
		m_PreviewParentMatrix);
	m_Status = string("Previewing ") + asset.pLabel +
		" 2.5 m to the right of the scene character.";
	return true;
}

void Client::CCharacterPreviewPanel::Render_Selector(
	const bool_t isLocked,
	const string& strLockReason)
{
	ImGui::SeparatorText("Target");
	ImGui::BeginDisabled(isLocked);
	const bool_t sceneSelected = m_pPreviewBody.expired();
	if (ImGui::Selectable("Scene Character", sceneSelected))
		Release(true);

	for (const ANIMATION_PREVIEW_ASSET& asset : ANIMATION_PREVIEW_ASSETS)
	{
		ImGui::PushID(asset.pId);
		const bool_t isSelected =
			!m_pPreviewBody.expired() && m_pPreviewAsset == &asset;
		if (ImGui::Selectable(asset.pLabel, isSelected) && !isSelected)
			Select_Asset(asset);
		ImGui::PopID();
	}
	ImGui::EndDisabled();
	if (isLocked && !strLockReason.empty())
		ImGui::TextDisabled("%s", strLockReason.c_str());
	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Separator();
}
