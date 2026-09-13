#include "Effect_Tool_Internal.h"
#include "AnimationSkillBindingDocument.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "CharacterSpec.h"
#include "CombatHUDViewModel.h"
#include "Effect_Catalog.h"
#include "Effect_Object.h"
#include "Effect_Playback.h"
#include "Effect_PresentationService.h"
#include "Effect_RuntimeAuthority.h"
#include "GameInstance.h"
#include "Logic_DimensionMaster.h"
#include "MapEffectPresentationRuntime.h"
#include "Model.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "Transform.h"
#include "CharacterPreviewPanel.h"

bool_t Client::CEffect_Tool::Synchronize_Artist31470FullPreview(
	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation)
{
	m_strPreviewAnimationStatus.clear();
	if (nullptr == pPreparation ||
		5u != pPreparation->Get_AnchorRequests().size())
	{
		m_strPreviewAnimationStatus =
			"Artist F exact five-anchor preparation is unavailable.";
		return false;
	}
	std::string CatalogStatus;
	if (!Ensure_PlayerSkillCatalog(CatalogStatus))
	{
		m_strPreviewAnimationStatus =
			"Artist F animation catalog unavailable: " + CatalogStatus;
		return false;
	}
	const vector<PLAYER_SKILL_DEFINITION>& Skills =
		CPlayerSkillCatalog::Get_Skills();
	const auto Skill = std::find_if(Skills.begin(), Skills.end(),
		[](const PLAYER_SKILL_DEFINITION& Candidate)
		{
			return Candidate.eCharacterClass ==
				LostArk::Shared::CHARACTER_CLASS_ID::ARTIST &&
				Candidate.iSkillId == 31470u;
		});
	if (Skill == Skills.end())
	{
		m_strPreviewAnimationStatus =
			"Artist F animation catalog row 31470 is unavailable.";
		return false;
	}
	const char* pAnimationAsset = Animation_AssetName(
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST);
	if (nullptr == pAnimationAsset ||
		(CAnimationTargetService::Resolve_AssetName() != pAnimationAsset &&
		 !m_pCharacterPreviewPanel->Select_TargetAsset(pAnimationAsset)))
	{
		m_strPreviewAnimationStatus =
			"Artist preview character could not be staged.";
		return false;
	}
	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Artist preview animation model is unavailable.";
		return false;
	}
	float4x4_t PreviewRoot{};
	if (!CAnimationTargetService::Resolve_RootTransform(&PreviewRoot))
	{
		m_strPreviewAnimationStatus =
			"Artist preview root transform is unavailable.";
		return false;
	}
	for (const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
		pPreparation->Get_AnchorRequests())
	{
		const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
		if (!Request.bFollow || Request.strRuntimeAnchorSlotId.empty() ||
			Request.strRuntimeBoneName.empty() ||
			!pModel->Has_Bone(Request.strRuntimeBoneName.c_str()))
		{
			m_strPreviewAnimationStatus =
				"Artist F required animation anchor is unavailable: " +
				Request.strRuntimeBoneName;
			return false;
		}
		EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
		XMStoreFloat4x4(&AnchorBuild.RawBone,
			pModel->Get_BoneMatrix(Request.strRuntimeBoneName.c_str()));
		AnchorBuild.OwnerWorld = PreviewRoot;
		float4x4_t Anchor{};
		if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(
			AnchorBuild, Anchor))
		{
			m_strPreviewAnimationStatus =
				"Artist F required animation anchor import transform is invalid: " +
				Request.strRuntimeBoneName;
			return false;
		}
	}
	ANIMATION_SKILL_BINDING_DOCUMENT Bindings;
	std::string BindingStatus;
	if (!CAnimationSkillBindingDocument::Load(pAnimationAsset,
		LostArk::Shared::CHARACTER_CLASS_ID::ARTIST, Skills,
		Collect_AnimationClipNames(pModel),
		Bindings, BindingStatus))
	{
		m_strPreviewAnimationStatus =
			"Artist F animation binding unavailable: " + BindingStatus;
		return false;
	}
	const auto Binding = std::find_if(Bindings.Bindings.begin(),
		Bindings.Bindings.end(), [](const ANIMATION_SKILL_BINDING& Candidate)
		{
			return Candidate.iSkillId == 31470u;
		});
	if (Binding == Bindings.Bindings.end())
	{
		m_strPreviewAnimationStatus =
			"Artist F animation binding row 31470 is unavailable.";
		return false;
	}
	vector<ANIMATION_SKILL_CLIP> StagedClips;
	for (const ANIMATION_SKILL_STAGE& Stage : Binding->Stages)
	{
		StagedClips.insert(
			StagedClips.end(),
			Stage.Clips.begin(), Stage.Clips.end());
	}
	if (StagedClips.empty())
	{
		m_strPreviewAnimationStatus =
			"Artist F animation binding contains no clips.";
		return false;
	}
	const ANIMATION_SKILL_CLIP FirstClip =
		StagedClips.front();
	const bool_t bSingleClip = 1u == StagedClips.size();
	if (!pModel->Start_Animation(
		FirstClip.strClipName.c_str(), bSingleClip && m_bPreviewLoop))
	{
		m_strPreviewAnimationStatus =
			"Artist F first animation clip is unavailable: " +
			FirstClip.strClipName;
		return false;
	}
	m_SynchronizedAnimationClips.assign(
		StagedClips.begin(), StagedClips.end());
	m_iSynchronizedAnimationClipIndex = 0u;
	m_iSynchronizedAnimationLoopEpoch = 0u;
	m_iSynchronizedAnimationTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	pModel->Set_AnimationSpeed(FirstClip.fPlayRate);
	pModel->Set_AnimPaused(true);
	m_strPreviewAnimationStatus = "Artist F animation prepared at zero: " +
		FirstClip.strClipName + " (skill 31470).";
	return true;
}

void Client::CEffect_Tool::Update_ReconstructedDiagnosticRoot()
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedDiagnosticActive())
	{
		m_bReconstructedDiagnosticActive = false;
		return;
	}
	matrix_t CameraWorld = XMLoadFloat4x4(
		CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW));
	float4x4_t DiagnosticRoot{};
	XMStoreFloat4x4(&DiagnosticRoot,
		XMMatrixTranslation(-2.25f, -0.4f, 4.5f) * CameraWorld);
	pObject->Set_RootWorld(DiagnosticRoot);
	pObject->Set_Visible(true);
}

bool_t Client::CEffect_Tool::Prepare_Artist31470HistoricalPoseBinding(
	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation,
	CAnimationHistoricalPoseBinding& OutPoseBinding,
	f32_t& fOutDurationSeconds,
	std::string& strOutError) const
{
	if (nullptr == pPreparation ||
		5u != pPreparation->Get_AnchorRequests().size() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		strOutError =
			"Artist F historical anchor target identity is invalid.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "Artist F historical anchor model is unavailable.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip ||
		m_SynchronizedAnimationClips.front().strClipName != pCurrentClip)
	{
		strOutError = "Artist F historical anchor clip identity changed.";
		return false;
	}

	std::vector<std::string> BoneNames;
	BoneNames.reserve(pPreparation->Get_AnchorRequests().size());
	for (const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding :
		pPreparation->Get_AnchorRequests())
	{
		const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
		if (!Request.bFollow || Request.strRuntimeAnchorSlotId.empty() ||
			Request.strRuntimeBoneName.empty())
		{
			strOutError =
				"Artist F historical anchor request is incomplete: " +
				Binding.strOwnerEmitterId;
			return false;
		}
		BoneNames.push_back(Request.strRuntimeBoneName);
	}

	CAnimationHistoricalPoseBinding StagedBinding;
	if (!CAnimationTargetService::Prepare_HistoricalPoseBinding(
			m_iSynchronizedAnimationTargetGeneration,
			iAnimationIndex, BoneNames, StagedBinding) ||
		StagedBinding.Get_BoneCount() != BoneNames.size())
	{
		strOutError =
			"Artist F historical bone binding could not be prepared.";
		return false;
	}
	f32_t fDurationSeconds = StagedBinding.Get_DurationSeconds();
	const uint32_t iPlayMs = m_SynchronizedAnimationClips.front().iPlayMs;
	if (0u != iPlayMs)
	{
		fDurationSeconds = (std::min)(fDurationSeconds,
			static_cast<f32_t>(iPlayMs) * 0.001f);
	}
	if (!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f)
	{
		strOutError = "Artist F historical animation duration is invalid.";
		return false;
	}

	OutPoseBinding = std::move(StagedBinding);
	fOutDurationSeconds = fDurationSeconds;
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Build_Artist31470HistoricalTransformSample(
	const std::shared_ptr<const
		EFFECT_RECONSTRUCTED_RUNTIME_PREPARATION>& pPreparation,
	const CAnimationHistoricalPoseBinding& PoseBinding,
	const f32_t fAnimationDurationSeconds,
	const f32_t fEffectSampleTimeSeconds,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (nullptr == pPreparation ||
		5u != pPreparation->Get_AnchorRequests().size() ||
		!std::isfinite(fAnimationDurationSeconds) ||
		fAnimationDurationSeconds <= 0.f ||
		!std::isfinite(fEffectSampleTimeSeconds) ||
		fEffectSampleTimeSeconds < 0.f || !PoseBinding.Is_Valid() ||
		PoseBinding.Get_BoneCount() !=
			pPreparation->Get_AnchorRequests().size() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		strOutError =
			"Artist F historical transform sample identity is invalid.";
		return false;
	}

	const f32_t fAnimationEndSeconds = (std::min)(
		fAnimationDurationSeconds, PoseBinding.Get_DurationSeconds());
	const f32_t fAnimationSampleTimeSeconds = std::clamp(
		fEffectSampleTimeSeconds, 0.f, fAnimationEndSeconds);
	ANIMATION_HISTORICAL_POSE_SAMPLE PoseSample;
	if (!CAnimationTargetService::Sample_HistoricalPose(
			PoseBinding, fAnimationSampleTimeSeconds, PoseSample) ||
		PoseSample.BoneCombinedMatrices.size() !=
			pPreparation->Get_AnchorRequests().size())
	{
		strOutError =
			"Artist F historical animation pose sampling failed.";
		return false;
	}

	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
	Staged.RootWorld = PoseSample.RootWorld;
	for (size_t iAnchor = 0u;
		iAnchor < pPreparation->Get_AnchorRequests().size(); ++iAnchor)
	{
		const EFFECT_RECONSTRUCTED_ANCHOR_BINDING& Binding =
			pPreparation->Get_AnchorRequests()[iAnchor];
		const EFFECT_RUNTIME_PROGRAM_ANCHOR_REQUEST& Request = Binding.Request;
		if (!Request.bFollow || Request.strRuntimeAnchorSlotId.empty() ||
			Request.strRuntimeBoneName.empty())
		{
			strOutError =
				"Artist F historical anchor request became invalid: " +
				Binding.strOwnerEmitterId;
			return false;
		}
		EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
		AnchorBuild.RawBone = PoseSample.BoneCombinedMatrices[iAnchor];
		AnchorBuild.OwnerWorld = PoseSample.RootWorld;
		float4x4_t BoneWorld{};
		if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(
				AnchorBuild, BoneWorld))
		{
			strOutError =
				"Artist F historical anchor import transform is invalid: " +
				Request.strRuntimeBoneName;
			return false;
		}

		const auto& Local = Request.SocketLocalTransform;
		const matrix_t SocketLocal = XMMatrixScaling(
			static_cast<f32_t>(Local.vScale[0]),
			static_cast<f32_t>(Local.vScale[1]),
			static_cast<f32_t>(Local.vScale[2])) *
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(static_cast<f32_t>(Local.vRotationDegrees[0])),
				XMConvertToRadians(static_cast<f32_t>(Local.vRotationDegrees[1])),
				XMConvertToRadians(static_cast<f32_t>(Local.vRotationDegrees[2]))) *
			XMMatrixTranslation(
				static_cast<f32_t>(Local.vPosition[0]),
				static_cast<f32_t>(Local.vPosition[1]),
				static_cast<f32_t>(Local.vPosition[2]));
		float4x4_t AnchorWorld{};
		XMStoreFloat4x4(&AnchorWorld,
			SocketLocal * XMLoadFloat4x4(&BoneWorld));
		const auto [It, bInserted] = Staged.SourceAnchorWorlds.emplace(
			Request.strRuntimeAnchorSlotId, AnchorWorld);
		if (!bInserted && 0 != std::memcmp(
				&It->second, &AnchorWorld, sizeof(float4x4_t)))
		{
			strOutError =
				"Artist F duplicate historical anchor slot disagrees: " +
				Request.strRuntimeAnchorSlotId;
			return false;
		}
	}
	if (Staged.SourceAnchorWorlds.empty())
	{
		strOutError = "Artist F historical anchor map is empty.";
		return false;
	}

	OutSample = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Prepare_ReconstructedSourceRuntimeTransformHistory()
{
	m_strPreviewAnimationStatus.clear();
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		m_strPreviewAnimationStatus =
			"Artist F historical anchor target identity is invalid.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		m_strPreviewAnimationStatus =
			"Artist F historical anchor model is unavailable.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip ||
		m_SynchronizedAnimationClips.front().strClipName != pCurrentClip)
	{
		m_strPreviewAnimationStatus =
			"Artist F historical anchor clip identity changed.";
		return false;
	}

	const auto Preparation = pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == Preparation || 5u != Preparation->Get_AnchorRequests().size())
	{
		m_strPreviewAnimationStatus =
			"Artist F exact five-anchor preparation is unavailable.";
		return false;
	}
	std::string Error;
	CAnimationHistoricalPoseBinding StagedBinding;
	f32_t fDurationSeconds = 0.f;
	if (!Prepare_Artist31470HistoricalPoseBinding(
			Preparation, StagedBinding, fDurationSeconds, Error))
	{
		m_strPreviewAnimationStatus = std::move(Error);
		return false;
	}

	m_ReconstructedSourceRuntimePoseBinding = std::move(StagedBinding);
	m_fPreviewDurationSeconds = fDurationSeconds;
	m_strPreviewAnimationStatus =
		"Artist F historical root plus five ordered anchor samples prepared.";
	return true;
}

bool_t Client::CEffect_Tool::Build_ReconstructedSourceRuntimeTransformSample(
	const f32_t fEffectSampleTimeSeconds,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (!m_bReconstructedSourceRuntimeActive ||
		!std::isfinite(fEffectSampleTimeSeconds) ||
		fEffectSampleTimeSeconds < 0.f ||
		!m_ReconstructedSourceRuntimePoseBinding.Is_Valid() ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() !=
			Animation_AssetName(LostArk::Shared::CHARACTER_CLASS_ID::ARTIST))
	{
		strOutError =
			"Artist F historical transform sample identity is invalid.";
		return false;
	}
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	const auto Preparation = nullptr == pObject ? nullptr :
		pObject->Get_ReconstructedRuntimePreparation();
	if (nullptr == pObject || !pObject->Is_ReconstructedSourceRuntimeActive() ||
		nullptr == Preparation || 5u != Preparation->Get_AnchorRequests().size() ||
		m_ReconstructedSourceRuntimePoseBinding.Get_BoneCount() !=
			Preparation->Get_AnchorRequests().size())
	{
		strOutError =
			"Artist F historical transform preparation was lost.";
		return false;
	}
	return Build_Artist31470HistoricalTransformSample(
		Preparation, m_ReconstructedSourceRuntimePoseBinding,
		m_fPreviewDurationSeconds, fEffectSampleTimeSeconds,
		OutSample, strOutError);
}

bool_t Client::CEffect_Tool::Prepare_ValtanBossPatternTransformHistory(
	const BOSS_PATTERN_EFFECT_BINDING& Binding,
	const EFFECT_DOCUMENT_DESC& Document,
	std::string& strOutError)
{
	m_bValtanBossPatternTransformHistoryRequired = true;
	m_bValtanBossPatternTransformHistoryActive = false;
	m_strValtanBossPatternPreviewEffectAssetId = Document.strEffectAssetId;
	m_strValtanBossPatternAnchorSlotId.clear();
	m_strValtanBossPatternBoneName.clear();
	m_fValtanBossPatternAnimationDurationSeconds = 0.f;
	m_ValtanBossPatternPoseBinding = {};

	if (!Matches_ValtanExactHistoryBinding(
			Binding.strBindingId, Binding.strEffectAssetId,
			Document.strEffectAssetId) ||
		Binding.strRuntimeClipName.empty() ||
		Binding.strRuntimeBoneName.empty() ||
		CAnimationTargetService::Resolve_AssetName() != VALTAN_ANIMATION_ASSET_NAME ||
		1u != m_SynchronizedAnimationClips.size() ||
		0u != m_iSynchronizedAnimationClipIndex ||
		m_SynchronizedAnimationClips.front().strClipName !=
			Binding.strRuntimeClipName ||
		0u == m_iSynchronizedAnimationTargetGeneration ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration())
	{
		strOutError =
			"Valtan 420633 transform history target identity is invalid.";
		return false;
	}

	size_t iVisibleExecutionCount = 0u;
	size_t iFollowCarrierCount = 0u;
	EFFECT_TRANSFORM_DESC SocketLocalTransform{};
	bool_t bHasSocketLocalTransform = false;
	for (const EFFECT_ELEMENT_DESC& Element : Document.Elements)
	{
		if (!Element.bVisible ||
			!Is_EffectElementAuthoringExecutionTarget(Element))
		{
			continue;
		}
		++iVisibleExecutionCount;
		const EFFECT_ACTION_CUE_ATTACHMENT_DESC& Attachment =
			Element.ActionCueAttachment;
		if (!Attachment.bEnabled || !Attachment.bFollow ||
			Attachment.strRuntimeAnchorSlotId.empty() ||
			Attachment.strRuntimeBoneName != Binding.strRuntimeBoneName)
		{
			strOutError =
				"Valtan 420633 visible carrier lost its exact follow attachment.";
			return false;
		}
		if (m_strValtanBossPatternAnchorSlotId.empty())
		{
			m_strValtanBossPatternAnchorSlotId =
				Attachment.strRuntimeAnchorSlotId;
			m_strValtanBossPatternBoneName = Attachment.strRuntimeBoneName;
			SocketLocalTransform = Attachment.SocketLocalTransform;
			bHasSocketLocalTransform = true;
		}
		else if (m_strValtanBossPatternAnchorSlotId !=
				Attachment.strRuntimeAnchorSlotId ||
			m_strValtanBossPatternBoneName != Attachment.strRuntimeBoneName ||
			0 != std::memcmp(&SocketLocalTransform,
				&Attachment.SocketLocalTransform,
				sizeof(EFFECT_TRANSFORM_DESC)))
		{
			strOutError =
				"Valtan 420633 visible follow carriers disagree on their anchor contract.";
			return false;
		}
		++iFollowCarrierCount;
	}
	const size_t iExpectedFollowCarrierCount =
		Document.strEffectAssetId == VALTAN_EXACT_HISTORY_V1_EFFECT_ASSET_ID ?
			5u : 3u;
	if (iExpectedFollowCarrierCount != iVisibleExecutionCount ||
		iExpectedFollowCarrierCount != iFollowCarrierCount ||
		!bHasSocketLocalTransform ||
		m_strValtanBossPatternAnchorSlotId != "B_EffectRoot" ||
		m_strValtanBossPatternBoneName != "b_effectroot")
	{
		strOutError =
			"Valtan 420633 preview has an invalid B_EffectRoot follow-carrier count.";
		return false;
	}

	const shared_ptr<Engine::CModel> pModel =
		CAnimationTargetService::Resolve_Model();
	if (nullptr == pModel)
	{
		strOutError = "Valtan 420633 preview model is unavailable.";
		return false;
	}
	const uint32_t iAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentClip = pModel->Get_AnimationName(iAnimationIndex);
	if (nullptr == pCurrentClip || Binding.strRuntimeClipName != pCurrentClip)
	{
		strOutError = "Valtan 420633 preview clip identity changed.";
		return false;
	}

	const std::array<std::string, 1u> BoneNames = {
		m_strValtanBossPatternBoneName };
	CAnimationHistoricalPoseBinding StagedBinding;
	if (!CAnimationTargetService::Prepare_HistoricalPoseBinding(
			m_iSynchronizedAnimationTargetGeneration, iAnimationIndex,
			BoneNames, StagedBinding) || 1u != StagedBinding.Get_BoneCount())
	{
		strOutError =
			"Valtan 420633 b_effectroot historical pose binding failed.";
		return false;
	}
	const f32_t fDurationSeconds = StagedBinding.Get_DurationSeconds();
	if (!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f)
	{
		strOutError = "Valtan 420633 animation duration is invalid.";
		return false;
	}

	m_ValtanBossPatternPoseBinding = std::move(StagedBinding);
	m_ValtanBossPatternSocketLocalTransform = SocketLocalTransform;
	m_fValtanBossPatternAnimationDurationSeconds = fDurationSeconds;
	m_bValtanBossPatternTransformHistoryActive = true;
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Build_ValtanBossPatternTransformSample(
	const f32_t fEffectSampleTimeSeconds,
	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
	std::string& strOutError) const
{
	if (!m_bValtanBossPatternTransformHistoryRequired ||
		!m_bValtanBossPatternTransformHistoryActive ||
		!m_ValtanBossPatternPoseBinding.Is_Valid() ||
		1u != m_ValtanBossPatternPoseBinding.Get_BoneCount() ||
		m_strValtanBossPatternPreviewEffectAssetId.empty() ||
		m_strValtanBossPatternAnchorSlotId != "B_EffectRoot" ||
		m_strValtanBossPatternBoneName != "b_effectroot" ||
		!std::isfinite(fEffectSampleTimeSeconds) ||
		fEffectSampleTimeSeconds < 0.f ||
		!std::isfinite(m_fValtanBossPatternAnimationDurationSeconds) ||
		m_fValtanBossPatternAnimationDurationSeconds <= 0.f ||
		m_iSynchronizedAnimationTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_AssetName() != VALTAN_ANIMATION_ASSET_NAME)
	{
		strOutError =
			"Valtan 420633 historical transform sample identity is invalid.";
		return false;
	}

	ANIMATION_HISTORICAL_POSE_SAMPLE PoseSample;
	const f32_t fAnimationSampleTimeSeconds = std::clamp(
		fEffectSampleTimeSeconds, 0.f,
		(std::min)(m_fValtanBossPatternAnimationDurationSeconds,
			m_ValtanBossPatternPoseBinding.Get_DurationSeconds()));
	if (!CAnimationTargetService::Sample_HistoricalPose(
			m_ValtanBossPatternPoseBinding, fAnimationSampleTimeSeconds,
			PoseSample) || 1u != PoseSample.BoneCombinedMatrices.size())
	{
		strOutError =
			"Valtan 420633 b_effectroot historical sampling failed.";
		return false;
	}

	EFFECT_SOURCE_BONE_ANCHOR_BUILD_DESC AnchorBuild;
	AnchorBuild.RawBone = PoseSample.BoneCombinedMatrices.front();
	if (!m_ValtanProductPreview.has_value())
	{
		strOutError =
			"Valtan 420633 cue scale policy is unavailable.";
		return false;
	}
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue =
		m_ValtanProductPreview->Cue;
	float4x4_t EffectiveOwnerRoot{};
	float4x4_t CueRoot{};
	if (!CEffectPresentationService::Build_CueScalePolicyAnchor(
			Cue.eScalePolicy, Cue.vWorldScale, PoseSample.RootWorld,
			EffectiveOwnerRoot) ||
		!CEffectPresentationService::Build_CueScalePolicyRoot(
			Cue.LocalTransform, Cue.eScalePolicy, Cue.vWorldScale,
			PoseSample.RootWorld, CueRoot))
	{
		strOutError =
			"Valtan 420633 cue scale policy transform is invalid.";
		return false;
	}
	AnchorBuild.OwnerWorld = EffectiveOwnerRoot;
	float4x4_t BoneWorld{};
	if (!CEffectPresentationService::Build_SourceBoneAnchorWorld(
			AnchorBuild, BoneWorld))
	{
		strOutError =
			"Valtan 420633 b_effectroot import transform is invalid.";
		return false;
	}

	const EFFECT_TRANSFORM_DESC& Local =
		m_ValtanBossPatternSocketLocalTransform;
	const matrix_t SocketLocal = XMMatrixScaling(
		Local.vScale.x, Local.vScale.y, Local.vScale.z) *
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(Local.vRotationDegrees.x),
			XMConvertToRadians(Local.vRotationDegrees.y),
			XMConvertToRadians(Local.vRotationDegrees.z)) *
		XMMatrixTranslation(
			Local.vPosition.x, Local.vPosition.y, Local.vPosition.z);
	float4x4_t AnchorWorld{};
	XMStoreFloat4x4(&AnchorWorld,
		SocketLocal * XMLoadFloat4x4(&BoneWorld));

	EFFECT_FIXED_STEP_TRANSFORM_SAMPLE Staged;
	Staged.RootWorld = CueRoot;
	Staged.SourceAnchorWorlds.emplace(
		m_strValtanBossPatternAnchorSlotId, AnchorWorld);
	OutSample = std::move(Staged);
	strOutError.clear();
	return true;
}

bool_t Client::CEffect_Tool::Seek_ValtanBossPatternTransformHistory(
	const shared_ptr<CEffectObject>& pObject,
	const f32_t fEffectSampleTimeSeconds,
	std::string& strOutError) const
{
	if (nullptr == pObject ||
		!m_bValtanBossPatternTransformHistoryRequired ||
		!m_bValtanBossPatternTransformHistoryActive)
	{
		strOutError =
			"Valtan 420633 exact transform history is unavailable.";
		return false;
	}
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fSampleTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strProviderError)
		{
			return Build_ValtanBossPatternTransformSample(
				fSampleTimeSeconds, OutSample, strProviderError);
		};
	return pObject->Set_SampleTimeWithTransformHistory(
		fEffectSampleTimeSeconds, TransformProvider, strOutError);
}

void Client::CEffect_Tool::Reset_ValtanBossPatternTransformHistory()
{
	m_ValtanBossPatternPoseBinding = {};
	m_ValtanBossPatternSocketLocalTransform = {};
	m_strValtanBossPatternPreviewEffectAssetId.clear();
	m_strValtanBossPatternAnchorSlotId.clear();
	m_strValtanBossPatternBoneName.clear();
	m_fValtanBossPatternAnimationDurationSeconds = 0.f;
	m_bValtanBossPatternTransformHistoryRequired = false;
	m_bValtanBossPatternTransformHistoryActive = false;
}

bool_t Client::CEffect_Tool::Update_ReconstructedSourceRuntimeTimeline(
	const f32_t fTimeDelta)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !m_bReconstructedSourceRuntimeActive)
	{
		m_bReconstructedSourceRuntimeActive = false;
		Reset_ReconstructedSourceRuntimeTimeline();
		return false;
	}
	const auto FailPreview = [this, &pObject](const std::string& Reason)
	{
		pObject->Set_Playing(false);
		pObject->Set_Visible(false);
		Set_SynchronizedAnimationPaused(true);
		Reset_SynchronizedAnimationSequence();
		Reset_ReconstructedSourceRuntimeTimeline();
		m_bReconstructedSourceRuntimeActive = false;
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus = Reason;
		return false;
	};
	if (pObject->Is_RenderFailureIsolated())
		return FailPreview(pObject->Get_Status());
	if (!pObject->Is_ReconstructedSourceRuntimeActive())
	{
		m_bReconstructedSourceRuntimeActive = false;
		Reset_ReconstructedSourceRuntimeTimeline();
		return false;
	}
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fSampleTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_ReconstructedSourceRuntimeTransformSample(
				fSampleTimeSeconds, OutSample, strOutError);
		};

	/* Tool previews are clocked only from the bound animation.  Keeping the
	   Object's autonomous update disabled prevents the same wall-clock delta
	   from being consumed by both the Level and the Effect Tool. */
	pObject->Set_Playing(false);
	if (m_bReconstructedSourceRuntimeStartPending)
	{
		/* Force the model's zero pose to refresh without exposing an unpaused
		   Engine update to the cache-build delta. */
		Seek_SynchronizedAnimationSequence(0.f);
		Set_SynchronizedAnimationPaused(true);
		f32_t fVisibleAnimationTimeSeconds = 0.f;
		if (!Try_ResolveSynchronizedAnimationTime(
				fVisibleAnimationTimeSeconds) ||
			std::abs(fVisibleAnimationTimeSeconds) > 1.0e-5f)
		{
			return FailPreview(
				"Artist F visible animation could not commit synchronized time zero.");
		}
		/* The expensive prepared-cache frame has already passed through Engine
		   update while both participants were paused.  Publish an exact zero frame
		   before allowing the animation clock to advance. */
		std::string Error;
		if (!pObject->Set_SampleTimeWithTransformHistory(
				0.f, TransformProvider, Error))
		{
			return FailPreview(
				"Artist F zero-frame history sample failed: " + Error);
		}
		if (std::abs(pObject->Get_PreviewFixedStepClockSeconds()) > 1.0e-6)
		{
			return FailPreview(
				"Artist F zero-frame effect clock diverged from animation time zero.");
		}
		m_fPreviewTimeSeconds = 0.f;
		m_fReconstructedSourceRuntimeClockSeconds = 0.f;
		m_bReconstructedSourceRuntimeStartPending = false;
		pObject->Set_Visible(m_bPreviewVisibleRequested);
		Set_SynchronizedAnimationPaused(!m_bPreviewPlaying);
	m_strPreviewStatus = m_bPreviewPlaying ?
			"Artist Core F (33) is running from synchronized time zero; Product remains OFF." :
			"Artist Core F (33) is paused at synchronized time zero; Product remains OFF.";
		return true;
	}
	if (m_bReconstructedSourceRuntimeNaturalTailActive)
	{
		const f32_t fTailDelta = std::isfinite(fTimeDelta) ?
			std::clamp(fTimeDelta, 0.f, 0.1f) : 0.f;
		if (m_bPreviewPlaying && fTailDelta > 0.f)
		{
			std::string Error;
			if (!pObject->Advance_PreviewWithTransformHistory(
					fTailDelta, TransformProvider, Error))
			{
				return FailPreview(
					"Artist F Natural Stop anchor history failed: " + Error);
			}
			m_fReconstructedSourceRuntimeTailSeconds += fTailDelta;
		}
		if (pObject->Is_Finished())
		{
			m_bReconstructedSourceRuntimeNaturalTailActive = false;
			m_bPreviewPlaying = false;
			m_strPreviewStatus =
				"Artist Core F (33) Natural Stop tail completed; Product remains OFF.";
		}
		return true;
	}

	f32_t fAnimationTimeSeconds = 0.f;
	if (!Try_ResolveSynchronizedAnimationTime(fAnimationTimeSeconds))
	{
		return FailPreview(
			"Artist F preview stopped: synchronized animation time is unavailable.");
	}
	fAnimationTimeSeconds = std::clamp(
		fAnimationTimeSeconds, 0.f, m_fPreviewDurationSeconds);
	const f32_t fPreviousTimeSeconds =
		m_fReconstructedSourceRuntimeClockSeconds;
	const bool_t bTimelineWrapped =
		fAnimationTimeSeconds + 0.0001f < fPreviousTimeSeconds;

	std::string Error;
	if (bTimelineWrapped)
	{
		if (!pObject->Set_SampleTimeWithTransformHistory(
				fAnimationTimeSeconds, TransformProvider, Error))
		{
			return FailPreview(
				"Artist F loop history replay failed: " + Error);
		}
	}
	else if (m_bPreviewPlaying &&
		fAnimationTimeSeconds > fPreviousTimeSeconds)
	{
		if (!pObject->Advance_PreviewWithTransformHistory(
				fAnimationTimeSeconds - fPreviousTimeSeconds,
				TransformProvider, Error))
		{
			return FailPreview(
				"Artist F fixed-step anchor history failed: " + Error);
		}
	}
	const f64_t fEffectClockSeconds =
		pObject->Get_PreviewFixedStepClockSeconds();
	if (!std::isfinite(fEffectClockSeconds) ||
		std::abs(fEffectClockSeconds -
			static_cast<f64_t>(fAnimationTimeSeconds)) > 1.0e-5)
	{
		return FailPreview(
			"Artist F effect clock diverged from its animation clock.");
	}
	m_fPreviewTimeSeconds = fAnimationTimeSeconds;
	m_fReconstructedSourceRuntimeClockSeconds = fAnimationTimeSeconds;

	if (m_bPreviewPlaying && !m_bPreviewLoop &&
		fAnimationTimeSeconds + 0.0001f >= m_fPreviewDurationSeconds)
	{
		Set_SynchronizedAnimationPaused(true);
		m_bReconstructedSourceRuntimeNaturalTailActive =
			!pObject->Is_Finished();
		m_fReconstructedSourceRuntimeTailSeconds = 0.f;
		if (m_bReconstructedSourceRuntimeNaturalTailActive)
		{
			m_strPreviewStatus =
				"Artist F animation reached its synchronized end; Natural Stop particle tail is running.";
		}
		else
		{
			m_bPreviewPlaying = false;
			m_strPreviewStatus =
				"Artist Core F (33) reached its synchronized end and completed; Product remains OFF.";
		}
	}
	return true;
}

bool_t Client::CEffect_Tool::Seek_ReconstructedSourceRuntimeTimeline(
	const f32_t fSampleTimeSeconds)
{
	const shared_ptr<CEffectObject> pObject = m_pWorldPreviewObject.lock();
	if (nullptr == pObject || !m_bReconstructedSourceRuntimeActive ||
		!std::isfinite(fSampleTimeSeconds))
	{
		return false;
	}
	const f32_t fClampedTime = std::clamp(
		fSampleTimeSeconds, 0.f, m_fPreviewDurationSeconds);
	const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER TransformProvider =
		[this](const f32_t fHistoryTimeSeconds,
			EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& OutSample,
			std::string& strOutError)
		{
			return Build_ReconstructedSourceRuntimeTransformSample(
				fHistoryTimeSeconds, OutSample, strOutError);
		};
	std::string Error;
	/* History replay is validated and committed before the visible model seek.
	   Internal fixed steps never seek or mutate the live animation cursor. */
	if (!pObject->Set_SampleTimeWithTransformHistory(
			fClampedTime, TransformProvider, Error))
	{
		m_strPreviewStatus =
			"Artist F synchronized history seek failed: " + Error;
		return false;
	}
	if (std::abs(pObject->Get_PreviewFixedStepClockSeconds() -
			static_cast<f64_t>(fClampedTime)) > 1.0e-5)
	{
		m_strPreviewStatus =
			"Artist F synchronized history seek produced a divergent effect clock.";
		return false;
	}
	m_bPreviewPlaying = false;
	m_bReconstructedSourceRuntimeStartPending = false;
	m_bReconstructedSourceRuntimeNaturalTailActive = false;
	m_fReconstructedSourceRuntimeTailSeconds = 0.f;
	m_fPreviewTimeSeconds = fClampedTime;
	m_fReconstructedSourceRuntimeClockSeconds = fClampedTime;
	pObject->Set_Playing(false);
	Seek_SynchronizedAnimationSequence(fClampedTime);
	Set_SynchronizedAnimationPaused(true);
	f32_t fVisibleAnimationTimeSeconds = 0.f;
	if (!Try_ResolveSynchronizedAnimationTime(
			fVisibleAnimationTimeSeconds) ||
		std::abs(fVisibleAnimationTimeSeconds - fClampedTime) > 1.0e-4f)
	{
		pObject->Set_Visible(false);
		Reset_SynchronizedAnimationSequence();
		Reset_ReconstructedSourceRuntimeTimeline();
		m_bReconstructedSourceRuntimeActive = false;
		m_bPreviewPlaying = false;
		m_bPreviewVisibleRequested = false;
		m_strPreviewStatus =
			"Artist F visible animation seek diverged from the effect clock.";
		return false;
	}
	pObject->Set_Visible(m_bPreviewVisibleRequested);
	m_strPreviewStatus =
		"Artist Core F (33) sampled from the synchronized animation clock; Product remains OFF.";
	return true;
}

void Client::CEffect_Tool::Reset_ReconstructedSourceRuntimeTimeline()
{
	m_bReconstructedSourceRuntimeStartPending = false;
	m_bReconstructedSourceRuntimeNaturalTailActive = false;
	m_fReconstructedSourceRuntimeClockSeconds = 0.f;
	m_fReconstructedSourceRuntimeTailSeconds = 0.f;
}
