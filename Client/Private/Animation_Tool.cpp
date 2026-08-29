#include "imgui.h"

#include "Animation_Tool.h"

#include "ActionPresentationTimeline.h"
#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "BalanceTool.h"
#include "BossTool.h"
#include "CameraTool.h"
#include "Character.h"
#include "DataJson.h"
#include "Effect_Catalog.h"
#include "Effect_Tool.h"
#include "EffectAuthoringTransfer.h"
#include "GameInstance.h"
#include "HitAreaWire.h"
#include "Model.h"
#include "Part_Body.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include "Transform.h"
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
#include <io.h>
#include <limits>
#include <span>
#include <system_error>
#include <unordered_set>

namespace
{
	constexpr const char_t* EVENT_FILE_MAGIC = "LOSTARK_ANIM_EVENTS";
	/* v6 separates position follow/snapshot from anchor/action-facing rotation. */
	constexpr int32_t EVENT_FILE_VERSION = 6;
	constexpr int32_t MAX_EVENT_COUNT = 100000;

	/* Used when a clip carries no usable rate, which would otherwise make the
	frame <-> millisecond conversion divide by zero. */
	constexpr f32_t DEFAULT_TICK_RATE = 30.f;
	/* Both Valtan bodies author into one document. That is only true because
	Tools/ModelAssetConverter/bake_ghost_valtan_animset.py puts the product clip
	vocabulary on the ghost rig, so a chain written on either body names clips
	the other also owns, and promote_valtan_animation_chains.py still joins this
	one document to the one presentation model it pins.

	The ghost body also keeps its own rpbf_02.ao_* clips, which the product body
	does not have. Authoring one of those into this shared document would make
	that promotion reject a clip the product model cannot play, so the clip list
	only offers the vocabulary both bodies share. */
	constexpr const char_t* CUSTOM_CHAIN_CLIP_PREFIX = "mesh_";

	struct CUSTOM_CHAIN_PROFILE
	{
		const char_t* pAssetName;
		const wchar_t* pFileName;
		const char_t* pFileLabel;
		const char_t* pOccurrencePrefix;
		const char_t* pWindowTitle;
		/* The clip a finished chain returns to. Both bodies answer to the same
		name once the donor is attached; naming a clip the body does not have
		leaves it frozen on its last pose instead of idling. */
		const char_t* pIdleClip;
	};

	constexpr std::array<CUSTOM_CHAIN_PROFILE, 2u> CUSTOM_CHAIN_PROFILES = {
		CUSTOM_CHAIN_PROFILE{
			"Valtan",
			L"Valtan.presentation.debug.json",
			"Data/Valtan/Valtan.presentation.debug.json",
			"valtan.debug",
			"Valtan Animation Sequence Intake",
			"mesh_idle_battle_1"
		},
		CUSTOM_CHAIN_PROFILE{
			"Valtan_Ghost_MN_RPBF_02",
			L"Valtan.presentation.debug.json",
			"Data/Valtan/Valtan.presentation.debug.json",
			"valtan.debug",
			"Valtan Animation Sequence Intake (Ghost body)",
			"mesh_idle_battle_1"
		}
	};

	const CUSTOM_CHAIN_PROFILE* Find_CustomChainProfile(
		const std::string& strAssetName)
	{
		for (const CUSTOM_CHAIN_PROFILE& Profile : CUSTOM_CHAIN_PROFILES)
		{
			if (strAssetName == Profile.pAssetName)
				return &Profile;
		}
		return nullptr;
	}

	constexpr std::array<std::string_view, 7u>
		VALTAN_PATTERN_MASTER_ORDER = {
			"VALTAN_WHIRLWIND",
			"VALTAN_DASH_CHARGE",
			"VALTAN_FOUR_SLASH",
			"VALTAN_FIST_IN_OUT",
			"VALTAN_HIGH_JUMP",
			"VALTAN_FLOOR_WIPE_130",
			"VALTAN_ARENA_BREAK_109",
		};

	const Client::VALTAN_PATTERN_VIEW* Find_ValtanPatternMaster(
		const Client::VALTAN_PATTERN_TREE_VIEW& View,
		const std::string_view strPatternId)
	{
		const auto FindIn = [strPatternId](
			const std::vector<Client::VALTAN_PATTERN_VIEW>& Patterns)
			-> const Client::VALTAN_PATTERN_VIEW*
		{
			const auto Pattern = std::find_if(
				Patterns.begin(), Patterns.end(),
				[strPatternId](const Client::VALTAN_PATTERN_VIEW& Candidate)
				{
					return Candidate.bAuthoringMasterManaged &&
						Candidate.strPatternId == strPatternId;
				});
			return Pattern == Patterns.end() ? nullptr : &*Pattern;
		};
		if (const Client::VALTAN_PATTERN_VIEW* pPattern = FindIn(View.Rotation))
			return pPattern;
		return FindIn(View.Gimmicks);
	}

	bool_t Try_ResolveValtanArenaPatternAction(
		const std::string_view strStageKind,
		LostArk::Shared::WORLD_ENTITY_ACTION& eOutAction)
	{
		using LostArk::Shared::WORLD_ENTITY_ACTION;
		if ("WINDUP" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_WINDUP;
		else if ("ACTIVE" == strStageKind || "GROGGY" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_ACTIVE;
		else if ("RECOVERY" == strStageKind || "PART_BREAK" == strStageKind)
			eOutAction = WORLD_ENTITY_ACTION::PATTERN_RECOVERY;
		else
			return false;
		return true;
	}

	void Skip_Space(const char_t*& p)
	{
		while (' ' == *p || '\t' == *p)
			++p;
	}

	bool_t Read_Quoted(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		if ('\"' != *p)
			return false;
		++p;
		out.clear();
		while ('\0' != *p && '\"' != *p)
			out.push_back(*p++);
		if ('\"' != *p)
			return false;
		++p;
		return true;
	}

	bool_t Read_Token(const char_t*& p, std::string& out)
	{
		Skip_Space(p);
		out.clear();
		while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			out.push_back(*p++);
		return !out.empty();
	}

	/* key=value, where value may be quoted so paths with spaces survive. */
	bool_t Read_Pair(const char_t*& p, std::string& key, std::string& value)
	{
		Skip_Space(p);
		key.clear();
		while ('\0' != *p && '=' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
			key.push_back(*p++);
		if ('=' != *p || key.empty())
			return false;
		++p;

		value.clear();
		if ('\"' == *p)
		{
			++p;
			while ('\0' != *p && '\"' != *p)
				value.push_back(*p++);
			if ('\"' != *p)
				return false;
			++p;
		}
		else
		{
			while ('\0' != *p && ' ' != *p && '\t' != *p && '\r' != *p && '\n' != *p)
				value.push_back(*p++);
		}
		return true;
	}

	bool_t Parse_Integer(const std::string& text, int32_t& outValue)
	{
		if (text.empty())
			return false;

		const char_t* begin = text.data();
		const char_t* end = begin + text.size();
		const auto result = std::from_chars(begin, end, outValue);
		return std::errc{} == result.ec && result.ptr == end;
	}

	bool_t Parse_Float(const std::string& text, f32_t& outValue)
	{
		if (text.empty())
			return false;
		char_t* pEnd = nullptr;
		outValue = std::strtof(text.c_str(), &pEnd);
		return pEnd == text.c_str() + text.size() &&
			std::isfinite(outValue);
	}

	bool_t Has_RemainingToken(const char_t* p)
	{
		Skip_Space(p);
		return '\0' != *p && '\r' != *p && '\n' != *p;
	}

	bool_t Is_SafeQuotedText(const std::string& text)
	{
		return std::string::npos == text.find_first_of("\"\r\n");
	}

	bool_t Contains_NoCase(const char_t* pText, const char_t* pNeedle)
	{
		if (nullptr == pText)
			return false;
		if ('\0' == pNeedle[0])
			return true;

		for (const char_t* p = pText; *p != '\0'; ++p)
		{
			const char_t* a = p;
			const char_t* b = pNeedle;
			while (*a != '\0' && *b != '\0' &&
				tolower(static_cast<unsigned char>(*a)) == tolower(static_cast<unsigned char>(*b)))
			{
				++a;
				++b;
			}
			if ('\0' == *b)
				return true;
		}
		return false;
	}

	/* Start_Animation cuts straight to the new clip's first frame, which is what
	   the product Valtan still does. The preview blends the way CCharacter
	   already does so the animator judges motion rather than the seam. The
	   blend snapshot has to be taken before the track reset overwrites the
	   bones, hence the two calls in this order. Clips repeated back to back
	   keep their hard restart: the pose is identical, so there is nothing to
	   blend, and a re-strike is meant to read as a new strike. */
	bool_t Start_PreviewClip(
		const shared_ptr<Engine::CModel>& pModel,
		const char_t* pClipName,
		const bool_t bLoop,
		const f32_t fBlendSeconds)
	{
		if (nullptr == pModel || nullptr == pClipName)
			return false;
		if (fBlendSeconds > 0.f)
			pModel->Set_Animation(pClipName, bLoop, fBlendSeconds);
		return pModel->Start_Animation(pClipName, bLoop);
	}
}

Client::CAnimation_Tool::CAnimation_Tool(
	shared_ptr<CCharacterPreviewPanel> pPreviewPanel,
	CBalanceTool* const pBalanceTool,
	CBossTool* const pBossTool)
	: m_pPreviewPanel(std::move(pPreviewPanel))
	, m_pBalanceTool(pBalanceTool)
	, m_pBossTool(pBossTool)
{
}

Client::CAnimation_Tool::~CAnimation_Tool()
{
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
		m_bValtanCombatObjectSoundCuesDirty;
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
	m_AssetName = assetName;
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
	m_bValtanPatternMasterPlaying = false;
	m_bValtanPatternMasterPaused = false;
	m_bShowValtanSourceReferenceWindow = false;
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

std::string Client::CAnimation_Tool::Get_EventFilePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Authored") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animevents")).string();
}

std::string Client::CAnimation_Tool::Get_SkillReferencePath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".skilltiming")).string();
}

std::string Client::CAnimation_Tool::Get_ClipMapPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipmap")).string();
}

std::string Client::CAnimation_Tool::Get_ClipNotifyPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".animnotify")).string();
}

std::string Client::CAnimation_Tool::Get_ClipSeqPath() const
{
	return CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipseq")).string();
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
			return;
		}
	}
}

void Client::CAnimation_Tool::Render()
{
	ImGui::SetNextWindowSize(ImVec2(460.f, 720.f), ImGuiCond_FirstUseEver);
	/* A scrollbar only moves when the content is longer than the window. Left
	   alone the window keeps whatever height it was dragged to, or restored to
	   from imgui.ini, and once that is taller than the display the content
	   always fits: the bar is drawn full height and nothing scrolls, while the
	   rows past the bottom edge sit off-screen. Capping the height at the work
	   area is what puts the overflow back inside the window. */
	if (const ImGuiViewport* pViewport = ImGui::GetMainViewport())
	{
		ImGui::SetNextWindowSizeConstraints(
			ImVec2(320.f, 200.f),
			ImVec2(FLT_MAX, pViewport->WorkSize.y));
	}
	const bool_t isTargetLocked = Is_AnyDocumentDirty();
	m_pPreviewPanel->Set_SessionLock(
		CHARACTER_PREVIEW_LOCK_OWNER::ANIMATION_TOOL,
		isTargetLocked,
		"Save or discard Animation Events, Skill Bindings, and Workbench Sound bindings before changing target.");
	m_pPreviewPanel->Refresh_Level();

	if (!ImGui::Begin(
		"Action Presentation Workbench",
		nullptr,
		ImGuiWindowFlags_AlwaysVerticalScrollbar))
	{
		ImGui::End();
		return;
	}

	m_pPreviewPanel->Render_Selector(false, {});
	if (!Sync_AssetName())
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
			"Playback-only preview: authored events and gameplay skill bindings are disabled.");
		ImGui::BeginDisabled(
			m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
		Render_Playback(pModel);
		ImGui::EndDisabled();
		if ("Valtan" == m_AssetName)
		{
			Render_ValtanPatternPreview(pModel);
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
			m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying);
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

const char_t* Client::CAnimation_Tool::ValtanPatternMasterPathName(
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	switch (ePath)
	{
	case VALTAN_PATTERN_PREVIEW_PATH::NORMAL:
		return "Normal";
	case VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY:
		return "Wall -> Groggy -> Recovery";
	case VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK:
		return "Wall -> Groggy -> Part Break";
	default:
		return "Invalid";
	}
}

std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CAnimation_Tool::Collect_ValtanPatternMasterPatterns() const
{
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_ValtanPatternMasterView.Get_PatternCount());
	std::unordered_set<std::string> Collected;
	for (const std::string_view strPatternId : VALTAN_PATTERN_MASTER_ORDER)
	{
		if (const VALTAN_PATTERN_VIEW* pPattern = Find_ValtanPatternMaster(
			m_ValtanPatternMasterView, strPatternId))
		{
			Patterns.push_back(pPattern);
			Collected.insert(pPattern->strPatternId);
		}
	}
	const auto AppendProductPresentation = [&Patterns, &Collected](
		const std::vector<VALTAN_PATTERN_VIEW>& Source)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Source)
		{
			if (!Pattern.Stages.empty() &&
				Collected.insert(Pattern.strPatternId).second)
			{
				Patterns.push_back(&Pattern);
			}
		}
	};
	AppendProductPresentation(m_ValtanPatternMasterView.Rotation);
	AppendProductPresentation(m_ValtanPatternMasterView.Gimmicks);
	return Patterns;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternMaster()
{
	VALTAN_PATTERN_TREE_VIEW Staged;
	std::string Status;
	if (!CValtanPatternTree::Load(Staged, Status))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master reload rejected; current admitted view and pose preserved: " +
			Status;
		return false;
	}

	size_t iManagedPatternCount = 0u;
	const auto CountManaged = [&iManagedPatternCount](
		const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
	{
		for (const VALTAN_PATTERN_VIEW& Pattern : Patterns)
		{
			if (Pattern.bAuthoringMasterManaged)
				++iManagedPatternCount;
		}
	};
	CountManaged(Staged.Rotation);
	CountManaged(Staged.Gimmicks);
	if (iManagedPatternCount < VALTAN_PATTERN_MASTER_ORDER.size())
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master reload rejected; expected the 7 baseline managed patterns, found " +
			std::to_string(iManagedPatternCount) +
			". Current admitted view and pose preserved.";
		return false;
	}
	for (const std::string_view strPatternId : VALTAN_PATTERN_MASTER_ORDER)
	{
		if (nullptr == Find_ValtanPatternMaster(Staged, strPatternId))
		{
			m_strValtanPatternMasterStatus =
				"Valtan Pattern Master reload rejected; required pattern is missing: " +
				std::string(strPatternId) +
				". Current admitted view and pose preserved.";
			return false;
		}
	}

	m_ValtanPatternMasterView = std::move(Staged);
	/* Sound is a fail-open presentation lane.  Its loader still stages the
	   complete document and only commits on a valid gameplay/animation join;
	   a rejected refresh preserves the last admitted lane while animation and
	   Server gameplay remain usable. */
	(void)Reload_ValtanPatternSoundCues();
	(void)Reload_ValtanPatternShakeCues();
	(void)Reload_ValtanCombatObjectSoundCues();
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_ValtanPatternMasterPatterns();
	m_iValtanPatternMasterSelected = std::clamp(
		m_iValtanPatternMasterSelected, 0,
		static_cast<int32_t>(Patterns.size() - 1u));
	m_strValtanPatternMasterStatus =
		"Valtan Product presentation admitted: " +
		std::to_string(Patterns.size()) + " patterns (" +
		std::to_string(iManagedPatternCount) + " authoring-master managed) from "
		"Data/Valtan/Valtan.gameplay.json + Valtan.presentation.json. " +
		Status;
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternSoundCues()
{
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanPatternSoundCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanPatternSoundCueStatus =
			"Sound lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternSoundCues = std::move(Staged);
	m_bValtanPatternSoundCuesReady = true;
	m_strValtanPatternSoundCueStatus =
		"Admitted " + std::to_string(m_ValtanPatternSoundCues.Cues.size()) +
		" Valtan sound cue occurrences.";
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanPatternShakeCues()
{
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanPatternShakeCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanPatternShakeCueStatus =
			"Camera/Shake lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanPatternShakeCues = std::move(Staged);
	m_bValtanPatternShakeCuesReady = true;
	m_strValtanPatternShakeCueStatus =
		"Admitted " + std::to_string(m_ValtanPatternShakeCues.Cues.size()) +
		" Valtan camera-shake cue occurrences.";
	return true;
}

bool_t Client::CAnimation_Tool::Reload_ValtanCombatObjectSoundCues()
{
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT Staged;
	std::string Status;
	if (!CValtanCombatObjectSoundCueDocument::Load_Source(Staged, Status))
	{
		m_strValtanCombatObjectSoundCueStatus =
			"Server-hit Sound lane reload rejected; the previous admitted lane was preserved: " +
			Status;
		return false;
	}
	m_ValtanCombatObjectSoundCues = std::move(Staged);
	m_bValtanCombatObjectSoundCuesReady = true;
	m_strValtanCombatObjectSoundCueStatus =
		"Admitted " +
		std::to_string(m_ValtanCombatObjectSoundCues.Cues.size()) +
		" Server-hit-qualified Valtan Sound cue(s).";
	return true;
}

bool_t Client::CAnimation_Tool::Preview_ValtanSoundAsset(
	const std::string& strResourceAssetId)
{
	const std::filesystem::path SoundPath =
		CRuntimeAssetRoot::Resolve(strResourceAssetId);
	std::error_code Error;
	if (SoundPath.empty() ||
		!std::filesystem::is_regular_file(SoundPath, Error) || Error)
	{
		m_strValtanPatternSoundCueStatus =
			"Sound asset preview rejected: Resources-relative asset is missing or invalid: " +
			strResourceAssetId + ".";
		return false;
	}
	if (FAILED(CGameInstance::Get().Play_Sound(SoundPath.wstring(), 1.f)))
	{
		m_strValtanPatternSoundCueStatus =
			"Sound asset preview failed: " + strResourceAssetId + ".";
		return false;
	}
	m_strValtanPatternSoundCueStatus =
		"Previewing sound asset: " + strResourceAssetId + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Build_ValtanPatternMasterTimeline(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM>& OutPlaylist,
	uint32_t& iOutDurationMs,
	std::string& strOutStatus) const
{
	OutPlaylist.clear();
	iOutDurationMs = 0u;
	strOutStatus.clear();
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		strOutStatus = "Animation target changed before the master timeline was staged.";
		return false;
	}
	if (Pattern.Stages.empty())
	{
		strOutStatus = "Selected pattern has no admitted stages in the split "
			"Valtan.gameplay.json + Valtan.presentation.json source.";
		return false;
	}

	std::vector<const VALTAN_STAGE_VIEW*> StagePath;
	if (!CValtanPatternTree::Build_PreviewStagePath(
		Pattern, ePath, StagePath, strOutStatus))
	{
		return false;
	}

	uint64_t iTimelineMs = 0u;
	for (const VALTAN_STAGE_VIEW* pStage : StagePath)
	{
		if (nullptr == pStage || 0u == pStage->iDurationMs ||
			pStage->strActionId.empty())
		{
			strOutStatus =
				"Master branch graph contains an incomplete animation stage.";
			return false;
		}
		const VALTAN_STAGE_VIEW& Stage = *pStage;
		LostArk::Shared::WORLD_ENTITY_ACTION eStageAction =
			LostArk::Shared::WORLD_ENTITY_ACTION::END;
		if (!Try_ResolveValtanArenaPatternAction(
				Stage.strStageKind, eStageAction) ||
			LostArk::Shared::WORLD_ENTITY_ACTION::END == eStageAction)
		{
			strOutStatus = "Master stage kind has no Arena snapshot action: " +
				Stage.strStageKind + ".";
			return false;
		}
		const uint64_t iStageTimelineStartMs = iTimelineMs;
		if (Stage.bSuppressAnimation)
		{
			if (!Stage.ClipOccurrences.empty())
			{
				strOutStatus = "NONE animation stage unexpectedly owns clips: " +
					Stage.strActionId + ".";
				return false;
			}
			VALTAN_PATTERN_MASTER_PLAY_ITEM Item;
			Item.strPatternId = Pattern.strPatternId;
			Item.strPatternDisplayName = Pattern.strDisplayName;
			Item.strStageId = Stage.strStageId;
			Item.strSequenceRole = Stage.strSequenceRole;
			Item.strStageKind = Stage.strStageKind;
			Item.strActionId = Stage.strActionId;
			Item.iAuthoringWallMs = Stage.iDurationMs;
			Item.iTimelineStartMs = static_cast<uint32_t>(iTimelineMs);
			Item.iStageTimelineStartMs =
				static_cast<uint32_t>(iStageTimelineStartMs);
			Item.iOccurrenceNumber = 1u;
			Item.iOccurrenceCount = 1u;
			Item.bSuppressAnimation = true;
			OutPlaylist.push_back(std::move(Item));
			iTimelineMs += Stage.iDurationMs;
			if (iTimelineMs > static_cast<uint64_t>(
					(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
			continue;
		}
		if (Stage.ClipOccurrences.empty())
		{
			strOutStatus = "Master branch graph contains an unbound animation stage: " +
				Stage.strActionId + ".";
			return false;
		}
		const size_t iPlayableOccurrenceCount =
			Stage.iAuthoringRepeatCount > 1u ?
				static_cast<size_t>(Stage.iAuthoringRepeatCount) :
				Stage.ClipOccurrences.size();
		if (0u == iPlayableOccurrenceCount ||
			iPlayableOccurrenceCount != Stage.ClipOccurrences.size())
		{
			strOutStatus = "Master repeatCount does not own exactly its explicit occurrences for " +
				Stage.strActionId + ".";
			return false;
		}

		uint64_t iStageAnimationWallMs = 0u;
		for (size_t iClip = 0u; iClip < iPlayableOccurrenceCount; ++iClip)
		{
			const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
				Stage.ClipOccurrences[iClip];
			if (Clip.strClipOccurrenceId.empty() || Clip.strClipName.empty() ||
				0u == Clip.iAuthoringWallMs ||
				!std::isfinite(Clip.fPlayRate) || Clip.fPlayRate <= 0.f)
			{
				strOutStatus = "Master occurrence is incomplete: " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}

			uint32_t iAnimationIndex = (std::numeric_limits<uint32_t>::max)();
			for (uint32_t iAnimation = 0u;
				iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
			{
				const char_t* pName = pModel->Get_AnimationName(iAnimation);
				if (nullptr != pName && Clip.strClipName == pName)
				{
					iAnimationIndex = iAnimation;
					break;
				}
			}
			if ((std::numeric_limits<uint32_t>::max)() == iAnimationIndex)
			{
				strOutStatus = "Scene Valtan model is missing master clip " +
					Clip.strClipName + ".";
				return false;
			}

			f32_t fTrackPosition = 0.f;
			f32_t fTrackDuration = 0.f;
			const f32_t fTickRate =
				pModel->Get_AnimationTickPerSecond(iAnimationIndex);
			if (!std::isfinite(fTickRate) || fTickRate <= 0.f ||
				!pModel->Get_AnimationProgress(
					iAnimationIndex, fTrackPosition, fTrackDuration) ||
				!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
			{
				strOutStatus = "Master clip has no valid model clock: " +
					Clip.strClipName + ".";
				return false;
			}
			const ACTION_PRESENTATION_CLIP_TIMING Timing{
				fTrackDuration / fTickRate,
				Clip.iPlayMs,
				Clip.fPlayRate,
				Clip.bLoop,
				static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
			f32_t fSourceDuration = 0.f;
			f32_t fSourceWallDuration = 0.f;
			if (!CActionPresentationTimeline::Resolve_ClipDuration(
				Timing, fSourceDuration, fSourceWallDuration))
			{
				strOutStatus = "Master source window is outside the model clip: " +
					Clip.strClipOccurrenceId + ".";
				return false;
			}

			VALTAN_PATTERN_MASTER_PLAY_ITEM Item;
			Item.strPatternId = Pattern.strPatternId;
			Item.strPatternDisplayName = Pattern.strDisplayName;
			Item.strStageId = Stage.strStageId;
			Item.strSequenceRole = Stage.strSequenceRole;
			Item.strStageKind = Stage.strStageKind;
			Item.strActionId = Stage.strActionId;
			Item.strClipOccurrenceId = Clip.strClipOccurrenceId;
			Item.strClipName = Clip.strClipName;
			Item.iSourceStartMs = Clip.iSourceStartMs;
			Item.iPlayMs = Clip.iPlayMs;
			Item.iAuthoringWallMs = Clip.iAuthoringWallMs;
			Item.iTimelineStartMs = static_cast<uint32_t>(iTimelineMs);
			Item.iStageTimelineStartMs =
				static_cast<uint32_t>(iStageTimelineStartMs);
			Item.iOccurrenceNumber = static_cast<uint32_t>(iClip + 1u);
			Item.iOccurrenceCount = static_cast<uint32_t>(
				iPlayableOccurrenceCount);
			Item.fPlayRate = Clip.fPlayRate;
			Item.bRepeatUntilStageEnd = Clip.bLoop;
			OutPlaylist.push_back(std::move(Item));

			iStageAnimationWallMs += Clip.iAuthoringWallMs;
			iTimelineMs += Clip.iAuthoringWallMs;
			if (iTimelineMs > static_cast<uint64_t>(
				(std::numeric_limits<uint32_t>::max)()))
			{
				strOutStatus = "Master authoring timeline duration overflowed.";
				return false;
			}
		}
		if (iStageAnimationWallMs != Stage.iDurationMs)
		{
			strOutStatus = "Master occurrences do not fill Server stage " +
				Stage.strStageId + ".";
			return false;
		}
	}
	if (OutPlaylist.empty() || 0u == iTimelineMs)
	{
		strOutStatus = "Master authoring timeline is empty.";
		return false;
	}
	iOutDurationMs = static_cast<uint32_t>(iTimelineMs);
	strOutStatus = "Admitted " + Pattern.strPatternId + " / " +
		ValtanPatternMasterPathName(ePath) + " / " +
		std::to_string(OutPlaylist.size()) + " presentation items / " +
		std::to_string(iOutDurationMs) + " ms.";
	return true;
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_PATTERN_PREVIEW_PATH ePath)
{
	std::vector<VALTAN_PATTERN_MASTER_PLAY_ITEM> StagedPlaylist;
	uint32_t iStagedDurationMs = 0u;
	std::string Status;
	if (!Build_ValtanPatternMasterTimeline(
		Pattern, ePath, pModel, StagedPlaylist, iStagedDurationMs, Status))
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; current model pose preserved: " +
			Status;
		return false;
	}

	const shared_ptr<CValtan> PreviewBoss =
		CAnimationTargetService::Resolve_Boss();
	if (nullptr == PreviewBoss || PreviewBoss->Get_BodyModel() != pModel)
	{
		m_strValtanPatternMasterStatus =
			"Valtan Pattern Master play rejected; the staged model is not the current local Valtan boss.";
		return false;
	}

	const uint32_t iPreviousAnimation = pModel->Get_CurrentAnimIndex();
	f32_t fPreviousPosition = 0.f;
	f32_t fPreviousDuration = 0.f;
	const bool_t bPreviousTrack = pModel->Get_AnimationProgress(
		iPreviousAnimation, fPreviousPosition, fPreviousDuration);
	const bool_t bPreviousPaused = pModel->Is_AnimPaused();
	const bool_t bPreviousLoop = pModel->Is_AnimLoop();
	const f32_t fPreviousSpeed = m_bValtanPatternPreviewPlaying ?
		m_fValtanPatternPreviewSpeed : 1.f;
	if (m_bValtanPatternPreviewPlaying)
	{
		Reset_ValtanPatternPreviewState(
			"Source reference preview yielded to Valtan Pattern Master.");
	}

	m_ValtanPatternMasterPlaylist = std::move(StagedPlaylist);
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = iStagedDurationMs;
	m_eValtanPatternMasterPath = ePath;
	m_bValtanPatternMasterPlaying = true;
	m_bValtanPatternMasterPaused = false;
	m_ValtanPatternMasterModel = pModel;
	m_ValtanPatternMasterBoss = PreviewBoss;
	m_iValtanPatternMasterTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_strValtanPatternMasterStatus = Status;
	if (Activate_ValtanPatternMasterItem(pModel, 0u, 0.f))
	{
		Update_ValtanPatternMasterHitAreaPreview();
		return true;
	}

	Reset_ValtanPatternMasterPreviewState(
		"Valtan Pattern Master play failed; previous model pose restored.");
	if (bPreviousTrack &&
		pModel->Start_Animation(iPreviousAnimation, bPreviousLoop))
	{
		pModel->Set_AnimationSpeed(fPreviousSpeed);
		pModel->Set_AnimTrackPosition(iPreviousAnimation, fPreviousPosition);
		pModel->Set_AnimPaused(bPreviousPaused);
		pModel->Play_Animation(0.f);
	}
	return false;
}

bool_t Client::CAnimation_Tool::Apply_ValtanPatternMasterPose(
	const shared_ptr<Engine::CModel>& pModel,
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item,
	const f32_t fLocalWallSeconds,
	const bool_t bForceAnimationEdge) const
{
	if (nullptr == pModel || !std::isfinite(fLocalWallSeconds) ||
		fLocalWallSeconds < 0.f)
	{
		return false;
	}
	const shared_ptr<CValtan> Boss = m_ValtanPatternMasterBoss.lock();
	if (nullptr == Boss || Boss->Get_BodyModel() != pModel)
		return false;
	LostArk::Shared::WORLD_ENTITY_ACTION ePatternAction =
		LostArk::Shared::WORLD_ENTITY_ACTION::END;
	if (!Try_ResolveValtanArenaPatternAction(
			Item.strStageKind, ePatternAction))
	{
		return false;
	}
	const f32_t fStageWallSeconds =
		(static_cast<f32_t>(Item.iTimelineStartMs -
			Item.iStageTimelineStartMs) * 0.001f) +
		fLocalWallSeconds;
	if (!Boss->Apply_LocalPatternPresentationSample(
			ePatternAction,
			Item.strActionId,
			fStageWallSeconds,
			bForceAnimationEdge))
	{
		return false;
	}
	/* Animation Tool owns the wall clock and samples the Product presentation
	   pose explicitly. Pausing the model prevents a second local frame clock
	   from drifting between those samples. */
	pModel->Set_AnimPaused(true);
	return true;
}

bool_t Client::CAnimation_Tool::Activate_ValtanPatternMasterItem(
	const shared_ptr<Engine::CModel>& pModel,
	const std::size_t iItem,
	const f32_t fLocalWallSeconds)
{
	if (nullptr == pModel ||
		iItem >= m_ValtanPatternMasterPlaylist.size())
	{
		return false;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[iItem];
	const f32_t fDurationSeconds =
		static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f;
	/* The master timeline drives the pose itself: it holds the model paused and
	writes the authored source time every frame.  A blend cannot finish on that
	path -- CModel::Play_Animation feeds Update_AnimBlend a zero delta while the
	model is paused, so m_fBlendElapsed never advances and every bone is pulled
	back to the captured blend-from pose at ratio zero.  Start the occurrence
	without a blend; the very next Apply_ValtanPatternMasterPose owns the pose. */
	if (!std::isfinite(fLocalWallSeconds) || fLocalWallSeconds < 0.f ||
		fLocalWallSeconds > fDurationSeconds + 0.000001f)
	{
		return false;
	}
	m_iValtanPatternMasterItem = iItem;
	m_fValtanPatternMasterItemElapsedSeconds =
		std::clamp(fLocalWallSeconds, 0.f, fDurationSeconds);
	if (!Apply_ValtanPatternMasterPose(
		pModel, Item, m_fValtanPatternMasterItemElapsedSeconds, true))
	{
		return false;
	}
	m_strValtanPatternMasterStatus = Item.bSuppressAnimation ?
		"Playing admitted NONE stage (boss pose hold) | " +
			Item.strStageId + " / " + Item.strSequenceRole + "." :
		"Playing admitted occurrence " + Item.strClipOccurrenceId + " | " +
			Item.strStageId + " / " + Item.strSequenceRole + ".";
	return true;
}

bool_t Client::CAnimation_Tool::Seek_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const f32_t fTimelineSeconds,
	const bool_t bPause)
{
	if (!m_bValtanPatternMasterPlaying || nullptr == pModel ||
		m_ValtanPatternMasterPlaylist.empty() ||
		!std::isfinite(fTimelineSeconds) || fTimelineSeconds < 0.f)
	{
		return false;
	}
	const f32_t fTimelineMs = std::clamp(
		fTimelineSeconds * 1000.f, 0.f,
		static_cast<f32_t>(m_iValtanPatternMasterDurationMs));
	for (size_t iItem = 0u;
		iItem < m_ValtanPatternMasterPlaylist.size(); ++iItem)
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[iItem];
		const f32_t fEndMs = static_cast<f32_t>(
			Item.iTimelineStartMs + Item.iAuthoringWallMs);
		const bool_t bLast =
			iItem + 1u == m_ValtanPatternMasterPlaylist.size();
		if (fTimelineMs < fEndMs || bLast)
		{
			f32_t fLocalSeconds = std::clamp(
				(fTimelineMs - static_cast<f32_t>(Item.iTimelineStartMs)) *
					0.001f,
				0.f,
				static_cast<f32_t>(Item.iAuthoringWallMs) * 0.001f);
			/* Sampling an exact looping endpoint wraps to source time zero.
			   The timeline endpoint means the final visible pose, so only that
			   endpoint is moved to the previous representable wall time. */
			if (bLast && fTimelineMs >= fEndMs && fLocalSeconds > 0.f)
				fLocalSeconds = std::nextafter(fLocalSeconds, 0.f);
			if (!Activate_ValtanPatternMasterItem(
				pModel, iItem, fLocalSeconds))
			{
				return false;
			}
			m_bValtanPatternMasterPaused = bPause;
			Update_ValtanPatternMasterHitAreaPreview();
			return true;
		}
	}
	return false;
}

void Client::CAnimation_Tool::Advance_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bValtanPatternMasterPlaying ||
		m_iValtanPatternMasterItem >= m_ValtanPatternMasterPlaylist.size())
	{
		return;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
	const f32_t fAbsoluteSeconds =
		static_cast<f32_t>(Item.iTimelineStartMs) * 0.001f +
		m_fValtanPatternMasterItemElapsedSeconds;
	if (fAbsoluteSeconds * 1000.f + 0.001f >=
		static_cast<f32_t>(m_iValtanPatternMasterDurationMs))
	{
		Stop_ValtanPatternMasterPreview(
			pModel,
			"Valtan Pattern Master timeline completed; idle restored.");
		return;
	}
	if (!Seek_ValtanPatternMasterPreview(pModel, fAbsoluteSeconds, false))
	{
		Stop_ValtanPatternMasterPreview(
			pModel,
			"Valtan Pattern Master timeline failed to advance; idle restored.");
	}
}

void Client::CAnimation_Tool::Stop_ValtanPatternMasterPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& status)
{
	if (nullptr != pModel)
	{
		m_bLoop = true;
		pModel->Set_AnimationSpeed(1.f);
		if (!pModel->Start_Animation("mesh_idle_battle_1", true))
			pModel->Set_AnimPaused(true);
	}
	Reset_ValtanPatternMasterPreviewState(status);
}

void Client::CAnimation_Tool::Reset_ValtanPatternMasterPreviewState(
	const std::string& status)
{
	const shared_ptr<Engine::CModel> PreviewModel =
		m_ValtanPatternMasterModel.lock();
	const shared_ptr<CValtan> PreviewBoss =
		m_ValtanPatternMasterBoss.lock();
	m_bValtanPatternMasterPlaying = false;
	m_bValtanPatternMasterPaused = false;
	m_iValtanPatternMasterItem = 0u;
	m_fValtanPatternMasterItemElapsedSeconds = 0.f;
	m_iValtanPatternMasterDurationMs = 0u;
	m_ValtanPatternMasterPlaylist.clear();
	m_strValtanPatternMasterStatus = status;
	m_ValtanPatternMasterModel.reset();
	m_ValtanPatternMasterBoss.reset();
	m_iValtanPatternMasterTargetGeneration = 0u;
	if (nullptr != PreviewBoss)
	{
		PreviewBoss->Reset_LocalPatternPresentationSample();
	}
	else if (nullptr != PreviewModel)
	{
		PreviewModel->Set_AnimationSpeed(1.f);
		PreviewModel->Set_AnimPaused(false);
	}
#ifdef _DEBUG
	if (nullptr != PreviewBoss)
	{
		PreviewBoss->Clear_PatternHitAreaPreview();
	}
#endif
}

void Client::CAnimation_Tool::Update_ValtanPatternMasterHitAreaPreview()
{
#ifdef _DEBUG
	const shared_ptr<CValtan> Boss = CAnimationTargetService::Resolve_Boss();
	if (nullptr == Boss)
		return;
	if (!m_bValtanPatternMasterPlaying ||
		m_iValtanPatternMasterItem >= m_ValtanPatternMasterPlaylist.size())
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
		m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
	const f32_t fStageSeconds =
		(static_cast<f32_t>(Item.iTimelineStartMs -
			Item.iStageTimelineStartMs) * 0.001f) +
		m_fValtanPatternMasterItemElapsedSeconds;
	Boss->Set_PatternHitAreaPreview(Item.strActionId, fStageSeconds);
#endif
}

void Client::CAnimation_Tool::Render_ValtanPresentationLanes(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	struct JOINED_COMBAT_OBJECT_SOUND final
	{
		VALTAN_COMBAT_OBJECT_SOUND_CUE* pCue = nullptr;
		uint32_t iHitOffsetMs = 0u;
	};
	const std::vector<std::string> ValtanSoundEventNames =
		CSoundCueCatalog::Collect_EventNames("Valtan");

	ImGui::SeparatorText("Joined Presentation Lanes");
	ImGui::TextDisabled(
		"Server Stage -> Animation -> Effect -> Sound Asset -> Camera/Shake -> World Event -> Combat Object");
	if (!m_strValtanPatternSoundCueStatus.empty())
		ImGui::TextWrapped("Sound: %s", m_strValtanPatternSoundCueStatus.c_str());
	if (!m_strValtanPatternShakeCueStatus.empty())
		ImGui::TextWrapped("Camera/Shake: %s",
			m_strValtanPatternShakeCueStatus.c_str());
	if (!m_strValtanCombatObjectSoundCueStatus.empty())
		ImGui::TextWrapped("Server-hit Sound: %s",
			m_strValtanCombatObjectSoundCueStatus.c_str());

	for (std::size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		std::vector<const VALTAN_PATTERN_SOUND_CUE*> SoundCues;
		if (m_bValtanPatternSoundCuesReady)
		{
			for (const VALTAN_PATTERN_SOUND_CUE& Cue :
				m_ValtanPatternSoundCues.Cues)
			{
				if (Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId)
				{
					SoundCues.push_back(&Cue);
				}
			}
		}
		std::vector<const VALTAN_PATTERN_SHAKE_CUE*> ShakeCues;
		if (m_bValtanPatternShakeCuesReady)
		{
			for (const VALTAN_PATTERN_SHAKE_CUE& Cue :
				m_ValtanPatternShakeCues.Cues)
			{
				if (Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId)
				{
					ShakeCues.push_back(&Cue);
				}
			}
		}
		std::vector<const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW*> WorldEvents;
		for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
			Pattern.WorldEventTriggerRefs)
		{
			if (Event.strPatternId == Pattern.strPatternId &&
				Event.strStageId == Stage.strStageId)
			{
				WorldEvents.push_back(&Event);
			}
		}
		std::vector<JOINED_COMBAT_OBJECT_SOUND> CombatObjectSoundCues;
		std::size_t iMissingCombatObjectSounds = 0u;
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
			Stage.CombatObjectEffects)
		{
			if (CombatObject.HitIds.size() != CombatObject.HitOffsetsMs.size())
			{
				iMissingCombatObjectSounds += (std::max)(
					CombatObject.HitIds.size(), CombatObject.HitOffsetsMs.size());
				continue;
			}
			for (std::size_t iHit = 0u; iHit < CombatObject.HitIds.size(); ++iHit)
			{
				VALTAN_COMBAT_OBJECT_SOUND_CUE* pJoinedCue = nullptr;
				if (m_bValtanCombatObjectSoundCuesReady)
				{
					const auto Found = std::find_if(
						m_ValtanCombatObjectSoundCues.Cues.begin(),
						m_ValtanCombatObjectSoundCues.Cues.end(),
						[&CombatObject, &iHit](
							const VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue)
						{
							return Cue.strCombatObjectArchetypeId ==
									CombatObject.strCombatObjectArchetypeId &&
								Cue.strHitId == CombatObject.HitIds[iHit];
						});
					if (Found != m_ValtanCombatObjectSoundCues.Cues.end())
						pJoinedCue = &*Found;
				}
				if (nullptr == pJoinedCue)
				{
					++iMissingCombatObjectSounds;
					continue;
				}
				CombatObjectSoundCues.push_back(
					{ pJoinedCue, CombatObject.HitOffsetsMs[iHit] });
			}
		}

		ImGui::PushID(static_cast<int32_t>(iStage));
		const std::string StageLabel = Stage.strStageId + " | " +
			std::to_string(Stage.iDurationMs) + " ms | animation " +
			std::to_string(Stage.ClipOccurrences.size()) + " | effect " +
			std::to_string(Stage.ProductCues.size() +
				Stage.CombatObjectEffects.size()) + " | sound " +
			std::to_string(SoundCues.size() + CombatObjectSoundCues.size()) +
			" | camera/shake " +
			std::to_string(Stage.CameraInvocations.size() + ShakeCues.size()) +
			" | world " + std::to_string(WorldEvents.size());
		if (ImGui::TreeNodeEx(
			StageLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("Server action: %s", Stage.strActionId.c_str());

			ImGui::SeparatorText("Animation");
			if (Stage.bSuppressAnimation)
				ImGui::TextDisabled("NONE (hold current boss pose)");
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
				Stage.ClipOccurrences)
			{
				ImGui::BulletText(
					"%s | %s | source +%u ms | play %u ms | wall %u ms",
					Clip.strClipOccurrenceId.c_str(), Clip.strClipName.c_str(),
					Clip.iSourceStartMs, Clip.iPlayMs, Clip.iAuthoringWallMs);
			}

			ImGui::SeparatorText("Effect");
			if (Stage.ProductCues.empty() && Stage.CombatObjectEffects.empty())
				ImGui::TextDisabled("No Product Effect cue on this stage.");
			for (std::size_t iCue = 0u; iCue < Stage.ProductCues.size(); ++iCue)
			{
				const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue =
					Stage.ProductCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s | anchor %s | %s",
					Cue.strOccurrenceId.c_str(), Cue.strEffectAssetId.c_str(),
					Cue.strAnchorSlotId.c_str(),
					Cue.bUsesStageClock ? "stage clock" :
						Cue.strClipOccurrenceId.c_str());
				ImGui::SameLine();
				if (ImGui::SmallButton("Open Effect Tool"))
				{
					m_strEffectToolOpenPatternId = Pattern.strPatternId;
					m_strEffectToolOpenStageId = Stage.strStageId;
					m_strEffectToolOpenCueOccurrenceId = Cue.strOccurrenceId;
					m_strEffectToolOpenEffectAssetId = Cue.strEffectAssetId;
					m_hasEffectToolOpenRequest = true;
				}
				ImGui::PopID();
			}
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& CombatObject :
				Stage.CombatObjectEffects)
			{
				ImGui::BulletText(
					"Server combat object %s x%u -> %s",
					CombatObject.strCombatObjectArchetypeId.c_str(),
					CombatObject.iSpawnValue,
					CombatObject.strEffectAssetId.c_str());
				for (std::size_t iHit = 0u;
					iHit < CombatObject.HitOffsetsMs.size(); ++iHit)
				{
					const char_t* pHitId = iHit < CombatObject.HitIds.size() ?
						CombatObject.HitIds[iHit].c_str() : "MISSING_HIT_ID";
					ImGui::TextDisabled(
						"  Server hit %s at combat-object +%u ms",
						pHitId, CombatObject.HitOffsetsMs[iHit]);
				}
			}

			ImGui::SeparatorText("Sound");
			if (SoundCues.empty() && CombatObjectSoundCues.empty())
				ImGui::TextDisabled("No Sound cue on this stage.");
			for (std::size_t iCue = 0u; iCue < SoundCues.size(); ++iCue)
			{
				const VALTAN_PATTERN_SOUND_CUE& Cue = *SoundCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | event %s | clip +%u ms | %s",
					Cue.strOccurrenceId.c_str(), Cue.strSoundEvent.c_str(),
					Cue.iStartMs,
					VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
						Cue.eRepeatPolicy ? "each loop" : "once");
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				if (Variants.empty())
				{
					ImGui::TextColored(
						ImVec4(1.f, 0.35f, 0.25f, 1.f),
						"  MISSING: event has no WAV asset variant.");
				}
				for (std::size_t iVariant = 0u;
					iVariant < Variants.size(); ++iVariant)
				{
					ImGui::PushID(static_cast<int32_t>(iVariant));
					if (ImGui::SmallButton("Preview WAV"))
						(void)Preview_ValtanSoundAsset(Variants[iVariant]);
					ImGui::SameLine();
					ImGui::TextDisabled("%s", Variants[iVariant].c_str());
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			if (!CombatObjectSoundCues.empty())
				ImGui::SeparatorText("Server Hit Sound");
			ImGui::PushID("server-hit-sounds");
			for (std::size_t iCue = 0u;
				iCue < CombatObjectSoundCues.size(); ++iCue)
			{
				const JOINED_COMBAT_OBJECT_SOUND& Joined =
					CombatObjectSoundCues[iCue];
				VALTAN_COMBAT_OBJECT_SOUND_CUE& Cue = *Joined.pCue;
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s + %s | event %s | object +%u ms",
					Cue.strBindingId.c_str(),
					Cue.strCombatObjectArchetypeId.c_str(), Cue.strHitId.c_str(),
					Cue.strSoundEvent.c_str(), Joined.iHitOffsetMs);
				ImGui::SetNextItemWidth(360.f);
				if (ImGui::BeginCombo("Impact Sound Event", Cue.strSoundEvent.c_str()))
				{
					for (const std::string& EventName : ValtanSoundEventNames)
					{
						const bool_t bSelected = EventName == Cue.strSoundEvent;
						if (ImGui::Selectable(EventName.c_str(), bSelected) && !bSelected)
						{
							Cue.strSoundEvent = EventName;
							m_bValtanCombatObjectSoundCuesDirty = true;
							m_strValtanCombatObjectSoundCueStatus =
								"UNSAVED: " + Cue.strBindingId + " -> " + EventName;
						}
						if (bSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				const std::vector<std::string>& Variants =
					CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
				for (std::size_t iVariant = 0u;
					iVariant < Variants.size(); ++iVariant)
				{
					ImGui::PushID(static_cast<int32_t>(iVariant));
					if (ImGui::SmallButton("Preview Impact WAV"))
						(void)Preview_ValtanSoundAsset(Variants[iVariant]);
					ImGui::SameLine();
					ImGui::TextDisabled("%s", Variants[iVariant].c_str());
					ImGui::PopID();
				}
				ImGui::PopID();
			}
			ImGui::PopID();
			if (0u != iMissingCombatObjectSounds)
			{
				ImGui::TextColored(
					ImVec4(1.f, 0.25f, 0.20f, 1.f),
					"COVERAGE GAP: %zu Server combat-object hit(s) have no exact Sound binding.",
					iMissingCombatObjectSounds);
			}

			ImGui::SeparatorText("Camera / Shake");
			if (Stage.CameraInvocations.empty() && ShakeCues.empty())
				ImGui::TextDisabled("No Camera or Shake cue on this stage.");
			for (std::size_t iCue = 0u;
				iCue < Stage.CameraInvocations.size(); ++iCue)
			{
				const VALTAN_CAMERA_INVOCATION_VIEW& Invocation =
					Stage.CameraInvocations[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s -> %s | %s +%u ms | %s %u ms",
					Invocation.strCameraInvocationId.c_str(),
					Invocation.strCameraCueId.c_str(),
					Invocation.strTrigger.c_str(), Invocation.iStartOffsetMs,
					Invocation.strDurationPolicy.c_str(), Invocation.iDurationMs);
				ImGui::SameLine();
				if (ImGui::SmallButton("Open Camera Tool"))
				{
					m_strCameraToolOpenCueId = Invocation.strCameraCueId;
					m_hasCameraToolOpenRequest = true;
				}
				ImGui::PopID();
			}
			ImGui::PushID("camera-shakes");
			for (std::size_t iCue = 0u; iCue < ShakeCues.size(); ++iCue)
			{
				const VALTAN_PATTERN_SHAKE_CUE& Cue = *ShakeCues[iCue];
				ImGui::PushID(static_cast<int32_t>(iCue));
				ImGui::BulletText(
					"%s | %s +%u ms | %.3f s | F/R/U/FOV amp %.3g/%.3g/%.3g/%.3g | %s",
					Cue.strOccurrenceId.c_str(), Cue.strClipOccurrenceId.c_str(),
					Cue.iStartMs, Cue.Spec.fDurationSeconds,
					Cue.Spec.Forward.fAmplitude, Cue.Spec.Right.fAmplitude,
					Cue.Spec.Up.fAmplitude, Cue.Spec.Fov.fAmplitude,
					VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
						Cue.eRepeatPolicy ? "each loop" : "once");
				ImGui::PopID();
			}
			ImGui::PopID();

			ImGui::SeparatorText("World Event / Runtime UI");
			if (WorldEvents.empty())
				ImGui::TextDisabled("No world-event trigger on this stage.");
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW* const pEvent :
				WorldEvents)
			{
				ImGui::BulletText(
					"%s/%s -> %s",
					pEvent->strPatternId.c_str(), pEvent->strStageId.c_str(),
					pEvent->strTriggerKind.c_str());
			}
			ImGui::TextDisabled(
				"HUD/UI observes replicated Server state; this stage does not author a second UI command path.");

			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CAnimation_Tool::Render_ValtanPatternMaster(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SeparatorText("Valtan Action Presentation Workbench");
	ImGui::TextWrapped(
		"One joined view over Server Stage, Animation, Effect, Sound Asset, and Combat Object. Pattern Offline samples the Product animation locally; Server Replay/Live submits the same stable pattern ID to the real Arena authority, where movement, hit, grab, damage, Effect, and Sound run.");
	ImGui::TextDisabled(
		"Encounter target: Valtan (Product Server authority). KakulSaydon remains unavailable until a Product world, encounter, pattern source, and Server runtime are admitted.");

	ImGui::SeparatorText("Server Arena Environment");
	ImGui::TextDisabled(
		"These presets use the active Valtan room's destruction transaction; no wall, collision or navigation state is changed locally.");
	ImGui::PushID("WorkbenchArenaPreset");
	ImGui::BeginDisabled(nullptr == m_pBossTool);
	const auto arenaPresetButton = [this](
		const char_t* label,
		const LostArk::Shared::VALTAN_ARENA_PRESET preset)
	{
		if (!ImGui::Button(label))
			return;
		std::string status;
		(void)m_pBossTool->Set_ServerArenaPreset(preset, status);
		m_strValtanPatternMasterStatus = std::move(status);
	};
	arenaPresetButton(
		"Fresh / All Walls", LostArk::Shared::VALTAN_ARENA_PRESET::FRESH);
	ImGui::SameLine();
	arenaPresetButton(
		"Circle / Walls Gone",
		LostArk::Shared::VALTAN_ARENA_PRESET::CIRCLE_WALLS_GONE);
	arenaPresetButton(
		"Break 3 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::THREE_OCLOCK_BROKEN);
	ImGui::SameLine();
	arenaPresetButton(
		"Break 9 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::NINE_OCLOCK_BROKEN);
	arenaPresetButton(
		"Break 3 + 9 O'Clock",
		LostArk::Shared::VALTAN_ARENA_PRESET::BOTH_SIDES_BROKEN);
	ImGui::EndDisabled();
	if (nullptr != m_pBossTool)
	{
		const std::string arenaStatus =
			m_pBossTool->Get_ServerArenaPresetStatus();
		if (!arenaStatus.empty())
			ImGui::TextWrapped("Arena: %s", arenaStatus.c_str());
	}
	ImGui::PopID();

	if (!m_bValtanPatternMasterLoadAttempted)
	{
		m_bValtanPatternMasterLoadAttempted = true;
		Reload_ValtanPatternMaster();
	}
	ImGui::BeginDisabled(
		m_bValtanPatternMasterPlaying || m_bValtanPatternPreviewPlaying ||
		m_bValtanCombatObjectSoundCuesDirty);
	if (ImGui::SmallButton("Reload Valtan Pattern Master"))
		Reload_ValtanPatternMaster();
	ImGui::EndDisabled();
	if (m_bValtanCombatObjectSoundCuesDirty && ImGui::IsItemHovered(
			ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("Save the edited Server-hit Sound binding before reload.");
	}

	/* Opened ahead of the baseline admission below: assembling a chain by
	   hand must stay possible even when the master itself refuses to load. */
	if (ImGui::SmallButton("Open Animation Sequence Intake"))
		m_bShowValtanCustomChainWindow = true;
	if (m_bShowValtanCustomChainWindow)
		Render_ValtanCustomChainWindow(pModel);

	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_ValtanPatternMasterPatterns();
	const bool_t bReady =
		Patterns.size() >= VALTAN_PATTERN_MASTER_ORDER.size();
	if (!bReady)
	{
		ImGui::TextDisabled(
			"No admitted Valtan pattern master is available; current model is untouched.");
		if (!m_strValtanPatternMasterStatus.empty())
			ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());
		return;
	}

	m_iValtanPatternMasterSelected = std::clamp(
		m_iValtanPatternMasterSelected, 0,
		static_cast<int32_t>(Patterns.size() - 1u));
	const VALTAN_PATTERN_VIEW* pSelected =
		Patterns[static_cast<size_t>(m_iValtanPatternMasterSelected)];
	if (pSelected->bManualServerAudition)
	{
		ImGui::TextDisabled(
			"Manual Server audition | phase %u | source chain %s | automatic rotation disabled",
			pSelected->iAuthoringPhase,
			pSelected->strSourceAnimationChainId.c_str());
	}
	ImGui::BeginDisabled(m_bValtanPatternMasterPlaying);
	if (ImGui::BeginCombo(
		"Pattern##ValtanPatternMaster",
		(pSelected->strPatternId + " | " +
			pSelected->strDisplayName).c_str()))
	{
		for (size_t iPattern = 0u; iPattern < Patterns.size(); ++iPattern)
		{
			const VALTAN_PATTERN_VIEW& Pattern = *Patterns[iPattern];
			const std::string Label = Pattern.strPatternId + " | " +
				Pattern.strDisplayName;
			if (ImGui::Selectable(
				Label.c_str(),
				static_cast<int32_t>(iPattern) ==
					m_iValtanPatternMasterSelected))
			{
				m_iValtanPatternMasterSelected =
					static_cast<int32_t>(iPattern);
				m_eValtanPatternMasterPath =
					VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
			}
		}
		ImGui::EndCombo();
	}
	if ("VALTAN_DASH_CHARGE" == pSelected->strPatternId &&
		ImGui::BeginCombo(
			"Dash authoring path##ValtanPatternMaster",
			ValtanPatternMasterPathName(m_eValtanPatternMasterPath)))
	{
		for (const VALTAN_PATTERN_PREVIEW_PATH eCandidate : {
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
			VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK })
		{
			if (ImGui::Selectable(
				ValtanPatternMasterPathName(eCandidate),
				eCandidate == m_eValtanPatternMasterPath))
			{
				m_eValtanPatternMasterPath = eCandidate;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SeparatorText("Playback Modes");
	if (ImGui::Button("Pattern Offline"))
	{
		Start_ValtanPatternMasterPreview(
			pModel, *pSelected, m_eValtanPatternMasterPath);
	}
	ImGui::SameLine();
	if (ImGui::Button("Server Replay / Live"))
	{
		std::string Status;
		if (nullptr == m_pBossTool)
			Status = "Server Replay/Live is unavailable: Boss Tool service is missing.";
		else
			(void)m_pBossTool->Play_ServerPattern(
				pSelected->strPatternId, Status);
		m_strValtanPatternMasterStatus = std::move(Status);
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_bValtanPatternMasterPlaying);
	if (ImGui::Button(
		m_bValtanPatternMasterPaused ? "Resume Master" : "Pause Master"))
	{
		m_bValtanPatternMasterPaused = !m_bValtanPatternMasterPaused;
		pModel->Set_AnimPaused(true);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop Master"))
	{
		Stop_ValtanPatternMasterPreview(
			pModel, "Valtan Pattern Master preview stopped; idle restored.");
	}
	ImGui::EndDisabled();

	ImGui::SeparatorText("Authoring Revision");
	const bool_t bDraftDirty = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Is_ValtanDraftDirty();
	ImGui::TextDisabled(
		"%s | gameplay wall clock and presentation remain separate joined domains",
		bDraftDirty ? "UNSAVED GAMEPLAY DRAFT" : "saved source");
	if (m_bValtanCombatObjectSoundCuesDirty)
		ImGui::TextColored(ImVec4(1.f, 0.75f, 0.2f, 1.f),
			"UNSAVED SERVER-HIT SOUND BINDING");
	ImGui::BeginDisabled(nullptr == m_pBalanceTool);
	if (ImGui::Button("Save"))
	{
		std::string Status;
		std::string SoundStatus;
		const bool_t bSoundReady = !m_bValtanCombatObjectSoundCuesDirty ||
			CValtanCombatObjectSoundCueDocument::Validate_SourceDraft(
				m_ValtanCombatObjectSoundCues, SoundStatus);
		if (!bSoundReady)
		{
			Status = "Save validation failed; Product and Sound source were preserved: " +
				SoundStatus;
		}
		else if (m_pBalanceTool->Save_ValtanProduct(Status))
		{
			if (m_bValtanCombatObjectSoundCuesDirty)
			{
				const std::string ProductStatus = Status;
				if (CValtanCombatObjectSoundCueDocument::Save_Source(
						m_ValtanCombatObjectSoundCues, SoundStatus))
				{
					m_bValtanCombatObjectSoundCuesDirty = false;
					std::string RuntimeStatus =
						"No active Valtan; binding applies on the next spawn.";
					if (const shared_ptr<CValtan> Boss =
							CAnimationTargetService::Resolve_Boss())
					{
						(void)Boss->Reload_CombatObjectSoundCues(RuntimeStatus);
					}
					Status = ProductStatus + " " + SoundStatus + " " + RuntimeStatus;
					(void)Reload_ValtanPatternMaster();
				}
				else
				{
					Status = ProductStatus +
						" Product is saved, but Sound source was not replaced: " +
						SoundStatus;
				}
			}
			else
			{
				(void)Reload_ValtanPatternMaster();
			}
		}
		m_strValtanPatternMasterStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	if (nullptr != m_pBalanceTool)
	{
		ImGui::TextDisabled("Runtime activation: %s",
			m_pBalanceTool->Get_ValtanCandidateApplyClass().empty() ? "NONE" :
				m_pBalanceTool->Get_ValtanCandidateApplyClass().c_str());
	}

	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
		"Preview speed##ValtanPatternMaster",
		&m_fValtanPatternPreviewSpeed, 0.25f, 3.f, "%.2fx"))
	{
		m_fValtanPatternPreviewSpeed = std::clamp(
			m_fValtanPatternPreviewSpeed, 0.25f, 3.f);
	}
	if (m_bValtanPatternMasterPlaying &&
		m_iValtanPatternMasterItem < m_ValtanPatternMasterPlaylist.size())
	{
		const VALTAN_PATTERN_MASTER_PLAY_ITEM& Item =
			m_ValtanPatternMasterPlaylist[m_iValtanPatternMasterItem];
		f32_t fTimelineSeconds =
			static_cast<f32_t>(Item.iTimelineStartMs) * 0.001f +
			m_fValtanPatternMasterItemElapsedSeconds;
		const f32_t fDurationSeconds =
			static_cast<f32_t>(m_iValtanPatternMasterDurationMs) * 0.001f;
		char_t TimelineFormat[96]{};
		snprintf(
			TimelineFormat, sizeof(TimelineFormat),
			"%%.3f s / %.3f s", fDurationSeconds);
		ImGui::SetNextItemWidth(-1.f);
		if (ImGui::SliderFloat(
			"##ValtanPatternMasterTimeline",
			&fTimelineSeconds, 0.f, fDurationSeconds, TimelineFormat))
		{
			if (!Seek_ValtanPatternMasterPreview(
				pModel, fTimelineSeconds, true))
			{
				m_strValtanPatternMasterStatus =
					"Master seek rejected; current admitted pose preserved.";
			}
		}
		if (Item.bSuppressAnimation)
		{
			ImGui::Text(
				"Now %s / %s | animation NONE | wall %u ms",
				Item.strStageId.c_str(), Item.strSequenceRole.c_str(),
				Item.iAuthoringWallMs);
			ImGui::TextDisabled(
				"Boss pose hold; no additional Valtan animation clip is started.");
		}
		else
		{
			ImGui::Text(
				"Now %s / %s | occurrence %u/%u | wall %u ms",
				Item.strStageId.c_str(), Item.strSequenceRole.c_str(),
				Item.iOccurrenceNumber, Item.iOccurrenceCount,
				Item.iAuthoringWallMs);
			ImGui::TextDisabled(
				"%s | source +%u ms, play %u ms, rate %.6g, repeatUntilStageEnd=%s",
				Item.strClipName.c_str(), Item.iSourceStartMs, Item.iPlayMs,
				Item.fPlayRate,
				Item.bRepeatUntilStageEnd ? "true" : "false");
		}
	}
	if (!m_strValtanPatternMasterStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternMasterStatus.c_str());

	ImGui::SeparatorText("Phase-1 weighted normal selection");
	ImGui::TextDisabled(
		"%s | five admitted normal patterns; health-bar mechanics keep queue precedence",
		m_ValtanPatternMasterView.NormalSelection.strSelectionMode.c_str());
	for (const VALTAN_NORMAL_SELECTION_RANGE_VIEW& Range :
		m_ValtanPatternMasterView.NormalSelection.Ranges)
	{
		ImGui::BulletText(
			"%s | bars %u > HP bar > %u",
			Range.strRotationId.c_str(), Range.iFromHealthBar,
			Range.iToHealthBar);
	}
	for (const std::string& PatternId :
		m_ValtanPatternMasterView.NormalSelection.PatternIds)
	{
		const auto Found = std::find_if(
			Patterns.begin(), Patterns.end(),
			[&PatternId](const VALTAN_PATTERN_VIEW* pPattern)
			{
				return nullptr != pPattern &&
					pPattern->strPatternId == PatternId;
			});
		if (Found == Patterns.end())
			continue;
		const VALTAN_PATTERN_VIEW& Pattern = **Found;
		ImGui::TextDisabled(
			"%s | weight %u | range %.1f..%.1f | max consecutive %u | %s / %s",
			Pattern.strPatternId.c_str(), Pattern.iSelectionWeight,
			Pattern.fMinimumRange, Pattern.fMaximumRange,
			Pattern.iMaximumConsecutiveUses,
			Pattern.strArmorRequirement.c_str(),
			Pattern.strPhaseRequirement.c_str());
	}

	ImGui::SeparatorText("Counter reaction animation layers (reference only)");
	ImGui::TextDisabled(
		"These exact legacy Encounter actions are not admitted into the seven-pattern Phase-1 pool.");
	for (const VALTAN_COUNTER_REACTION_LAYER_VIEW& Layer :
		m_ValtanPatternMasterView.CounterReactionLayers)
	{
		ImGui::PushID(Layer.strReactionLayerId.c_str());
		const std::string Label = Layer.strReactionLayerId + " | " +
			Layer.strOwnerPatternId + "/" + Layer.strOwnerStageId;
		if (ImGui::TreeNodeEx(Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow))
		{
			for (const auto* pAction :
				{ &Layer.Window, &Layer.Success, &Layer.Failure })
			{
				ImGui::TextDisabled("action %s", pAction->strActionId.c_str());
				for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
					pAction->ClipOccurrences)
				{
					ImGui::BulletText(
						"%s | %s", Clip.strClipOccurrenceId.c_str(),
						Clip.strClipName.c_str());
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::SeparatorText("Presentation sources");
	for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
		pSelected->PresentationSources)
	{
		ImGui::BulletText(
			"action %u | sequence %u | %s",
			Source.iSourceActionId, Source.iSequenceIndex,
			Source.strRole.c_str());
	}

	ImGui::SeparatorText("Ordered stages / sequence roles");
	std::vector<const VALTAN_STAGE_VIEW*> PreviewPath;
	std::string PreviewPathStatus;
	if (CValtanPatternTree::Build_PreviewStagePath(
		*pSelected, m_eValtanPatternMasterPath,
		PreviewPath, PreviewPathStatus))
	{
		std::string PathLabel = "Current path: ";
		for (size_t iStage = 0u; iStage < PreviewPath.size(); ++iStage)
		{
			if (0u != iStage)
				PathLabel += " -> ";
			PathLabel += PreviewPath[iStage]->strStageId;
		}
		ImGui::TextDisabled("%s", PathLabel.c_str());
	}
	else
	{
		ImGui::TextDisabled("Current path rejected: %s",
			PreviewPathStatus.c_str());
	}
	for (size_t iStage = 0u; iStage < pSelected->Stages.size(); ++iStage)
	{
		const VALTAN_STAGE_VIEW& Stage = pSelected->Stages[iStage];
		ImGui::PushID(static_cast<int32_t>(iStage));
		char_t Label[512]{};
		snprintf(
			Label, sizeof(Label),
			"%02zu  %s | %s | %u ms | %s | repeatCount %u | %zu occurrences",
			iStage + 1u, Stage.strStageId.c_str(),
			Stage.strSequenceRole.c_str(), Stage.iDurationMs,
			Stage.strAnimationEndPolicy.c_str(),
			Stage.iAuthoringRepeatCount, Stage.ClipOccurrences.size());
		if (ImGui::TreeNodeEx(Label, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextDisabled("action %s | %s | damage %s",
				Stage.strActionId.c_str(), Stage.strStageKind.c_str(),
				Stage.strServerDamageProfileId.empty() ? "NONE" :
					Stage.strServerDamageProfileId.c_str());
			if (nullptr != m_pBalanceTool)
			{
				uint32_t iDraftDurationMs = Stage.iDurationMs;
				std::string DraftStatus;
				if (m_pBalanceTool->Get_ValtanStageDurationDraft(
					pSelected->strPatternId, Stage.strStageId,
					iDraftDurationMs, DraftStatus))
				{
					const uint32_t iStepMs = 100u;
					const uint32_t iFastStepMs = 1000u;
					ImGui::SetNextItemWidth(190.f);
					if (ImGui::InputScalar(
						"Server wall / blank timeline ms",
						ImGuiDataType_U32, &iDraftDurationMs,
						&iStepMs, &iFastStepMs, "%u"))
					{
						iDraftDurationMs = std::clamp(
							iDraftDurationMs, 1u, 600000u);
						(void)m_pBalanceTool->Set_ValtanStageDurationDraft(
							pSelected->strPatternId, Stage.strStageId,
							iDraftDurationMs, DraftStatus);
						m_strValtanPatternMasterStatus = DraftStatus;
					}
					if (iDraftDurationMs != Stage.iDurationMs)
						ImGui::TextColored(
							ImVec4(1.f, 0.70f, 0.20f, 1.f),
							"Draft %u ms | saved source %u ms",
							iDraftDurationMs, Stage.iDurationMs);
					if ("VALTAN_HIGH_JUMP" == pSelected->strPatternId &&
						"AIRBORNE" == Stage.strStageId)
					{
						ImGui::TextDisabled(
							"Axe flight / blank timeline: mesh_att_battle_8_01_loop repeats to this Server boundary.");
					}
				}
				else if (!DraftStatus.empty())
				{
					ImGui::TextDisabled("Draft unavailable: %s", DraftStatus.c_str());
				}
			}
			for (size_t iClip = 0u;
				iClip < Stage.ClipOccurrences.size(); ++iClip)
			{
				const VALTAN_CLIP_OCCURRENCE_VIEW& Clip =
					Stage.ClipOccurrences[iClip];
				ImGui::BulletText(
					"%zu. %s | %s | wall %u ms",
					iClip + 1u, Clip.strClipName.c_str(),
					Clip.strClipOccurrenceId.c_str(),
					Clip.iAuthoringWallMs);
				ImGui::Indent();
				ImGui::TextDisabled(
					"sourceStartMs=%u playMs=%u playRate=%.6g repeatUntilStageEnd=%s",
					Clip.iSourceStartMs, Clip.iPlayMs, Clip.fPlayRate,
					Clip.bLoop ? "true" : "false");
				ImGui::Unindent();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	Render_ValtanPresentationLanes(*pSelected);
}

void Client::CAnimation_Tool::Render_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	Render_ValtanPatternMaster(pModel);
	ImGui::SeparatorText("Secondary / Read-only Source Reference (1-67)");
	ImGui::TextWrapped(
		"Historical reference only: Valtan.patternpreview.json and Valtan.clipseq are source evidence, not the Product pattern master. Every source clip plays once for review; environment, movement, effects, and damage are not simulated here.");

	if (!m_bValtanPatternPreviewLoadAttempted)
	{
		m_bValtanPatternPreviewLoadAttempted = true;
		std::string status;
		VALTAN_PATTERN_PREVIEW_DOCUMENT staged;
		if (CValtanPatternPreviewDocument::Load(
				"Valtan", Collect_ClipNames(pModel), staged, status))
		{
			m_ValtanPatternPreviewDocument = std::move(staged);
			m_strValtanPatternPreviewStatus = status;
		}
		else
		{
			m_strValtanPatternPreviewStatus =
				"Pattern preview unavailable; current animation preserved: " + status;
		}
	}

	const bool_t bReady =
		67u == m_ValtanPatternPreviewDocument.Patterns.size();
	ImGui::BeginDisabled(!bReady);
	if (ImGui::Button("Play All 1-67"))
		Start_ValtanPatternPreview(pModel, 1u, 67u);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
			"Speed##ValtanPatternPreview",
			&m_fValtanPatternPreviewSpeed,
			0.5f,
			3.f,
			"%.1fx"))
	{
		if (m_bValtanPatternPreviewPlaying)
			pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
	}

	const int32_t iPatternCount = static_cast<int32_t>(
		m_ValtanPatternPreviewDocument.Patterns.size());
	if (iPatternCount > 0)
	{
		m_iValtanPatternPreviewSelected = std::clamp(
			m_iValtanPatternPreviewSelected, 0, iPatternCount - 1);
	}
	if (ImGui::Button("< Pattern") && m_iValtanPatternPreviewSelected > 0)
	{
		--m_iValtanPatternPreviewSelected;
		Start_ValtanPatternPreview(
			pModel,
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1),
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1));
	}
	ImGui::SameLine();
	if (ImGui::Button("Play Selected") && iPatternCount > 0)
	{
		const uint32_t Number =
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1);
		Start_ValtanPatternPreview(pModel, Number, Number);
	}
	ImGui::SameLine();
	if (ImGui::Button("Pattern >") &&
		m_iValtanPatternPreviewSelected + 1 < iPatternCount)
	{
		++m_iValtanPatternPreviewSelected;
		Start_ValtanPatternPreview(
			pModel,
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1),
			static_cast<uint32_t>(m_iValtanPatternPreviewSelected + 1));
	}

	ImGui::BeginDisabled(!m_bValtanPatternPreviewPlaying);
	if (ImGui::Button(m_bValtanPatternPreviewPaused ? "Resume" : "Pause Sequence"))
	{
		m_bValtanPatternPreviewPaused = !m_bValtanPatternPreviewPaused;
		pModel->Set_AnimPaused(m_bValtanPatternPreviewPaused);
	}
	ImGui::SameLine();
	if (ImGui::Button("Replay Step"))
		Activate_ValtanPatternPreviewItem(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Skip Step"))
		Advance_ValtanPatternPreview(pModel);
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		Stop_ValtanPatternPreview(pModel, "Pattern preview stopped; idle restored.");
	ImGui::EndDisabled();
	ImGui::EndDisabled();

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size() &&
		m_ValtanPatternPreviewPlaylist[
			m_iValtanPatternPreviewItem].iPatternNumber >= 1u)
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		m_iValtanPatternPreviewSelected = static_cast<int32_t>(
			Item.iPatternNumber - 1u);
	}

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		ImGui::Text(
			"Now %02u  [%s]  step %u/%u  %s",
			Item.iPatternNumber,
			CValtanPatternPreviewDocument::Evidence_Name(Item.eEvidence),
			Item.iStepNumber,
			Item.iStepCount,
			Item.bPatternMarker ? "(no body animation)" : Item.strClipName.c_str());
		if (Item.bPatternMarker)
		{
			ImGui::TextDisabled("Source sequence: pattern marker only");
		}
		else
		{
			ImGui::Text(
				"Source action %u  sequence %d  repeat %u/%u  source step %u/%u",
				Item.iSourceActionId,
				Item.iSequenceIndex,
				Item.iSequenceRepeatNumber,
				Item.iSequenceRepeatCount,
				Item.iSourceStepNumber,
				Item.iSourceStepCount);
			ImGui::TextWrapped(
				"%s  [%s]",
				Item.strSequenceName.c_str(),
				Item.strSequenceMode.c_str());
		}
		const f32_t Duration = (std::max)(
			0.001f, m_fValtanPatternPreviewItemDurationSeconds);
		ImGui::ProgressBar(
			std::clamp(m_fValtanPatternPreviewElapsedSeconds / Duration, 0.f, 1.f),
			ImVec2(-1.f, 0.f));
		ImGui::TextWrapped("%s", Item.strPatternLabel.c_str());
		ImGui::TextDisabled("%s", Item.strNote.c_str());
	}
	if (!m_strValtanPatternPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strValtanPatternPreviewStatus.c_str());

	if (bReady && iPatternCount > 0)
	{
		const VALTAN_PATTERN_PREVIEW_ENTRY& SelectedPattern =
			m_ValtanPatternPreviewDocument.Patterns[
				static_cast<std::size_t>(m_iValtanPatternPreviewSelected)];
		ImGui::SeparatorText("Selected source sequences");
		if (SelectedPattern.Sequences.empty())
		{
			ImGui::TextDisabled("No body-animation source sequence for this pattern.");
		}
		else
		{
			for (const VALTAN_PATTERN_PREVIEW_SOURCE_SEQUENCE_REF& Sequence :
				SelectedPattern.Sequences)
			{
				ImGui::BulletText(
					"action %u  sequence %d  repeat x%u  [%s]",
					Sequence.iSourceActionId,
					Sequence.iSequenceIndex,
					Sequence.iRepeat,
					Sequence.strSequenceMode.c_str());
				ImGui::Indent();
				ImGui::TextWrapped("%s", Sequence.strSequenceName.c_str());
				ImGui::Unindent();
			}
		}
	}

	if (ImGui::SmallButton("Open Read-only Source Sequence Window"))
		m_bShowValtanSourceReferenceWindow = true;
	if (m_bShowValtanSourceReferenceWindow)
		Render_ValtanPatternReferenceWindow(pModel);

	if (bReady)
	{
		const bool_t bListVisible = ImGui::BeginChild(
			"##ValtanPatternPreviewList",
			ImVec2(0.f, 260.f),
			ImGuiChildFlags_Borders,
			ImGuiWindowFlags_NoScrollWithMouse);
		if (bListVisible)
		{
			for (int32_t Index = 0; Index < iPatternCount; ++Index)
			{
				const VALTAN_PATTERN_PREVIEW_ENTRY& Pattern =
					m_ValtanPatternPreviewDocument.Patterns[Index];
				char_t Label[512]{};
				snprintf(
					Label,
					sizeof(Label),
					"%02u  [%s]  %s",
					Pattern.iNumber,
					CValtanPatternPreviewDocument::Evidence_Name(Pattern.eEvidence),
					Pattern.strLabel.c_str());
				ImGui::PushID(Index);
				if (ImGui::Selectable(
						Label, Index == m_iValtanPatternPreviewSelected))
				{
					m_iValtanPatternPreviewSelected = Index;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", Pattern.strNote.c_str());
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
}

void Client::CAnimation_Tool::Render_ValtanPatternReferenceWindow(
	const shared_ptr<Engine::CModel>& pModel)
{
	ImGui::SetNextWindowSize(ImVec2(460.f, 560.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(540.f, 60.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(
		"Valtan Source Reference (Read-only)",
		&m_bShowValtanSourceReferenceWindow))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Secondary source evidence only. One button per read-only Valtan.clipseq sequence; Product authoring must use Valtan Pattern Master in the main Animation Tool window.");

	if (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		ImGui::Text("Now step %u/%u  %s",
			Item.iStepNumber,
			Item.iStepCount,
			Item.bPatternMarker ? "(no body animation)" :
				Item.strClipName.c_str());
		ImGui::TextWrapped("%s", Item.strPatternLabel.c_str());
		const f32_t Duration = (std::max)(
			0.001f, m_fValtanPatternPreviewItemDurationSeconds);
		ImGui::ProgressBar(
			std::clamp(
				m_fValtanPatternPreviewElapsedSeconds / Duration, 0.f, 1.f),
			ImVec2(-1.f, 0.f));
		if (ImGui::Button(
			m_bValtanPatternPreviewPaused ? "Resume" : "Pause"))
		{
			m_bValtanPatternPreviewPaused = !m_bValtanPatternPreviewPaused;
			pModel->Set_AnimPaused(m_bValtanPatternPreviewPaused);
		}
		ImGui::SameLine();
		if (ImGui::Button("Replay"))
			Activate_ValtanPatternPreviewItem(pModel);
		ImGui::SameLine();
		if (ImGui::Button("Stop"))
		{
			Stop_ValtanPatternPreview(
				pModel, "Pattern preview stopped; idle restored.");
		}
	}
	else
	{
		ImGui::TextDisabled("Nothing playing.");
	}
	ImGui::SetNextItemWidth(120.f);
	if (ImGui::SliderFloat(
			"Speed##ValtanPatternReference",
			&m_fValtanPatternPreviewSpeed, 0.5f, 3.f, "%.1fx") &&
		m_bValtanPatternPreviewPlaying)
	{
		pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
	}

	ImGui::Checkbox(
		"Raid patterns only (420xxx)", &m_bValtanRaidSequencesOnly);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##valtanpatternfilter",
		"filter by skill id or name",
		m_ValtanPatternFilter,
		sizeof(m_ValtanPatternFilter));

	if (ImGui::BeginChild(
		"##ValtanPatternReferenceButtons",
		ImVec2(0.f, 0.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (m_ClipSeqs.empty())
		{
			ImGui::TextDisabled(
				"No clip sequences loaded from Valtan.clipseq.");
		}
		const int32_t iSequenceCount =
			static_cast<int32_t>(m_ClipSeqs.size());
		for (int32_t Index = 0; Index < iSequenceCount; ++Index)
		{
			const CLIP_SEQ& Seq = m_ClipSeqs[Index];
			if (m_bValtanRaidSequencesOnly &&
				(Seq.iSkillId < 420000 || Seq.iSkillId > 429999))
			{
				continue;
			}
			char_t SkillId[16]{};
			snprintf(SkillId, sizeof(SkillId), "%d", Seq.iSkillId);
			if (!Contains_NoCase(SkillId, m_ValtanPatternFilter) &&
				!Contains_NoCase(Seq.name.c_str(), m_ValtanPatternFilter))
			{
				continue;
			}
			char_t Label[512]{};
			snprintf(Label, sizeof(Label), "%s  seq%d [%s]  (%zu clips)##%d",
				Seq.name.empty() ? SkillId : Seq.name.c_str(),
				Seq.iSeqIndex,
				Seq.sMode.c_str(),
				Seq.clips.size(),
				Index);
			ImGui::PushID(Index);
			const bool_t bPlayingThis = m_bValtanPatternPreviewPlaying &&
				m_iValtanSequenceSelected == Index;
			if (bPlayingThis)
			{
				ImGui::PushStyleColor(
					ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.24f, 1.f));
			}
			if (ImGui::Button(Label, ImVec2(-1.f, 0.f)))
				Start_ValtanSequencePreview(
					pModel, static_cast<std::size_t>(Index));
			if (bPlayingThis)
				ImGui::PopStyleColor();
			if (ImGui::IsItemHovered())
			{
				std::string clipList;
				for (const std::string& clip : Seq.clips)
				{
					if (!clipList.empty())
						clipList += "\n";
					clipList += clip;
				}
				ImGui::SetTooltip("skill %d  seq %d\n%s",
					Seq.iSkillId, Seq.iSeqIndex, clipList.c_str());
			}
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::End();
}

bool_t Client::CAnimation_Tool::Start_ValtanSequencePreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::size_t iSequenceIndex)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because the animation target changed.";
		return false;
	}
	if (iSequenceIndex >= m_ClipSeqs.size())
		return false;
	const CLIP_SEQ& Seq = m_ClipSeqs[iSequenceIndex];
	if (Seq.clips.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because the sequence has no clips.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	staged.reserve(Seq.clips.size());
	const uint32_t iStepCount = static_cast<uint32_t>(Seq.clips.size());
	std::string label = Seq.name.empty() ?
		std::to_string(Seq.iSkillId) : Seq.name;
	label += " seq" + std::to_string(Seq.iSeqIndex) +
		" [" + Seq.sMode + "]";
	const bool_t bHasCuts = Seq.cuts.size() == Seq.clips.size();
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM item;
		item.strPatternLabel = label;
		item.iSourceActionId = static_cast<uint32_t>(Seq.iSkillId);
		item.iSequenceIndex = Seq.iSeqIndex;
		item.iSequenceRepeatNumber = 1u;
		item.iSequenceRepeatCount = 1u;
		item.iSourceStepNumber = iStep + 1u;
		item.iSourceStepCount = iStepCount;
		item.strSequenceName = Seq.name;
		item.strSequenceMode = Seq.sMode;
		item.strClipName = Seq.clips[iStep];
		item.iStepNumber = iStep + 1u;
		item.iStepCount = iStepCount;
		if (bHasCuts && std::isfinite(Seq.cuts[iStep]))
		{
			/* The source stage hands over here; a near-zero cut is a stage
			the original skips through instantly. */
			if (Seq.cuts[iStep] < 0.02f)
				continue;
			item.fAuthoredDurationSeconds = Seq.cuts[iStep];
		}
		else
		{
			const auto length = m_ClipLength.find(item.strClipName);
			if (m_ClipLength.end() != length && length->second > 0.f &&
				std::isfinite(length->second))
			{
				item.fAuthoredDurationSeconds = length->second;
			}
		}
		staged.push_back(std::move(item));
	}
	if (staged.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Sequence preview start rejected because every step is skipped.";
		return false;
	}
	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the read-only source reference.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = static_cast<int32_t>(iSequenceIndex);
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus =
		"Playing source sequence " + label + ".";
	return Activate_ValtanPatternPreviewItem(pModel);
}

std::filesystem::path Client::CAnimation_Tool::Get_CustomChainFilePath() const
{
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (nullptr == pProfile)
		return std::filesystem::path{};
	return CProjectDataRoot::Resolve(
		std::filesystem::path(L"Valtan") / pProfile->pFileName);
}

bool_t Client::CAnimation_Tool::Read_CustomChainDocument(
	const std::filesystem::path& source,
	std::vector<CUSTOM_CHAIN_ENTRY>& Out,
	std::string& strOutError) const
{
	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, source.c_str(), L"rb") || nullptr == file)
	{
		strOutError = "could not open " + source.filename().string();
		return false;
	}
	std::string text;
	char_t buffer[4096]{};
	size_t read = 0u;
	while (0u < (read = fread(buffer, 1u, sizeof(buffer), file)))
		text.append(buffer, read);
	fclose(file);
	return Parse_CustomChainDocument(text, Out, strOutError);
}

bool_t Client::CAnimation_Tool::Parse_CustomChainDocument(
	const std::string& text,
	std::vector<CUSTOM_CHAIN_ENTRY>& Out,
	std::string& strOutError) const
{
	DATA_JSON_VALUE root;
	std::string error;
	if (!CDataJson::Parse(text, root, error) || !root.Is_Object())
	{
		strOutError = "document is unreadable: " + error;
		return false;
	}
	const DATA_JSON_VALUE* pChains = root.Find("chains");
	if (nullptr == pChains || !pChains->Is_Array())
	{
		strOutError = "document has no chain array";
		return false;
	}

	/* Staged first so a malformed row leaves the caller's library intact. */
	std::vector<CUSTOM_CHAIN_ENTRY> staged;
	for (const DATA_JSON_VALUE& Chain : pChains->Get_Array())
	{
		if (!Chain.Is_Object())
			continue;
		const DATA_JSON_VALUE* pId = Chain.Find("chainId");
		const DATA_JSON_VALUE* pAnimation = Chain.Find("animation");
		if (nullptr == pId || !pId->Is_String() || pId->Get_String().empty() ||
			nullptr == pAnimation || !pAnimation->Is_Object())
		{
			continue;
		}
		const DATA_JSON_VALUE* pOccurrences = pAnimation->Find("occurrences");
		if (nullptr == pOccurrences || !pOccurrences->Is_Array())
			continue;

		CUSTOM_CHAIN_ENTRY Entry;
		Entry.chainId = pId->Get_String();
		if (const DATA_JSON_VALUE* pPattern = Chain.Find("targetPatternId"))
		{
			if (pPattern->Is_String())
				Entry.targetPatternId = pPattern->Get_String();
		}
		if (const DATA_JSON_VALUE* pStage = Chain.Find("targetStageId"))
		{
			if (pStage->Is_String())
				Entry.targetStageId = pStage->Get_String();
		}
		for (const DATA_JSON_VALUE& Occurrence : pOccurrences->Get_Array())
		{
			if (!Occurrence.Is_Object())
				continue;
			const DATA_JSON_VALUE* pClip = Occurrence.Find("clip");
			const DATA_JSON_VALUE* pPlayMs = Occurrence.Find("playMs");
			if (nullptr == pClip || !pClip->Is_String() ||
				nullptr == pPlayMs || !pPlayMs->Is_Number())
			{
				continue;
			}
			CUSTOM_CHAIN_STEP Step;
			Step.clipName = pClip->Get_String();
			Step.fDurationSeconds =
				static_cast<f32_t>(pPlayMs->Get_Number()) * 0.001f;
			Entry.steps.push_back(std::move(Step));
		}
		if (!Entry.steps.empty())
			staged.push_back(std::move(Entry));
	}

	Out = std::move(staged);
	return true;
}

bool_t Client::CAnimation_Tool::Load_CustomChainLibrary()
{
	const std::filesystem::path source = Get_CustomChainFilePath();
	if (source.empty())
	{
		m_CustomChainLibrary.clear();
		m_strCustomChainStatus =
			"Custom chains are not admitted for this target.";
		return false;
	}
	std::error_code existsError;
	if (!std::filesystem::exists(source, existsError) || existsError)
	{
		/* Nothing saved yet is the normal first state, not a failure. */
		m_CustomChainLibrary.clear();
		m_strCustomChainStatus.clear();
		return true;
	}

	std::vector<CUSTOM_CHAIN_ENTRY> staged;
	std::string error;
	if (!Read_CustomChainDocument(source, staged, error))
	{
		m_strCustomChainStatus =
			"Debug chain file rejected; saved chains preserved: " + error;
		return false;
	}

	m_CustomChainLibrary = std::move(staged);
	m_strCustomChainStatus = "Loaded " +
		std::to_string(m_CustomChainLibrary.size()) + " saved chains.";
	return true;
}

bool_t Client::CAnimation_Tool::Save_CustomChainLibrary()
{
	const std::filesystem::path destination = Get_CustomChainFilePath();
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (destination.empty() || nullptr == pProfile)
	{
		m_strCustomChainStatus =
			"Save rejected: custom chains are not admitted for this target.";
		return false;
	}
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_strCustomChainStatus =
			"Save failed to create the Data directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, temporary.c_str(), L"wb") || nullptr == file)
	{
		m_strCustomChainStatus =
			"Save failed to open a temporary file; previous chains preserved.";
		return false;
	}

	bool_t bWritten = 0 <= fprintf(file,
		"{\n"
		"  \"schema\": \"lostark.valtan-pattern-presentation-debug\",\n"
		"  \"formatVersion\": 1,\n"
		"  \"bossArchetypeId\": \"BOSS_VALTAN\",\n"
		"  \"encounterId\": \"ENCOUNTER_VALTAN\",\n"
		"  \"chains\": [\n");
	for (size_t iChain = 0u; iChain < m_CustomChainLibrary.size(); ++iChain)
	{
		const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[iChain];
		/* A step left at zero has no authored wall, so the chain cannot claim
		   it fills a Server stage exactly. The distinction is recorded rather
		   than guessed away, because merging it is a decision, not a rename. */
		bool_t bEveryStepAuthored = true;
		for (const CUSTOM_CHAIN_STEP& Step : Entry.steps)
		{
			if (Step.fDurationSeconds <= 0.f)
				bEveryStepAuthored = false;
		}
		bWritten = bWritten && 0 <= fprintf(file,
			"    {\n"
			"      \"chainId\": \"%s\",\n"
			"      \"targetPatternId\": \"%s\",\n"
			"      \"targetStageId\": \"%s\",\n"
			"      \"animation\": {\n"
			"        \"endPolicy\": \"%s\",\n"
			"        \"repeatCount\": %zu,\n"
			"        \"occurrences\": [\n",
			CDataJson::Escape(Entry.chainId).c_str(),
			CDataJson::Escape(Entry.targetPatternId).c_str(),
			CDataJson::Escape(Entry.targetStageId).c_str(),
			bEveryStepAuthored ? "EXACT" : "NATIVE_CLIP_LENGTHS",
			Entry.steps.size());
		for (size_t iStep = 0u; iStep < Entry.steps.size(); ++iStep)
		{
			const CUSTOM_CHAIN_STEP& Step = Entry.steps[iStep];
			const int32_t iPlayMs = static_cast<int32_t>(
				std::lround(Step.fDurationSeconds * 1000.f));
			bWritten = bWritten && 0 <= fprintf(file,
				"          {\n"
				"            \"clipOccurrenceId\": \"%s.%s.clip.%02zu\",\n"
				"            \"clip\": \"%s\",\n"
				"            \"mappingBasis\": \"PROJECT_AUTHORED\",\n"
				"            \"sourceStartMs\": 0,\n"
				"            \"playMs\": %d,\n"
				"            \"playRate\": 1.0,\n"
				"            \"repeatUntilStageEnd\": false\n"
				"          }%s\n",
				pProfile->pOccurrencePrefix,
				CDataJson::Escape(Entry.chainId).c_str(),
				iStep + 1u,
				CDataJson::Escape(Step.clipName).c_str(),
				iPlayMs < 0 ? 0 : iPlayMs,
				iStep + 1u == Entry.steps.size() ? "" : ",");
		}
		bWritten = bWritten && 0 <= fprintf(file,
			"        ]\n"
			"      }\n"
			"    }%s\n",
			iChain + 1u == m_CustomChainLibrary.size() ? "" : ",");
	}
	bWritten = bWritten && 0 <= fprintf(file, "  ]\n}\n");
	const bool_t bClosed = 0 == fclose(file);

	if (!bWritten || !bClosed)
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_strCustomChainStatus =
			"Save failed while writing; previous chains preserved.";
		return false;
	}

	std::error_code replaceError;
	std::filesystem::rename(temporary, destination, replaceError);
	if (replaceError)
	{
		std::filesystem::remove(destination, replaceError);
		std::filesystem::rename(temporary, destination, replaceError);
	}
	if (replaceError)
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_strCustomChainStatus = "Save failed to replace the file: " +
			replaceError.message();
		return false;
	}

	m_strCustomChainStatus = "Saved " +
		std::to_string(m_CustomChainLibrary.size()) + " chains to " +
		destination.filename().string() + ".";
	return true;
}

void Client::CAnimation_Tool::Render_ValtanCustomChainWindow(
	const shared_ptr<Engine::CModel>& pModel)
{
	const CUSTOM_CHAIN_PROFILE* pProfile =
		Find_CustomChainProfile(m_AssetName);
	if (nullptr == pProfile)
	{
		m_bShowValtanCustomChainWindow = false;
		return;
	}

	if (!ImGui::Begin(
		pProfile->pWindowTitle, &m_bShowValtanCustomChainWindow))
	{
		ImGui::End();
		return;
	}

	ImGui::TextWrapped(
		"Assemble a chain by hand out of the model's own clips. Seconds carry "
		"the same meaning the source cuts do: 0 plays the clip's native length, "
		"a shorter value cuts it, a longer one loops it until the step is "
		"filled. Save Animation Intake writes the review source only; it does "
		"not silently replace Server gameplay or Product presentation.");

	const bool_t bBusy =
		m_bValtanPatternPreviewPlaying || m_bValtanPatternMasterPlaying;

	ImGui::BeginDisabled(bBusy || m_CustomChainSteps.empty());
	if (ImGui::Button("Play Chain"))
	{
		/* The transport that owns step, progress, pause and speed already
		   lives in the source reference window, so playback opens it. */
		if (Start_ValtanCustomChainPreview(pModel))
			m_bShowValtanSourceReferenceWindow = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear Steps"))
		m_CustomChainSteps.clear();
	ImGui::EndDisabled();

	ImGui::SetNextItemWidth(160.f);
	ImGui::SliderFloat(
		"Blend##customchainblend", &m_fPreviewBlendSeconds, 0.f, 0.5f, "%.3f s");
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(
			"Preview-only cross-fade when a step changes clip. 0 matches the "
			"product Valtan, which does not blend yet; 0.12 matches the value "
			"CCharacter already uses.");
	}

	if (!m_bCustomChainLibraryLoadAttempted)
	{
		m_bCustomChainLibraryLoadAttempted = true;
		Load_CustomChainLibrary();
	}

	ImGui::SeparatorText("Saved Animation Intake");
	ImGui::TextDisabled("%s", pProfile->pFileLabel);
	ImGui::BeginDisabled(bBusy);
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputTextWithHint("##chainid", "chain name",
		m_CustomChainId, sizeof(m_CustomChainId));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(160.f);
	ImGui::InputTextWithHint("##chaintargetpattern", "target patternId",
		m_CustomChainTargetPatternId, sizeof(m_CustomChainTargetPatternId));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(120.f);
	ImGui::InputTextWithHint("##chaintargetstage", "target stageId",
		m_CustomChainTargetStageId, sizeof(m_CustomChainTargetStageId));

	if (ImGui::Button("Save Animation Intake"))
	{
		if ('\0' == m_CustomChainId[0])
		{
			m_strCustomChainStatus = "Save rejected: name the chain first.";
		}
		else if (m_CustomChainSteps.empty())
		{
			m_strCustomChainStatus = "Save rejected: the chain owns no step.";
		}
		else
		{
			CUSTOM_CHAIN_ENTRY Entry;
			Entry.chainId = m_CustomChainId;
			Entry.targetPatternId = m_CustomChainTargetPatternId;
			Entry.targetStageId = m_CustomChainTargetStageId;
			Entry.steps = m_CustomChainSteps;
			/* Same name replaces in place so re-saving a chain being tuned
			   does not grow a pile of near-identical entries. */
			const std::vector<CUSTOM_CHAIN_ENTRY> Previous =
				m_CustomChainLibrary;
			bool_t bReplaced = false;
			for (CUSTOM_CHAIN_ENTRY& Existing : m_CustomChainLibrary)
			{
				if (Existing.chainId != Entry.chainId)
					continue;
				Existing = Entry;
				bReplaced = true;
				break;
			}
			if (!bReplaced)
				m_CustomChainLibrary.push_back(std::move(Entry));
			if (!Save_CustomChainLibrary())
				m_CustomChainLibrary = Previous;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload File"))
	{
		Load_CustomChainLibrary();
	}
	ImGui::EndDisabled();

	if (!m_strCustomChainStatus.empty())
		ImGui::TextWrapped("%s", m_strCustomChainStatus.c_str());

	if (!m_CustomChainLibrary.empty())
	{
		size_t iDeleteEntry = m_CustomChainLibrary.size();
		ImGui::BeginDisabled(bBusy);
		for (size_t iEntry = 0u; iEntry < m_CustomChainLibrary.size(); ++iEntry)
		{
			const CUSTOM_CHAIN_ENTRY& Entry = m_CustomChainLibrary[iEntry];
			ImGui::PushID(static_cast<int32_t>(iEntry) + 20000);
			if (ImGui::SmallButton("Load"))
			{
				m_CustomChainSteps = Entry.steps;
				snprintf(m_CustomChainId, sizeof(m_CustomChainId), "%s",
					Entry.chainId.c_str());
				snprintf(m_CustomChainTargetPatternId,
					sizeof(m_CustomChainTargetPatternId), "%s",
					Entry.targetPatternId.c_str());
				snprintf(m_CustomChainTargetStageId,
					sizeof(m_CustomChainTargetStageId), "%s",
					Entry.targetStageId.c_str());
				m_strCustomChainStatus =
					"Loaded chain " + Entry.chainId + " into the steps.";
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Delete"))
				iDeleteEntry = iEntry;
			ImGui::SameLine();
			ImGui::Text("%s  |  %s / %s  |  %zu steps",
				Entry.chainId.c_str(),
				Entry.targetPatternId.empty() ? "-" :
					Entry.targetPatternId.c_str(),
				Entry.targetStageId.empty() ? "-" :
					Entry.targetStageId.c_str(),
				Entry.steps.size());
			ImGui::PopID();
		}
		ImGui::EndDisabled();
		if (iDeleteEntry < m_CustomChainLibrary.size())
		{
			const std::vector<CUSTOM_CHAIN_ENTRY> Previous =
				m_CustomChainLibrary;
			m_CustomChainLibrary.erase(
				m_CustomChainLibrary.begin() +
				static_cast<std::ptrdiff_t>(iDeleteEntry));
			if (!Save_CustomChainLibrary())
				m_CustomChainLibrary = Previous;
		}
	}

	ImGui::SeparatorText("Steps");
	if (m_CustomChainSteps.empty())
	{
		ImGui::TextDisabled("No step yet. Add clips from the list below.");
	}
	else
	{
		f32_t fTotalSeconds = 0.f;
		size_t iRemove = m_CustomChainSteps.size();
		size_t iMoveUp = m_CustomChainSteps.size();
		size_t iMoveDown = m_CustomChainSteps.size();
		ImGui::BeginDisabled(bBusy);
		for (size_t iStep = 0u; iStep < m_CustomChainSteps.size(); ++iStep)
		{
			CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
			ImGui::PushID(static_cast<int32_t>(iStep));
			ImGui::Text("%2zu", iStep + 1u);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80.f);
			ImGui::InputFloat("##seconds", &Step.fDurationSeconds, 0.f, 0.f, "%.3f");
			if (!std::isfinite(Step.fDurationSeconds) ||
				Step.fDurationSeconds < 0.f)
			{
				Step.fDurationSeconds = 0.f;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("^"))
				iMoveUp = iStep;
			ImGui::SameLine();
			if (ImGui::SmallButton("v"))
				iMoveDown = iStep;
			ImGui::SameLine();
			if (ImGui::SmallButton("x"))
				iRemove = iStep;
			ImGui::SameLine();
			ImGui::TextUnformatted(Step.clipName.c_str());
			fTotalSeconds += Step.fDurationSeconds;
			ImGui::PopID();
		}
		ImGui::EndDisabled();
		ImGui::Text(
			"%zu steps, %.3f s of authored length (steps left at 0 add their "
			"native length on top).",
			m_CustomChainSteps.size(), fTotalSeconds);

		/* The list is edited after the row loop so the vector never moves while
		   ImGui is still drawing from it. */
		if (iRemove < m_CustomChainSteps.size())
		{
			m_CustomChainSteps.erase(
				m_CustomChainSteps.begin() +
				static_cast<std::ptrdiff_t>(iRemove));
		}
		else if (iMoveUp > 0u && iMoveUp < m_CustomChainSteps.size())
		{
			std::swap(
				m_CustomChainSteps[iMoveUp - 1u], m_CustomChainSteps[iMoveUp]);
		}
		else if (iMoveDown + 1u < m_CustomChainSteps.size())
		{
			std::swap(
				m_CustomChainSteps[iMoveDown], m_CustomChainSteps[iMoveDown + 1u]);
		}
	}

	ImGui::SeparatorText("Clips");
	ImGui::TextDisabled(
		"Only %s* clips are listed: both Valtan bodies share that vocabulary, "
		"so a chain authored here plays on either.",
		CUSTOM_CHAIN_CLIP_PREFIX);
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##customchainfilter",
		"filter by clip name",
		m_CustomChainFilter,
		sizeof(m_CustomChainFilter));

	ImGui::BeginDisabled(bBusy);
	if (ImGui::BeginChild(
		"##customchainclips",
		ImVec2(0.f, 260.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		for (uint32_t iAnimation = 0u;
			iAnimation < pModel->Get_NumAnimations(); ++iAnimation)
		{
			const char_t* pName = pModel->Get_AnimationName(iAnimation);
			if (nullptr == pName ||
				!Contains_NoCase(pName, m_CustomChainFilter))
			{
				continue;
			}
			/* The ghost body carries its own rpbf_02.ao_* clips on top of the
			   shared vocabulary. They play, but the product body has no clip of
			   that name, so authoring one into this shared document would break
			   the promotion that pins the product model. */
			if (0 != std::strncmp(
				pName,
				CUSTOM_CHAIN_CLIP_PREFIX,
				std::strlen(CUSTOM_CHAIN_CLIP_PREFIX)))
			{
				continue;
			}
			ImGui::PushID(static_cast<int32_t>(iAnimation));
			if (ImGui::SmallButton("+"))
			{
				CUSTOM_CHAIN_STEP Step;
				Step.clipName = pName;
				m_CustomChainSteps.push_back(std::move(Step));
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(pName);
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::EndDisabled();

	ImGui::End();
}

bool_t Client::CAnimation_Tool::Start_ValtanCustomChainPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Custom chain start rejected because the animation target changed.";
		return false;
	}
	if (m_CustomChainSteps.empty())
	{
		m_strValtanPatternPreviewStatus =
			"Custom chain start rejected because it owns no step.";
		return false;
	}

	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	staged.reserve(m_CustomChainSteps.size());
	const uint32_t iStepCount =
		static_cast<uint32_t>(m_CustomChainSteps.size());
	for (uint32_t iStep = 0u; iStep < iStepCount; ++iStep)
	{
		const CUSTOM_CHAIN_STEP& Step = m_CustomChainSteps[iStep];
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM item;
		item.strPatternLabel = "Custom chain";
		item.iSequenceIndex = -1;
		item.iSequenceRepeatNumber = 1u;
		item.iSequenceRepeatCount = 1u;
		item.iSourceStepNumber = iStep + 1u;
		item.iSourceStepCount = iStepCount;
		item.strSequenceName = "Custom chain";
		item.strSequenceMode = "CUSTOM";
		item.strClipName = Step.clipName;
		item.iStepNumber = iStep + 1u;
		item.iStepCount = iStepCount;
		/* A non-positive or non-finite entry means the animator has not chosen
		   a length yet, which the playlist already reads as the native one. */
		item.fAuthoredDurationSeconds =
			std::isfinite(Step.fDurationSeconds) && Step.fDurationSeconds > 0.f ?
				Step.fDurationSeconds : 0.f;
		staged.push_back(std::move(item));
	}

	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the custom chain.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = -1;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus = "Playing the custom chain: " +
		std::to_string(iStepCount) + " steps.";
	return Activate_ValtanPatternPreviewItem(pModel);
}

bool_t Client::CAnimation_Tool::Start_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const uint32_t iFirstPattern,
	const uint32_t iLastPattern)
{
	if (nullptr == pModel || CAnimationTargetService::Resolve_Model() != pModel)
	{
		m_strValtanPatternPreviewStatus =
			"Pattern preview start rejected because the animation target changed.";
		return false;
	}
	std::vector<VALTAN_PATTERN_PREVIEW_PLAY_ITEM> staged;
	std::string status;
	if (!CValtanPatternPreviewDocument::Build_Playlist(
			m_ValtanPatternPreviewDocument,
			iFirstPattern,
			iLastPattern,
			staged,
			status))
	{
		m_strValtanPatternPreviewStatus =
			"Pattern preview start rejected; current pose preserved: " + status;
		return false;
	}
	if (m_bValtanPatternMasterPlaying)
	{
		Reset_ValtanPatternMasterPreviewState(
			"Valtan Pattern Master yielded to the read-only source reference.");
	}

	m_ValtanPatternPreviewPlaylist = std::move(staged);
	m_iValtanPatternPreviewItem = 0u;
	m_bValtanPatternPreviewPlaying = true;
	m_bValtanPatternPreviewPaused = false;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_ValtanPatternPreviewModel = pModel;
	m_iValtanPatternPreviewTargetGeneration =
		CAnimationTargetService::Resolve_TargetGeneration();
	m_iValtanSequenceSelected = -1;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
	m_strValtanPatternPreviewStatus = status;
	return Activate_ValtanPatternPreviewItem(pModel);
}

bool_t Client::CAnimation_Tool::Activate_ValtanPatternPreviewItem(
	const shared_ptr<Engine::CModel>& pModel)
{
	constexpr f32_t PATTERN_MARKER_DURATION_SECONDS = 0.45f;
	std::string skippedStatus;
	const auto RecordSkip = [&skippedStatus](const std::string& reason)
	{
		if (!skippedStatus.empty())
			skippedStatus += " ";
		skippedStatus += reason;
	};

	while (m_bValtanPatternPreviewPlaying &&
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size())
	{
		const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
			m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
		m_fValtanPatternPreviewElapsedSeconds = 0.f;
		m_fValtanPatternPreviewItemDurationSeconds = 0.f;
		m_bValtanPatternPreviewPaused = false;

		if (Item.bPatternMarker)
		{
			if (!pModel->Start_Animation("mesh_idle_battle_1", true))
			{
				RecordSkip(
					"Skipped pattern marker because the idle clip is unavailable.");
				++m_iValtanPatternPreviewItem;
				continue;
			}

			m_bLoop = true;
			m_fValtanPatternPreviewItemDurationSeconds =
				PATTERN_MARKER_DURATION_SECONDS;
			pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
			m_strValtanPatternPreviewStatus = skippedStatus;
			if (!m_strValtanPatternPreviewStatus.empty())
				m_strValtanPatternPreviewStatus += " ";
			m_strValtanPatternPreviewStatus +=
				"Pattern " + std::to_string(Item.iPatternNumber) +
				" step " + std::to_string(Item.iStepNumber) + "/" +
				std::to_string(Item.iStepCount) +
				" marker started for 450 ms.";
			return true;
		}

		if (!Start_PreviewClip(
			pModel, Item.strClipName.c_str(), false, m_fPreviewBlendSeconds))
		{
			RecordSkip(
				"Skipped unavailable clip without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		const uint32_t AnimationIndex = pModel->Get_CurrentAnimIndex();
		f32_t Position = 0.f;
		f32_t DurationTicks = 0.f;
		const f32_t TickRate =
			pModel->Get_AnimationTickPerSecond(AnimationIndex);
		if (!std::isfinite(TickRate) || TickRate <= 0.f ||
			!pModel->Get_AnimationProgress(
				AnimationIndex, Position, DurationTicks) ||
			!std::isfinite(DurationTicks) || DurationTicks <= 0.f)
		{
			RecordSkip(
				"Skipped clip with invalid native duration without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		const f32_t NativeDurationSeconds = DurationTicks / TickRate;
		if (!std::isfinite(NativeDurationSeconds) ||
			NativeDurationSeconds <= 0.f)
		{
			RecordSkip(
				"Skipped clip with invalid native duration without stopping Play All: " +
				Item.strClipName + ".");
			++m_iValtanPatternPreviewItem;
			continue;
		}

		f32_t fItemDurationSeconds = NativeDurationSeconds;
		bool_t bLoopClip = false;
		std::string durationNote = " started for its full native duration (";
		if (Item.fAuthoredDurationSeconds > 0.f)
		{
			fItemDurationSeconds = Item.fAuthoredDurationSeconds;
			bLoopClip =
				Item.fAuthoredDurationSeconds > NativeDurationSeconds + 0.001f;
			durationNote = bLoopClip ?
				" started looping for its source stage length (" :
				" started for its source stage length (";
			if (bLoopClip &&
				!Start_PreviewClip(
					pModel, Item.strClipName.c_str(), true,
					m_fPreviewBlendSeconds))
			{
				RecordSkip(
					"Skipped unavailable clip without stopping Play All: " +
					Item.strClipName + ".");
				++m_iValtanPatternPreviewItem;
				continue;
			}
		}

		m_bLoop = bLoopClip;
		m_fValtanPatternPreviewItemDurationSeconds = fItemDurationSeconds;
		pModel->Set_AnimationSpeed(m_fValtanPatternPreviewSpeed);
		m_strValtanPatternPreviewStatus = skippedStatus;
		if (!m_strValtanPatternPreviewStatus.empty())
			m_strValtanPatternPreviewStatus += " ";
		m_strValtanPatternPreviewStatus +=
			"Pattern " + std::to_string(Item.iPatternNumber) +
			" step " + std::to_string(Item.iStepNumber) + "/" +
			std::to_string(Item.iStepCount) +
			durationNote +
			std::to_string(fItemDurationSeconds) + " s).";
		return true;
	}

	if (!skippedStatus.empty())
		skippedStatus += " ";
	Stop_ValtanPatternPreview(
		pModel, skippedStatus + "Pattern preview completed; idle restored.");
	return false;
}

void Client::CAnimation_Tool::Advance_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel)
{
	if (!m_bValtanPatternPreviewPlaying)
		return;
	const bool_t bHadItem =
		m_iValtanPatternPreviewItem < m_ValtanPatternPreviewPlaylist.size();
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM Finished = bHadItem ?
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem] :
		VALTAN_PATTERN_PREVIEW_PLAY_ITEM{};
	const f32_t fFinishedDurationSeconds =
		m_fValtanPatternPreviewItemDurationSeconds;
	++m_iValtanPatternPreviewItem;
	if (m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		Stop_ValtanPatternPreview(
			pModel, "Pattern preview completed; idle restored.");
		return;
	}
	Activate_ValtanPatternPreviewItem(pModel);
	/* The pattern hit clock keeps counting only while the next started item
	   continues the same source sequence pass; anything else restarts it. */
	if (!m_bValtanPatternPreviewPlaying ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		m_fValtanPatternHitTimelineBaseSeconds = 0.f;
		return;
	}
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Current =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	const bool_t bSameTimeline = bHadItem &&
		!Finished.bPatternMarker && !Current.bPatternMarker &&
		Finished.iPatternNumber == Current.iPatternNumber &&
		Finished.iSourceActionId == Current.iSourceActionId &&
		Finished.iSequenceIndex == Current.iSequenceIndex &&
		Finished.iSequenceRepeatNumber == Current.iSequenceRepeatNumber;
	if (bSameTimeline)
		m_fValtanPatternHitTimelineBaseSeconds += fFinishedDurationSeconds;
	else
		m_fValtanPatternHitTimelineBaseSeconds = 0.f;
}

void Client::CAnimation_Tool::Stop_ValtanPatternPreview(
	const shared_ptr<Engine::CModel>& pModel,
	const std::string& status)
{
	if (nullptr != pModel)
	{
		m_bLoop = true;
		pModel->Set_AnimationSpeed(1.f);
		/* Each authoring body names its own idle; an unknown target keeps the
		   product clip so the existing Valtan paths are unchanged. */
		const CUSTOM_CHAIN_PROFILE* pProfile =
			Find_CustomChainProfile(m_AssetName);
		const char_t* pIdleClip = nullptr != pProfile ?
			pProfile->pIdleClip : "mesh_idle_battle_1";
		if (!pModel->Start_Animation(pIdleClip, true))
			pModel->Set_AnimPaused(true);
	}
	Reset_ValtanPatternPreviewState(status);
}

void Client::CAnimation_Tool::Reset_ValtanPatternPreviewState(
	const std::string& status)
{
	m_bValtanPatternPreviewPlaying = false;
	m_bValtanPatternPreviewPaused = false;
	m_iValtanPatternPreviewItem = 0u;
	m_fValtanPatternPreviewElapsedSeconds = 0.f;
	m_fValtanPatternPreviewItemDurationSeconds = 0.f;
	m_ValtanPatternPreviewPlaylist.clear();
	m_strValtanPatternPreviewStatus = status;
	m_bLoop = true;
	m_ValtanPatternPreviewModel.reset();
	m_iValtanPatternPreviewTargetGeneration = 0u;
	m_fValtanPatternHitTimelineBaseSeconds = 0.f;
#ifdef _DEBUG
	if (const shared_ptr<CValtan> Boss =
		CAnimationTargetService::Resolve_Boss())
	{
		Boss->Clear_PatternHitAreaPreview();
	}
#endif
}

void Client::CAnimation_Tool::Update_ValtanPatternHitAreaPreview()
{
#ifdef _DEBUG
	const shared_ptr<CValtan> Boss = CAnimationTargetService::Resolve_Boss();
	if (nullptr == Boss)
		return;
	if (!m_bValtanPatternPreviewPlaying ||
		m_iValtanPatternPreviewItem >= m_ValtanPatternPreviewPlaylist.size())
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	const VALTAN_PATTERN_PREVIEW_PLAY_ITEM& Item =
		m_ValtanPatternPreviewPlaylist[m_iValtanPatternPreviewItem];
	if (Item.bPatternMarker || 0u == Item.iSourceActionId)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}

	if (!m_bValtanEncounterReferenceLoadAttempted)
	{
		m_bValtanEncounterReferenceLoadAttempted = true;
		std::string status;
		if (!m_ValtanEncounterReference.Load(CProjectDataRoot::Resolve(
				std::filesystem::path(L"Encounters") / L"Valtan" /
				L"ValtanEncounter.json"), status))
		{
			m_ValtanEncounterReference.Clear();
		}
	}
	if (!m_ValtanEncounterReference.Is_Ready())
		return;

	const ENCOUNTER_PATTERN_REFERENCE* pPattern = nullptr;
	for (const ENCOUNTER_PATTERN_REFERENCE& pattern :
		m_ValtanEncounterReference.Get_Patterns())
	{
		if (std::find(pattern.sourceActionIds.begin(),
				pattern.sourceActionIds.end(),
				Item.iSourceActionId) != pattern.sourceActionIds.end())
		{
			pPattern = &pattern;
			break;
		}
	}
	if (nullptr == pPattern)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}

	/* The stage lookup runs on the whole-pattern clock: seconds of finished
	   same-sequence items plus the current item's local elapsed. */
	const f32_t fTimelineMs = (m_fValtanPatternHitTimelineBaseSeconds +
		m_fValtanPatternPreviewElapsedSeconds) * 1000.f;
	const ENCOUNTER_STAGE_REFERENCE* pStage = nullptr;
	for (const ENCOUNTER_STAGE_REFERENCE& stage : pPattern->stages)
	{
		const f32_t fStartMs = static_cast<f32_t>(stage.iStartOffsetMs);
		if (fTimelineMs >= fStartMs &&
			fTimelineMs < fStartMs + static_cast<f32_t>(stage.iDurationMs))
		{
			pStage = &stage;
			break;
		}
	}
	if (nullptr == pStage)
	{
		Boss->Clear_PatternHitAreaPreview();
		return;
	}
	Boss->Set_PatternHitAreaPreview(
		pStage->actionId,
		(fTimelineMs - static_cast<f32_t>(pStage->iStartOffsetMs)) / 1000.f);
#endif
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

Client::ANIMATION_SKILL_BINDING*
Client::CAnimation_Tool::Find_SkillBinding(
	const LostArk::Shared::SKILL_ID skillId)
{
	for (ANIMATION_SKILL_BINDING& binding :
		m_SkillBindingDocument.Bindings)
	{
		if (binding.iSkillId == skillId)
			return &binding;
	}
	return nullptr;
}

bool_t Client::CAnimation_Tool::Load_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	std::string status;
	if (!CAnimationSkillBindingDocument::Load(
		m_AssetName,
		characterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		staged,
		status))
	{
		m_SkillBindingStatus =
			"Load rejected; current Skill Bindings preserved: " + status;
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = -1;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = false;
	m_SkillBindingStatus = status;
	return true;
}

bool_t Client::CAnimation_Tool::Save_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		m_SkillBindingStatus =
			"Skill Bindings can only be saved for the selected Scene Character.";
		return false;
	}

	std::string status;
	if (!CAnimationSkillBindingDocument::Save_Atomic(
		m_SkillBindingDocument,
		m_AssetName,
		pCharacter->Get_Spec()->eCharacterClass,
		CPlayerSkillCatalog::Get_Skills(),
		Collect_ClipNames(pModel),
		status))
	{
		m_SkillBindingStatus =
			"Save rejected; destination and current bindings preserved: " + status;
		return false;
	}

	m_bSkillBindingDirty = false;
	if (!pCharacter->Reload_SkillAnimationBindings())
	{
		m_SkillBindingStatus = status +
			" [saved, but the live Character kept its previous binding set]";
		return true;
	}
	m_SkillBindingStatus = status + " [live Character refreshed]";
	return true;
}

bool_t Client::CAnimation_Tool::Create_SkillBindingDraft(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (nullptr == pModel || m_AssetName.empty())
		return false;
	const char_t* currentClip = pModel->Get_AnimationName(
		pModel->Get_CurrentAnimIndex());
	if (nullptr == currentClip)
	{
		m_SkillBindingStatus =
			"Select one model clip before creating a repair draft.";
		return false;
	}

	ANIMATION_SKILL_BINDING_DOCUMENT staged;
	staged.strAnimationAssetId = m_AssetName;
	staged.eCharacterClass = characterClass;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass != characterClass)
			continue;
		ANIMATION_SKILL_BINDING binding;
		binding.iSkillId = definition.iSkillId;
		const bool_t isStaged =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
				definition.eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition.eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition.eSkillKind;
		const std::size_t stageCount =
			isStaged ? definition.iComboStageCount : 1u;
		ANIMATION_SKILL_STAGE seedStage;
		seedStage.Clips.assign(
			1u, ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
		binding.Stages.assign(stageCount, seedStage);
		staged.Bindings.push_back(std::move(binding));
	}
	if (staged.Bindings.empty())
	{
		m_SkillBindingStatus =
			"PlayerSkills has no definitions for this Character class.";
		return false;
	}

	m_SkillBindingDocument = std::move(staged);
	m_iSelectedSkillBinding = 0;
	m_iSelectedSkillClip = 0;
	m_bSkillBindingDirty = true;
	m_SkillBindingStatus =
		"Created a complete repair draft. Every row currently uses " +
		std::string(currentClip) +
		"; assign intended clips before Save.";
	return true;
}

void Client::CAnimation_Tool::Render_SkillBindingReloadConfirmation(
	const shared_ptr<Engine::CModel>& pModel,
	const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
{
	if (m_bSkillBindingReloadConfirmationRequested)
	{
		ImGui::OpenPopup("Discard unsaved Skill Animation Bindings?");
		m_bSkillBindingReloadConfirmationRequested = false;
	}
	if (!ImGui::BeginPopupModal(
		"Discard unsaved Skill Animation Bindings?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	ImGui::TextUnformatted(
		"Reload replaces only the unsaved key/skill animation binding document.");
	if (ImGui::Button("Discard Bindings and Reload"))
	{
		if (Load_SkillBindings(pModel, characterClass))
			ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();
	ImGui::EndPopup();
}

void Client::CAnimation_Tool::Render_SkillBindings(
	const shared_ptr<Engine::CModel>& pModel,
	const shared_ptr<CCharacter>& pCharacter)
{
	ImGui::SeparatorText("Key -> Skill Animation");
	if (nullptr == pCharacter || nullptr == pCharacter->Get_Spec() ||
		nullptr == pCharacter->Get_Spec()->pAssetName ||
		m_AssetName != pCharacter->Get_Spec()->pAssetName)
	{
		ImGui::TextDisabled(
			"Select Scene Character to author gameplay key bindings."
			" Reference-only preview assets are not playable classes.");
		return;
	}

	const CHARACTER_SPEC* spec = pCharacter->Get_Spec();
	if (!m_bSkillBindingLoadAttempted)
	{
		m_bSkillBindingLoadAttempted = true;
		Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_SkillBindingDocument.Bindings.empty())
	{
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());
		ImGui::TextWrapped(
			"The Character remains available even when this document is missing or "
			"invalid. Create a complete in-memory repair draft from the currently "
			"selected model clip, then assign each skill and Save.");
		if (ImGui::Button("Create Repair Draft from Current Clip"))
			Create_SkillBindingDraft(pModel, spec->eCharacterClass);
		return;
	}

	ImGui::TextWrapped(
		"PlayerSkills owns key -> skillId and Server timing. This panel saves only "
		"the approved skillId -> ordered presentation clips. BA1/BA2/... are "
		"indexed directly by the replicated comboStage.");
	if (ImGui::Button("Save Skill Bindings"))
		Save_SkillBindings(pModel, pCharacter);
	ImGui::SameLine();
	if (ImGui::Button("Reload Skill Bindings"))
	{
		if (m_bSkillBindingDirty)
			m_bSkillBindingReloadConfirmationRequested = true;
		else
			Load_SkillBindings(pModel, spec->eCharacterClass);
	}
	if (m_bSkillBindingDirty)
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("*");
	}
	Render_SkillBindingReloadConfirmation(pModel, spec->eCharacterClass);
	if (!m_SkillBindingStatus.empty())
		ImGui::TextWrapped("%s", m_SkillBindingStatus.c_str());

	const uint32_t currentAnimationIndex = pModel->Get_CurrentAnimIndex();
	const char_t* currentClip =
		pModel->Get_AnimationName(currentAnimationIndex);
	ImGui::TextDisabled(
		"Current clip: %s",
		nullptr != currentClip ? currentClip : "(none)");

	std::vector<const PLAYER_SKILL_DEFINITION*> classSkills;
	for (const PLAYER_SKILL_DEFINITION& definition :
		CPlayerSkillCatalog::Get_Skills())
	{
		if (definition.eCharacterClass == spec->eCharacterClass)
			classSkills.push_back(&definition);
	}
	const auto slotRank = [](const std::string& slot)
	{
		constexpr const char_t* preferred[] =
		{
			"Q", "W", "E", "R", "A", "S", "D", "F",
			"T", "V", "ALT_V", "LMB"
		};
		for (int32_t index = 0; index < static_cast<int32_t>(std::size(preferred)); ++index)
		{
			if (slot == preferred[index])
				return index;
		}
		return static_cast<int32_t>(std::size(preferred));
	};
	std::sort(classSkills.begin(), classSkills.end(),
		[&](const PLAYER_SKILL_DEFINITION* left,
			const PLAYER_SKILL_DEFINITION* right)
		{
			const int32_t leftRank = slotRank(left->strInputSlot);
			const int32_t rightRank = slotRank(right->strInputSlot);
			return leftRank != rightRank ? leftRank < rightRank :
				left->strInputSlot < right->strInputSlot;
		});

	if (!ImGui::BeginChild(
		"##skillbindings",
		ImVec2(0.f, 300.f),
		ImGuiChildFlags_Borders,
		ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}
	for (const PLAYER_SKILL_DEFINITION* definition : classSkills)
	{
		const std::string slotLabel =
			"ALT_V" == definition->strInputSlot ? "ALT+V" :
			("LMB" == definition->strInputSlot ? "BA / LMB" :
				definition->strInputSlot);
		ANIMATION_SKILL_BINDING* binding =
			Find_SkillBinding(definition->iSkillId);
		if (nullptr == binding)
		{
			ImGui::TextColored(
				ImVec4(1.f, 0.35f, 0.35f, 1.f),
				"%-8s  %u missing authored binding",
				slotLabel.c_str(),
				definition->iSkillId);
			continue;
		}
		const int32_t bindingIndex = static_cast<int32_t>(
			binding - m_SkillBindingDocument.Bindings.data());
		ImGui::PushID(bindingIndex);
		const bool_t isCombo =
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO ==
			definition->eSkillKind;
		const bool_t isStaged = isCombo ||
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD ==
				definition->eSkillKind ||
			LostArk::Shared::PLAYER_SKILL_KIND::COUNTER ==
				definition->eSkillKind;
		char_t header[192]{};
		snprintf(
			header,
			sizeof(header),
			"%s  %u  %s  [%s]",
			slotLabel.c_str(),
			definition->iSkillId,
			definition->strDisplayName.c_str(),
			isCombo ? "COMBO" : "ACTIVE");
		if (ImGui::TreeNodeEx("##binding", ImGuiTreeNodeFlags_DefaultOpen, "%s", header))
		{
			for (int32_t stageIndex = 0;
				stageIndex < static_cast<int32_t>(binding->Stages.size());
				++stageIndex)
			{
				ImGui::PushID(stageIndex);
				const ANIMATION_SKILL_STAGE& stage = binding->Stages[stageIndex];
				if (isStaged)
					ImGui::TextDisabled("BA%d", stageIndex + 1);
				for (int32_t clipIndex = 0;
					clipIndex < static_cast<int32_t>(stage.Clips.size());
					++clipIndex)
				{
					ImGui::PushID(clipIndex);
					char_t clipLabel[MAX_PATH + 32]{};
					if (isStaged)
					{
						snprintf(
							clipLabel,
							sizeof(clipLabel),
							"  BA%d.%d  %s",
							stageIndex + 1,
							clipIndex + 1,
							stage.Clips[clipIndex].strClipName.c_str());
					}
					else
					{
						snprintf(
							clipLabel,
							sizeof(clipLabel),
							"clip%d  %s",
							clipIndex + 1,
							stage.Clips[clipIndex].strClipName.c_str());
					}
					const bool_t selected =
						m_iSelectedSkillBinding == bindingIndex &&
						m_iSelectedSkillStage == stageIndex &&
						m_iSelectedSkillClip == clipIndex;
					if (ImGui::Selectable(clipLabel, selected))
					{
						m_iSelectedSkillBinding = bindingIndex;
						m_iSelectedSkillStage = stageIndex;
						m_iSelectedSkillClip = clipIndex;
						Select_Clip(pModel, stage.Clips[clipIndex].strClipName);
					}
					ImGui::PopID();
				}
				ImGui::PopID();
			}

			int32_t selectedStage =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillStage : 0;
			if (selectedStage < 0 ||
				selectedStage >= static_cast<int32_t>(binding->Stages.size()))
			{
				selectedStage = 0;
			}
			std::vector<ANIMATION_SKILL_CLIP>& selectedClips =
				binding->Stages[selectedStage].Clips;
			int32_t selectedClip =
				m_iSelectedSkillBinding == bindingIndex ?
				m_iSelectedSkillClip : 0;
			if (selectedClip < 0 ||
				selectedClip >= static_cast<int32_t>(selectedClips.size()))
			{
				selectedClip = 0;
			}
			std::size_t boundClips = 0u;
			for (const ANIMATION_SKILL_STAGE& stage : binding->Stages)
				boundClips += stage.Clips.size();

			ImGui::BeginDisabled(nullptr == currentClip);
			if (ImGui::Button("Assign Current Clip to Selected Step"))
			{
				selectedClips[selectedClip].strClipName = currentClip;
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillStage = selectedStage;
				m_iSelectedSkillClip = selectedClip;
				m_bSkillBindingDirty = true;
				m_SkillBindingStatus =
					"Assigned " + std::string(currentClip) + " to " +
					slotLabel + (isStaged ?
						(" BA" + std::to_string(selectedStage + 1) + "." +
							std::to_string(selectedClip + 1)) :
						(" clip" + std::to_string(selectedClip + 1)));
			}
			ImGui::SameLine();
			/* A stage may hold several clips, so this adds inside the selected
			stage. The stage count itself stays Server-owned. */
			if (ImGui::Button("Add Current Step") && boundClips < 16u)
			{
				selectedClips.insert(
					selectedClips.begin() + selectedClip + 1,
					ANIMATION_SKILL_CLIP{ currentClip, 0u, 1.f });
				m_iSelectedSkillBinding = bindingIndex;
				m_iSelectedSkillStage = selectedStage;
				m_iSelectedSkillClip = selectedClip + 1;
				m_bSkillBindingDirty = true;
			}
			ImGui::EndDisabled();

			if (selectedClips.size() > 1u)
			{
				if (ImGui::SmallButton("Up") && selectedClip > 0)
				{
					std::swap(
						selectedClips[selectedClip],
						selectedClips[selectedClip - 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = selectedClip - 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Down") &&
					selectedClip + 1 < static_cast<int32_t>(selectedClips.size()))
				{
					std::swap(
						selectedClips[selectedClip],
						selectedClips[selectedClip + 1]);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = selectedClip + 1;
					m_bSkillBindingDirty = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove Step"))
				{
					selectedClips.erase(selectedClips.begin() + selectedClip);
					m_iSelectedSkillBinding = bindingIndex;
					m_iSelectedSkillStage = selectedStage;
					m_iSelectedSkillClip = std::min(
						selectedClip,
						static_cast<int32_t>(selectedClips.size()) - 1);
					m_bSkillBindingDirty = true;
				}
			}
			if (isStaged)
			{
				ImGui::TextDisabled(
					"BA stage count is fixed by PlayerSkills comboStages (%zu).",
					definition->iComboStageCount);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
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
		}
	}

	if (0 == iShown)
		ImGui::TextUnformatted("No clip matches the filter.");

	ImGui::EndChild();
}

bool_t Client::CAnimation_Tool::Write_EventsToPath(
	const std::filesystem::path& path,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"wb") || nullptr == file)
	{
		outStatus = "Could not open temporary Animation Events file.";
		return false;
	}

	bool_t writeSucceeded = 0 <= fprintf(
		file,
		"%s %d \"%s\" %d\n",
		EVENT_FILE_MAGIC,
		EVENT_FILE_VERSION,
		m_AssetName.c_str(),
		static_cast<int32_t>(events.size()));

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		const char_t* kindName = Kind_Name(event.eKind);
		if (EVENT_KIND::HIT == event.eKind)
		{
			const HIT_PARAMS& hit = event.hit;
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d rep=%d repms=%d "
				"fz=%d fzin=%d fzout=%d push=%d pushr=%d "
				"area=%d ar=%d aa=%d ah=%d ax=%d arem=%d maxt=%d%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs,
				hit.iRepeatCount, hit.iRepeatMs,
				hit.iFreezeMs, hit.iFreezeInMs, hit.iFreezeOutMs,
				hit.iPushMs, hit.iPushRange,
				hit.iAreaType, hit.iAreaRange, hit.iAreaAngle,
				hit.iAreaHeight, hit.iAreaOffsetX, hit.iAreaInner,
				hit.iMaxTargets,
				event.bImported ? " src=orig" : "");
		}
		else if (Is_Window(event.eKind))
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d endms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.iEndMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
		else if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d endms=%d payload=\"%s\" "
					"effectref=asset anchor=\"%s\" follow=%s orientation=%s stop=%s "
					"px=%.9g py=%.9g pz=%.9g rx=%.9g ry=%.9g rz=%.9g "
					"sx=%.9g sy=%.9g sz=%.9g%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.iEndMs,
					event.sPayload.c_str(), event.sAnchorSlotId.c_str(),
					EFFECT_FOLLOW_POLICY::FOLLOW == event.eFollowPolicy ?
						"follow" : "snapshot",
					EFFECT_ORIENTATION_POLICY::ANCHOR ==
						event.eOrientationPolicy ? "anchor" : "action_facing",
					EFFECT_STOP_POLICY::NATURAL == event.eStopPolicy ?
						"natural" : "cue_end",
					event.EffectLocalTransform.vPosition.x,
					event.EffectLocalTransform.vPosition.y,
					event.EffectLocalTransform.vPosition.z,
					event.EffectLocalTransform.vRotationDegrees.x,
					event.EffectLocalTransform.vRotationDegrees.y,
					event.EffectLocalTransform.vRotationDegrees.z,
					event.EffectLocalTransform.vScale.x,
					event.EffectLocalTransform.vScale.y,
					event.EffectLocalTransform.vScale.z,
					event.bImported ? " src=orig" : "");
			}
			else
			{
				writeSucceeded = writeSucceeded && 0 <= fprintf(
					file,
					"\"%s\" %s startms=%d payload=\"%s\" "
					"effectref=source%s\n",
					event.clipName.c_str(), kindName,
					event.iStartMs, event.sPayload.c_str(),
					event.bImported ? " src=orig" : "");
			}
		}
		else
		{
			writeSucceeded = writeSucceeded && 0 <= fprintf(
				file,
				"\"%s\" %s startms=%d payload=\"%s\"%s\n",
				event.clipName.c_str(), kindName,
				event.iStartMs, event.sPayload.c_str(),
				event.bImported ? " src=orig" : "");
		}
	}

	const bool_t flushSucceeded = 0 == fflush(file);
	const bool_t commitSucceeded =
		flushSucceeded && 0 == _commit(_fileno(file));
	const bool_t closeSucceeded = 0 == fclose(file);
	if (!writeSucceeded || !flushSucceeded ||
		!commitSucceeded || !closeSucceeded)
	{
		outStatus = "Could not flush Animation Events temporary file.";
		return false;
	}

	return true;
}

bool_t Client::CAnimation_Tool::Validate_Events(
	const shared_ptr<Engine::CModel>& pModel,
	const std::vector<ANIM_EVENT>& events,
	std::string& outStatus) const
{
	outStatus.clear();
	if (nullptr == pModel || m_AssetName.empty() ||
		events.size() > static_cast<size_t>(MAX_EVENT_COUNT))
	{
		outStatus = "Animation event document header is invalid.";
		return false;
	}

	for (const ANIM_EVENT& event : events)
	{
		const bool_t bAdmittedEffect =
			EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind;
		bool_t hasClip = false;
		for (uint32_t index = 0;
			index < pModel->Get_NumAnimations();
			++index)
		{
			const char_t* clipName =
				pModel->Get_AnimationName(index);
			if (nullptr != clipName && event.clipName == clipName)
			{
				hasClip = true;
				break;
			}
		}

		if (!hasClip ||
			!Is_SafeQuotedText(event.clipName) ||
			!Is_SafeQuotedText(event.sPayload) ||
			event.iStartMs < 0 ||
			event.iEndMs < event.iStartMs ||
			(!Is_Window(event.eKind) &&
				event.iEndMs != event.iStartMs &&
				!(bAdmittedEffect &&
					EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy)) ||
			ETOI(event.eKind) < 0 ||
			ETOI(event.eKind) >= ETOI(EVENT_KIND::END))
		{
			outStatus =
				"Animation event clip, time, kind, or payload is invalid.";
			return false;
		}

		if (EVENT_KIND::HIT == event.eKind &&
			(event.hit.iRepeatCount < 1 ||
				event.hit.iRepeatCount > 1024 ||
				event.hit.iRepeatMs < 0))
		{
			outStatus = "Animation HIT repeat contract is invalid.";
			return false;
		}

		if (EVENT_KIND::EFFECT == event.eKind)
		{
			if (EFFECT_REFERENCE_KIND::NONE ==
				event.eEffectReferenceKind)
			{
				outStatus =
					"Animation EFFECT row has no reference kind.";
				return false;
			}
			if (EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind)
			{
				const EFFECT_TRANSFORM_DESC& local =
					event.EffectLocalTransform;
				if (!CEffectCatalog::Contains(event.sPayload) ||
					event.sAnchorSlotId.empty() ||
					!Is_SafeQuotedText(event.sAnchorSlotId) ||
					EFFECT_FOLLOW_POLICY::END == event.eFollowPolicy ||
					EFFECT_ORIENTATION_POLICY::END ==
						event.eOrientationPolicy ||
					(EFFECT_ORIENTATION_POLICY::ACTION_FACING ==
						event.eOrientationPolicy &&
						"root" != event.sAnchorSlotId) ||
					EFFECT_STOP_POLICY::END == event.eStopPolicy ||
					!std::isfinite(local.vPosition.x) ||
					!std::isfinite(local.vPosition.y) ||
					!std::isfinite(local.vPosition.z) ||
					!std::isfinite(local.vRotationDegrees.x) ||
					!std::isfinite(local.vRotationDegrees.y) ||
					!std::isfinite(local.vRotationDegrees.z) ||
					!std::isfinite(local.vScale.x) ||
					!std::isfinite(local.vScale.y) ||
					!std::isfinite(local.vScale.z) ||
					local.vScale.x <= 0.f || local.vScale.y <= 0.f ||
					local.vScale.z <= 0.f ||
					(EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
						event.iEndMs <= event.iStartMs))
				{
					outStatus =
						"Admitted Effect cue ID, anchor, policy, or local transform is invalid.";
					return false;
				}
			}
		}
		else if (EFFECT_REFERENCE_KIND::NONE !=
			event.eEffectReferenceKind)
		{
			outStatus =
				"Only Animation EFFECT rows may carry an effect reference.";
			return false;
		}
	}

	return true;
}

bool_t Client::CAnimation_Tool::Events_AreEqual(
	const std::vector<ANIM_EVENT>& left,
	const std::vector<ANIM_EVENT>& right) const
{
	if (left.size() != right.size())
		return false;

	const auto hitsAreEqual = [](const HIT_PARAMS& a, const HIT_PARAMS& b)
	{
		return a.iRepeatCount == b.iRepeatCount &&
			a.iRepeatMs == b.iRepeatMs &&
			a.iFreezeMs == b.iFreezeMs &&
			a.iFreezeInMs == b.iFreezeInMs &&
			a.iFreezeOutMs == b.iFreezeOutMs &&
			a.iPushMs == b.iPushMs &&
			a.iPushRange == b.iPushRange &&
			a.iAreaType == b.iAreaType &&
			a.iAreaRange == b.iAreaRange &&
			a.iAreaAngle == b.iAreaAngle &&
			a.iAreaHeight == b.iAreaHeight &&
			a.iAreaOffsetX == b.iAreaOffsetX &&
			a.iAreaInner == b.iAreaInner &&
			a.iMaxTargets == b.iMaxTargets;
	};
	const auto effectTransformsAreEqual = [](
		const EFFECT_TRANSFORM_DESC& a,
		const EFFECT_TRANSFORM_DESC& b)
	{
		return a.vPosition.x == b.vPosition.x &&
			a.vPosition.y == b.vPosition.y &&
			a.vPosition.z == b.vPosition.z &&
			a.vRotationDegrees.x == b.vRotationDegrees.x &&
			a.vRotationDegrees.y == b.vRotationDegrees.y &&
			a.vRotationDegrees.z == b.vRotationDegrees.z &&
			a.vScale.x == b.vScale.x &&
			a.vScale.y == b.vScale.y &&
			a.vScale.z == b.vScale.z;
	};

	for (size_t index = 0; index < left.size(); ++index)
	{
		const ANIM_EVENT& a = left[index];
		const ANIM_EVENT& b = right[index];
		if (a.clipName != b.clipName ||
			a.eKind != b.eKind ||
			a.iStartMs != b.iStartMs ||
			a.iEndMs != b.iEndMs ||
			a.sPayload != b.sPayload ||
			a.eEffectReferenceKind != b.eEffectReferenceKind ||
			a.sAnchorSlotId != b.sAnchorSlotId ||
			a.eFollowPolicy != b.eFollowPolicy ||
			a.eOrientationPolicy != b.eOrientationPolicy ||
			a.eStopPolicy != b.eStopPolicy ||
			!effectTransformsAreEqual(a.EffectLocalTransform,
				b.EffectLocalTransform) ||
			a.bImported != b.bImported ||
			!hitsAreEqual(a.hit, b.hit))
		{
			return false;
		}
	}
	return true;
}

bool_t Client::CAnimation_Tool::Load_EventsFromPath(
	const std::filesystem::path& path,
	const shared_ptr<Engine::CModel>& pModel,
	std::vector<ANIM_EVENT>& outEvents,
	int32_t& outSourceVersion,
	std::string& outStatus) const
{
	outEvents.clear();
	outSourceVersion = {};
	outStatus.clear();

	FILE* file = nullptr;
	if (0 != _wfopen_s(&file, path.c_str(), L"rb") || nullptr == file)
	{
		outStatus = "No event file: " + path.string();
		return false;
	}

	int32_t lineNumber = 1;
	const auto fail = [&](
		const std::string& message) -> bool_t
	{
		fclose(file);
		outStatus = message + " [line " +
			std::to_string(lineNumber) + "]";
		return false;
	};

	char_t line[4096]{};
	if (nullptr == fgets(line, sizeof(line), file))
		return fail("Animation event file is empty.");

	const char_t* cursor = line;
	std::string magic;
	std::string versionToken;
	std::string owner;
	std::string countToken;
	int32_t version = {};
	int32_t declaredCount = {};
	if (!Read_Token(cursor, magic) ||
		!Read_Token(cursor, versionToken) ||
		!Read_Quoted(cursor, owner) ||
		!Read_Token(cursor, countToken) ||
		Has_RemainingToken(cursor) ||
		EVENT_FILE_MAGIC != magic ||
		!Parse_Integer(versionToken, version) ||
		!Parse_Integer(countToken, declaredCount) ||
		version < 1 || version > EVENT_FILE_VERSION ||
		declaredCount < 0 || declaredCount > MAX_EVENT_COUNT ||
		owner != m_AssetName)
	{
		return fail("Animation event header is invalid.");
	}

	std::vector<ANIM_EVENT> staged;
	staged.reserve(static_cast<size_t>(declaredCount));

	while (nullptr != fgets(line, sizeof(line), file))
	{
		++lineNumber;
		if (staged.size() >= static_cast<size_t>(declaredCount))
			return fail("Animation event row count exceeds the header.");

		const size_t lineLength = strlen(line);
		if (lineLength == sizeof(line) - 1u &&
			'\n' != line[lineLength - 1u] && !feof(file))
		{
			return fail("Animation event row is too long.");
		}

		cursor = line;
		std::string clipName;
		std::string kindName;
		if (!Read_Quoted(cursor, clipName) ||
			!Read_Token(cursor, kindName))
		{
			return fail("Malformed Animation event row.");
		}

		ANIM_EVENT event{};
		event.clipName = clipName;
		if ("HIT" == kindName)
			event.eKind = EVENT_KIND::HIT;
		else if ("CANCEL" == kindName)
			event.eKind = EVENT_KIND::CANCEL;
		else if ("SUPERARMOR" == kindName)
			event.eKind = EVENT_KIND::SUPERARMOR;
		else if ("INVULN" == kindName)
			event.eKind = EVENT_KIND::INVULN;
		else if ("MOVE" == kindName)
			event.eKind = EVENT_KIND::MOVE;
		else if ("SOUND" == kindName)
			event.eKind = EVENT_KIND::SOUND;
		else if ("EFFECT" == kindName)
		{
			event.eKind = EVENT_KIND::EFFECT;
			if (version < 4)
			{
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			}
		}
		else if ("SHAKE" == kindName)
			event.eKind = EVENT_KIND::SHAKE;
		else if ("COUNTER" == kindName)
			event.eKind = EVENT_KIND::COUNTER;
		else if ("STAGE" == kindName)
			event.eKind = EVENT_KIND::STAGE;
		else
			return fail("Unknown Animation event kind.");

		int32_t startFrame = {};
		int32_t endFrame = {};
		int32_t intervalFrame = {};
		bool_t hasStartFrame = false;
		bool_t hasEndFrame = false;
		bool_t hasStartMs = false;
		bool_t hasEndMs = false;
		bool_t hasEffectReference = false;
		bool_t hasOrientation = false;

		if (1 == version)
		{
			std::string startToken;
			std::string endToken;
			if (EVENT_KIND::HIT != event.eKind ||
				!Read_Token(cursor, startToken) ||
				!Read_Token(cursor, endToken) ||
				Has_RemainingToken(cursor) ||
				!Parse_Integer(startToken, startFrame) ||
				!Parse_Integer(endToken, endFrame))
			{
				return fail("Invalid v1 Animation HIT row.");
			}
			hasStartFrame = true;
			hasEndFrame = true;
		}
		else
		{
			std::unordered_set<std::string> keys;
			while (Has_RemainingToken(cursor))
			{
				std::string key;
				std::string value;
				if (!Read_Pair(cursor, key, value) ||
					!keys.insert(key).second)
				{
					return fail(
						"Malformed or duplicate Animation event field.");
				}

				if ("payload" == key)
				{
					event.sPayload = value;
					continue;
				}
				if ("src" == key)
				{
					if ("orig" != value)
						return fail("Unknown Animation event source.");
					event.bImported = true;
					continue;
				}
				if ("effectref" == key)
				{
					if (version < 4 ||
						EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"effectref is only valid for v4-v6 EFFECT rows.");
					}
					if ("source" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
					}
					else if ("asset-id" == value || "asset" == value)
					{
						event.eEffectReferenceKind =
							EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID;
					}
					else
						return fail("Unknown Animation effect reference kind.");
					hasEffectReference = true;
					continue;
				}
				if ("anchor" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("anchor is only valid for v5/v6 EFFECT rows.");
					event.sAnchorSlotId = value;
					continue;
				}
				if ("follow" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("follow is only valid for v5/v6 EFFECT rows.");
					if ("follow" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
					else if ("snapshot" == value)
						event.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
					else
						return fail("Unknown Effect follow policy.");
					continue;
				}
				if ("orientation" == key)
				{
					if (version < 6 || EVENT_KIND::EFFECT != event.eKind)
					{
						return fail(
							"orientation is only valid for v6 EFFECT rows.");
					}
					if ("anchor" == value)
					{
						event.eOrientationPolicy =
							EFFECT_ORIENTATION_POLICY::ANCHOR;
					}
					else if ("action_facing" == value)
					{
						event.eOrientationPolicy =
							EFFECT_ORIENTATION_POLICY::ACTION_FACING;
					}
					else
						return fail("Unknown Effect orientation policy.");
					hasOrientation = true;
					continue;
				}
				if ("stop" == key)
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("stop is only valid for v5/v6 EFFECT rows.");
					if ("natural" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					else if ("cue_end" == value)
						event.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
					else
						return fail("Unknown Effect stop policy.");
					continue;
				}
				if (key == "px" || key == "py" || key == "pz" ||
					key == "rx" || key == "ry" || key == "rz" ||
					key == "sx" || key == "sy" || key == "sz")
				{
					if (version < 5 || EVENT_KIND::EFFECT != event.eKind)
						return fail("Effect local transform is only valid in v5/v6.");
					f32_t number = 0.f;
					if (!Parse_Float(value, number))
						return fail("Animation Effect transform number is invalid.");
					if ("px" == key) event.EffectLocalTransform.vPosition.x = number;
					else if ("py" == key) event.EffectLocalTransform.vPosition.y = number;
					else if ("pz" == key) event.EffectLocalTransform.vPosition.z = number;
					else if ("rx" == key) event.EffectLocalTransform.vRotationDegrees.x = number;
					else if ("ry" == key) event.EffectLocalTransform.vRotationDegrees.y = number;
					else if ("rz" == key) event.EffectLocalTransform.vRotationDegrees.z = number;
					else if ("sx" == key) event.EffectLocalTransform.vScale.x = number;
					else if ("sy" == key) event.EffectLocalTransform.vScale.y = number;
					else if ("sz" == key) event.EffectLocalTransform.vScale.z = number;
					continue;
				}

				int32_t number = {};
				if (!Parse_Integer(value, number))
					return fail("Animation event number is invalid.");

				if ("start" == key)
				{
					startFrame = number;
					hasStartFrame = true;
				}
				else if ("end" == key)
				{
					endFrame = number;
					hasEndFrame = true;
				}
				else if ("count" == key)
					event.hit.iRepeatCount = number;
				else if ("interval" == key)
					intervalFrame = number;
				else if ("startms" == key)
				{
					event.iStartMs = number;
					hasStartMs = true;
				}
				else if ("endms" == key)
				{
					event.iEndMs = number;
					hasEndMs = true;
				}
				else if ("rep" == key)
					event.hit.iRepeatCount = number;
				else if ("repms" == key)
					event.hit.iRepeatMs = number;
				else if ("fz" == key)
					event.hit.iFreezeMs = number;
				else if ("fzin" == key)
					event.hit.iFreezeInMs = number;
				else if ("fzout" == key)
					event.hit.iFreezeOutMs = number;
				else if ("push" == key)
					event.hit.iPushMs = number;
				else if ("pushr" == key)
					event.hit.iPushRange = number;
				else if ("area" == key)
					event.hit.iAreaType = number;
				else if ("ar" == key)
					event.hit.iAreaRange = number;
				else if ("aa" == key)
					event.hit.iAreaAngle = number;
				else if ("ah" == key)
					event.hit.iAreaHeight = number;
				else if ("ax" == key)
					event.hit.iAreaOffsetX = number;
				else if ("arem" == key)
					event.hit.iAreaInner = number;
				else if ("maxt" == key)
					event.hit.iMaxTargets = number;
				else
					return fail("Unknown Animation event field.");
			}
		}

		if (2 == version)
		{
			if (!hasStartFrame ||
				(Is_Window(event.eKind) && !hasEndFrame))
			{
				return fail("v2 Animation event time is missing.");
			}
			if (!hasEndFrame)
				endFrame = startFrame;
		}
		else if (version >= 3)
		{
			if (!hasStartMs ||
				(Is_Window(event.eKind) && !hasEndMs))
			{
				return fail("Animation event millisecond time is missing.");
			}
			if (!hasEndMs)
				event.iEndMs = event.iStartMs;
		}

		if (version < 3)
		{
			const f32_t tickRate =
				Get_ClipTickRate(pModel, event.clipName);
			if (2 == version &&
				intervalFrame > 0 &&
				event.hit.iRepeatCount <= 1)
			{
				event.hit.iRepeatCount =
					(endFrame - startFrame) / intervalFrame + 1;
			}
			if (event.hit.iRepeatCount > 1)
			{
				event.hit.iRepeatMs = intervalFrame > 0 ?
					Frame_To_Ms(intervalFrame, tickRate) :
					Frame_To_Ms(
						endFrame - startFrame,
						tickRate) /
						(event.hit.iRepeatCount - 1);
				endFrame = startFrame;
			}
			event.iStartMs = Frame_To_Ms(startFrame, tickRate);
			event.iEndMs = Frame_To_Ms(endFrame, tickRate);
		}

		if (event.hit.iRepeatCount < 1)
			event.hit.iRepeatCount = 1;
		if (!Is_Window(event.eKind) &&
			!(EVENT_KIND::EFFECT == event.eKind &&
				EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
					event.eEffectReferenceKind &&
				EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy))
			event.iEndMs = event.iStartMs;
		if (3 == version && EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			event.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		}
		if (version >= 4 &&
			EVENT_KIND::EFFECT == event.eKind &&
			!hasEffectReference)
		{
			if (event.bImported)
				event.eEffectReferenceKind =
					EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
			else
				return fail("Authored v4-v6 EFFECT row requires effectref.");
		}
		if (hasOrientation &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID !=
				event.eEffectReferenceKind)
		{
			return fail(
				"orientation is only valid for admitted Effect asset rows.");
		}
		if (EVENT_KIND::EFFECT == event.eKind &&
			EFFECT_REFERENCE_KIND::EFFECT_ASSET_ID ==
				event.eEffectReferenceKind &&
			EFFECT_STOP_POLICY::CUE_END == event.eStopPolicy &&
			(!hasEndMs || event.iEndMs <= event.iStartMs))
		{
			return fail("cue_end requires endms greater than startms.");
		}

		staged.push_back(std::move(event));
	}

	if (0 != ferror(file))
		return fail("Animation event file read failed.");
	if (0 != fclose(file))
	{
		outStatus = "Animation event file close failed.";
		return false;
	}
	file = nullptr;

	if (staged.size() != static_cast<size_t>(declaredCount))
	{
		outStatus = "Animation event row count does not match the header.";
		return false;
	}

	outEvents = std::move(staged);
	outSourceVersion = version;
	return true;
}

bool_t Client::CAnimation_Tool::Save_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	std::string validationStatus;
	if (!Validate_Events(pModel, m_Events, validationStatus))
	{
		m_Status = "Save rejected: " + validationStatus;
		return false;
	}

	const std::filesystem::path destination{ Get_EventFilePath() };
	std::error_code directoryError;
	std::filesystem::create_directories(
		destination.parent_path(), directoryError);
	if (directoryError)
	{
		m_Status = "Save failed to create authoring directory: " +
			directoryError.message();
		return false;
	}

	std::filesystem::path temporary = destination;
	temporary += L".tmp";
	std::error_code removeError;
	std::filesystem::remove(temporary, removeError);
	if (removeError)
	{
		m_Status = "Save failed to clear stale temporary file: " +
			removeError.message();
		return false;
	}

	std::string transactionStatus;
	if (!Write_EventsToPath(
		temporary, m_Events, transactionStatus))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Save failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	if (!Load_EventsFromPath(
		temporary,
		pModel,
		staged,
		sourceVersion,
		transactionStatus) ||
		EVENT_FILE_VERSION != sourceVersion ||
		!Validate_Events(pModel, staged, transactionStatus) ||
		!Events_AreEqual(staged, m_Events))
	{
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		if (transactionStatus.empty())
		{
			transactionStatus =
				"Animation event round trip changed the document.";
		}
		m_Status = "Save validation failed; previous file preserved: " +
			transactionStatus;
		return false;
	}

	if (!MoveFileExW(
		temporary.c_str(),
		destination.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
	{
		const DWORD errorCode = GetLastError();
		std::error_code cleanupError;
		std::filesystem::remove(temporary, cleanupError);
		m_Status = "Atomic replace failed; previous file preserved. Win32 error " +
			std::to_string(errorCode) + ".";
		return false;
	}

	m_bDirty = false;
	m_Status = "Saved " + std::to_string(m_Events.size()) +
		" event(s) atomically to " + destination.string();
	return true;
}

bool_t Client::CAnimation_Tool::Load_Events(
	const shared_ptr<Engine::CModel>& pModel)
{
	const std::filesystem::path path{ Get_EventFilePath() };
	std::vector<ANIM_EVENT> staged;
	int32_t sourceVersion = {};
	std::string loadStatus;
	if (!Load_EventsFromPath(
		path, pModel, staged, sourceVersion, loadStatus) ||
		!Validate_Events(pModel, staged, loadStatus))
	{
		m_Status =
			"Load rejected; current document preserved: " + loadStatus;
		return false;
	}

	m_Events = std::move(staged);
	m_iSelectedEvent = -1;
	/* v3 already stores milliseconds and can be represented by v4 without moving
	any marker. Loading it must not create a false unsaved document that blocks a
	Character Select target change. Only v1/v2 frame conversion is dirty. */
	m_bDirty = sourceVersion < 3;
	m_Status = "Loaded " + std::to_string(m_Events.size()) +
		" event(s) from " + path.string();
	if (m_bDirty)
	{
		m_Status += " [converted from v" +
			std::to_string(sourceVersion) +
			" in memory; review and Save explicitly]";
	}
	else if (sourceVersion < EVENT_FILE_VERSION)
	{
		m_Status +=
			" [legacy source references preserved; the next authored Save writes v6]";
	}
	return true;
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

bool_t Client::CAnimation_Tool::Load_ClipMap()
{
	m_ClipMap.clear();

	const std::string path = Get_ClipMapPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[1024]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_MAP")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string clip;
		if (!Read_Quoted(p, clip))
			continue;

		CLIP_INFO info{};
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("skill" == key)
				info.iSkillId = atoi(value.c_str());
			else if ("name" == key)
				info.name = value;
		}

		m_ClipMap[clip] = info;
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipNotify()
{
	m_ClipNotify.clear();

	const std::string path = Get_ClipNotifyPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_ANIM_NOTIFY")
	{
		fclose(pFile);
		return false;
	}

	std::vector<NOTIFY_ROW>* pCurrent = nullptr;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		/* A clip header opens with a quoted name; its rows start with "n". */
		std::string token;
		const char_t* pProbe = p;
		if (Read_Quoted(pProbe, token))
		{
			pCurrent = &m_ClipNotify[token];

			/* The header also carries the clip's own length, which the chain
			offsets are summed from. */
			std::string hkey, hvalue;
			while (Read_Pair(pProbe, hkey, hvalue))
			{
				if ("len" == hkey)
					m_ClipLength[token] = static_cast<f32_t>(atof(hvalue.c_str()));
			}
			continue;
		}

		if (!Read_Token(p, token) || "n" != token || nullptr == pCurrent)
			continue;

		NOTIFY_ROW row{};
		bool_t bKnown = false;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("t" == key)
				row.fTime = static_cast<f32_t>(atof(value.c_str()));
			else if ("d" == key)
				row.fDuration = static_cast<f32_t>(atof(value.c_str()));
			else if ("asset" == key)
				row.sAsset = value;
			else if ("label" == key)
				row.sLabel = value;
			else if ("kind" == key)
			{
				bKnown = true;
				if ("HIT" == value)
					row.eKind = EVENT_KIND::HIT;
				else if ("CANCEL" == value)
					row.eKind = EVENT_KIND::CANCEL;
				else if ("SUPERARMOR" == value)
					row.eKind = EVENT_KIND::SUPERARMOR;
				else if ("COUNTER" == value)
					row.eKind = EVENT_KIND::COUNTER;
				else if ("SOUND" == value)
					row.eKind = EVENT_KIND::SOUND;
				else if ("EFFECT" == value)
					row.eKind = EVENT_KIND::EFFECT;
				else if ("SHAKE" == value)
					row.eKind = EVENT_KIND::SHAKE;
				else if ("STAGE" == value)
					row.eKind = EVENT_KIND::STAGE;
				else
					bKnown = false;
			}
		}

		if (bKnown)
			pCurrent->push_back(row);
	}

	fclose(pFile);
	return true;
}

bool_t Client::CAnimation_Tool::Load_ClipSeq()
{
	m_ClipSeqs.clear();
	m_ClipChainCounts.clear();

	const std::string path = Get_ClipSeqPath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_SEQ")
	{
		fclose(pFile);
		return false;
	}

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken) || !Read_Quoted(p, name))
			continue;

		CLIP_SEQ seq{};
		seq.iSkillId = atoi(idToken.c_str());
		seq.name = name;

		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				seq.iSeqIndex = atoi(value.c_str());
			else if ("mode" == key)
				seq.sMode = value;
			else if ("clips" == key)
			{
				/* comma separated, in playback order */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);
					if (!one.empty())
						seq.clips.push_back(one);
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		/* One-clip chains are kept: they are still what the skill casts, and the
		runtime reads the same file. */
		if (!seq.clips.empty())
			m_ClipSeqs.push_back(seq);
	}

	for (const CLIP_SEQ& seq : m_ClipSeqs)
	{
		for (const std::string& clip : seq.clips)
			++m_ClipChainCounts[clip];
	}

	fclose(pFile);
	Load_ClipCuts();
	return true;
}

void Client::CAnimation_Tool::Load_ClipCuts()
{
	const std::string path = CProjectDataRoot::Resolve(
		filesystem::path(L"Animation/Reference") /
		filesystem::path(m_AssetName) /
		filesystem::path(m_AssetName + ".clipcuts")).string();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_CLIP_CUTS")
	{
		fclose(pFile);
		return;
	}

	std::map<std::pair<int32_t, int32_t>, std::vector<f32_t>> cutsBySequence;
	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;
		std::string idToken;
		if (!Read_Token(p, idToken))
			continue;

		int32_t iSeqIndex = 0;
		std::vector<f32_t> cuts;
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("seq" == key)
				iSeqIndex = atoi(value.c_str());
			else if ("cuts" == key)
			{
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string one = value.substr(start,
						std::string::npos == comma ?
							std::string::npos : comma - start);
					if (!one.empty())
						cuts.push_back(
							static_cast<f32_t>(atof(one.c_str())));
					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}
		if (!cuts.empty())
			cutsBySequence[{ atoi(idToken.c_str()), iSeqIndex }] =
				std::move(cuts);
	}
	fclose(pFile);

	for (CLIP_SEQ& seq : m_ClipSeqs)
	{
		const auto found =
			cutsBySequence.find({ seq.iSkillId, seq.iSeqIndex });
		if (cutsBySequence.end() != found &&
			found->second.size() == seq.clips.size())
		{
			seq.cuts = found->second;
		}
	}
}

int32_t Client::CAnimation_Tool::Import_Notifies(const char_t* pClipName, f32_t fTickRate,
	int32_t& iShapedHits)
{
	iShapedHits = 0;

	const auto it = m_ClipNotify.find(pClipName);
	if (m_ClipNotify.end() == it)
		return 0;

	/* Drop only what a previous import put here so hand-authored rows survive. */
	for (auto e = m_Events.begin(); e != m_Events.end(); )
		e = (e->bImported && e->clipName == pClipName) ? m_Events.erase(e) : e + 1;

	const SKILL_TIMING* pRef = m_bImportKind[ETOI(EVENT_KIND::HIT)]
		? Find_ReferenceRow(pClipName) : nullptr;
	int32_t iHitOrdinal = nullptr != pRef ? Count_PrecedingChainHits(pClipName) : 0;

	int32_t iAdded = 0;
	for (const NOTIFY_ROW& row : it->second)
	{
		if (!m_bImportKind[ETOI(row.eKind)])
			continue;

		ANIM_EVENT evt{};
		evt.clipName = pClipName;
		evt.eKind = row.eKind;
		if (EVENT_KIND::EFFECT == evt.eKind)
			evt.eEffectReferenceKind =
				EFFECT_REFERENCE_KIND::SOURCE_REFERENCE;
		evt.bImported = true;
		evt.iStartMs = static_cast<int32_t>(row.fTime * 1000.f + 0.5f);
		evt.iEndMs = Is_Window(row.eKind)
			? evt.iStartMs + static_cast<int32_t>(row.fDuration * 1000.f + 0.5f)
			: evt.iStartMs;
		/* Point kinds name what they fire; a window instead carries the game's
		note on what it is for, which is the only thing distinguishing a move
		cancel from a dodge cancel. */
		evt.sPayload = Is_Window(row.eKind) ? row.sLabel : row.sAsset;

		/* The game spawns the same particle from several attach points; for
		authoring those collapse to one row. */
		bool_t bDuplicate = false;
		for (const ANIM_EVENT& other : m_Events)
		{
			if (other.bImported && other.clipName == evt.clipName &&
				other.eKind == evt.eKind && other.iStartMs == evt.iStartMs &&
				other.iEndMs == evt.iEndMs && other.sPayload == evt.sPayload)
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate)
			continue;

		if (EVENT_KIND::HIT == evt.eKind && nullptr != pRef)
		{
			const int32_t iLast = static_cast<int32_t>(pRef->hits.size()) - 1;
			const SKILL_HIT& src = pRef->hits[iHitOrdinal < iLast ? iHitOrdinal : iLast];
			++iHitOrdinal;
			if (src.hit.iAreaType > 0)
			{
				evt.hit = src.hit;
				++iShapedHits;
			}
		}

		m_Events.push_back(evt);
		++iAdded;
	}

	m_iSelectedEvent = -1;
	m_bDirty = true;
	return iAdded;
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

bool_t Client::CAnimation_Tool::Load_SkillReference()
{
	m_SkillRef.clear();

	const std::string path = Get_SkillReferencePath();

	FILE* pFile = nullptr;
	if (0 != fopen_s(&pFile, path.c_str(), "r") || nullptr == pFile)
		return false;

	char_t szLine[2048]{};
	if (nullptr == fgets(szLine, sizeof(szLine), pFile))
	{
		fclose(pFile);
		return false;
	}

	const char_t* p = szLine;
	std::string magic, versionToken, owner, countToken;
	if (!Read_Token(p, magic) || !Read_Token(p, versionToken) ||
		!Read_Quoted(p, owner) || !Read_Token(p, countToken) ||
		magic != "LOSTARK_SKILL_TIMING")
	{
		fclose(pFile);
		return false;
	}

	/* Set once the row being read gets a hit line of its own, so the windows read
	off the skill line are only a stand-in for v1 files. */
	bool_t bCurrentHasDetail = false;

	while (nullptr != fgets(szLine, sizeof(szLine), pFile))
	{
		p = szLine;

		std::string idToken, name;
		if (!Read_Token(p, idToken))
			continue;

		/* A v2 "hit" continuation line details the skill line above it. v1 files
		have none, and an older build drops them because they carry no quoted
		name where it expects one. */
		if (("hit" == idToken || "shape" == idToken) && !m_SkillRef.empty())
		{
			const bool_t bShape = "shape" == idToken;

			if (!bCurrentHasDetail)
			{
				bCurrentHasDetail = true;
				m_SkillRef.back().hits.clear();
			}

			SKILL_HIT hit{};
			hit.bTimed = !bShape;
			HIT_PARAMS& hp = hit.hit;

			std::string key, value;
			while (Read_Pair(p, key, value))
			{
				const int32_t v = atoi(value.c_str());

				if ("t" == key)
					hit.iTimeMs = v;
				else if ("w" == key)
					hit.iWidthMs = v;
				else if ("timed" == key)
					hit.bTimed = (0 != v);
				else if ("rep" == key)
					hp.iRepeatCount = v;
				else if ("repms" == key)
					hp.iRepeatMs = v;
				else if ("fz" == key)
					hp.iFreezeMs = v;
				else if ("fzin" == key)
					hp.iFreezeInMs = v;
				else if ("fzout" == key)
					hp.iFreezeOutMs = v;
				else if ("push" == key)
					hp.iPushMs = v;
				else if ("pushr" == key)
					hp.iPushRange = v;
				else if ("area" == key)
					hp.iAreaType = v;
				else if ("ar" == key)
					hp.iAreaRange = v;
				else if ("aa" == key)
					hp.iAreaAngle = v;
				else if ("ah" == key)
					hp.iAreaHeight = v;
				else if ("ax" == key)
					hp.iAreaOffsetX = v;
				else if ("arem" == key)
					hp.iAreaInner = v;
				else if ("maxt" == key)
					hp.iMaxTargets = v;
				else if ("pks" == key)
					hit.sSourceKeys = value;
			}

			if (hp.iRepeatCount < 1)
				hp.iRepeatCount = 1;

			if (bShape)
			{
				bool_t bDuplicate = false;
				for (const SKILL_HIT& other : m_SkillRef.back().hits)
				{
					const HIT_PARAMS& op = other.hit;
					if (!other.bTimed &&
						op.iAreaType == hp.iAreaType &&
						op.iAreaRange == hp.iAreaRange &&
						op.iAreaAngle == hp.iAreaAngle &&
						op.iAreaHeight == hp.iAreaHeight &&
						op.iAreaOffsetX == hp.iAreaOffsetX &&
						op.iAreaInner == hp.iAreaInner &&
						op.iMaxTargets == hp.iMaxTargets &&
						op.iRepeatCount == hp.iRepeatCount &&
						op.iRepeatMs == hp.iRepeatMs &&
						op.iFreezeMs == hp.iFreezeMs &&
						op.iFreezeInMs == hp.iFreezeInMs &&
						op.iFreezeOutMs == hp.iFreezeOutMs &&
						op.iPushMs == hp.iPushMs &&
						op.iPushRange == hp.iPushRange)
					{
						bDuplicate = true;
						break;
					}
				}
				if (bDuplicate)
					continue;
			}

			m_SkillRef.back().hits.push_back(hit);
			continue;
		}

		if (!Read_Quoted(p, name))
			continue;

		SKILL_TIMING row{};
		row.iSkillId = atoi(idToken.c_str());
		row.name = name;

		/* v1 has no hit lines, so its windows stand in as bare hits with no combat
		detail. A following hit line replaces them. */
		std::string key, value;
		while (Read_Pair(p, key, value))
		{
			if ("sa" == key)
				row.iSuperArmor = atoi(value.c_str());
			else if ("move" == key)
				row.iMoveSpeed = atoi(value.c_str());
			else if ("base" == key)
				row.iBaseSkillId = atoi(value.c_str());
			else if ("hits" == key)
			{
				/* value is "a-b,c-d": comma-separated second windows. */
				size_t start = 0;
				while (start <= value.size())
				{
					const size_t comma = value.find(',', start);
					const std::string span = value.substr(start,
						std::string::npos == comma ? std::string::npos : comma - start);

					const size_t dash = span.find('-');
					if (std::string::npos != dash)
					{
						const f32_t a = static_cast<f32_t>(atof(span.substr(0, dash).c_str()));
						const f32_t b = static_cast<f32_t>(atof(span.substr(dash + 1).c_str()));

						SKILL_HIT hit{};
						hit.iTimeMs = static_cast<int32_t>(a * 1000.f + 0.5f);
						hit.iWidthMs = static_cast<int32_t>((b - a) * 1000.f + 0.5f);
						row.hits.push_back(hit);
					}

					if (std::string::npos == comma)
						break;
					start = comma + 1;
				}
			}
		}

		m_SkillRef.push_back(row);
		bCurrentHasDetail = false;
	}

	fclose(pFile);
	return true;
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
