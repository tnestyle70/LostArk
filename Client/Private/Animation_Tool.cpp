#include "imgui.h"
#include "Animation_Tool_Internal.h"
#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "BalanceTool.h"
#include "ValtanBossTool.h"
#include "CameraTool.h"
#include "Character.h"
#include "Effect_Catalog.h"
#include "Effect_Tool.h"
#include "EffectAuthoringTransfer.h"
#include "GameInstance.h"
#include "HitAreaWire.h"
#include "Model.h"
#include "SoundCueCatalog.h"
#include "Valtan.h"
#include <charconv>
#include <algorithm>
#include <array>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <sstream>
#include <span>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include "CharacterPreviewPanel.h"




Client::CAnimation_Tool::CAnimation_Tool(
	shared_ptr<CCharacterPreviewPanel> pPreviewPanel,
	CBalanceTool* const pBalanceTool,
	CValtanBossTool* const pValtanBossTool,
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pPreviewPanel(std::move(pPreviewPanel))
	, m_pPreviewDevice(std::move(pDevice))
	, m_pPreviewContext(std::move(pContext))
	, m_pBalanceTool(pBalanceTool)
	, m_pValtanBossTool(pValtanBossTool)
{
}

Client::CAnimation_Tool::~CAnimation_Tool()
{
	Reset_KoukuCompositionEffects();
	Apply_KoukuSaydonPreviewScale(m_KoukuScaledPreviewModel.lock(), 1.f);
	if (nullptr != m_hValtanPatternCreateProcess)
	{
		/* The child owns an all-or-nothing source/Product transaction.  Closing
		   our observation handle on tool teardown must not terminate it midway;
		   the preserved request and diagnostic paths remain the recovery trail. */
		CloseHandle(static_cast<HANDLE>(m_hValtanPatternCreateProcess));
		m_hValtanPatternCreateProcess = nullptr;
	}
	if (nullptr != m_pPreviewPanel)
		m_pPreviewPanel->Set_SessionLock(
			CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, false, {});
}

bool_t Client::CAnimation_Tool::Consume_EffectToolOpenRequest(
	EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& outRequest)
{
	if (!m_hasEffectToolOpenRequest)
		return false;
	outRequest.strPatternId = std::move(m_strEffectToolOpenPatternId);
	outRequest.strStageId = std::move(m_strEffectToolOpenStageId);
	outRequest.strCueOccurrenceId =
		std::move(m_strEffectToolOpenCueOccurrenceId);
	outRequest.strEffectAssetId = std::move(m_strEffectToolOpenEffectAssetId);
	m_strEffectToolOpenPatternId.clear();
	m_strEffectToolOpenStageId.clear();
	m_strEffectToolOpenCueOccurrenceId.clear();
	m_strEffectToolOpenEffectAssetId.clear();
	m_hasEffectToolOpenRequest = false;
	return true;
}

bool_t Client::CAnimation_Tool::Consume_CameraToolOpenRequest(
	CAMERA_TOOL_OPEN_REQUEST& outRequest)
{
	if (!m_hasCameraToolOpenRequest)
		return false;
	outRequest.strCueId = std::move(m_strCameraToolOpenCueId);
	m_strCameraToolOpenCueId.clear();
	m_hasCameraToolOpenRequest = false;
	return true;
}

void Client::CAnimation_Tool::Update(
	const f32_t fTimeDelta,
	const bool_t bIsActiveTool)
{
	// MainApp calls this after the level updates all parts. Every return path
	// must publish the hammer after the final body pose, including inactive UI.
	struct PREVIEW_WEAPON_POSE_SYNC final
	{
		CCharacterPreviewPanel* panel;
		~PREVIEW_WEAPON_POSE_SYNC() { if (nullptr != panel) panel->Synchronize_PreviewWeapon(); }
	} weaponPoseSync{ m_pPreviewPanel.get() };
	Poll_ValtanPatternCreateCommand();
	if (!bIsActiveTool || m_KoukuScaledPreviewModel.lock() != CAnimationTargetService::Resolve_Model())
		Apply_KoukuSaydonPreviewScale(m_KoukuScaledPreviewModel.lock(), 1.f);

	if (m_bKoukuSaydonCompositionAnimationPreviewPending || m_bKoukuSaydonCompositionPatternPreviewPending)
		(void)Start_PendingKoukuSaydonCompositionPreview(CAnimationTargetService::Resolve_Model());
	if (m_bKoukuCompositionTimelinePlaying)
	{
		const auto model = m_KoukuSaydonPatternPreviewModel.lock();
		if (!bIsActiveTool || nullptr == model || CAnimationTargetService::Resolve_Model() != model ||
			m_iKoukuSaydonPatternPreviewTargetGeneration != CAnimationTargetService::Resolve_TargetGeneration())
		{
			Stop_KoukuSaydonPatternPreview(model, "Composition preview stopped; target or tool changed.");
			return;
		}
		if (!m_bKoukuSaydonPatternPreviewPaused && std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
			m_fKoukuCompositionPreviewClockMs += static_cast<double>(fTimeDelta) * 1000.0;
		if (!m_bKoukuSaydonPatternPreviewPaused &&
			m_fKoukuCompositionPreviewClockMs >= m_iKoukuCompositionPreviewDurationMs)
		{
			m_fKoukuCompositionPreviewClockMs = m_iKoukuCompositionPreviewDurationMs;
			m_bKoukuSaydonPatternPreviewPaused = true;
			m_Status = m_strKoukuSaydonPatternStatus =
				"Composition preview completed; end pose held. Resume restarts from zero.";
		}
		Sample_KoukuSaydonCompositionPreview(model);
		return;
	}

	if (m_bValtanPatternMasterPlaying)
	{
		const shared_ptr<Engine::CModel> PreviewModel =
			m_ValtanPatternMasterModel.lock();
		const shared_ptr<CValtan> PreviewBoss =
			m_ValtanPatternMasterBoss.lock();
		if (nullptr == PreviewModel || nullptr == PreviewBoss ||
			0u == m_iValtanPatternMasterTargetGeneration ||
			m_iValtanPatternMasterTargetGeneration !=
				CAnimationTargetService::Resolve_TargetGeneration() ||
			CAnimationTargetService::Resolve_Model() != PreviewModel ||
			CAnimationTargetService::Resolve_Boss() != PreviewBoss ||
			PreviewBoss->Get_BodyModel() != PreviewModel)
		{
			if (nullptr != PreviewModel)
			{
				Stop_ValtanPatternMasterPreview(
					PreviewModel,
					"Valtan Pattern Master preview cancelled because the animation target changed; the original boss was restored to idle.");
			}
			else
			{
				Reset_ValtanPatternMasterPreviewState(
					"Valtan Pattern Master preview cancelled because the animation target changed.");
			}
			return;
		}
		if (!bIsActiveTool)
		{
			Stop_ValtanPatternMasterPreview(
				PreviewModel,
				"Valtan Pattern Master preview stopped because Animation Tool was deactivated; idle restored.");
			return;
		}
		if (m_iValtanPatternMasterItem >=
			m_ValtanPatternMasterPlaylist.size())
		{
			Stop_ValtanPatternMasterPreview(
				PreviewModel,
				"Valtan Pattern Master preview stopped because its admitted timeline became invalid; idle restored.");
			return;
		}

		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		const char_t* pCurrentClip = PreviewModel->Get_AnimationName(
			PreviewModel->Get_CurrentAnimIndex());
		if (nullptr == pCurrentClip || Item.strClipName != pCurrentClip)
		{
			Stop_ValtanPatternMasterPreview(
				PreviewModel,
				"Valtan Pattern Master preview stopped because another control replaced its clip; idle restored.");
			return;
		}
		if (!m_bValtanPatternMasterPaused &&
			std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
		{
			m_fValtanPatternMasterItemElapsedSeconds +=
				fTimeDelta * m_fValtanPatternPreviewSpeed;
			const f32_t fItemDurationSeconds =
				static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f;
			if (m_fValtanPatternMasterItemElapsedSeconds + 0.000001f >=
				fItemDurationSeconds)
			{
				Advance_ValtanPatternMasterPreview(PreviewModel);
			}
			else if (!Apply_ValtanPatternMasterPose(
				PreviewModel, Item,
				m_fValtanPatternMasterItemElapsedSeconds, false))
			{
				Stop_ValtanPatternMasterPreview(
					PreviewModel,
					"Valtan Pattern Master preview stopped because its source-clock sample failed; idle restored.");
				return;
			}
		}
		Update_ValtanPatternMasterHitAreaPreview();
		return;
	}

	if (m_bKoukuSaydonPatternPreviewPlaying)
	{
		const shared_ptr<Engine::CModel> PreviewModel =
			m_KoukuSaydonPatternPreviewModel.lock();
		if (nullptr == PreviewModel ||
			0u == m_iKoukuSaydonPatternPreviewTargetGeneration ||
			m_iKoukuSaydonPatternPreviewTargetGeneration !=
				CAnimationTargetService::Resolve_TargetGeneration() ||
			CAnimationTargetService::Resolve_Model() != PreviewModel)
		{
			if (nullptr != PreviewModel)
			{
				Stop_KoukuSaydonPatternPreview(
					PreviewModel,
					"KoukuSaydon Pattern preview cancelled because the animation target changed; the original model was restored to idle.");
			}
			else
			{
				Reset_KoukuSaydonPatternPreviewState(
					"KoukuSaydon Pattern preview cancelled because the animation target changed.");
			}
			return;
		}
		if (!bIsActiveTool)
		{
			Stop_KoukuSaydonPatternPreview(
				PreviewModel,
				"KoukuSaydon Pattern preview stopped because Animation Tool was deactivated; idle restored.");
			return;
		}
		if (m_iKoukuSaydonPatternPreviewClip >= m_KoukuSaydonPatternPreviewClips.size())
		{
			Stop_KoukuSaydonPatternPreview(
				PreviewModel,
				"KoukuSaydon Pattern preview stopped because its staged clip list became invalid; idle restored.");
			return;
		}

		const KOUKU_SAYDON_ANIMATION_PATTERN_CLIP& Clip =
			m_KoukuSaydonPatternPreviewClips[m_iKoukuSaydonPatternPreviewClip];
		const char_t* const pCurrentClip = PreviewModel->Get_AnimationName(
			PreviewModel->Get_CurrentAnimIndex());
		if (nullptr == pCurrentClip || Clip.strRuntimeClip != pCurrentClip)
		{
			Stop_KoukuSaydonPatternPreview(
				PreviewModel,
				"KoukuSaydon Pattern preview stopped because another control replaced its clip; idle restored.");
			return;
		}
		if (!m_bKoukuSaydonPatternPreviewPaused &&
			std::isfinite(fTimeDelta) && fTimeDelta > 0.f)
		{
			m_fKoukuSaydonPatternPreviewElapsedSeconds += fTimeDelta;
			if (m_fKoukuSaydonPatternPreviewElapsedSeconds + 0.000001f >=
				m_fKoukuSaydonPatternPreviewClipDurationSeconds)
			{
				Advance_KoukuSaydonPatternPreview(PreviewModel);
			}
		}
		return;
	}

	if (!m_bValtanPatternPreviewPlaying)
		return;

	const shared_ptr<Engine::CModel> PreviewModel =
		m_ValtanPatternPreviewModel.lock();
	if (nullptr == PreviewModel ||
		0u == m_iValtanPatternPreviewTargetGeneration ||
		m_iValtanPatternPreviewTargetGeneration !=
			CAnimationTargetService::Resolve_TargetGeneration() ||
		CAnimationTargetService::Resolve_Model() != PreviewModel)
	{
		if (nullptr != PreviewModel)
			PreviewModel->Set_AnimationSpeed(1.f);
		Reset_ValtanPatternPreviewState(
			"Pattern preview cancelled because the animation target changed.");
		return;
	}

	if (!bIsActiveTool)
	{
		Stop_ValtanPatternPreview(
			PreviewModel,
			"Pattern preview stopped because Animation Tool was deactivated; idle restored.");
		return;
	}
	if (m_bValtanPatternPreviewPaused ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		return;
	}

	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	const char_t* pExpectedClip = Item.bPatternMarker ?
		"mesh_idle_battle_1" : Item.strClipName.c_str();
	const char_t* pCurrentClip = PreviewModel->Get_AnimationName(
		PreviewModel->Get_CurrentAnimIndex());
	if (nullptr == pCurrentClip ||
		std::string_view{ pExpectedClip } != std::string_view{ pCurrentClip })
	{
		Stop_ValtanPatternPreview(
			PreviewModel,
			"Pattern preview stopped because another control replaced the current source clip; idle restored.");
		return;
	}

	if (!std::isfinite(fTimeDelta) || fTimeDelta <= 0.f)
		return;
	m_fValtanPatternPreviewElapsedSeconds +=
		fTimeDelta * m_fValtanPatternPreviewSpeed;
	if (m_fValtanPatternPreviewElapsedSeconds >=
		m_fValtanPatternPreviewItemDurationSeconds)
	{
		Advance_ValtanPatternPreview(PreviewModel);
	}
	Update_ValtanPatternHitAreaPreview();
}

shared_ptr<Engine::CModel> Client::CAnimation_Tool::Resolve_Model() const
{
	return CAnimationTargetService::Resolve_Model();
}

shared_ptr<Client::CCharacter> Client::CAnimation_Tool::Resolve_Character() const
{
	return CAnimationTargetService::Resolve_Character();
}

bool_t Client::CAnimation_Tool::Is_AnyDocumentDirty() const
{
	return m_bDirty || m_bSkillBindingDirty ||
		m_bValtanPatternSoundCuesDirty ||
		m_bValtanCombatObjectSoundCuesDirty ||
		m_bValtanPatternAnimationBindingDirty || m_bKoukuSaydonActionDirty ||
		m_bKoukuSaydonPatternDirty;
}

void Client::CAnimation_Tool::On_LevelChanged()
{
	/* LEVEL is an enum, not a level-instance generation.  MainApp calls this
	   after every committed Change_Level so re-entering VALTAN_ARENA with the
	   same enum cannot inherit an earlier failed/successful staging latch. */
	m_iValtanAutoPreviewAttemptGeneration = 0u;
	m_iValtanAutoPreviewSuccessGeneration = 0u;
	m_bValtanAutoPreviewSuppressedForServerPlayback = false;
	m_bValtanWorkspaceTabInitialized = false;
	m_ValtanPatternSoundDurationModel.reset();
	m_ValtanPatternSoundClipDurations.clear();
}

bool_t Client::CAnimation_Tool::Get_ActionCompositionSequenceCatalog(
	std::vector<COMPOSITION_SEQUENCE_VIEW>& OutSequences,
	std::string& strOutStatus)
{
	std::vector<COMPOSITION_SEQUENCE_VIEW> Staged;
	if (!Load_ValtanCompositionSequenceLibrary(Staged, strOutStatus) ||
		!Load_KoukuSaydonCompositionSequenceLibrary(Staged, strOutStatus))
	{
		OutSequences.clear();
		return false;
	}
	std::array<std::size_t, 4u> CategoryCounts{};
	for (const COMPOSITION_SEQUENCE_VIEW& Sequence : Staged)
	{
		if ("Valtan" == Sequence.strCategory)
			++CategoryCounts[0u];
		else if ("Kouku" == Sequence.strCategory)
			++CategoryCounts[1u];
		else if ("Large Kouku" == Sequence.strCategory)
			++CategoryCounts[2u];
		else if ("Saydon" == Sequence.strCategory)
			++CategoryCounts[3u];
	}
	OutSequences = std::move(Staged);
	strOutStatus = "Loaded meaningful typed Sequences once: Valtan " +
		std::to_string(CategoryCounts[0u]) + ", Kouku " +
		std::to_string(CategoryCounts[1u]) + ", Large Kouku " +
		std::to_string(CategoryCounts[2u]) + ", Saydon " +
		std::to_string(CategoryCounts[3u]) + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Preview_CompositionAnimationResource(
	const COMPOSITION_ANIMATION_RESOURCE& resource,
	std::string& strOutStatus)
{
	if (resource.strRuntimeClip.empty() || resource.iDurationMs == 0u ||
		resource.iDurationMs > 600000u)
	{
		strOutStatus = m_Status = m_strKoukuSaydonPatternStatus =
			"Resource preview requires a clip and a playback window within 600 seconds.";
		return false;
	}
	KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE occurrence;
	occurrence.strOccurrenceId = "resource.animation.1";
	occurrence.strProfileId = resource.strProfileId;
	occurrence.strSourceStageId = "RAW";
	occurrence.strSourceSlotId = resource.strRuntimeClip;
	occurrence.strRuntimeClip = resource.strRuntimeClip;
	occurrence.iPlayMs = resource.iDurationMs;
	occurrence.strEndPolicy = resource.strEndPolicy;
	KOUKU_SAYDON_COMPOSITION_STAGE stage;
	stage.strStageId = "RESOURCE_STAGE_1";
	stage.strStageKind = "ACTIVE";
	stage.iDurationMs = resource.iDurationMs;
	stage.AnimationOccurrences.push_back(std::move(occurrence));
	KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
	pattern.strPatternId = "RESOURCE_PREVIEW";
	pattern.strDisplayName = resource.strRuntimeClip;
	pattern.Stages.push_back(std::move(stage));
	return Preview_CompositionResourcePattern(pattern,
		resource.strTargetAssetName, strOutStatus);
}

bool_t Client::CAnimation_Tool::Preview_CompositionResourcePattern(
	const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
	const std::string& targetAssetName,
	std::string& strOutStatus,
	const std::uint32_t startClockMs,
	const bool_t startPaused)
{
	const auto reject = [&](const std::string& reason) {
		strOutStatus = m_Status = m_strKoukuSaydonPatternStatus = reason;
		return false;
	};
	if (!Is_CompositionAnimationTargetAsset(targetAssetName))
		return reject("Composition preview target is not registered: " + targetAssetName);
	if (std::none_of(pattern.Stages.begin(), pattern.Stages.end(), [](const auto& stage) {
		return !stage.AnimationOccurrences.empty(); }))
		return reject("Composition Pattern preview has no animation occurrence.");
	if (Is_AnyDocumentDirty() && !m_AssetName.empty() && m_AssetName != targetAssetName)
		return reject("Save or discard the current Animation document before changing its preview body.");
	if (nullptr == m_pPreviewPanel)
		return reject("Composition preview panel is unavailable.");
	if (!m_pPreviewPanel->Select_TargetAsset(targetAssetName))
		return reject("Composition preview body could not open: " + m_pPreviewPanel->Get_Status());
	if (!Sync_AssetName())
		return reject(m_Status);

	// Target identity is explicit for Resources; profile aliases are checked
	// per row against this body instead of entering a boss-specific workspace.
	m_strPendingCompositionPreviewTargetAssetName = targetAssetName;
	m_PendingKoukuSaydonCompositionPatternPreview = pattern;
	m_PendingKoukuSaydonCompositionAnimationPreview = {};
	m_bKoukuSaydonCompositionPatternPreviewPending = true;
	m_bKoukuSaydonCompositionAnimationPreviewPending = false;
	m_iPendingKoukuCompositionStartClockMs = startClockMs;
	m_bPendingKoukuCompositionStartPaused = startPaused;
	const bool_t started = Start_PendingKoukuSaydonCompositionPreview(
		CAnimationTargetService::Resolve_Model());
	strOutStatus = m_strKoukuSaydonPatternStatus;
	return started;
}

bool_t Client::CAnimation_Tool::Sync_AssetName()
{
	const string assetName =
		CAnimationTargetService::Resolve_AssetName();
	if (assetName.empty())
	{
		if (Is_AnyDocumentDirty())
		{
			m_PendingAssetName.clear();
			m_Status =
				"Animation target disappeared while an authored document is dirty. "
				"The unsaved document is preserved.";
			return false;
		}
		return true;
	}
	if (m_AssetName == assetName)
	{
		m_PendingAssetName.clear();
		return true;
	}
	if (Is_AnyDocumentDirty() && !m_AssetName.empty())
	{
		m_PendingAssetName = assetName;
		m_Status =
			"Target changed while an Animation document is dirty. "
			"Return to the original target and Save, or discard explicitly.";
		return false;
	}

	Adopt_AssetName(assetName);
	return true;
}

void Client::CAnimation_Tool::Adopt_AssetName(
	const std::string& assetName)
{
	/* A different class means every loaded file belongs to the wrong asset. Drop
		them rather than mixing two classes' clips in one list. */
	if (m_bKoukuSaydonPatternPreviewPlaying)
	{
		const shared_ptr<Engine::CModel> PreviewModel =
			m_KoukuSaydonPatternPreviewModel.lock();
		if (nullptr != PreviewModel)
		{
			Stop_KoukuSaydonPatternPreview(
				PreviewModel,
				"KoukuSaydon Pattern preview stopped before the animation target changed; idle restored.");
		}
		else
		{
			Reset_KoukuSaydonPatternPreviewState(
				"KoukuSaydon Pattern preview stopped before the animation target changed.");
		}
	}
	m_AssetName = assetName;
	m_bValtanDataWorkspaceRequested = "Valtan" == assetName;
	std::string strNextKoukuSaydonProfile;
	if (const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* pCurrent =
			Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId);
		nullptr != pCurrent && assetName == pCurrent->pPreviewAssetName)
	{
		strNextKoukuSaydonProfile = pCurrent->pProfileId;
	}
	else if (const KOUKU_SAYDON_ACTION_PROFILE_CONTRACT* pDefault =
			Default_KoukuSaydonActionProfileForAsset(assetName))
	{
		strNextKoukuSaydonProfile = pDefault->pProfileId;
	}
	else if ("MN_RPCT_00" == assetName || "MN_RPCT_03" == assetName || "MN_RPCZ_00-1" == assetName)
	{
		strNextKoukuSaydonProfile = assetName;
	}
	const std::uint32_t iRequestedKoukuSaydonSourceActionId =
		m_iRequestedKoukuSaydonSourceActionId;
	Reset_KoukuSaydonActionDocumentState(true);
	m_strKoukuSaydonProfileId = std::move(strNextKoukuSaydonProfile);
	if (!m_strKoukuSaydonProfileId.empty())
		m_iRequestedKoukuSaydonSourceActionId = iRequestedKoukuSaydonSourceActionId;
	m_PendingAssetName.clear();
	m_Events.clear();
	m_SkillRef.clear();
	m_ClipMap.clear();
	m_ClipNotify.clear();
	m_ClipLength.clear();
	m_ClipSeqs.clear();
	m_ClipChainCounts.clear();
	m_DuplicateBodyClips.clear();
	m_bDuplicateScanDone = false;
	m_SkillBindingDocument = {};
	m_iSelectedEvent = -1;
	m_iRefWireSkillId = 0;
	m_iRefWireHitIndex = -1;
	m_bDirty = false;
	m_bSkillBindingDirty = false;
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_SkillBindingStatus.clear();
	m_bLoadAttempted = false;
	m_bRefLoadAttempted = false;
	m_bClipMapLoadAttempted = false;
	m_bClipNotifyLoadAttempted = false;
	m_bClipSeqLoadAttempted = false;
	m_bSkillBindingLoadAttempted = false;
	m_ValtanPatternMasterView = {};
	m_ValtanPatternMasterPlaylist.clear();
	m_bValtanPatternMasterLoadAttempted = false;
	m_eValtanPatternMasterAdmission =
		VALTAN_VIEW_ADMISSION::UNLOADED;
	m_bValtanPatternMasterPlaying = false;
	m_bValtanPatternMasterPaused = false;
	m_bShowValtanSourceReferenceWindow = false;
	m_eValtanWorkbenchSelection =
		VALTAN_WORKBENCH_SELECTION_KIND::STAGE;
	m_eValtanWorkbenchDetailOwner =
		VALTAN_WORKBENCH_DETAIL_OWNER::GAMEPLAY;
	m_bValtanWorkbenchFocusDetailRequested = false;
	m_bValtanWorkspaceTabInitialized = false;
	m_strValtanWorkbenchPatternId.clear();
	m_strValtanWorkbenchStageId.clear();
	m_ValtanPatternAnimationBindingDraft = {};
	m_bValtanPatternAnimationBindingLoadAttempted = false;
	m_bValtanPatternAnimationBindingReady = false;
	m_bValtanPatternAnimationBindingDirty = false;
	m_strValtanPatternAnimationBindingBaselineSourceBytes.clear();
	m_strValtanPatternAnimationBindingStatus.clear();
	m_strValtanAnimationBindingDetailActionId.clear();
	m_strValtanAnimationBindingNewClip.clear();
	m_ValtanPatternSoundCues = {};
	m_bValtanPatternSoundCuesReady = false;
	m_bValtanPatternSoundCuesDirty = false;
	m_iValtanPatternSoundDraftGeneration = 0u;
	m_bValtanPatternSoundRuntimeApplyReady = false;
	m_ValtanPatternSoundRuntimeAppliedRevision = {};
	m_strValtanPatternSoundCueBaselineSourceBytes.clear();
	m_strValtanPatternSoundCueStatus.clear();
	m_strValtanPatternSoundAddClipOccurrenceId.clear();
	m_strValtanPatternSoundAddEvent.clear();
	m_iValtanPatternSoundAddStartMs = 0u;
	m_eValtanPatternSoundAddRepeatPolicy =
		VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	m_iValtanPatternMasterSelected = 0;
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = 0u;
	m_eValtanPatternMasterPath = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	m_strValtanPatternMasterStatus.clear();
	m_hasCameraToolOpenRequest = false;
	m_strCameraToolOpenCueId.clear();
	m_ValtanPatternMasterModel.reset();
	m_ValtanPatternMasterBoss.reset();
	m_iValtanPatternMasterTargetGeneration = 0u;
	m_ValtanPatternPreviewDocument = {};
	m_ValtanPatternPreviewPlaylist.clear();
	m_bValtanPatternPreviewLoadAttempted = false;
	m_bValtanPatternPreviewPlaying = false;
	m_bValtanPatternPreviewPaused = false;
	m_iValtanPatternPreviewSelected = 0;
	m_iValtanPatternPreviewItem = 0u;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	m_fValtanPatternPreviewSpeed = 1.f;
	m_strValtanPatternPreviewStatus.clear();
	m_ValtanPatternPreviewModel.reset();
	m_iValtanPatternPreviewTargetGeneration = 0u;
	m_ValtanEncounterReference.Clear();
	m_bValtanEncounterReferenceLoadAttempted = false;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	/* Chain steps name clips of the body they were built from and the library
	is one authoring body's document, so both are dropped with the target. Kept
	across a switch they would be saved into the next body's file. */
	m_CustomChainSteps.clear();
	m_CustomChainLibrary.clear();
	Invalidate_ValtanPatternCreateExactSourceSelection();
	m_bCustomChainLibraryLoadAttempted = false;
	m_bShowValtanCustomChainWindow = false;
	m_CustomChainFilter[0] = '\0';
	m_CustomChainId[0] = '\0';
	m_CustomChainTargetPatternId[0] = '\0';
	m_CustomChainTargetStageId[0] = '\0';
	m_strCustomChainStatus.clear();
}

void Client::CAnimation_Tool::Render_TargetConflict()
{
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Text("Unsaved document: %s", m_AssetName.c_str());
	if (!m_PendingAssetName.empty())
	{
		ImGui::Text("Pending target: %s", m_PendingAssetName.c_str());
		if (ImGui::Button("Discard Unsaved Animation Documents and Switch"))
			Adopt_AssetName(m_PendingAssetName);
		return;
	}

	ImGui::TextUnformatted("The original target is no longer available.");
	if (ImGui::Button("Discard Orphaned Animation Documents"))
		Adopt_AssetName(std::string{});
}

/* Preview lifecycle now belongs to CCharacterPreviewPanel so Effect authoring
   can read the same target. The tool keeps only the dirty-document policy that
   decides when the target may change. */

bool_t Client::CAnimation_Tool::Is_Window(EVENT_KIND eKind)
{
	return ETOI(eKind) < ETOI(EVENT_KIND::SOUND);
}

const char_t* Client::CAnimation_Tool::Kind_Name(EVENT_KIND eKind)
{
	switch (eKind)
	{
	case EVENT_KIND::CANCEL:     return "CANCEL";
	case EVENT_KIND::SUPERARMOR: return "SUPERARMOR";
	case EVENT_KIND::INVULN:     return "INVULN";
	case EVENT_KIND::MOVE:       return "MOVE";
	case EVENT_KIND::COUNTER:    return "COUNTER";
	case EVENT_KIND::SOUND:      return "SOUND";
	case EVENT_KIND::EFFECT:     return "EFFECT";
	case EVENT_KIND::SHAKE:      return "SHAKE";
	case EVENT_KIND::STAGE:      return "STAGE";
	case EVENT_KIND::HIT:
	default:                     return "HIT";
	}
}

const char_t* Client::CAnimation_Tool::Area_Name(int32_t iAreaType)
{
	switch (iAreaType)
	{
	case 1:  return "circle";
	case 2:  return "box";
	case 3:  return "fan";
	default: return "none";
	}
}

f32_t Client::CAnimation_Tool::Get_ClipTickRate(const shared_ptr<Engine::CModel>& pModel,
	const std::string& clipName)
{
	if (nullptr != pModel)
	{
		const uint32_t iCount = pModel->Get_NumAnimations();
		for (uint32_t i = 0; i < iCount; ++i)
		{
			const char_t* pName = pModel->Get_AnimationName(i);
			if (nullptr != pName && clipName == pName)
			{
				const f32_t fRate = pModel->Get_AnimationTickPerSecond(i);
				if (fRate > 0.f)
					return fRate;
				break;
			}
		}
	}
	return DEFAULT_TICK_RATE;
}

int32_t Client::CAnimation_Tool::Ms_To_Frame(int32_t iMs, f32_t fTickRate)
{
	if (fTickRate <= 0.f)
		fTickRate = DEFAULT_TICK_RATE;

	const f32_t fFrame = static_cast<f32_t>(iMs) * fTickRate / 1000.f;
	return static_cast<int32_t>(fFrame >= 0.f ? fFrame + 0.5f : fFrame - 0.5f);
}

int32_t Client::CAnimation_Tool::Frame_To_Ms(int32_t iFrame, f32_t fTickRate)
{
	if (fTickRate <= 0.f)
		fTickRate = DEFAULT_TICK_RATE;

	const f32_t fMs = static_cast<f32_t>(iFrame) * 1000.f / fTickRate;
	return static_cast<int32_t>(fMs >= 0.f ? fMs + 0.5f : fMs - 0.5f);
}

/* Hit 0 lands at the window start and each repeat iRepeatMs later. The window
width is the tolerance of every hit, not a span the hits are spread across --
that is how the extracted game data is shaped. */
int32_t Client::CAnimation_Tool::Get_TickMs(const ANIM_EVENT& evt, int32_t iTickIndex)
{
	if (iTickIndex <= 0)
		return evt.iStartMs;

	return evt.iStartMs + evt.hit.iRepeatMs * iTickIndex;
}

int32_t Client::CAnimation_Tool::Get_ActiveTick(const ANIM_EVENT& evt, int32_t iMs)
{
	const int32_t iWidth = evt.iEndMs - evt.iStartMs;
	const int32_t iCount = evt.hit.iRepeatCount > 1 ? evt.hit.iRepeatCount : 1;

	for (int32_t i = 0; i < iCount; ++i)
	{
		const int32_t iTick = Get_TickMs(evt, i);
		if (iMs >= iTick && iMs <= iTick + iWidth)
			return i;
	}
	return -1;
}

int32_t Client::CAnimation_Tool::Get_ChainOffsetMs(const CLIP_SEQ& seq, int32_t iIndex) const
{
	f32_t fSeconds = 0.f;
	for (int32_t i = 0; i < iIndex && i < static_cast<int32_t>(seq.clips.size()); ++i)
	{
		const auto it = m_ClipLength.find(seq.clips[i]);
		if (m_ClipLength.end() != it)
			fSeconds += it->second;
	}
	return static_cast<int32_t>(fSeconds * 1000.f + 0.5f);
}

const Client::CAnimation_Tool::CLIP_INFO*
Client::CAnimation_Tool::Find_ClipInfo(const char_t* pClipName) const
{
	if (nullptr == pClipName)
		return nullptr;

	const auto it = m_ClipMap.find(pClipName);
	return m_ClipMap.end() == it ? nullptr : &it->second;
}

void Client::CAnimation_Tool::Select_Clip(const shared_ptr<Engine::CModel>& pModel,
	const std::string& clipName)
{
	const uint32_t iCount = pModel->Get_NumAnimations();
	for (uint32_t i = 0; i < iCount; ++i)
	{
		const char_t* pName = pModel->Get_AnimationName(i);
		if (nullptr != pName && clipName == pName)
		{
			pModel->Set_Animation(i, m_bLoop);
			pModel->Set_AnimTrackPosition(i, 0.f);
			m_iSelectedEvent = -1;
			Apply_KoukuSaydonPreviewScale(pModel, Resolve_LargeNamePreviewScale(clipName));
			return;
		}
	}
}

void Client::CAnimation_Tool::Render()
{
	ImVec2 workbenchDefaultSize{
		WORKBENCH_DEFAULT_WIDTH, WORKBENCH_DEFAULT_HEIGHT };
	ImVec2 workbenchMinimumSize{
		WORKBENCH_FALLBACK_MIN_WIDTH, WORKBENCH_FALLBACK_MIN_HEIGHT };
	ImVec2 workbenchMaximumSize{ FLT_MAX, FLT_MAX };
	const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	if (nullptr != pViewport)
	{
		/* The saved imgui.ini size is still respected above this floor. On a
		   smaller monitor the floor contracts to the usable work area instead
		   of forcing the Workbench outside the viewport. */
		workbenchMaximumSize = ImVec2(
			(std::max)(WORKBENCH_FALLBACK_MIN_WIDTH, pViewport->WorkSize.x),
			(std::max)(WORKBENCH_FALLBACK_MIN_HEIGHT, pViewport->WorkSize.y));
		workbenchDefaultSize = ImVec2(
			(std::min)(WORKBENCH_DEFAULT_WIDTH, workbenchMaximumSize.x),
			(std::min)(WORKBENCH_DEFAULT_HEIGHT, workbenchMaximumSize.y));
		workbenchMinimumSize = ImVec2(
			(std::min)(WORKBENCH_FALLBACK_MIN_WIDTH, workbenchMaximumSize.x),
			(std::min)(WORKBENCH_FALLBACK_MIN_HEIGHT, workbenchMaximumSize.y));
		if (m_bResetWorkbenchLayoutRequested)
		{
			ImGui::SetNextWindowPos(
				pViewport->WorkPos, ImGuiCond_Always);
		}
	}
	ImGui::SetNextWindowSize(
		workbenchDefaultSize,
		m_bResetWorkbenchLayoutRequested ?
			ImGuiCond_Always : ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(
		workbenchMinimumSize, workbenchMaximumSize);
	m_bResetWorkbenchLayoutRequested = false;
	const char_t* const pTargetLockReason =
		"Save or discard Animation Events, Skill Bindings, Valtan Pattern Animation Bindings, Valtan Pattern Sound, KoukuSaydon Action Bindings/Patterns, and Workbench Sound bindings before changing target.";
	const bool_t isTargetLocked = Is_AnyDocumentDirty();
	m_pPreviewPanel->Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL,
		isTargetLocked,
		pTargetLockReason);
	m_pPreviewPanel->Refresh_Level();
	const uint32_t iCurrentLevel =
		CGameInstance::Get().Get_CurrentLevelID();
	const bool_t bValtanArena =
		ETOUI(LEVEL::VALTAN_ARENA) == iCurrentLevel;
	std::string strResolvedAssetName;
	shared_ptr<CValtan> pResolvedValtanBoss;
	shared_ptr<Engine::CModel> pResolvedModel;
	bool_t bExactValtanPreview = false;
	uint64_t iResolvedTargetGeneration = 0u;
	const auto RefreshResolvedTarget = [&]()
	{
		strResolvedAssetName = CAnimationTargetService::Resolve_AssetName();
		pResolvedValtanBoss = CAnimationTargetService::Resolve_Boss();
		pResolvedModel = Resolve_Model();
		iResolvedTargetGeneration =
			CAnimationTargetService::Resolve_TargetGeneration();
		bExactValtanPreview = "Valtan" == strResolvedAssetName &&
			nullptr != pResolvedValtanBoss && nullptr != pResolvedModel &&
			pResolvedValtanBoss->Get_BodyModel() == pResolvedModel;
	};
	RefreshResolvedTarget();
	const auto TryStageValtanPreview = [&]()
	{
		/* A dirty Valtan owner may reconstitute its same typed preview body.  The
		   selector remains locked for every user-driven target change, and locks
		   owned by other tools still reject this programmatic same-owner retry. */
		const bool_t bTemporarilyReleaseOwnLock =
			isTargetLocked && Is_ValtanDocumentDirty();
		if (bTemporarilyReleaseOwnLock)
		{
			m_pPreviewPanel->Set_SessionLock(
				CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL, false, {});
		}
		const bool_t bSelected =
			m_pPreviewPanel->Select_TargetAsset("Valtan");
		if (bTemporarilyReleaseOwnLock)
		{
			m_pPreviewPanel->Set_SessionLock(
				CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL,
				true, pTargetLockReason);
		}
		RefreshResolvedTarget();
		if (!bSelected || !bExactValtanPreview)
		{
			m_Status =
				"Valtan Model View could not be staged; canonical Pattern data-only mode remains available: " +
				m_pPreviewPanel->Get_Status();
			return false;
		}
		m_iValtanAutoPreviewSuccessGeneration = iResolvedTargetGeneration;
		m_iValtanAutoPreviewAttemptGeneration = iResolvedTargetGeneration;
		m_bValtanAutoPreviewSuppressedForServerPlayback = false;
		if (m_AssetName.empty() && Is_ValtanDocumentDirty())
			m_AssetName = "Valtan";
		m_Status =
			"Valtan Model View staged from the exact dedicated boss preview; canonical Product workspace preserved.";
		return true;
	};
	if (!bValtanArena)
	{
		m_iValtanAutoPreviewAttemptGeneration = 0u;
		m_iValtanAutoPreviewSuccessGeneration = 0u;
	}
	else
	{
		m_bValtanDataWorkspaceRequested = true;
		if (bExactValtanPreview)
		{
			m_iValtanAutoPreviewAttemptGeneration = iResolvedTargetGeneration;
			m_iValtanAutoPreviewSuccessGeneration = iResolvedTargetGeneration;
		}
		else if (!m_bValtanAutoPreviewSuppressedForServerPlayback &&
			m_iValtanAutoPreviewAttemptGeneration != iResolvedTargetGeneration)
		{
			m_iValtanAutoPreviewAttemptGeneration = iResolvedTargetGeneration;
			if (!isTargetLocked || Is_ValtanDocumentDirty())
				(void)TryStageValtanPreview();
		}
	}

	if (!ImGui::Begin(
		"Animation Clip Tool###AnimationClipToolResizableV1",
		nullptr,
		ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		ImGui::End();
		return;
	}

	if (ImGui::SmallButton("Reset Animation Tool Layout"))
	{
		m_bResetWorkbenchLayoutRequested = true;
		m_fKoukuSaydonActionListWidth = KOUKU_SAYDON_ACTION_LIST_DEFAULT_WIDTH;
		m_Status =
			"Animation Tool layout reset requested; the viewport-aware size applies next frame.";
	}
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Clip preview and Animation Sequence Intake only; Pattern composition lives in the independent Valtan Action Workbench.");

	const bool_t bDirtyNonValtanOwner = isTargetLocked &&
		!Is_ValtanDocumentDirty() && !m_AssetName.empty() &&
		"Valtan" != m_AssetName;
	bool_t bValtanWorkspaceMode = !bDirtyNonValtanOwner &&
		(bValtanArena || m_bValtanDataWorkspaceRequested ||
			"Valtan" == m_AssetName || bExactValtanPreview);
	if (bValtanWorkspaceMode)
	{
		/* Target picking is secondary to Product pattern authoring.  Keeping this
		   collapsed prevents the generic model inventory from pushing the joined
		   Pattern shell below the first screen. */
		if (ImGui::CollapsingHeader("Target / Model View"))
			m_pPreviewPanel->Render_Selector(false, {});
	}
	else
	{
		m_pPreviewPanel->Render_Selector(false, {});
	}
	RefreshResolvedTarget();
	bValtanWorkspaceMode = !bDirtyNonValtanOwner &&
		(bValtanArena || m_bValtanDataWorkspaceRequested ||
			"Valtan" == m_AssetName || bExactValtanPreview);
	if (bValtanWorkspaceMode)
	{
		bool_t bTargetSynchronized = true;
		if (bExactValtanPreview)
		{
			if (m_AssetName.empty() && Is_ValtanDocumentDirty())
				m_AssetName = "Valtan";
			else
				bTargetSynchronized = Sync_AssetName();
		}
		else if (Is_ValtanDocumentDirty())
		{
			m_PendingAssetName = strResolvedAssetName;
			m_Status = strResolvedAssetName.empty() ?
				"The exact Valtan preview disappeared while a Valtan typed draft is dirty. The draft and canonical workspace are preserved." :
				"A scene target replaced the exact Valtan preview while a Valtan typed draft is dirty. The scene model is ignored and the draft is preserved.";
			bTargetSynchronized = false;
		}
		if (!bTargetSynchronized)
			Render_TargetConflict();

		const shared_ptr<Engine::CModel> pValtanPreviewModel =
			bExactValtanPreview ? pResolvedModel : nullptr;
		const bool_t bValtanDataOnlyMode = nullptr == pValtanPreviewModel;
		ImGui::SeparatorText("Valtan Workspace Capability");
		ImGui::TextColored(
			bValtanDataOnlyMode ? ImVec4(1.f, 0.75f, 0.2f, 1.f) :
				ImVec4(0.35f, 0.85f, 0.45f, 1.f),
			"Preview Model: %s", bValtanDataOnlyMode ? "MISSING" : "READY");
		if (bValtanDataOnlyMode)
		{
			ImGui::TextWrapped(
				"The dedicated Valtan Model View is required for clip playback and Sequence Intake. Pattern/Stage data remains available in Valtan Action Workbench; no scene player model is substituted here.");
			if (ImGui::SmallButton("Retry Valtan Model View"))
				(void)TryStageValtanPreview();
		}
		if (!m_Status.empty())
			ImGui::TextWrapped("%s", m_Status.c_str());
		Render_ValtanAnimationSourceWorkspace(pValtanPreviewModel);
		ImGui::End();
		return;
	}

	const bool_t bTargetSynchronized = Sync_AssetName();
	if (!bTargetSynchronized)
	{
		Render_TargetConflict();
		ImGui::End();
		return;
	}
	const shared_ptr<Engine::CModel> pModel = Resolve_Model();
	if (nullptr == pModel)
	{
		ImGui::TextUnformatted("No animated character resolved.");
		ImGui::Separator();
		ImGui::TextUnformatted(
			"Enter Character Select or Development from the Lobby.");
		ImGui::End();
		return;
	}

	if (m_AssetName.empty())
	{
		ImGui::TextUnformatted("The selected target carries no asset name.");
		ImGui::End();
		return;
	}

	const auto previewAsset = std::find_if(
		ANIMATION_PREVIEW_ASSETS.begin(),
		ANIMATION_PREVIEW_ASSETS.end(),
		[this](const ANIMATION_PREVIEW_ASSET& asset)
		{
			return nullptr != asset.pAssetName &&
				m_AssetName == asset.pAssetName;
		});
	if (previewAsset != ANIMATION_PREVIEW_ASSETS.end() &&
		previewAsset->bPlaybackOnly)
	{
		if (!m_bClipMapLoadAttempted)
		{
			m_bClipMapLoadAttempted = true;
			Load_ClipMap();
		}
		if (!m_bClipSeqLoadAttempted)
		{
			m_bClipSeqLoadAttempted = true;
			Load_ClipSeq();
		}
		if (!m_bClipNotifyLoadAttempted)
		{
			m_bClipNotifyLoadAttempted = true;
			Load_ClipNotify();
		}
		if (!m_bRefLoadAttempted)
		{
			m_bRefLoadAttempted = true;
			Load_SkillReference();
		}
		ImGui::Text(
			"Asset: %s   Animations: %u",
			m_AssetName.c_str(),
			pModel->Get_NumAnimations());
		ImGui::TextDisabled(
			"Playback-only preview: Product events and gameplay skill bindings are disabled.");
		ImGui::BeginDisabled(
			m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying ||
			m_bKoukuSaydonPatternPreviewPlaying);
		Render_Playback(pModel);
		ImGui::EndDisabled();
		if ("Valtan" == m_AssetName)
		{
			Render_ValtanPatternPreview(pModel);
		}
		else if (nullptr != Find_KoukuSaydonActionProfile(m_strKoukuSaydonProfileId))
		{
			Render_KoukuSaydonActionBindings(pModel);
		}
		else if ("MN_RPCT_00" == m_AssetName || "MN_RPCT_03" == m_AssetName || "MN_RPCZ_00-1" == m_AssetName)
		{
			ImGui::SeparatorText("KoukuSaydon Clip Donor");
			ImGui::TextWrapped(
				"%s exposes its physical clips for local preview. Select a clip "
				"from this body's list to inspect or play it.",
				m_AssetName.c_str());
		}
		else if (nullptr != Find_CustomChainProfile(m_AssetName))
		{
			/* This body owns no admitted pattern master, so only the
			   hand-assembled chain workflow opens for it. Playback reuses the
			   same transport window the source sequences use, which reads the
			   playlist rather than any Valtan document. */
			ImGui::SeparatorText("Animation Sequence Intake");
			ImGui::TextWrapped(
				"No pattern master is admitted for this body. Chains assembled "
				"here are saved as Animation Intake and are not Product patterns "
				"until the reviewed promotion pipeline admits them.");
			if (ImGui::SmallButton("Open Animation Sequence Intake"))
				m_bShowValtanCustomChainWindow = true;
			if (m_bShowValtanCustomChainWindow)
				Render_ValtanCustomChainWindow(pModel);
			if (m_bShowValtanSourceReferenceWindow)
				Render_ValtanPatternReferenceWindow(pModel);
		}
		ImGui::BeginDisabled(
			m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying ||
			m_bKoukuSaydonPatternPreviewPlaying);
		/* The source chain rows are not drawn for Valtan: the Custom Chain
		   window owns chain playback now, and the per-clip buttons crowded the
		   panel the animator actually works in. Other assets keep them. */
		Render_NotifyReference(pModel);
		Render_HitAreaWires(pModel);
		Render_SkillReference(pModel, true);
		if (!m_Status.empty())
			ImGui::TextWrapped("%s", m_Status.c_str());
		ImGui::SeparatorText("Clips");
		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputTextWithHint(
			"##filter",
			"filter by name",
			m_Filter,
			sizeof(m_Filter));
		Render_AnimationList(pModel);
		ImGui::EndDisabled();
		ImGui::End();
		return;
	}

	/* Load once the target actually exists, so a missing file is not reported
	before the level is even open. */
	if (!m_bLoadAttempted)
	{
		m_bLoadAttempted = true;
		Load_Events(pModel);
	}
	Consume_EffectTransfer(pModel);

	/* The game-extracted skill timing is optional; a missing file just hides the
	reference panel and never blocks event authoring. */
	if (!m_bRefLoadAttempted)
	{
		m_bRefLoadAttempted = true;
		Load_SkillReference();
	}

	/* Also optional: without it the clip list just shows the English clip names. */
	if (!m_bClipMapLoadAttempted)
	{
		m_bClipMapLoadAttempted = true;
		Load_ClipMap();
	}

	if (!m_bClipSeqLoadAttempted)
	{
		m_bClipSeqLoadAttempted = true;
		Load_ClipSeq();
	}

	if (!m_bClipNotifyLoadAttempted)
	{
		m_bClipNotifyLoadAttempted = true;
		Load_ClipNotify();

		/* Timing kinds default on; effects and sounds are the noisy ones and are
		better pulled in deliberately. */
		m_bImportKind[ETOI(EVENT_KIND::HIT)] = true;
		m_bImportKind[ETOI(EVENT_KIND::CANCEL)] = true;
		m_bImportKind[ETOI(EVENT_KIND::SUPERARMOR)] = true;
		m_bImportKind[ETOI(EVENT_KIND::SHAKE)] = true;
	}

	ImGui::Text("Asset: %s   Animations: %u", m_AssetName.c_str(), pModel->Get_NumAnimations());

	Render_Playback(pModel);
	Render_ClipChain(pModel);
	Render_SkillBindings(pModel, Resolve_Character());
	Render_HitEvents(pModel);
	Render_HitAreaWires(pModel);
	Render_SkillReference(pModel, false);

	ImGui::SeparatorText("Clips");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##filter", "filter by name", m_Filter, sizeof(m_Filter));

	Render_AnimationList(pModel);

	ImGui::End();
}

void Client::CAnimation_Tool::Consume_EffectTransfer(
	const shared_ptr<Engine::CModel>& pModel)
{
	EFFECT_AUTHORING_CUE_TRANSFER Transfer;
	if (!CEffectAuthoringTransfer::Consume(Transfer))
		return;
	if (Transfer.iTargetGeneration !=
		CAnimationTargetService::Resolve_TargetGeneration() ||
		Transfer.strAnimationAssetId != m_AssetName ||
		Transfer.strClipName.empty() ||
		!CEffectCatalog::Contains(Transfer.strEffectAssetId) ||
		Transfer.ePivotKind >= EFFECT_CUE_PIVOT_KIND::END ||
		Transfer.eOrientationPolicy >= EFFECT_ORIENTATION_POLICY::END ||
		(EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
			Transfer.eOrientationPolicy &&
			(Transfer.ePivotKind != EFFECT_CUE_PIVOT_KIND::PLAYER_ROOT ||
			 "root" != Transfer.strAnchorSlotId)))
	{
		m_Status = "Effect cue transfer rejected because the animation target or admitted Effect changed.";
		return;
	}

	ANIM_EVENT Event;
	Event.clipName = Transfer.strClipName;
	Event.eKind = EVENT_KIND::EFFECT;
	Event.iStartMs = static_cast<int32_t>(Transfer.iTimeMs);
	Event.iEndMs = EFFECT_STOP_POLICY::CUE_END == Transfer.eStopPolicy ?
		Event.iStartMs + static_cast<int32_t>(Transfer.iDurationMs) :
		Event.iStartMs;
	Event.sPayload = Transfer.strEffectAssetId;
	Event.eEffectReferenceKind = EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
	Event.sAnchorSlotId = Transfer.strAnchorSlotId;
	Event.eFollowPolicy = Transfer.eFollowPolicy;
	Event.eOrientationPolicy = Transfer.eOrientationPolicy;
	Event.eStopPolicy = Transfer.eStopPolicy;
	Event.EffectLocalTransform = Transfer.LocalTransform;

	const auto Duplicate = std::find_if(
		m_Events.begin(), m_Events.end(),
		[&Event](const ANIM_EVENT& Existing)
		{
			return Existing.eKind == EVENT_KIND::EFFECT &&
				Existing.eEffectReferenceKind ==
					EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID &&
				Existing.clipName == Event.clipName &&
				Existing.iStartMs == Event.iStartMs &&
				Existing.sPayload == Event.sPayload &&
				Existing.sAnchorSlotId == Event.sAnchorSlotId;
		});
	if (Duplicate != m_Events.end())
	{
		m_Status = "Effect cue transfer rejected because the same cue already exists.";
		return;
	}

	std::vector<ANIM_EVENT> Staged = m_Events;
	Staged.push_back(Event);
	std::string ValidationStatus;
	if (!Validate_Events(pModel, Staged, ValidationStatus))
	{
		m_Status = "Effect cue transfer rejected: " + ValidationStatus;
		return;
	}
	m_Events = std::move(Staged);
	m_iSelectedEvent = static_cast<int32_t>(m_Events.size() - 1u);
	strncpy_s(m_PayloadEdit, Event.sPayload.c_str(), _TRUNCATE);
	m_bDirty = true;
	Select_Clip(pModel, Event.clipName);
	const f32_t fTickRate = Get_ClipTickRate(pModel, Event.clipName);
	pModel->Set_AnimPaused(true);
	pModel->Set_AnimTrackPosition(
		pModel->Get_CurrentAnimIndex(),
		static_cast<f32_t>(Event.iStartMs) * fTickRate / 1000.f);
	m_iSelectedEvent = static_cast<int32_t>(m_Events.size() - 1u);
	m_Status = "Added admitted Effect cue from Effect Tool; review and Save explicitly.";
}

void Client::CAnimation_Tool::Render_Playback(const shared_ptr<Engine::CModel>& pModel)
{
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const bool_t bPaused = pModel->Is_AnimPaused();

	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	ImGui::Text("%s [%u] %s", bPaused ? "Paused:" : "Playing:",
		iCurrentIndex, nullptr != pCurrentName ? pCurrentName : "(none)");

	if (const CLIP_INFO* pInfo = Find_ClipInfo(pCurrentName))
	{
		ImGui::SameLine();
		ImGui::TextDisabled("= %s (%d)", pInfo->name.c_str(), pInfo->iSkillId);
	}

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack =
		pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;

	/* Track positions are in ticks. Blender exports one key per frame, so 1 tick == 1 frame. */
	if (bHasTrack)
	{
		f32_t fScrub = fPosition;

		/* SliderFloat feeds only the slider value to its format string, so the
		duration has to be baked in and the value slot escaped as %%. */
		char_t szScrubFormat[64]{};
		snprintf(szScrubFormat, sizeof(szScrubFormat), "frame %%.1f / %.0f", fDuration);

		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::SliderFloat("##scrub", &fScrub, 0.f, fDuration, szScrubFormat))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, fScrub);
		}
	}
	else
	{
		ImGui::TextUnformatted("Track: n/a");
	}

	if (ImGui::Button(bPaused ? "Play" : "Pause"))
		pModel->Set_AnimPaused(!bPaused);

	ImGui::SameLine();
	ImGui::BeginDisabled(!bHasTrack);
	if (ImGui::Button("< Frame"))
	{
		pModel->Set_AnimPaused(true);
		pModel->Set_AnimTrackPosition(iCurrentIndex, fPosition - 1.f);
	}
	ImGui::SameLine();
	if (ImGui::Button("Frame >"))
	{
		pModel->Set_AnimPaused(true);
		pModel->Set_AnimTrackPosition(iCurrentIndex, fPosition + 1.f);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Restart"))
		pModel->Set_AnimTrackPosition(iCurrentIndex, 0.f);

	ImGui::SameLine();
	if (ImGui::Checkbox("Loop", &m_bLoop))
		pModel->Set_Animation(iCurrentIndex, m_bLoop);
}

/* The chains this clip belongs to. A skill plays its clips in a fixed order and
each tripod build takes a different route, so the same clip shows up in several.
Clicking a step both navigates and tells the reference panel where that step
starts on the whole-cast clock. */
void Client::CAnimation_Tool::Render_ClipChain(const shared_ptr<Engine::CModel>& pModel)
{
	if (m_ClipSeqs.empty())
		return;

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;

	int32_t iMatches = 0;
	for (size_t iSeq = 0u; iSeq < m_ClipSeqs.size(); ++iSeq)
	{
		const CLIP_SEQ& seq = m_ClipSeqs[iSeq];
		int32_t iPos = -1;
		for (int32_t i = 0; i < static_cast<int32_t>(seq.clips.size()); ++i)
		{
			if (seq.clips[i] == pCurrentName)
			{
				iPos = i;
				break;
			}
		}
		if (iPos < 0)
			continue;

		if (0 == iMatches)
			ImGui::SeparatorText("Chain");
		++iMatches;

		ImGui::PushID(seq.iSkillId * 100 + seq.iSeqIndex);

		const int32_t iOffsetMs = Get_ChainOffsetMs(seq, iPos);
		/* The numbered buttons below select one clip. Pressing the chain itself
		   runs every step in authored order, which is the only way to judge how
		   the sequence reads as one motion. The transport lives in the source
		   reference window, so opening it is part of starting playback. */
		char_t szChain[192]{};
		snprintf(szChain, sizeof(szChain), "%s  seq%d  [%s]  %d/%d",
			seq.name.c_str(), seq.iSeqIndex,
			seq.sMode.empty() ? "?" : seq.sMode.c_str(),
			iPos + 1, static_cast<int32_t>(seq.clips.size()));
		if (ImGui::Button(szChain))
		{
			if (Start_ValtanSequencePreview(pModel, iSeq))
				m_bShowValtanSourceReferenceWindow = true;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Play all %d clips in order.",
				static_cast<int32_t>(seq.clips.size()));

		ImGui::SameLine();
		char_t szUse[64]{};
		snprintf(szUse, sizeof(szUse), "use offset %d ms", iOffsetMs);
		if (ImGui::SmallButton(szUse))
		{
			m_iCastOffsetMs = iOffsetMs;
			m_Status = "Cast offset set to " + std::to_string(iOffsetMs) +
				" ms from " + seq.name + " seq" + std::to_string(seq.iSeqIndex);
		}

		for (int32_t i = 0; i < static_cast<int32_t>(seq.clips.size()); ++i)
		{
			if (i > 0)
				ImGui::SameLine(0.f, 2.f);

			ImGui::PushID(i);
			const bool_t bHere = (i == iPos);
			if (bHere)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

			char_t szStep[16]{};
			snprintf(szStep, sizeof(szStep), "%d", i + 1);
			if (ImGui::SmallButton(szStep))
				Select_Clip(pModel, seq.clips[i]);

			if (bHere)
				ImGui::PopStyleColor();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s\nstarts %d ms into the cast",
					seq.clips[i].c_str(), Get_ChainOffsetMs(seq, i));

			ImGui::PopID();
		}

		ImGui::PopID();
	}
}

void Client::CAnimation_Tool::Render_NotifyReference(
	const shared_ptr<Engine::CModel>& pModel)
{
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;

	const auto it = m_ClipNotify.find(pCurrentName);
	if (m_ClipNotify.end() == it || it->second.empty())
		return;

	char_t szHeader[64]{};
	snprintf(szHeader, sizeof(szHeader), "Original Notifies (%d)",
		static_cast<int32_t>(it->second.size()));
	ImGui::SeparatorText(szHeader);

	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack = pModel->Get_AnimationProgress(
		iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;
	const f32_t fNowSec =
		bHasTrack && fRate > 0.f ? fPosition / fRate : -1.f;

	if (!ImGui::BeginChild("##notifyref", ImVec2(0.f, 180.f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	int32_t iRow = 0;
	for (const NOTIFY_ROW& row : it->second)
	{
		const bool_t bActive = fNowSec >= 0.f && fNowSec >= row.fTime &&
			fNowSec <= row.fTime + (std::max)(row.fDuration, 0.05f);
		const char_t* pText = !row.sAsset.empty() ?
			row.sAsset.c_str() : row.sLabel.c_str();

		char_t szRow[256]{};
		const int32_t iFrame =
			static_cast<int32_t>(row.fTime * fRate + 0.5f);
		if (row.fDuration > 0.f)
			snprintf(szRow, sizeof(szRow), "f%-4d %-6s %.2f-%.2fs  %s",
				iFrame, Kind_Name(row.eKind), row.fTime,
				row.fTime + row.fDuration, pText);
		else
			snprintf(szRow, sizeof(szRow), "f%-4d %-6s %.2fs  %s",
				iFrame, Kind_Name(row.eKind), row.fTime, pText);

		ImGui::PushID(iRow++);
		if (ImGui::Selectable(szRow, bActive))
		{
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, row.fTime * fRate);
			if (EVENT_KIND::HIT == row.eKind && !row.sAsset.empty())
				Bind_ReferenceWire(row.sAsset);
		}
		ImGui::PopID();
	}

	ImGui::EndChild();
}

void Client::CAnimation_Tool::Bind_ReferenceWire(const std::string& sourceKey)
{
	for (const SKILL_TIMING& ref : m_SkillRef)
	{
		for (size_t h = 0; h < ref.hits.size(); ++h)
		{
			const std::string& keys = ref.hits[h].sSourceKeys;
			size_t pos = 0;
			bool_t bMatch = false;
			while (std::string::npos != (pos = keys.find(sourceKey, pos)))
			{
				const size_t end = pos + sourceKey.size();
				if ((0 == pos || ',' == keys[pos - 1]) &&
					(keys.size() == end || ',' == keys[end]))
				{
					bMatch = true;
					break;
				}
				pos = end;
			}
			if (!bMatch)
				continue;

			m_iRefWireSkillId = ref.iSkillId;
			m_iRefWireHitIndex = static_cast<int32_t>(h);
			m_Status = "Reference wire bound to SkillEffect " + sourceKey +
				" of " + ref.name;
			return;
		}
	}
	m_Status = "SkillEffect " + sourceKey +
		" has no judgement shape in the skill timing reference.";
}

std::vector<std::string> Client::CAnimation_Tool::Collect_ClipNames(
	const shared_ptr<Engine::CModel>& pModel) const
{
	std::vector<std::string> clips;
	if (nullptr == pModel)
		return clips;
	clips.reserve(pModel->Get_NumAnimations());
	for (uint32_t index = 0; index < pModel->Get_NumAnimations(); ++index)
	{
		const char_t* clipName = pModel->Get_AnimationName(index);
		if (nullptr != clipName)
			clips.emplace_back(clipName);
	}
	return clips;
}

void Client::CAnimation_Tool::Render_HitEvents(const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Events");

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
	{
		ImGui::TextUnformatted("No clip selected.");
		return;
	}

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack =
		pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;

	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	const int32_t iFrame = static_cast<int32_t>(fPosition);
	const int32_t iNowMs = Frame_To_Ms(iFrame, fRate);

	auto Add_Event = [&](EVENT_KIND eKind)
	{
		ANIM_EVENT evt{};
		evt.clipName = pCurrentName;
		evt.eKind = eKind;
		if (EVENT_KIND::EFFECT == eKind)
		{
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
		evt.iStartMs = iNowMs;
		evt.iEndMs = iNowMs;
		m_Events.push_back(evt);
		m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
		m_PayloadEdit[0] = '\0';
		m_bDirty = true;
	};

	/* Everything is added at the playhead; window kinds then grow an end. */
	if (ImGui::Button("Hit"))
		Add_Event(EVENT_KIND::HIT);
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		Add_Event(EVENT_KIND::CANCEL);
	ImGui::SameLine();
	if (ImGui::Button("SuperArmor"))
		Add_Event(EVENT_KIND::SUPERARMOR);
	ImGui::SameLine();
	if (ImGui::Button("Invuln"))
		Add_Event(EVENT_KIND::INVULN);
	ImGui::SameLine();
	if (ImGui::Button("Move"))
		Add_Event(EVENT_KIND::MOVE);

	if (ImGui::Button("Sound"))
		Add_Event(EVENT_KIND::SOUND);
	ImGui::SameLine();
	if (ImGui::Button("Effect"))
		Add_Event(EVENT_KIND::EFFECT);

	ImGui::SameLine();
	if (ImGui::Button("Save"))
		Save_Events(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		if (m_bDirty)
			m_bReloadConfirmationRequested = true;
		else
			Load_Events(pModel);
	}
	Render_ReloadConfirmation(pModel);

	ImGui::SameLine();
	ImGui::TextDisabled("%.0f fps", fRate);
	ImGui::SameLine();
	ImGui::Checkbox("show areas", &m_bShowHitAreas);

	/* Original notifies for this clip, lifted from the game's Action table. */
	const auto itNotify = m_ClipNotify.find(pCurrentName);
	if (m_ClipNotify.end() != itNotify)
	{
		int32_t iAvailable = 0;
		for (const NOTIFY_ROW& row : itNotify->second)
		{
			if (m_bImportKind[ETOI(row.eKind)])
				++iAvailable;
		}

		char_t szImport[96]{};
		snprintf(szImport, sizeof(szImport), "Import original (%d of %d)",
			iAvailable, static_cast<int32_t>(itNotify->second.size()));

		if (ImGui::Button(szImport))
		{
			int32_t iShapedHits = 0;
			const int32_t iAdded = Import_Notifies(pCurrentName, fRate, iShapedHits);
			m_Status = "Imported " + std::to_string(iAdded) +
				" original notify event(s) onto " + pCurrentName;
			if (iShapedHits > 0)
				m_Status += ", " + std::to_string(iShapedHits) +
					" HIT shape(s) from skill timing reference";
		}

		ImGui::SameLine();
		ImGui::TextDisabled("|");

		for (int32_t k = 0; k < ETOI(EVENT_KIND::END); ++k)
		{
			const EVENT_KIND eKind = static_cast<EVENT_KIND>(k);
			if (EVENT_KIND::INVULN == eKind || EVENT_KIND::MOVE == eKind)
				continue;

			ImGui::SameLine();
			ImGui::Checkbox(Kind_Name(eKind), &m_bImportKind[k]);
		}
	}

	if (m_bDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}

	if (!m_Status.empty())
		ImGui::TextWrapped("%s", m_Status.c_str());

	if (!ImGui::BeginChild("##eventlist", ImVec2(0.f, 170.f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	int32_t iShown = 0;
	int32_t iRemoveIndex = -1;

	for (int32_t i = 0; i < static_cast<int32_t>(m_Events.size()); ++i)
	{
		ANIM_EVENT& evt = m_Events[i];
		if (evt.clipName != pCurrentName)
			continue;

		++iShown;
		ImGui::PushID(i);

		const bool_t bWindow = Is_Window(evt.eKind);
		const bool_t bHit = EVENT_KIND::HIT == evt.eKind;
		const bool_t bEffectCueWindow =
			EVENT_KIND::EFFECT == evt.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				evt.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == evt.eStopPolicy;

		int32_t iStartFrame = Ms_To_Frame(evt.iStartMs, fRate);
		int32_t iEndFrame = Ms_To_Frame(evt.iEndMs, fRate);

		/* A window highlights over its span (and over every repeat); a point event
		only on its own frame. */
		const int32_t iActiveTick = (bWindow && bHasTrack) ? Get_ActiveTick(evt, iNowMs) : -1;
		const bool_t bActive = bWindow ? (iActiveTick >= 0) : (iStartFrame == iFrame);
		if (bActive)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.f, 0.4f, 1.f));

		char_t szLabel[192]{};
		if (bHit && evt.hit.iRepeatCount > 1)
			snprintf(szLabel, sizeof(szLabel), "HIT %d - %d  x%d every %dms",
				iStartFrame, iEndFrame, evt.hit.iRepeatCount, evt.hit.iRepeatMs);
		else if (bWindow)
			snprintf(szLabel, sizeof(szLabel), "%s %d - %d  %s",
				Kind_Name(evt.eKind), iStartFrame, iEndFrame, evt.sPayload.c_str());
		else
			snprintf(szLabel, sizeof(szLabel), "%s %d  %s",
				Kind_Name(evt.eKind), iStartFrame,
				evt.sPayload.empty() ? "(unset)" : evt.sPayload.c_str());

		if (bWindow && bActive && evt.hit.iRepeatCount > 1)
		{
			char_t szActive[32]{};
			snprintf(szActive, sizeof(szActive), "  <== tick %d/%d",
				iActiveTick + 1, evt.hit.iRepeatCount);
			strcat_s(szLabel, szActive);
		}

		/* Imported rows are replaced wholesale on the next import, so they are
		worth telling apart from hand-authored ones. */
		if (evt.bImported)
			strcat_s(szLabel, "  *orig");

		if (ImGui::Selectable(szLabel, i == m_iSelectedEvent))
		{
			m_iSelectedEvent = i;
			pModel->Set_AnimPaused(true);
			pModel->Set_AnimTrackPosition(iCurrentIndex, static_cast<f32_t>(iStartFrame));
			/* Prime the payload editor with the row just opened. */
			strncpy_s(m_PayloadEdit, evt.sPayload.c_str(), _TRUNCATE);
		}

		if (bActive)
			ImGui::PopStyleColor();

		if (i == m_iSelectedEvent)
		{
			const int32_t iMaxFrame = bHasTrack ? static_cast<int32_t>(fDuration) : 0;

			/* Frames are the editing unit but milliseconds are what is stored, so
			only a frame the user actually moved is written back. Converting both
			ways every frame would round the stored value away. */
			if (bWindow || bEffectCueWindow)
			{
				ImGui::SetNextItemWidth(200.f);
				if (ImGui::DragIntRange2("start / end", &iStartFrame, &iEndFrame,
					0.2f, 0, iMaxFrame, "%d", "%d"))
				{
					evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
					evt.iEndMs = Frame_To_Ms(iEndFrame, fRate);
					m_bDirty = true;
				}
			}
			else
			{
				ImGui::SetNextItemWidth(120.f);
				if (ImGui::DragInt("frame", &iStartFrame, 0.2f, 0, iMaxFrame))
				{
					evt.iStartMs = Frame_To_Ms(iStartFrame, fRate);
					evt.iEndMs = evt.iStartMs;
					m_bDirty = true;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Remove"))
				iRemoveIndex = i;

			ImGui::SameLine();
			ImGui::TextDisabled("%d - %d ms", evt.iStartMs, evt.iEndMs);

			if (bHit)
				Render_HitDetail(evt);
			else
			{
				const char_t* pHint = "effect / particle key";
				if (EVENT_KIND::SOUND == evt.eKind)
					pHint = "sound cue key";
				else if (bWindow)
					pHint = "what this window is for";

				ImGui::SetNextItemWidth(-1.f);
				if (ImGui::InputTextWithHint("##payload", pHint,
					m_PayloadEdit, sizeof(m_PayloadEdit)))
				{
					evt.sPayload = m_PayloadEdit;
					m_bDirty = true;
				}
				if (EVENT_KIND::EFFECT == evt.eKind)
				{
					const bool_t bAsset =
						EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
							evt.eEffectReferenceKind;
					if (ImGui::RadioButton("Source Reference", !bAsset))
					{
						evt.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
						m_bDirty = true;
					}
					ImGui::SameLine();
					if (ImGui::RadioButton("Admitted EffectAssetId", bAsset))
					{
						evt.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
						const vector<string> IDs =
							CEffectCatalog::Get_EffectAssetIds();
						if (!IDs.empty() &&
							!CEffectCatalog::Contains(evt.sPayload))
						{
							evt.sPayload = IDs.front();
							strncpy_s(m_PayloadEdit,
								evt.sPayload.c_str(), _TRUNCATE);
						}
						m_bDirty = true;
					}
					if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
						evt.eEffectReferenceKind)
					{
						const vector<string> IDs =
							CEffectCatalog::Get_EffectAssetIds();
						if (ImGui::BeginCombo("Effect Asset",
							evt.sPayload.empty() ? "Select admitted Effect" :
								evt.sPayload.c_str()))
						{
							for (const string& ID : IDs)
							{
								if (ImGui::Selectable(ID.c_str(),
									ID == evt.sPayload))
								{
									evt.sPayload = ID;
									strncpy_s(m_PayloadEdit,
										ID.c_str(), _TRUNCATE);
									m_bDirty = true;
								}
							}
							ImGui::EndCombo();
						}
						char_t anchor[129]{};
						strncpy_s(anchor, evt.sAnchorSlotId.c_str(), _TRUNCATE);
						if (ImGui::InputText("Anchor (root or bone)",
							anchor, sizeof(anchor)))
						{
							evt.sAnchorSlotId = anchor;
							m_bDirty = true;
						}
						int32_t follow = ETOI(evt.eFollowPolicy);
						if (ImGui::RadioButton("Follow", 0 == follow))
						{
							evt.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton("Snapshot", 1 == follow))
						{
							evt.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
							m_bDirty = true;
						}
						int32_t orientation = ETOI(evt.eOrientationPolicy);
						if (ImGui::RadioButton(
							"Anchor Orientation", 0 == orientation))
						{
							evt.eOrientationPolicy =
								EFFECT_ORIENTATION_POLICY::ANCHOR;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton(
							"Action Facing", 1 == orientation))
						{
							evt.eOrientationPolicy =
								EFFECT_ORIENTATION_POLICY::ACTION_FACING;
							m_bDirty = true;
						}
						int32_t stop = ETOI(evt.eStopPolicy);
						if (ImGui::RadioButton("Natural Stop", 0 == stop))
						{
							evt.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
							evt.iEndMs = evt.iStartMs;
							m_bDirty = true;
						}
						ImGui::SameLine();
						if (ImGui::RadioButton("Cue End Stop", 1 == stop))
						{
							evt.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
							if (evt.iEndMs <= evt.iStartMs)
								evt.iEndMs = evt.iStartMs + 100;
							m_bDirty = true;
						}
						m_bDirty |= ImGui::InputFloat3("Effect Local Position",
							&evt.EffectLocalTransform.vPosition.x, "%.3f");
						m_bDirty |= ImGui::InputFloat3("Effect Local Rotation",
							&evt.EffectLocalTransform.vRotationDegrees.x, "%.3f");
						m_bDirty |= ImGui::InputFloat3("Effect Local Scale",
							&evt.EffectLocalTransform.vScale.x, "%.3f");
					}
				}
			}
		}

		ImGui::PopID();
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No event on this clip.");

	ImGui::EndChild();

	if (iRemoveIndex >= 0)
	{
		m_Events.erase(m_Events.begin() + iRemoveIndex);
		m_iSelectedEvent = -1;
		m_bDirty = true;
	}
}

/* The combat half of a HIT. Values here come straight from the extracted game
tables, so they stay in the source units (milliseconds and raw game distance)
rather than being normalised into frames or metres. */
void Client::CAnimation_Tool::Render_HitDetail(ANIM_EVENT& evt)
{
	HIT_PARAMS& p = evt.hit;

	if (ImGui::TreeNodeEx("##hitdetail", ImGuiTreeNodeFlags_DefaultOpen, "hit detail"))
	{
		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("repeat", &p.iRepeatCount, 0.1f, 1, 64))
		{
			if (p.iRepeatCount < 1)
				p.iRepeatCount = 1;
			m_bDirty = true;
		}

		if (p.iRepeatCount > 1)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("every (ms)", &p.iRepeatMs, 1.f, 0, 5000))
			{
				if (p.iRepeatMs < 0)
					p.iRepeatMs = 0;
				m_bDirty = true;
			}

			/* Spell the hit times out so they can be checked against the pose. */
			std::string ticks;
			for (int32_t t = 0; t < p.iRepeatCount && t < 16; ++t)
			{
				if (!ticks.empty())
					ticks += ", ";
				ticks += std::to_string(Get_TickMs(evt, t));
			}
			if (p.iRepeatCount > 16)
				ticks += ", ...";
			ImGui::TextWrapped("hits at: %s ms", ticks.c_str());
		}

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("freeze (ms)", &p.iFreezeMs, 1.f, 0, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragInt("in", &p.iFreezeInMs, 1.f, 0, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.f);
		if (ImGui::DragInt("out", &p.iFreezeOutMs, 1.f, 0, 5000))
			m_bDirty = true;

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("push (ms)", &p.iPushMs, 1.f, -5000, 5000))
			m_bDirty = true;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(110.f);
		if (ImGui::DragInt("distance", &p.iPushRange, 1.f, -5000, 5000))
			m_bDirty = true;
		if (p.iPushMs < 0)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(pull)");
		}

		ImGui::SetNextItemWidth(110.f);
		if (ImGui::SliderInt("area", &p.iAreaType, 0, 3, Area_Name(p.iAreaType)))
			m_bDirty = true;

		if (p.iAreaType > 0)
		{
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("range", &p.iAreaRange, 1.f, 0, 10000))
				m_bDirty = true;

			/* The official AreaAngle column is the box width in cm and the fan
			sweep in degrees; a circle leaves it at 0. */
			if (1 != p.iAreaType)
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(110.f);
				if (ImGui::DragInt(2 == p.iAreaType ? "width" : "angle",
					&p.iAreaAngle, 1.f, 0, 2 == p.iAreaType ? 10000 : 720))
				{
					m_bDirty = true;
				}
			}

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("height", &p.iAreaHeight, 1.f, 0, 10000))
				m_bDirty = true;
			ImGui::SameLine();
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("offset X", &p.iAreaOffsetX, 1.f, -10000, 10000))
				m_bDirty = true;

			if (2 != p.iAreaType)
			{
				ImGui::SetNextItemWidth(110.f);
				if (ImGui::DragInt("inner", &p.iAreaInner, 1.f, 0, 10000))
					m_bDirty = true;
			}

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::DragInt("max targets", &p.iMaxTargets, 0.1f, 0, 64))
				m_bDirty = true;
		}

		ImGui::TreePop();
	}
}

void Client::CAnimation_Tool::Render_HitAreaWires(
	const shared_ptr<Engine::CModel>& pModel) const
{
	if (!m_bShowHitAreas || nullptr == pModel)
		return;
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	if (nullptr == pCurrentName)
		return;
	float4x4_t Root{};
	if (!CAnimationTargetService::Resolve_RootTransform(&Root))
		return;

	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	const bool_t bHasTrack = pModel->Get_AnimationProgress(
		iCurrentIndex, fPosition, fDuration) && fDuration > 0.f;
	const f32_t fRate = Get_ClipTickRate(pModel, pCurrentName);
	const int32_t iNowMs = Frame_To_Ms(static_cast<int32_t>(fPosition), fRate);

	auto Draw_Area = [&](const HIT_PARAMS& p, ImU32 iColor)
	{
		CHitAreaWire::Draw(Root, { p.iAreaType, p.iAreaRange, p.iAreaAngle,
			p.iAreaHeight, p.iAreaOffsetX, p.iAreaInner }, iColor);
	};

	for (int32_t i = 0; i < static_cast<int32_t>(m_Events.size()); ++i)
	{
		const ANIM_EVENT& evt = m_Events[i];
		if (evt.clipName != pCurrentName || EVENT_KIND::HIT != evt.eKind ||
			evt.hit.iAreaType <= 0)
			continue;
		const bool_t bActive = bHasTrack && Get_ActiveTick(evt, iNowMs) >= 0;
		const bool_t bSelected = i == m_iSelectedEvent;
		if (!bActive && !bSelected)
			continue;
		Draw_Area(evt.hit, bActive ?
			IM_COL32(255, 70, 60, 255) : IM_COL32(255, 210, 90, 170));
	}

	if (m_iRefWireHitIndex >= 0)
	{
		for (const SKILL_TIMING& row : m_SkillRef)
		{
			if (row.iSkillId != m_iRefWireSkillId)
				continue;
			if (m_iRefWireHitIndex < static_cast<int32_t>(row.hits.size()))
				Draw_Area(row.hits[m_iRefWireHitIndex].hit,
					IM_COL32(80, 200, 255, 255));
			break;
		}
	}
}

void Client::CAnimation_Tool::Render_AnimationList(const shared_ptr<Engine::CModel>& pModel)
{
	/* A zero height means "every pixel left in the window", which let the list
	   swallow the panel and the mouse wheel with it: the sections above it
	   could not be reached again without dragging the scrollbar. A fixed height
	   keeps the wheel over the list scrolling the list and the wheel anywhere
	   else scrolling the tool window. */
	constexpr f32_t CLIP_LIST_HEIGHT = 280.f;
	if (!ImGui::BeginChild(
		"##cliplist", ImVec2(0.f, CLIP_LIST_HEIGHT),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	const uint32_t iNumAnimations = pModel->Get_NumAnimations();
	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	uint32_t iShown = 0;

	if (!m_bDuplicateScanDone)
	{
		m_bDuplicateScanDone = true;
		std::unordered_set<std::string> names;
		names.reserve(iNumAnimations);
		for (uint32_t i = 0; i < iNumAnimations; ++i)
		{
			const char_t* pName = pModel->Get_AnimationName(i);
			if (nullptr != pName)
				names.insert(pName);
		}
		for (const std::string& name : names)
		{
			if (0 == name.rfind("mesh_", 0))
				continue;
			if (0 < names.count("mesh_" + name))
				m_DuplicateBodyClips.insert(name);
		}
	}

	if (!m_DuplicateBodyClips.empty())
		ImGui::TextDisabled("%zu body clips hidden (same motion listed as mesh_*)",
			m_DuplicateBodyClips.size());

	for (uint32_t i = 0; i < iNumAnimations; ++i)
	{
		const char_t* pName = pModel->Get_AnimationName(i);
		if (nullptr == pName)
			continue;

		if (0 < m_DuplicateBodyClips.count(pName))
			continue;

		/* The clip names are English and the skill names Korean, so the filter has
		to look at both or a Korean search finds nothing. */
		const CLIP_INFO* pInfo = Find_ClipInfo(pName);
		if (!Contains_NoCase(pName, m_Filter) &&
			(nullptr == pInfo || !Contains_NoCase(pInfo->name.c_str(), m_Filter)))
			continue;

		++iShown;

		/* Mark clips that already carry events so authored work is easy to find. */
		int32_t iEventCount = 0;
		for (const ANIM_EVENT& evt : m_Events)
		{
			if (evt.clipName == pName)
				++iEventCount;
		}

		char_t szLabel[MAX_PATH + 160]{};
		snprintf(szLabel, sizeof(szLabel), "%3u  %s", i, pName);

		if (nullptr != pInfo)
		{
			char_t szSkill[128]{};
			snprintf(szSkill, sizeof(szSkill), "   %s (%d)",
				pInfo->name.c_str(), pInfo->iSkillId);
			strcat_s(szLabel, szSkill);
		}

		const auto chain = m_ClipChainCounts.find(pName);
		if (m_ClipChainCounts.end() != chain)
		{
			char_t szChain[24]{};
			snprintf(szChain, sizeof(szChain), "  <chain x%d>", chain->second);
			strcat_s(szLabel, szChain);
		}

		if (iEventCount > 0)
		{
			char_t szCount[16]{};
			snprintf(szCount, sizeof(szCount), "  [%d]", iEventCount);
			strcat_s(szLabel, szCount);
		}

		if (ImGui::Selectable(szLabel, i == iCurrentIndex))
		{
			/* Always show a newly picked clip from its start, but keep the pause state. */
			pModel->Set_Animation(i, m_bLoop);
			pModel->Set_AnimTrackPosition(i, 0.f);
			m_iSelectedEvent = -1;
			Apply_KoukuSaydonPreviewScale(pModel, Resolve_LargeNamePreviewScale(pName));
		}
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No clip matches the filter.");

	ImGui::EndChild();
}

void Client::CAnimation_Tool::Render_ReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (m_bReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Animation Events?");
		m_bReloadConfirmationRequested = false;
	}

	if (!ImGui::BeginPopupModal(
		"Discard unsaved Animation Events?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}

	ImGui::TextUnformatted(
		"Reload will replace the current unsaved Animation document.");
	if (ImGui::Button("Discard and Reload"))
	{
		if (Load_Events(pModel))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

const Client::CAnimation_Tool::SKILL_TIMING* Client::CAnimation_Tool::Find_ReferenceRow(
	const char_t* pClipName) const
{
	const CLIP_INFO* pInfo = Find_ClipInfo(pClipName);
	if (nullptr == pInfo || pInfo->iSkillId <= 0)
		return nullptr;

	const SKILL_TIMING* pVariant = nullptr;
	for (const SKILL_TIMING& row : m_SkillRef)
	{
		if (row.hits.empty())
			continue;
		if (row.iSkillId == pInfo->iSkillId)
			return &row;
		if (nullptr == pVariant && row.iBaseSkillId == pInfo->iSkillId)
			pVariant = &row;
	}
	return pVariant;
}

int32_t Client::CAnimation_Tool::Count_PrecedingChainHits(const char_t* pClipName) const
{
	for (const CLIP_SEQ& seq : m_ClipSeqs)
	{
		int32_t iCount = 0;
		for (const std::string& clip : seq.clips)
		{
			if (clip == pClipName)
				return iCount;
			const auto it = m_ClipNotify.find(clip);
			if (m_ClipNotify.end() != it)
				iCount += Count_DistinctHitNotifies(it->second);
		}
	}
	return 0;
}

int32_t Client::CAnimation_Tool::Count_DistinctHitNotifies(const std::vector<NOTIFY_ROW>& rows)
{
	int32_t iCount = 0;
	for (size_t i = 0; i < rows.size(); ++i)
	{
		if (EVENT_KIND::HIT != rows[i].eKind)
			continue;
		bool_t bDuplicate = false;
		for (size_t j = 0; j < i; ++j)
		{
			if (EVENT_KIND::HIT == rows[j].eKind &&
				rows[j].fTime == rows[i].fTime && rows[j].fDuration == rows[i].fDuration &&
				rows[j].sLabel == rows[i].sLabel)
			{
				bDuplicate = true;
				break;
			}
		}
		if (!bDuplicate)
			++iCount;
	}
	return iCount;
}

void Client::CAnimation_Tool::Render_SkillReference(
	const shared_ptr<Engine::CModel>& pModel,
	bool_t bReadOnly)
{
	if (m_SkillRef.empty())
		return;

	/* Its own floating window instead of a header embedded in the main tool, so it
	can be dragged out and placed wherever (e.g. next to the event list). */
	ImGui::SetNextWindowSize(ImVec2(520.f, 420.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(500.f, 60.f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Skill Timing Reference (game-extracted)"))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("Times run from the start of the whole skill cast (all its clips in "
		"sequence). For a mid-cast clip set the offset to where that clip begins in the "
		"cast so a stamp lands on the right local time.");

	ImGui::SetNextItemWidth(160.f);
	ImGui::DragInt("cast offset (ms)", &m_iCastOffsetMs, 5.f, 0, 100000);
	if (m_iCastOffsetMs < 0)
		m_iCastOffsetMs = 0;

	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##reffilter", "filter by id or name", m_RefFilter, sizeof(m_RefFilter));

	const uint32_t iCurrentIndex = pModel->Get_CurrentAnimIndex();
	const char_t* pCurrentName = pModel->Get_AnimationName(iCurrentIndex);
	const f32_t fRate = Get_ClipTickRate(pModel,
		nullptr != pCurrentName ? pCurrentName : "");

	if (!ImGui::BeginChild("##reflist", ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		ImGui::End();
		return;
	}

	for (const SKILL_TIMING& row : m_SkillRef)
	{
		char_t szId[32]{};
		snprintf(szId, sizeof(szId), "%d", row.iSkillId);
		if (!Contains_NoCase(szId, m_RefFilter) && !Contains_NoCase(row.name.c_str(), m_RefFilter))
			continue;

		ImGui::PushID(row.iSkillId);

		ImGui::Text("%d  %s", row.iSkillId, row.name.empty() ? "(no name)" : row.name.c_str());
		if (row.iSuperArmor > 0 || row.iMoveSpeed > 0)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("superarmor %d  move %d", row.iSuperArmor, row.iMoveSpeed);
		}

		/* The clips this skill actually plays, so the right one can be opened
		without guessing which English clip name matches the Korean skill. */
		const int32_t iOwner = row.iBaseSkillId > 0 ? row.iBaseSkillId : row.iSkillId;
		int32_t iClipButton = 0;
		for (const auto& entry : m_ClipMap)
		{
			if (entry.second.iSkillId != iOwner)
				continue;

			if (iClipButton > 0 && (iClipButton % 3) != 0)
				ImGui::SameLine();
			++iClipButton;

			ImGui::PushID(iClipButton);
			if (ImGui::SmallButton(entry.first.c_str()))
				Select_Clip(pModel, entry.first);
			ImGui::PopID();
		}

		for (size_t h = 0; h < row.hits.size(); ++h)
		{
			const SKILL_HIT& src = row.hits[h];

			/* Cast time minus where the current clip starts in the cast. */
			int32_t iStartMs = src.iTimeMs - m_iCastOffsetMs;
			if (iStartMs < 0)
				iStartMs = 0;
			const int32_t iEndMs = iStartMs + src.iWidthMs;

			ImGui::PushID(static_cast<int32_t>(h));

			if (bReadOnly)
			{
				if (src.hit.iAreaType > 0)
				{
					const bool_t bWired = row.iSkillId == m_iRefWireSkillId &&
						static_cast<int32_t>(h) == m_iRefWireHitIndex;
					const std::string firstKey = src.sSourceKeys.substr(
						0, src.sSourceKeys.find(','));
					char_t szWire[160]{};
					snprintf(szWire, sizeof(szWire),
						"wire  %s%d  angle %d  offset %d  hole %d  [%s]",
						Area_Name(src.hit.iAreaType), src.hit.iAreaRange,
						src.hit.iAreaAngle, src.hit.iAreaOffsetX,
						src.hit.iAreaInner,
						firstKey.empty() ? "-" : firstKey.c_str());
					if (ImGui::Selectable(szWire, bWired))
					{
						m_iRefWireSkillId = bWired ? 0 : row.iSkillId;
						m_iRefWireHitIndex =
							bWired ? -1 : static_cast<int32_t>(h);
					}
				}
				ImGui::PopID();
				continue;
			}

			char_t szBtn[192]{};
			if (src.bTimed)
				snprintf(szBtn, sizeof(szBtn), "stamp  %d-%d ms  ->  frame %d  %s%d",
					src.iTimeMs, src.iTimeMs + src.iWidthMs,
					Ms_To_Frame(iStartMs, fRate),
					Area_Name(src.hit.iAreaType), src.hit.iAreaRange);
			else
				/* No time in the source data; it lands at the playhead instead and
				only the combat values are worth copying. */
				snprintf(szBtn, sizeof(szBtn), "stamp (untimed)  %s%d  freeze %d",
					Area_Name(src.hit.iAreaType), src.hit.iAreaRange, src.hit.iFreezeMs);

			ImGui::BeginDisabled(nullptr == pCurrentName);
			if (ImGui::Button(szBtn))
			{
				ANIM_EVENT evt{};
				evt.clipName = pCurrentName;
				evt.eKind = EVENT_KIND::HIT;
				evt.hit = src.hit;

				if (src.bTimed)
				{
					evt.iStartMs = iStartMs;
					evt.iEndMs = iEndMs;
				}
				else
				{
					f32_t fPosition = 0.f;
					f32_t fDuration = 0.f;
					pModel->Get_AnimationProgress(iCurrentIndex, fPosition, fDuration);
					evt.iStartMs = Frame_To_Ms(static_cast<int32_t>(fPosition), fRate);
					evt.iEndMs = evt.iStartMs;
				}

				m_Events.push_back(evt);
				m_iSelectedEvent = static_cast<int32_t>(m_Events.size()) - 1;
				m_PayloadEdit[0] = '\0';
				m_bDirty = true;
				m_Status = "Stamped skill " + std::to_string(row.iSkillId) +
					" onto " + pCurrentName;
			}
			ImGui::EndDisabled();

			ImGui::PopID();
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::EndChild();
	ImGui::End();
}
