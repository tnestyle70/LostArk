#include "Valtan.h"

#include "ValtanPresentationAssetService.h"

#include "ActionPresentationTimeline.h"
#include "ActorCatalog.h"
#include "AnimationSkillBindingDocument.h"
#include "Body_Valtan.h"
#include "Collider.h"
#include "CameraShakeService.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "EncounterPatternReference.h"
#include "GameInstance.h"
#include "Model.h"
#include "Navigation.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "ValtanPatternTree.h"
#include <filesystem>
#ifdef _DEBUG
#include "HitAreaWire.h"
#endif

#include "Part_Equipment.h"
#include "Transform.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <span>
#include <tuple>
#include <vector>

namespace
{
	uint64_t g_iRaidBgmOwnershipGeneration = 0u;

	bool Is_ArenaCenterCueAnchor(const std::string_view slot)
	{
		return slot == "arena.center" || slot == "arena.center.facing" ||
			slot == "arena.center.target-follow";
	}

	bool Is_ArenaCenterTargetFollowCueAnchor(const std::string_view slot)
	{
		return slot == "arena.center.target-follow";
	}

	bool Is_PatternTargetSnapshotCueAnchor(const std::string_view slot)
	{
		return slot == "pattern.target.snapshot";
	}

	std::string Make_CombatObjectSoundSourceKey(
		const std::string_view archetypeId,
		const std::string_view hitId)
	{
		return std::string(archetypeId) + "\n" + std::string(hitId);
	}

	constexpr f32_t VALTAN_SERVER_TICK_HZ = 30.f;
	constexpr f32_t VALTAN_PRESENTATION_SEEK_EPSILON_SECONDS = 1.f / 120.f;
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
	constexpr f32_t NETWORK_INTERPOLATION_DELAY_TICKS = 2.f;
	constexpr f32_t NETWORK_PLAYBACK_SNAP_TICKS = 6.f;
	constexpr f32_t NETWORK_PLAYBACK_DRIFT_GAIN = 4.f;
	constexpr f32_t NETWORK_TELEPORT_DISTANCE_SQ = 100.f;
	constexpr f32_t NETWORK_TURN_DEGREES_PER_SECOND = 720.f;
	constexpr f32_t VALTAN_TARGET_FOLLOW_ROOT_YAW_OFFSET_DEGREES = 180.f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
	constexpr int32_t ROOT_MOTION_LOCK_ALL_AXES = -1;
	constexpr const char_t* VALTAN_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_WHIRLWIND";
	constexpr const char_t* VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";
	constexpr const char_t* VALTAN_IDLE_CINEMATIC_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC_IDLE";
	constexpr const char_t* VALTAN_GHOST_TRANSITION_PATTERN_ID =
		"VALTAN_GHOST_TRANSITION_15";
	constexpr const char_t* VALTAN_GHOST_PHASE_ACTION_ID =
		"valtan.mechanic.ghost-transition-15.ghost";
	constexpr const wchar_t* VALTAN_BGM_M05_ASSET_ID =
		L"Sound/BGM/Valtan/EventMixes/"
		L"bgm_heartrb_ed_m05_mscene_valtanrevive.wav";
	constexpr const wchar_t* VALTAN_BGM_M06_ASSET_ID =
		L"Sound/BGM/Valtan/EventMixes/"
		L"bgm_heartrb_ed_m06_battle_valtan_1stphase_out.wav";
	constexpr const wchar_t* VALTAN_BGM_M07_ASSET_ID =
		L"Sound/BGM/Valtan/M07_FakeDead__477456395.wav";
	constexpr const wchar_t* VALTAN_BGM_M08_ASSET_ID =
		L"Sound/BGM/Valtan/M08_ValtanPhase2__575767475.wav";
	constexpr const wchar_t* VALTAN_BGM_M09_ASSET_ID =
		L"Sound/BGM/Valtan/EventMixes/"
		L"bgm_heartrb_ed_m09_mscene_valtandead.wav";
#ifdef _DEBUG
	std::atomic_bool g_bPatternEffectV1AuditionEnabled = false;
#endif

	bool Is_ValidBossCombatState(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& state)
	{
		const bool_t hasShield = LostArk::Shared::Has_BossCombatFlag(
			state.iFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::SHIELDED);
		const bool_t isInvulnerable = LostArk::Shared::Has_BossCombatFlag(
			state.iFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::INVULNERABLE);
		const bool_t isGhostHidden = LostArk::Shared::Has_BossCombatFlag(
			state.iFlags,
			LostArk::Shared::BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN);
		return 0u != state.iStateRevision &&
			0u == (state.iFlags & static_cast<std::uint16_t>(
				~LostArk::Shared::BOSS_COMBAT_STATE_KNOWN_FLAG_MASK)) &&
			state.iCurrentStagger <= state.iMaximumStagger &&
			state.iCurrentShield <= state.iMaximumShield &&
			state.iResponseProgress <= state.iResponseThreshold &&
			(hasShield == (0u != state.iCurrentShield)) &&
			(!isGhostHidden ||
				(isInvulnerable && state.iGameplayPhase >= 3u)) &&
			0u != state.iGameplayPhase;
	}

	bool Is_SameBossCombatState(
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& left,
		const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& right)
	{
		return std::tie(
			left.iStateRevision,
			left.iAlivePartMask,
			left.iFlags,
			left.iCurrentStagger,
			left.iMaximumStagger,
			left.iCurrentShield,
			left.iMaximumShield,
			left.iResponseProgress,
			left.iResponseThreshold,
			left.iGameplayPhase) ==
			std::tie(
				right.iStateRevision,
				right.iAlivePartMask,
				right.iFlags,
				right.iCurrentStagger,
				right.iMaximumStagger,
				right.iCurrentShield,
				right.iMaximumShield,
				right.iResponseProgress,
				right.iResponseThreshold,
				right.iGameplayPhase);
	}

	bool Build_PatternTimeline(
		const std::shared_ptr<Engine::CModel>& pModel,
		const std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP> Clips,
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING>& OutTimings,
		std::vector<uint32_t>* pOutAnimationIndices = nullptr)
	{
		OutTimings.clear();
		if (nullptr != pOutAnimationIndices)
			pOutAnimationIndices->clear();
		if (nullptr == pModel || Clips.empty())
			return false;
		OutTimings.reserve(Clips.size());
		if (nullptr != pOutAnimationIndices)
			pOutAnimationIndices->reserve(Clips.size());
		for (const Client::BOSS_PATTERN_ANIMATION_CLIP& Clip : Clips)
		{
			uint32_t iAnimation = UINT32_MAX;
			for (uint32_t iCandidate = 0u;
				iCandidate < pModel->Get_NumAnimations(); ++iCandidate)
			{
				const char_t* pName = pModel->Get_AnimationName(iCandidate);
				if (nullptr != pName && Clip.strClipName == pName)
				{
					iAnimation = iCandidate;
					break;
				}
			}
			if (UINT32_MAX == iAnimation)
				return false;
			f32_t fTrackPosition = 0.f;
			f32_t fTrackDuration = 0.f;
			const f32_t fTicksPerSecond =
				pModel->Get_AnimationTickPerSecond(iAnimation);
			if (!pModel->Get_AnimationProgress(
					iAnimation, fTrackPosition, fTrackDuration) ||
				!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
				!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
			{
				return false;
			}
			Client::ACTION_PRESENTATION_CLIP_TIMING Timing{
				fTrackDuration / fTicksPerSecond,
				Clip.iPlayMs,
				Clip.fPlayRate,
				Clip.bLoop,
				static_cast<f32_t>(Clip.iSourceStartMs) * 0.001f };
			f32_t fSourceDuration = 0.f;
			f32_t fWallDuration = 0.f;
			if (!Client::CActionPresentationTimeline::Resolve_ClipDuration(
					Timing, fSourceDuration, fWallDuration))
			{
				return false;
			}
			OutTimings.push_back(Timing);
			if (nullptr != pOutAnimationIndices)
				pOutAnimationIndices->push_back(iAnimation);
		}
		return true;
	}

	bool Build_EffectV2OccurrenceClocks(
		const std::shared_ptr<Engine::CModel>& pModel,
		const std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP> Clips,
		std::vector<Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK>& OutClocks)
	{
		OutClocks.clear();
		if (Clips.empty())
			return true;
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
		if (!Build_PatternTimeline(pModel, Clips, Timings) ||
			Timings.size() != Clips.size())
		{
			return false;
		}
		OutClocks.reserve(Clips.size());
		f32_t fStageWallStartSeconds = 0.f;
		for (std::size_t iClip = 0u; iClip < Clips.size(); ++iClip)
		{
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fWallDurationSeconds = 0.f;
			if (!Client::CActionPresentationTimeline::Resolve_ClipDuration(
					Timings[iClip], fSourceDurationSeconds,
					fWallDurationSeconds) ||
				(Clips[iClip].bLoop && iClip + 1u != Clips.size()))
			{
				OutClocks.clear();
				return false;
			}
			Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK Clock;
			Clock.strClipOccurrenceId = Clips[iClip].strClipOccurrenceId;
			Clock.fStageWallStartSeconds = fStageWallStartSeconds;
			Clock.fSourceStartSeconds = Timings[iClip].fSourceStartSeconds;
			Clock.fSourceDurationSeconds = fSourceDurationSeconds;
			Clock.fLoopWallDurationSeconds = fWallDurationSeconds;
			Clock.fPlaybackRate = Timings[iClip].fPlayRate;
			Clock.bLoop = Timings[iClip].bLoop;
			OutClocks.push_back(std::move(Clock));
			fStageWallStartSeconds += fWallDurationSeconds;
		}
		return true;
	}

	bool Build_EffectV2OccurrenceClocksForAction(
		const std::shared_ptr<Engine::CModel>& pModel,
		const std::unordered_map<std::string,
			std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP>>& Bindings,
		const std::string_view strActionId,
		std::vector<Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK>& OutClocks)
	{
		OutClocks.clear();
		const auto Found = Bindings.find(std::string(strActionId));
		if (Bindings.end() == Found || Found->second.empty())
			return true;
		return Build_EffectV2OccurrenceClocks(
			pModel,
			std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
				Found->second.data(), Found->second.size()),
			OutClocks);
	}

	const std::string* Resolve_ValtanPresentationClip(
		const Client::BOSS_ACTOR_ENTRY& Actor,
		const LostArk::Shared::WORLD_ENTITY_ACTION action)
	{
		using LostArk::Shared::WORLD_ENTITY_ACTION;
		switch (action)
		{
		case WORLD_ENTITY_ACTION::IDLE:
			return &Actor.presentationClips.idle;
		case WORLD_ENTITY_ACTION::CHASE:
			return &Actor.presentationClips.chase;
		case WORLD_ENTITY_ACTION::PATTERN_WINDUP:
			return &Actor.presentationClips.patternWindup;
		case WORLD_ENTITY_ACTION::PATTERN_ACTIVE:
			return &Actor.presentationClips.patternActive;
		case WORLD_ENTITY_ACTION::PATTERN_RECOVERY:
			return &Actor.presentationClips.patternRecovery;
		case WORLD_ENTITY_ACTION::DEAD:
			return &Actor.presentationClips.dead;
		default:
			return nullptr;
		}
	}
}

#ifdef _DEBUG
void CValtan::Set_PatternEffectV1AuditionEnabled(const bool_t bEnabled)
{
	g_bPatternEffectV1AuditionEnabled.store(
		bEnabled, std::memory_order_relaxed);
}

bool_t CValtan::Is_PatternEffectV1AuditionEnabled()
{
	return g_bPatternEffectV1AuditionEnabled.load(
		std::memory_order_relaxed);
}
#endif

wstring_t CValtan::Build_ArmorModelPrototypeTag(
	const uint32_t iStateMask, const std::string_view archetypeId)
{
	wstring_t tag = TEXT("Prototype_Component_Model_ValtanArmorMask_") +
		std::to_wstring(iStateMask);
	if ("BOSS_VALTAN" != archetypeId)
		tag += L"_" + wstring_t(archetypeId.begin(), archetypeId.end());
	return tag;
}

wstring_t CValtan::Build_ArmorPartTag(const uint32_t iStateMask)
{
	return TEXT("Part_ArmorMask_") + std::to_wstring(iStateMask);
}


CValtan::CValtan(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CValtan::~CValtan()
{
	if (m_isRaidBgmEnabled && RAID_BGM_STATE::NONE != m_eRaidBgmState &&
		m_iRaidBgmOwnershipGeneration == g_iRaidBgmOwnershipGeneration)
		CGameInstance::Get().Stop_Music();
}

HRESULT CValtan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CValtan::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_INVALIDARG;
	VALTAN_DESC desc = *static_cast<VALTAN_DESC*>(pArg);
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(desc.strArchetypeId);
	if (nullptr == actor || actor->clientPresentationId != "boss.valtan.client.v1" ||
		(desc.isServerAuthoritative &&
			(("BOSS_VALTAN_GHOST" == desc.strArchetypeId) !=
			 (LostArk::Shared::INVALID_NET_ENTITY_ID != desc.iOwnerBossNetEntityId))))
	{
		return E_INVALIDARG;
	}
	if (!std::isfinite(desc.fScale) || desc.fScale <= 0.f ||
		desc.fScale > 100.f)
	{
		return E_INVALIDARG;
	}
	if (desc.fSpeedPerSec <= 0.f)
		desc.fSpeedPerSec = 3.f;
	if (desc.fRotationPerSec <= 0.f)
		desc.fRotationPerSec = 180.f;

	m_fMoveSpeed = desc.fSpeedPerSec;
	m_iPrototypeLevelIndex = desc.iPrototypeLevelIndex;
	m_isServerAuthoritative = desc.isServerAuthoritative;
	m_strArchetypeId = desc.strArchetypeId;
	m_strPresentationPartArchetypeId = desc.strArchetypeId;
	m_iOwnerBossNetEntityId = desc.iOwnerBossNetEntityId;
	m_isRaidBgmEnabled = m_isServerAuthoritative &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == m_iOwnerBossNetEntityId &&
		ETOUI(LEVEL::VALTAN_ARENA) == m_iPrototypeLevelIndex;
	m_pTargetTransform = desc.pTargetTransform;

	if (nullptr != desc.pNavigationPrototypeTag)
		m_strNavigationPrototypeTag = desc.pNavigationPrototypeTag;

	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;

	m_pTransformCom->Scale(desc.fScale, desc.fScale, desc.fScale);

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(desc.vPosition.x, desc.vPosition.y, desc.vPosition.z, 1.f));

	if (!std::isfinite(desc.fCollisionRadius) || desc.fCollisionRadius < 0.f)
		return E_INVALIDARG;

	if (FAILED(Ready_Components(desc.fCollisionRadius)))
		return E_FAIL;
	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;
	std::string PatternPresentationStatus;
	if (!m_isServerAuthoritative &&
		!Reload_PatternPresentationAuthoring(PatternPresentationStatus))
	{
		OutputDebugStringA((
			"[Client][Valtan] joined Product presentation isolated; catalog clips remain: " +
			PatternPresentationStatus + "\n").c_str());
	}
#ifdef _DEBUG
	if (m_isServerAuthoritative)
		Load_PatternHitAreaDebug();
#endif
	return S_OK;
}

#ifdef _DEBUG
void CValtan::Load_PatternHitAreaDebug()
{
	m_isPatternHitAreaDebugLoadAttempted = true;
	m_PatternHitAreaByActionId.clear();
	CEncounterPatternReference encounter;
	std::string status;
	if (!encounter.Load(CProjectDataRoot::Resolve(
			std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanEncounter.json"), status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern hit area debug isolated: " + status +
			"\n").c_str());
		return;
	}
	for (const ENCOUNTER_PATTERN_REFERENCE& pattern : encounter.Get_Patterns())
	{
		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern.stages)
		{
			const bool_t bHasStageHit = !stage.hitShape.empty() &&
				"NONE" != stage.hitShape &&
				(0u != stage.iHitCount || stage.bHasHitActivation);
			if (stage.actionId.empty() ||
				(!bHasStageHit && !stage.bHasCounterProxy))
			{
				continue;
			}
			PATTERN_HIT_AREA_DEBUG area{};
			if (bHasStageHit)
			{
				area.strHitShape = stage.hitShape;
				area.fOuterRadius = stage.fHitOuterRadius;
				area.fInnerRadius = stage.fHitInnerRadius;
				area.fAngleDegrees = stage.fHitAngleDegrees;
				area.fLength = stage.fHitLength;
				area.fHalfWidth = stage.fHitHalfWidth;
				area.iHitCount = stage.iHitCount;
				area.iHitIntervalMs = stage.iHitIntervalMs;
				area.iHitDelayMs = stage.iHitDelayMs;
				area.HitOffsetsMs = stage.hitOffsetsMs;
				area.iStageDurationMs = stage.iDurationMs;
				area.bHasActivation = stage.bHasHitActivation;
				area.iActivationStartMs = stage.iHitActivationStartMs;
				area.iActivationLifetimeMs =
					stage.iHitActivationLifetimeMs;
			}
			area.bHasCounterProxy = stage.bHasCounterProxy;
			area.strCounterProxyKind = stage.counterProxyKind;
			area.fCounterProxyForwardOffsetM =
				stage.fCounterProxyForwardOffsetM;
			area.fCounterProxyRightOffsetM =
				stage.fCounterProxyRightOffsetM;
			area.fCounterProxyRadiusM = stage.fCounterProxyRadiusM;
			area.fCounterProxyArcDegrees = stage.fCounterProxyArcDegrees;
			m_PatternHitAreaByActionId.emplace(
				stage.actionId, std::move(area));
		}
	}
}

void CValtan::Draw_PatternHitAreaDebug() const
{
	/* A tool preview clock overrides the server snapshot clock so the same
	   wires answer both the arena and the Animation Tool sequence buttons. */
	const bool_t isPreviewDriven = !m_strPreviewHitActionId.empty();
	const std::string& strActionId =
		isPreviewDriven ? m_strPreviewHitActionId : m_strServerActionId;
	if (nullptr == m_pTransformCom || strActionId.empty())
		return;
	const auto& HitAreas = isPreviewDriven && m_bLocalPatternAuthoringPreview ?
		m_LocalPreviewHitAreaByActionId : m_PatternHitAreaByActionId;
	const auto iter = HitAreas.find(strActionId);
	if (HitAreas.end() == iter)
		return;
	const PATTERN_HIT_AREA_DEBUG& area = iter->second;

	/* Mirror the Server hit clock exactly. An ACTIVE_WINDOW owns one half-open
	   [start, start + lifetime) interval. Otherwise, non-uniform contacts own
	   an ordered stage-relative offset vector and an empty vector retains the
	   legacy delay + k * interval pulse schedule. */
	constexpr f32_t MIN_VISIBLE_HIT_WINDOW_MS = 300.f;
	const f32_t fAgeMs = (isPreviewDriven ?
		m_fPreviewHitAgeSeconds : m_fServerActionAgeSeconds) * 1000.f;
	bool_t isHitWindow = false;
	if (area.bHasActivation)
	{
		const uint64_t iActivationEndMs =
			static_cast<uint64_t>(area.iActivationStartMs) +
			area.iActivationLifetimeMs;
		if (fAgeMs >= static_cast<f32_t>(area.iActivationStartMs) &&
			fAgeMs < static_cast<f32_t>(iActivationEndMs))
		{
			isHitWindow = true;
		}
	}
	else
	{
		for (uint32_t iTick = 0u; iTick < area.iHitCount; ++iTick)
		{
			const uint64_t iTickMs = area.HitOffsetsMs.empty() ?
				static_cast<uint64_t>(area.iHitDelayMs) +
					static_cast<uint64_t>(iTick) * area.iHitIntervalMs :
				area.HitOffsetsMs[iTick];
			const f32_t fTickMs = static_cast<f32_t>(iTickMs);
			if (fAgeMs >= fTickMs &&
				fAgeMs <= fTickMs + MIN_VISIBLE_HIT_WINDOW_MS)
			{
				isHitWindow = true;
				break;
			}
		}
	}
	/* Local authoring always keeps the Server-authored geometry inspectable for
	   the whole stage. Live presentation uses three independent process-global
	   switches: pulse, persistent stage geometry, and counter proxy. None of
	   these wires participates in Client hit judgement. */
	const bool_t isAuthoringGeometryWindow = isPreviewDriven &&
		fAgeMs >= 0.f &&
		fAgeMs <= static_cast<f32_t>(area.iStageDurationMs);
	const bool_t isLiveStageGeometryWindow = !isPreviewDriven &&
		m_isPatternStageGeometryDebugVisible && fAgeMs >= 0.f &&
		fAgeMs < static_cast<f32_t>(area.iStageDurationMs);
	const bool_t bDrawHitPulse = isHitWindow &&
		(isPreviewDriven || m_isPatternHitPulseDebugVisible);
	const bool_t bDrawStageGeometry =
		isAuthoringGeometryWindow || isLiveStageGeometryWindow;
	const bool_t bDrawCounterProxy = area.bHasCounterProxy &&
		(isPreviewDriven || m_isCounterProxyDebugVisible);
	if (!bDrawHitPulse && !bDrawStageGeometry && !bDrawCounterProxy)
		return;

	constexpr uint32_t PATTERN_HIT_COLOR_RGBA =
		255u | (60u << 8) | (200u << 16) | (255u << 24);
	constexpr uint32_t PATTERN_AUTHORING_GEOMETRY_COLOR_RGBA =
		255u | (185u << 8) | (40u << 16) | (210u << 24);
	constexpr uint32_t COUNTER_PROXY_COLOR_RGBA =
		60u | (180u << 8) | (255u << 16) | (255u << 24);
	constexpr f32_t METERS_TO_UNITS = 100.f;
	const matrix_t World =
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const vector_t vPosition = World.r[3];
	const vector_t vLook = XMVector3Normalize(XMVectorSetY(World.r[2], 0.f));

	const auto ToUnits = [](const f32_t fMeters)
	{
		return static_cast<int32_t>(fMeters * METERS_TO_UNITS + 0.5f);
	};
	const vector_t vBaseRight = XMVector3Normalize(XMVector3Cross(
		XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
	const auto Draw_WithYawOffset = [&](const f32_t fYawOffsetDegrees,
		const f32_t fForwardOffsetM, const f32_t fRightOffsetM,
		const HIT_AREA_SHAPE& Shape, const uint32_t iColor)
	{
		const vector_t vRotatedLook = XMVector3Rotate(vLook,
			XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f),
				XMConvertToRadians(fYawOffsetDegrees)));
		const vector_t vRight = XMVector3Normalize(XMVector3Cross(
			XMVectorSet(0.f, 1.f, 0.f, 0.f), vRotatedLook));
		const vector_t vRootPosition = vPosition +
			vLook * (fForwardOffsetM * METERS_TO_UNITS) +
			vBaseRight * (fRightOffsetM * METERS_TO_UNITS);
		float4x4_t Root{};
		XMStoreFloat4x4(&Root, XMMatrixSet(
			XMVectorGetX(vRight), 0.f, XMVectorGetZ(vRight), 0.f,
			0.f, 1.f, 0.f, 0.f,
			XMVectorGetX(vRotatedLook), 0.f, XMVectorGetZ(vRotatedLook), 0.f,
			XMVectorGetX(vRootPosition), XMVectorGetY(vRootPosition),
			XMVectorGetZ(vRootPosition), 1.f));
		CHitAreaWire::Draw(Root, Shape, iColor);
	};

	HIT_AREA_SHAPE Shape{};
	const bool_t bDrawHitGeometry =
		bDrawHitPulse || bDrawStageGeometry;
	const uint32_t iHitGeometryColor = bDrawHitPulse ?
		PATTERN_HIT_COLOR_RGBA : PATTERN_AUTHORING_GEOMETRY_COLOR_RGBA;
	if (bDrawHitGeometry &&
		("CIRCLE" == area.strHitShape || "RING" == area.strHitShape))
	{
		Shape.iAreaType = 1;
		Shape.iAreaRange = ToUnits(area.fOuterRadius);
		Shape.iAreaInner = ToUnits(area.fInnerRadius);
		Draw_WithYawOffset(0.f, 0.f, 0.f, Shape, iHitGeometryColor);
	}
	else if (bDrawHitGeometry && "CONE" == area.strHitShape)
	{
		Shape.iAreaType = 3;
		Shape.iAreaRange = ToUnits(area.fLength);
		Shape.iAreaAngle = static_cast<int32_t>(area.fAngleDegrees + 0.5f);
		Draw_WithYawOffset(0.f, 0.f, 0.f, Shape, iHitGeometryColor);
	}
	else if (bDrawHitGeometry && "BOX" == area.strHitShape)
	{
		Shape.iAreaType = 2;
		Shape.iAreaRange = ToUnits(area.fLength);
		Shape.iAreaAngle = ToUnits(area.fHalfWidth * 2.f);
		Draw_WithYawOffset(0.f, 0.f, 0.f, Shape, iHitGeometryColor);
	}
	else if (bDrawHitGeometry && ("CROSS" == area.strHitShape ||
		"SIX_DIRECTIONS" == area.strHitShape))
	{
		/* The server tests centered strips spanning [-length, +length] along
		   each strip axis and +-halfWidth across: forward and right for CROSS,
		   forward and the two 60-degree diagonals for SIX_DIRECTIONS. */
		Shape.iAreaType = 2;
		Shape.iAreaOffsetX = -ToUnits(area.fLength);
		Shape.iAreaRange = ToUnits(area.fLength * 2.f);
		Shape.iAreaAngle = ToUnits(area.fHalfWidth * 2.f);
		Draw_WithYawOffset(0.f, 0.f, 0.f, Shape, iHitGeometryColor);
		if ("CROSS" == area.strHitShape)
			Draw_WithYawOffset(90.f, 0.f, 0.f, Shape, iHitGeometryColor);
		else
		{
			Draw_WithYawOffset(60.f, 0.f, 0.f, Shape, iHitGeometryColor);
			Draw_WithYawOffset(-60.f, 0.f, 0.f, Shape, iHitGeometryColor);
		}
	}
	if (bDrawCounterProxy)
	{
		HIT_AREA_SHAPE CounterShape{};
		if ("BOSS_FORWARD_ARC" == area.strCounterProxyKind)
		{
			CounterShape.iAreaType = 3;
			CounterShape.iAreaAngle = static_cast<int32_t>(
				area.fCounterProxyArcDegrees + 0.5f);
		}
		else
		{
			CounterShape.iAreaType = 1;
		}
		CounterShape.iAreaRange = ToUnits(area.fCounterProxyRadiusM);
		Draw_WithYawOffset(
			0.f, area.fCounterProxyForwardOffsetM,
			area.fCounterProxyRightOffsetM, CounterShape,
			COUNTER_PROXY_COLOR_RGBA);
	}
}

void CValtan::Set_PatternHitAreaPreview(
	const std::string& stageActionId,
	const f32_t fStageAgeSeconds)
{
	/* A Development preview boss never took the server-authoritative load in
	   Initialize, so the display copy is admitted on first use here. */
	if (!m_bLocalPatternAuthoringPreview &&
		!m_isPatternHitAreaDebugLoadAttempted)
		Load_PatternHitAreaDebug();
	m_strPreviewHitActionId = stageActionId;
	m_fPreviewHitAgeSeconds = fStageAgeSeconds;
}

void CValtan::Clear_PatternHitAreaPreview()
{
	m_strPreviewHitActionId.clear();
	m_fPreviewHitAgeSeconds = 0.f;
}
#endif

void CValtan::Load_PatternBindings()
{
	std::string Status;
	if (!Reload_PatternPresentationAuthoring(Status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern bindings rejected; catalog clips remain: " +
			Status + "\n").c_str());
	}
}

bool_t CValtan::Reload_PatternBindings_WhileAdmitted(
	std::string& strOutStatus)
{
	if (nullptr == m_pBodyModelCom)
	{
		strOutStatus =
			"Valtan pattern binding reload requires an admitted body model.";
		return false;
	}
	std::vector<std::string> availableClips;
	availableClips.reserve(m_pBodyModelCom->Get_NumAnimations());
	for (uint32_t index = 0u;
		index < m_pBodyModelCom->Get_NumAnimations(); ++index)
	{
		const char_t* clip = m_pBodyModelCom->Get_AnimationName(index);
		if (nullptr != clip && '\0' != *clip)
			availableClips.emplace_back(clip);
	}
	Client::BOSS_PATTERN_ANIMATION_BINDING_DOCUMENT document;
	if (!Client::CValtanPatternAnimationBindingDocument::Load(
			"Valtan", "BOSS_VALTAN", availableClips, document, strOutStatus))
	{
		return false;
	}
	std::unordered_map<std::string,
		std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP>> staged;
	std::unordered_map<std::string, PATTERN_BODY_VISIBILITY_WINDOW>
		stagedBodyVisibility;
	for (const Client::BOSS_PATTERN_ANIMATION_BINDING& binding :
		document.Bindings)
	{
		if (binding.bSuppressAnimation)
		{
			staged.emplace(binding.strActionId,
				std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP>{});
			if (binding.bHasBodyHiddenWindow)
			{
				stagedBodyVisibility.emplace(binding.strActionId,
					PATTERN_BODY_VISIBILITY_WINDOW{
						binding.iBodyHiddenFromMs,
						binding.iBodyHiddenToMs });
			}
			continue;
		}
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
		if (!Build_PatternTimeline(m_pBodyModelCom,
				std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
					binding.Clips.data(), binding.Clips.size()), Timings))
		{
			strOutStatus =
				"Valtan pattern binding reload rejected a model source segment: " +
				binding.strActionId;
			return false;
		}
		staged.emplace(binding.strActionId, binding.Clips);
		if (binding.bHasBodyHiddenWindow)
		{
			stagedBodyVisibility.emplace(binding.strActionId,
				PATTERN_BODY_VISIBILITY_WINDOW{
					binding.iBodyHiddenFromMs,
					binding.iBodyHiddenToMs });
		}
	}
	m_PatternClipByActionId = std::move(staged);
	m_PatternBodyVisibilityByActionId = std::move(stagedBodyVisibility);
	m_iPatternPresentationClipOccurrenceIndex =
		(std::numeric_limits<std::size_t>::max)();
	strOutStatus = "Reloaded " +
		std::to_string(m_PatternClipByActionId.size()) +
		" Valtan Product animation action binding(s).";
	return true;
}

bool_t CValtan::Reload_PlayerHandGripLocalOffsets_WhileAdmitted(
	std::string& strOutStatus)
{
	CEncounterPatternReference encounter;
	if (!encounter.Load(CProjectDataRoot::Resolve(
			std::filesystem::path(L"Encounters") / L"Valtan" /
			L"ValtanEncounter.json"), strOutStatus))
	{
		strOutStatus =
			"Valtan player hand-grip Product admission rejected: " +
			strOutStatus;
		return false;
	}

	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET> staged;
	for (const ENCOUNTER_PATTERN_REFERENCE& pattern : encounter.Get_Patterns())
	{
		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern.stages)
		{
			if (!stage.gripLocalOffset.has_value())
				continue;
			if (stage.actionId.empty() ||
				!CPlayerHandGripTransform::Is_ValidGripLocalOffset(
					*stage.gripLocalOffset) ||
				!staged.emplace(stage.actionId, *stage.gripLocalOffset).second)
			{
				strOutStatus =
					"Valtan player hand-grip stable action identity is invalid: " +
					pattern.patternId + "/" + stage.stageId;
				return false;
			}
		}
	}
	m_PlayerHandGripLocalOffsetByActionId = std::move(staged);
	strOutStatus = "Reloaded " +
		std::to_string(m_PlayerHandGripLocalOffsetByActionId.size()) +
		" Valtan player hand-grip action binding(s).";
	return true;
}

bool_t CValtan::Try_Get_PlayerHandGripLocalOffset(
	const std::string_view actionId,
	Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const
{
	if (actionId.empty())
		return false;
	const auto found = m_PlayerHandGripLocalOffsetByActionId.find(
		std::string(actionId));
	if (m_PlayerHandGripLocalOffsetByActionId.end() == found ||
		!CPlayerHandGripTransform::Is_ValidGripLocalOffset(found->second))
	{
		return false;
	}
	outOffset = found->second;
	return true;
}

bool_t CValtan::Reload_PatternPresentationAuthoring(
	std::string& strOutStatus)
{
	if (m_isServerAuthoritative)
	{
		strOutStatus =
			"Server-authoritative Valtan reload requires an exact Server R -> presentation M receipt.";
		return false;
	}
	return Reload_PatternPresentationAuthoring_Impl(
		nullptr, nullptr, strOutStatus);
}

bool_t CValtan::Reload_PatternPresentationAuthoring(
	const LostArk::Shared::GameplayDataRevision& ExpectedServerRevision,
	const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT& ExpectedReceipt,
	std::string& strOutStatus)
{
	return Reload_PatternPresentationAuthoring_Impl(
		&ExpectedServerRevision, &ExpectedReceipt, strOutStatus);
}

bool_t CValtan::Reload_PatternPresentationAuthoring_Impl(
	const LostArk::Shared::GameplayDataRevision* pExpectedServerRevision,
	const Client::VALTAN_PRESENTATION_GENERATION_RECEIPT* pExpectedReceipt,
	std::string& strOutStatus)
{
	const bool_t bExact = nullptr != pExpectedServerRevision &&
		nullptr != pExpectedReceipt;
	if ((nullptr == pExpectedServerRevision) != (nullptr == pExpectedReceipt))
	{
		strOutStatus = "Valtan presentation receipt arguments are incomplete.";
		return false;
	}
	CValtanCanonicalProductReadAdmission CanonicalAdmission;
	Client::CValtanPresentationGenerationReadAdmission GenerationAdmission;
	if (bExact)
	{
		if (!GenerationAdmission.Acquire_Receipt(
				*pExpectedServerRevision, *pExpectedReceipt, strOutStatus))
		{
			return false;
		}
	}
	else
	{
		VALTAN_CANONICAL_READ_DIAGNOSTIC CanonicalDiagnostic;
		if (!CanonicalAdmission.Acquire(CanonicalDiagnostic))
		{
			strOutStatus = std::move(CanonicalDiagnostic.strStatus);
			return false;
		}
	}
	Client::VALTAN_PRESENTATION_GENERATION_RECEIPT CurrentPresentationReceipt;
	if (bExact && !GenerationAdmission.Try_Get_CurrentReceipt(
			CurrentPresentationReceipt))
	{
		strOutStatus =
			"Valtan presentation reload did not retain its current typed-source receipt.";
		return false;
	}

	const auto PreviousBindings = m_PatternClipByActionId;
	const auto PreviousBodyVisibility =
		m_PatternBodyVisibilityByActionId;
	const auto PreviousGripLocalOffsets =
		m_PlayerHandGripLocalOffsetByActionId;
	const auto PreviousEffectCues = m_PatternEffectCuesByActionId;
	const auto PreviousArenaCenters = m_PatternArenaCenterAnchors;
	const auto PreviousEffectAttempts = m_AttemptedPatternEffectOccurrenceKeys;
	const bool_t PreviousEffectScanValid = m_bPatternEffectCueScanAgeValid;
	const f32_t PreviousEffectScanAge = m_fPatternEffectCueScanAgeSeconds;
	const auto PreviousSoundCues = m_PatternSoundCuesByActionId;
	const auto PreviousSoundSourceReceipt = m_PatternSoundSourceReceipt;
	const auto PreviousSoundAttempts = m_AttemptedPatternSoundOccurrenceKeys;
	const bool_t PreviousSoundScanValid = m_bPatternSoundCueScanAgeValid;
	const f32_t PreviousSoundScanAge = m_fPatternSoundCueScanAgeSeconds;
	const auto PreviousCombatObjectSoundCues =
		m_CombatObjectSoundCuesBySource;
	const auto PreviousShakeCues = m_PatternShakeCuesByActionId;
	const auto PreviousShakeAttempts = m_AttemptedPatternShakeOccurrenceKeys;
	const bool_t PreviousShakeScanValid = m_bPatternShakeCueScanAgeValid;
	const f32_t PreviousShakeScanAge = m_fPatternShakeCueScanAgeSeconds;
	const std::size_t PreviousClipOccurrenceIndex =
		m_iPatternPresentationClipOccurrenceIndex;
	const auto PreviousPresentationReceipt =
		m_PresentationGenerationReceipt;

	const auto RestorePrevious = [this,
		&PreviousBindings, &PreviousBodyVisibility,
		&PreviousGripLocalOffsets,
		&PreviousEffectCues, &PreviousArenaCenters,
		&PreviousEffectAttempts, PreviousEffectScanValid,
		PreviousEffectScanAge, &PreviousSoundCues,
		&PreviousSoundSourceReceipt, &PreviousSoundAttempts,
		PreviousSoundScanValid, PreviousSoundScanAge,
		&PreviousCombatObjectSoundCues,
		&PreviousShakeCues, &PreviousShakeAttempts, PreviousShakeScanValid,
		PreviousShakeScanAge, PreviousClipOccurrenceIndex,
		&PreviousPresentationReceipt]()
	{
		m_PatternClipByActionId = PreviousBindings;
		m_PatternBodyVisibilityByActionId = PreviousBodyVisibility;
		m_PlayerHandGripLocalOffsetByActionId = PreviousGripLocalOffsets;
		m_PatternEffectCuesByActionId = PreviousEffectCues;
		m_PatternArenaCenterAnchors = PreviousArenaCenters;
		m_AttemptedPatternEffectOccurrenceKeys = PreviousEffectAttempts;
		m_bPatternEffectCueScanAgeValid = PreviousEffectScanValid;
		m_fPatternEffectCueScanAgeSeconds = PreviousEffectScanAge;
		m_PatternSoundCuesByActionId = PreviousSoundCues;
		m_PatternSoundSourceReceipt = PreviousSoundSourceReceipt;
		m_AttemptedPatternSoundOccurrenceKeys = PreviousSoundAttempts;
		m_bPatternSoundCueScanAgeValid = PreviousSoundScanValid;
		m_fPatternSoundCueScanAgeSeconds = PreviousSoundScanAge;
		m_CombatObjectSoundCuesBySource = PreviousCombatObjectSoundCues;
		m_PatternShakeCuesByActionId = PreviousShakeCues;
		m_AttemptedPatternShakeOccurrenceKeys = PreviousShakeAttempts;
		m_bPatternShakeCueScanAgeValid = PreviousShakeScanValid;
		m_fPatternShakeCueScanAgeSeconds = PreviousShakeScanAge;
		m_iPatternPresentationClipOccurrenceIndex =
			PreviousClipOccurrenceIndex;
		m_PresentationGenerationReceipt = PreviousPresentationReceipt;
	};

	std::string StepStatus;
	if (!Reload_PatternBindings_WhileAdmitted(StepStatus) ||
		!Reload_PlayerHandGripLocalOffsets_WhileAdmitted(StepStatus) ||
		!Reload_PatternEffectCues_WhileAdmitted(StepStatus) ||
		!Reload_PatternSoundCues_WhileAdmitted(StepStatus) ||
		!Reload_CombatObjectSoundCues_WhileAdmitted(StepStatus) ||
		!Reload_PatternShakeCues_WhileAdmitted(StepStatus))
	{
		RestorePrevious();
		strOutStatus =
			"Valtan joined presentation reload rejected; every previous animation/grip/effect/sound/combat-sound/shake cache was preserved: " +
			StepStatus;
		return false;
	}

	/* The component readers above write only this private scratch state while
	   the method is synchronous. Move it aside, restore the prior admitted
	   caches, then perform the post-read journal check before one aggregate
	   commit. Thus a late admission failure is byte/logically invisible to
	   gameplay even though validation reuses the established typed loaders. */
	auto StagedBindings = std::move(m_PatternClipByActionId);
	auto StagedBodyVisibility =
		std::move(m_PatternBodyVisibilityByActionId);
	auto StagedGripLocalOffsets =
		std::move(m_PlayerHandGripLocalOffsetByActionId);
	auto StagedEffectCues = std::move(m_PatternEffectCuesByActionId);
	auto StagedArenaCenters = std::move(m_PatternArenaCenterAnchors);
	auto StagedSoundCues = std::move(m_PatternSoundCuesByActionId);
	auto StagedSoundSourceReceipt =
		std::move(m_PatternSoundSourceReceipt);
	auto StagedCombatObjectSoundCues =
		std::move(m_CombatObjectSoundCuesBySource);
	auto StagedShakeCues = std::move(m_PatternShakeCuesByActionId);
	const bool_t bJoinedPresentationGenerationUnchanged =
		bExact && PreviousPresentationReceipt.Is_Valid() &&
		PreviousPresentationReceipt == CurrentPresentationReceipt;
	const bool_t bPatternSoundGenerationUnchanged =
		bJoinedPresentationGenerationUnchanged &&
		PreviousSoundSourceReceipt.Is_Valid() &&
		PreviousSoundSourceReceipt == StagedSoundSourceReceipt;
	RestorePrevious();
	if (bExact ?
		!GenerationAdmission.Validate_StillCurrent(StepStatus) :
		!CanonicalAdmission.Validate_StillCurrent(StepStatus))
	{
		strOutStatus =
			"Valtan joined presentation reload became stale; every previous cache was preserved: " +
			StepStatus;
		return false;
	}

	m_PatternClipByActionId = std::move(StagedBindings);
	m_PatternBodyVisibilityByActionId =
		std::move(StagedBodyVisibility);
	m_PlayerHandGripLocalOffsetByActionId =
		std::move(StagedGripLocalOffsets);
	m_PatternEffectCuesByActionId = std::move(StagedEffectCues);
	m_PatternArenaCenterAnchors = std::move(StagedArenaCenters);
	m_PatternSoundCuesByActionId = std::move(StagedSoundCues);
	m_PatternSoundSourceReceipt = std::move(StagedSoundSourceReceipt);
	m_CombatObjectSoundCuesBySource =
		std::move(StagedCombatObjectSoundCues);
	m_PatternShakeCuesByActionId = std::move(StagedShakeCues);
	/* Pattern Sound S is independent of the joined presentation M. An idle S
	   apply must not make already-attempted Effect/Shake occurrences eligible
	   again. Reset other lane cursors only when M itself changed; reset Sound
	   only when either its own S or its animation dependency generation changed. */
	if (!bJoinedPresentationGenerationUnchanged)
	{
		m_AttemptedPatternEffectOccurrenceKeys.clear();
		m_bPatternEffectCueScanAgeValid = false;
		m_fPatternEffectCueScanAgeSeconds = 0.f;
		m_AttemptedPatternShakeOccurrenceKeys.clear();
		m_bPatternShakeCueScanAgeValid = false;
		m_fPatternShakeCueScanAgeSeconds = 0.f;
		m_iPatternPresentationClipOccurrenceIndex =
			(std::numeric_limits<std::size_t>::max)();
	}
	if (!bPatternSoundGenerationUnchanged)
	{
		m_AttemptedPatternSoundOccurrenceKeys.clear();
		m_bPatternSoundCueScanAgeValid = false;
		m_fPatternSoundCueScanAgeSeconds = 0.f;
	}
	if (bExact)
		m_PresentationGenerationReceipt =
			std::move(CurrentPresentationReceipt);

	strOutStatus =
		bExact ?
		"Reloaded one exact Server R -> Valtan presentation M generation; Pattern Sound retained its typed receipt." :
		"Reloaded one non-authoritative Valtan Product preview generation.";
	return true;
}

bool_t CValtan::Reload_PatternBindings(std::string& strOutStatus)
{
	return Reload_PatternPresentationAuthoring(strOutStatus);
}

bool_t CValtan::Reload_PatternSoundCues(std::string& strOutStatus)
{
	return Reload_PatternPresentationAuthoring(strOutStatus);
}

bool_t CValtan::Reload_CombatObjectSoundCues(std::string& strOutStatus)
{
	return Reload_PatternPresentationAuthoring(strOutStatus);
}

bool_t CValtan::Apply_PatternPresentationSample(
	const std::string_view actionId,
	const std::string_view fallbackClipName,
	const f32_t fActionAgeSeconds,
	const bool_t bAnimationEdgeChanged,
	const std::size_t iCurrentClipOccurrenceIndex,
	std::size_t& iOutClipOccurrenceIndex)
{
	iOutClipOccurrenceIndex = iCurrentClipOccurrenceIndex;
	if (nullptr == m_pBodyModelCom || fallbackClipName.empty() ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f)
	{
		return false;
	}

	/* A present empty binding is the explicit Product NONE variant: this stage
	   owns no new boss animation and keeps the pose reached by the preceding
	   action. A genuinely missing optional binding still takes the catalog
	   fallback in both Arena presentation and local Animation Tool audition. */
	Client::BOSS_PATTERN_ANIMATION_CLIP FallbackClip;
	FallbackClip.strClipOccurrenceId = "catalog.fallback";
	FallbackClip.strClipName.assign(fallbackClipName);
	FallbackClip.strMappingBasis = "PROJECT_AUTHORED";
	FallbackClip.bLoop = true;
	std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP> ClipChain(
		&FallbackClip, 1u);
	if (!actionId.empty())
	{
		const auto& Bindings = m_bLocalPatternAuthoringPreview ?
			m_LocalPreviewClipByActionId : m_PatternClipByActionId;
		const auto Bound = Bindings.find(std::string(actionId));
		if (Bindings.end() != Bound)
		{
			if (Bound->second.empty())
			{
				m_pBodyModelCom->Set_AnimPaused(true);
				iOutClipOccurrenceIndex =
					(std::numeric_limits<std::size_t>::max)();
				return true;
			}
			ClipChain = std::span<
				const Client::BOSS_PATTERN_ANIMATION_CLIP>(
					Bound->second.data(), Bound->second.size());
		}
	}
	m_pBodyModelCom->Set_AnimPaused(false);

	std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
	std::vector<uint32_t> AnimationIndices;
	if (!Build_PatternTimeline(
			m_pBodyModelCom, ClipChain, Timings, &AnimationIndices))
	{
		return false;
	}
	Client::ACTION_PRESENTATION_SAMPLE Sample;
	if (!Client::CActionPresentationTimeline::Resolve_Sample(
			std::span<const Client::ACTION_PRESENTATION_CLIP_TIMING>(
				Timings.data(), Timings.size()),
			fActionAgeSeconds, Sample) ||
		Sample.iClipIndex >= AnimationIndices.size())
	{
		return false;
	}

	const uint32_t iTargetAnimation = AnimationIndices[Sample.iClipIndex];
	const Client::BOSS_PATTERN_ANIMATION_CLIP& TargetClip =
		ClipChain[Sample.iClipIndex];
	const bool_t bClipOccurrenceTransition =
		Client::CActionPresentationTimeline::Requires_ClipOccurrenceTransition(
			iCurrentClipOccurrenceIndex,
			Sample.iClipIndex,
			m_pBodyModelCom->Get_CurrentAnimIndex(),
			iTargetAnimation);
	if (bAnimationEdgeChanged || bClipOccurrenceTransition)
	{
		if (!m_pBodyModelCom->Start_Animation(
				TargetClip.strClipName.c_str(), TargetClip.bLoop))
		{
			return false;
		}
		Client::CEffectV2Runtime::Notify_Clip(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			TargetClip.strClipName.c_str());
	}
	else
	{
		m_pBodyModelCom->Set_Animation(
			iTargetAnimation, TargetClip.bLoop);
	}
	m_pBodyModelCom->Set_AnimationSpeed(TargetClip.fPlayRate);
	const f32_t fTicksPerSecond =
		m_pBodyModelCom->Get_AnimationTickPerSecond(iTargetAnimation);
	if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
		!m_pBodyModelCom->Set_AnimTrackPosition(
			iTargetAnimation,
			Sample.fClipSourceTimeSeconds * fTicksPerSecond))
	{
		return false;
	}
	m_pBodyModelCom->Play_Animation(0.f);
	iOutClipOccurrenceIndex = Sample.iClipIndex;
	return true;
}

bool_t CValtan::Apply_LocalPatternPresentationSample(
	const LostArk::Shared::WORLD_ENTITY_ACTION patternAction,
	const std::string_view actionId,
	const f32_t fActionAgeSeconds,
	const bool_t bForceAnimationEdge)
{
	if (m_isServerAuthoritative || actionId.empty() ||
		(LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_WINDUP != patternAction &&
		 LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_ACTIVE != patternAction &&
		 LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_RECOVERY != patternAction))
		return false;
	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss(m_strArchetypeId);
	const std::string* pFallbackClip = nullptr == pActor ? nullptr :
		Resolve_ValtanPresentationClip(*pActor, patternAction);
	if (nullptr == pFallbackClip)
		return false;

	std::size_t iAcceptedClipOccurrenceIndex =
		m_iPatternPresentationClipOccurrenceIndex;
	if (!Apply_PatternPresentationSample(
			actionId,
			*pFallbackClip,
			fActionAgeSeconds,
			bForceAnimationEdge,
			m_iPatternPresentationClipOccurrenceIndex,
			iAcceptedClipOccurrenceIndex))
	{
		return false;
	}
	m_iPatternPresentationClipOccurrenceIndex =
		iAcceptedClipOccurrenceIndex;
	const auto LocalStage =
		m_LocalPreviewStageIndexByActionId.find(std::string(actionId));
	if (m_bLocalPatternAuthoringPreview &&
		m_LocalPreviewStageIndexByActionId.end() != LocalStage)
	{
		const bool_t bStageEdge = m_strLocalPreviewActionId != actionId;
		const bool_t bClockRewind = m_bPatternEffectCueScanAgeValid &&
			fActionAgeSeconds + 0.00001f < m_fPatternEffectCueScanAgeSeconds;
		if (bStageEdge || bClockRewind)
		{
			const std::shared_ptr<CValtan> Owner =
				static_pointer_cast<CValtan>(shared_from_this());
			Stop_LocalPatternCombatObjectPreview();
			if (bClockRewind)
			{
				CEffectPresentationService::Stop_BossOwner(Owner);
			}
			else
			{
				/* Match the Product stage boundary: cue-end work stops, while an
				   already-active NATURAL document may finish.  Explicit seek/restart
				   uses Reset_LocalPatternPreviewTransport and removes everything. */
				(void)CEffectPresentationService::Stop_BossAction(
					Owner, m_iLocalPreviewEffectGeneration);
			}
			m_AttemptedPatternEffectOccurrenceKeys.clear();
			m_bPatternEffectCueScanAgeValid = false;
			m_fPatternEffectCueScanAgeSeconds = 0.f;
		}
		m_strLocalPreviewActionId.assign(actionId);
		m_iLocalPreviewStageIndex = LocalStage->second;
		std::string CombatObjectStatus;
		if (!Sync_LocalPatternCombatObjectPreview(
				actionId, fActionAgeSeconds, CombatObjectStatus))
		{
			m_strLocalPreviewCombatObjectStatus = CombatObjectStatus;
			OutputDebugStringA((
				"[Client][Valtan] local combat-object preview isolated: " +
				CombatObjectStatus + "\n").c_str());
			return false;
		}
		Spawn_DuePatternEffectCues(fActionAgeSeconds);
	}
	const Client::EFFECT_V2_TARGET EffectV2Target =
		Client::EFFECT_V2_TARGET::From_Valtan(
			static_pointer_cast<CValtan>(shared_from_this()));
	const std::string StageActionId(actionId);
	std::vector<Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK> EffectV2Clocks;
	if (!Build_EffectV2OccurrenceClocksForAction(
			m_pBodyModelCom,
			m_bLocalPatternAuthoringPreview ?
				m_LocalPreviewClipByActionId : m_PatternClipByActionId,
			StageActionId, EffectV2Clocks))
	{
		OutputDebugStringA((
			"[Client][Valtan] Effect V2 occurrence clock rejected for action: " +
			StageActionId + "\n").c_str());
		EffectV2Clocks.clear();
	}
	if (m_bLocalPatternAuthoringPreview)
	{
		Client::CEffectV2Runtime::Sync_StageAuthoring(
			EffectV2Target, StageActionId.c_str(), fActionAgeSeconds,
			EffectV2Clocks,
			Client::CEffectV2Catalog::Get().Get_Snapshot(),
			m_pDevice, m_pContext);
	}
	else
	{
		Client::CEffectV2Runtime::Sync_Stage(
			EffectV2Target, StageActionId.c_str(), fActionAgeSeconds,
			EffectV2Clocks,
			m_pDevice, m_pContext);
	}
	return true;
}

void CValtan::Stop_LocalPatternCombatObjectPreview()
{
	for (LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE& Instance :
		m_LocalPreviewCombatObjectInstances)
	{
		if (0u != Instance.iActiveHandle)
		{
			EFFECT_WORLD_ROOT_HANDLE Handle;
			Handle.iValue = Instance.iActiveHandle;
			CEffectPresentationService::Stop_WorldRoot(Handle);
			Instance.iActiveHandle = 0u;
		}
		for (uint64_t& iTerminalHandle : Instance.TerminalHandles)
		{
			if (0u == iTerminalHandle)
				continue;
			EFFECT_WORLD_ROOT_HANDLE Handle;
			Handle.iValue = iTerminalHandle;
			CEffectPresentationService::Stop_WorldRoot(Handle);
			iTerminalHandle = 0u;
		}
	}
	m_LocalPreviewCombatObjectInstances.clear();
	m_strLocalPreviewCombatObjectActionId.clear();
}

bool_t CValtan::Sync_LocalPatternCombatObjectPreview(
	const std::string_view actionId,
	const f32_t fActionAgeSeconds,
	std::string& strOutStatus)
{
	strOutStatus.clear();
	if (m_isServerAuthoritative || !m_bLocalPatternAuthoringPreview ||
		nullptr == m_pTransformCom || actionId.empty() ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f)
	{
		strOutStatus = "Local combat-object preview clock is invalid.";
		return false;
	}

	const auto Found = m_LocalPreviewCombatObjectsByActionId.find(
		std::string(actionId));
	if (m_LocalPreviewCombatObjectsByActionId.end() == Found)
	{
		if (!m_LocalPreviewCombatObjectInstances.empty())
			Stop_LocalPatternCombatObjectPreview();
		return true;
	}

	if (m_strLocalPreviewCombatObjectActionId != actionId)
	{
		Stop_LocalPatternCombatObjectPreview();
		float3_t BossPosition{};
		XMStoreFloat3(&BossPosition,
			m_pTransformCom->Get_State(STATE::POSITION));
		if (!std::isfinite(BossPosition.x) ||
			!std::isfinite(BossPosition.y) ||
			!std::isfinite(BossPosition.z) ||
			!std::isfinite(m_fPresentationYawDegrees))
		{
			strOutStatus =
				"Local combat-object preview has no finite stage-enter boss pose.";
			return false;
		}

		std::vector<LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE> StagedInstances;
		for (const LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE& Template : Found->second)
		{
			for (uint32_t iOrdinal = 0u; iOrdinal < Template.iCount; ++iOrdinal)
			{
				const f32_t fRelativeDegrees = Template.fStartAngleDegrees +
					Template.fAngleStepDegrees * static_cast<f32_t>(iOrdinal);
				const f32_t fWorldDegrees =
					m_fPresentationYawDegrees + fRelativeDegrees;
				const f32_t fRadians = XMConvertToRadians(fWorldDegrees);
				LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE Instance;
				Instance.Template = Template;
				Instance.iOrdinal = iOrdinal;
				Instance.vPosition = {
					BossPosition.x + std::sin(fRadians) * Template.fRadiusM,
					BossPosition.y,
					BossPosition.z + std::cos(fRadians) * Template.fRadiusM };
				Instance.fYawDegrees = fWorldDegrees;
				Instance.TerminalHandles.resize(
					Template.PresentationEvents.size(), 0u);
				Instance.TerminalAttempts.resize(
					Template.PresentationEvents.size(), false);
				StagedInstances.push_back(std::move(Instance));
			}
		}
		m_LocalPreviewCombatObjectInstances = std::move(StagedInstances);
		m_strLocalPreviewCombatObjectActionId.assign(actionId);
	}

	const std::shared_ptr<CValtan> Owner =
		std::static_pointer_cast<CValtan>(shared_from_this());
	const auto Rollback = [this, &strOutStatus](const std::string& Status)
	{
		Stop_LocalPatternCombatObjectPreview();
		strOutStatus = Status;
		m_strLocalPreviewCombatObjectStatus = Status;
		return false;
	};
	const f32_t fStageAgeMs = fActionAgeSeconds * 1000.f;
	for (LOCAL_PATTERN_COMBAT_OBJECT_INSTANCE& Instance :
		m_LocalPreviewCombatObjectInstances)
	{
		if (fStageAgeMs <
			static_cast<f32_t>(Instance.Template.iFirstSpawnOffsetMs))
		{
			if (0u != Instance.iActiveHandle)
			{
				EFFECT_WORLD_ROOT_HANDLE Handle;
				Handle.iValue = Instance.iActiveHandle;
				CEffectPresentationService::Stop_WorldRoot(Handle);
				Instance.iActiveHandle = 0u;
			}
			for (uint64_t& iTerminalHandle : Instance.TerminalHandles)
			{
				if (0u == iTerminalHandle)
					continue;
				EFFECT_WORLD_ROOT_HANDLE Handle;
				Handle.iValue = iTerminalHandle;
				CEffectPresentationService::Stop_WorldRoot(Handle);
				iTerminalHandle = 0u;
			}
			std::fill(Instance.TerminalAttempts.begin(),
				Instance.TerminalAttempts.end(), false);
			continue;
		}
		const f32_t fObjectAgeSeconds = fActionAgeSeconds -
			static_cast<f32_t>(Instance.Template.iFirstSpawnOffsetMs) * 0.001f;
		const f32_t fObjectAgeMs = fObjectAgeSeconds * 1000.f;
		const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* Visual =
			CActorCatalog::Find_BossCombatObjectVisual(
				m_strArchetypeId,
				Instance.Template.strCombatObjectArchetypeId,
				Instance.Template.strClientVisualId);
		if (nullptr == Visual ||
			Visual->effectAssetId != Instance.Template.strActiveEffectAssetId ||
			Visual->hitEffectAssetId !=
				Instance.Template.strTerminalEffectAssetId)
		{
			return Rollback(
				"Local combat-object preview catalog visual changed after staging.");
		}
		const float4x4_t Root = Visual->Make_WorldRoot(
			Instance.vPosition, Instance.fYawDegrees);
		if (fObjectAgeMs < static_cast<f32_t>(Instance.Template.iLifetimeMs))
		{
			if (0u == Instance.iActiveHandle)
			{
				EFFECT_WORLD_ROOT_SPAWN_DESC Desc;
				Desc.strEffectAssetId = Visual->effectAssetId;
				Desc.pBossBudgetAndLifetimeOwner = Owner;
				Desc.RootWorld = Root;
				Desc.strOccurrenceId =
					"valtan:local-preview:combat-object:generation:" +
					std::to_string(m_iLocalPreviewEffectGeneration) + "/stage:" +
					std::to_string(m_iLocalPreviewStageIndex) + "/archetype:" +
					Instance.Template.strCombatObjectArchetypeId + "/ordinal:" +
					std::to_string(Instance.iOrdinal) + "/active";
				Desc.iSpawnTick = m_iLocalPreviewEffectGeneration;
				Desc.fInitialSampleTimeSeconds = fObjectAgeSeconds;
				EFFECT_WORLD_ROOT_HANDLE Handle;
				std::string Status;
				if (!CEffectPresentationService::Spawn_WorldRoot(
						Desc, Handle, Status) ||
					!CEffectPresentationService::Seek_WorldRoot(
						Handle, fObjectAgeSeconds))
				{
					CEffectPresentationService::Stop_WorldRoot(Handle);
					return Rollback(Status.empty() ?
						"Local combat-object active Effect seek failed." : Status);
				}
				Instance.iActiveHandle = Handle.iValue;
			}
			else
			{
				EFFECT_WORLD_ROOT_HANDLE Handle;
				Handle.iValue = Instance.iActiveHandle;
				if (!CEffectPresentationService::Seek_WorldRoot(
						Handle, fObjectAgeSeconds))
				{
					return Rollback(
						"Local combat-object active Effect left its preview handle.");
				}
			}
		}
		else if (0u != Instance.iActiveHandle)
		{
			EFFECT_WORLD_ROOT_HANDLE Handle;
			Handle.iValue = Instance.iActiveHandle;
			CEffectPresentationService::Stop_WorldRoot(Handle);
			Instance.iActiveHandle = 0u;
		}

		for (size_t iEvent = 0u;
			iEvent < Instance.Template.PresentationEvents.size(); ++iEvent)
		{
			const LOCAL_PATTERN_COMBAT_OBJECT_EVENT& Event =
				Instance.Template.PresentationEvents[iEvent];
			if (fObjectAgeMs < static_cast<f32_t>(Event.iAtMs))
				continue;
			const f32_t fTerminalAgeSeconds = fObjectAgeSeconds -
				static_cast<f32_t>(Event.iAtMs) * 0.001f;
			if (!Instance.TerminalAttempts[iEvent])
			{
				Instance.TerminalAttempts[iEvent] = true;
				EFFECT_WORLD_ROOT_SPAWN_DESC Desc;
				Desc.strEffectAssetId = Visual->hitEffectAssetId;
				Desc.pBossBudgetAndLifetimeOwner = Owner;
				Desc.RootWorld = Root;
				Desc.strOccurrenceId =
					"valtan:local-preview:combat-object:generation:" +
					std::to_string(m_iLocalPreviewEffectGeneration) + "/stage:" +
					std::to_string(m_iLocalPreviewStageIndex) + "/archetype:" +
					Instance.Template.strCombatObjectArchetypeId + "/ordinal:" +
					std::to_string(Instance.iOrdinal) + "/event:" +
					Event.strPresentationEventId;
				Desc.iSpawnTick = m_iLocalPreviewEffectGeneration;
				Desc.fInitialSampleTimeSeconds = fTerminalAgeSeconds;
				EFFECT_WORLD_ROOT_HANDLE Handle;
				std::string Status;
				if (!CEffectPresentationService::Spawn_WorldRoot(
						Desc, Handle, Status) ||
					!CEffectPresentationService::Seek_WorldRoot(
						Handle, fTerminalAgeSeconds))
				{
					CEffectPresentationService::Stop_WorldRoot(Handle);
					return Rollback(Status.empty() ?
						"Local combat-object terminal Effect seek failed." : Status);
				}
				Instance.TerminalHandles[iEvent] = Handle.iValue;
			}
			else if (0u != Instance.TerminalHandles[iEvent])
			{
				EFFECT_WORLD_ROOT_HANDLE Handle;
				Handle.iValue = Instance.TerminalHandles[iEvent];
				/* A missing terminal handle means its authored NATURAL lifetime has
				   completed. Keep the attempt consumed until an explicit rewind. */
				if (!CEffectPresentationService::Seek_WorldRoot(
						Handle, fTerminalAgeSeconds))
				{
					Instance.TerminalHandles[iEvent] = 0u;
				}
			}
		}
	}
	strOutStatus = "Mirrored " +
		std::to_string(m_LocalPreviewCombatObjectInstances.size()) +
		" local combat-object Effect root(s).";
	m_strLocalPreviewCombatObjectStatus = strOutStatus;
	return true;
}

bool_t CValtan::Stage_LocalPatternAuthoringPreview(
	const Client::VALTAN_PATTERN_VIEW& Pattern,
	std::string& strOutStatus)
{
	if (m_isServerAuthoritative || nullptr == m_pBodyModelCom ||
		nullptr == m_pTransformCom ||
		Pattern.strPatternId.empty() || Pattern.Stages.empty())
	{
		strOutStatus =
			"Local Pattern draft preview requires one non-authoritative Valtan model and a non-empty Pattern.";
		return false;
	}
	std::unordered_map<std::string,
		std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP>> StagedBindings;
	std::unordered_map<std::string,
		std::vector<Client::VALTAN_PATTERN_EFFECT_CUE>> StagedEffectCues;
	std::unordered_map<std::string, uint32_t> StagedStageIndices;
	std::unordered_map<std::string, float3_t> StagedArenaCenters;
	std::unordered_map<std::string,
		std::vector<LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE>>
		StagedCombatObjects;
	size_t iStagedCombatObjectCount = 0u;
#ifdef _DEBUG
	std::unordered_map<std::string, PATTERN_HIT_AREA_DEBUG> StagedHitAreas;
#endif
	for (std::size_t iStage = 0u; iStage < Pattern.Stages.size(); ++iStage)
	{
		const Client::VALTAN_STAGE_VIEW& Stage = Pattern.Stages[iStage];
		if (Stage.strActionId.empty() ||
			StagedBindings.contains(Stage.strActionId))
		{
			strOutStatus =
				"Local Pattern draft preview rejected a missing or duplicated action: " +
				Stage.strActionId + ".";
			return false;
		}
		std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP> Clips;
		if (!Stage.bSuppressAnimation)
		{
			if (Stage.ClipOccurrences.empty())
			{
				strOutStatus =
					"Local Pattern draft preview has an unbound animation action: " +
					Stage.strActionId + ".";
				return false;
			}
			Clips.reserve(Stage.ClipOccurrences.size());
			for (const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Source :
				Stage.ClipOccurrences)
			{
				Client::BOSS_PATTERN_ANIMATION_CLIP Clip;
				Clip.strClipOccurrenceId = Source.strClipOccurrenceId;
				Clip.strClipName = Source.strClipName;
				Clip.strMappingBasis = Source.strMappingBasis;
				Clip.iSourceStartMs = Source.iSourceStartMs;
				Clip.iPlayMs = Source.iPlayMs;
				Clip.fPlayRate = Source.fPlayRate;
				Clip.bLoop = Source.bLoop;
				Clips.push_back(std::move(Clip));
			}
			std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
			if (!Build_PatternTimeline(
					m_pBodyModelCom,
					std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
						Clips.data(), Clips.size()), Timings))
			{
				strOutStatus =
					"Local Pattern draft preview rejected a clip/source clock for " +
					Stage.strActionId + ".";
				return false;
			}
		}
		for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Source :
			Stage.ProductCues)
		{
			if (Source.strPatternId != Pattern.strPatternId ||
				Source.strStageId != Stage.strStageId ||
				Source.strActionId != Stage.strActionId ||
				Source.strOccurrenceId.empty() ||
				Source.strEffectAssetId.empty())
			{
				strOutStatus =
					"Local Pattern draft preview rejected an Effect cue identity for " +
					Stage.strActionId + ".";
				return false;
			}
			if (Source.bUsesStageClock &&
				(!Stage.ClipOccurrences.empty() ||
				 !Source.strClipOccurrenceId.empty() ||
				 Source.iStageOffsetMs >= Stage.iDurationMs))
			{
				strOutStatus =
					"Local Pattern draft preview rejected a stage-clock Effect cue binding: " +
					Source.strOccurrenceId + ".";
				return false;
			}
			if (!Source.bUsesStageClock && std::none_of(
					Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
					[&Source](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
					{
						return Clip.strClipOccurrenceId == Source.strClipOccurrenceId;
					}))
			{
				strOutStatus =
					"Local Pattern draft preview rejected an Effect cue clip occurrence: " +
					Source.strOccurrenceId + ".";
				return false;
			}
			if (!Is_ArenaCenterCueAnchor(Source.strAnchorSlotId) &&
				!Is_PatternTargetSnapshotCueAnchor(Source.strAnchorSlotId) &&
				"root" != Source.strAnchorSlotId &&
				!m_pBodyModelCom->Has_Bone(Source.strAnchorSlotId.c_str()))
			{
				strOutStatus =
					"Local Pattern draft preview rejected an Effect cue anchor: " +
					Source.strOccurrenceId + ".";
				return false;
			}

			Client::VALTAN_PATTERN_EFFECT_CUE Cue;
			Cue.strBindingId = Source.strBindingId;
			Cue.strOccurrenceId = Source.strOccurrenceId;
			Cue.strPatternId = Source.strPatternId;
			Cue.strStageId = Source.strStageId;
			Cue.strActionId = Source.strActionId;
			Cue.strClipOccurrenceId = Source.strClipOccurrenceId;
			Cue.strEffectAssetId = Source.strEffectAssetId;
			Cue.strV1EffectAssetId = Source.strV1EffectAssetId;
			Cue.strAnchorSlotId = Source.strAnchorSlotId;
			Cue.LocalTransform = Source.LocalTransform;
			Cue.eFollowPolicy = Source.eFollowPolicy;
			Cue.eStopPolicy = Source.eStopPolicy;
			Cue.eRepeatPolicy = "each_loop" == Source.strRepeatPolicy ?
				Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP :
				Client::VALTAN_PATTERN_EFFECT_REPEAT_POLICY::ONCE;
			Cue.eScalePolicy = Source.eScalePolicy;
			Cue.vWorldScale = Source.vWorldScale;
			Cue.bHasExplicitScalePolicy = Source.bHasExplicitScalePolicy;
			Cue.iStartMs = Source.bUsesStageClock ?
				Source.iStageOffsetMs : Source.iSourceStartMs;
			Cue.iEndMs = Source.iSourceEndMs;
			Cue.bHasSourceEnd = Source.bHasSourceEnd;
			Cue.bUsesStageClock = Source.bUsesStageClock;
			Cue.iStageIndex = static_cast<uint32_t>(iStage);
			Cue.iStageDurationMs = Stage.iDurationMs;
			StagedEffectCues[Stage.strActionId].push_back(std::move(Cue));
		}
		for (const Client::VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Source :
			Stage.CombatObjectEffects)
		{
			/* Player-relative volleys need Server-resolved player poses and remain
			   intentionally absent from local preview. Boss-relative radial rows are
			   fully deterministic from the fixed stage-enter boss pose. */
			if ("BOSS_RELATIVE" != Source.strVolleyPolicy ||
				"RADIAL" != Source.strVolleyLayout)
			{
				continue;
			}
			const bool_t bFiniteLayout =
				std::isfinite(Source.fVolleyRadiusM) &&
				std::isfinite(Source.fVolleyStartAngleDegrees) &&
				std::isfinite(Source.fVolleyAngleStepDegrees);
			if ("ENTER" != Source.strTrigger || Source.iSpawnValue < 2u ||
				Source.iSpawnValue > 8u || !bFiniteLayout ||
				Source.fVolleyRadiusM <= 0.f ||
				Source.fVolleyAngleStepDegrees <= 0.f ||
				Source.fVolleyAngleStepDegrees *
					static_cast<f32_t>(Source.iSpawnValue) > 360.00001f ||
				0u == Source.iLifetimeMs ||
				Source.iFirstSpawnOffsetMs >= Stage.iDurationMs)
			{
				strOutStatus =
					"Local Pattern draft preview rejected a boss-relative combat-object volley: " +
					Source.strCombatObjectArchetypeId + ".";
				return false;
			}
			const BOSS_COMBAT_OBJECT_VISUAL_ENTRY* Visual =
				CActorCatalog::Find_BossCombatObjectVisual(
					m_strArchetypeId,
					Source.strCombatObjectArchetypeId,
					Source.strClientVisualId);
			if (nullptr == Visual || Visual->effectAssetId.empty() ||
				Visual->effectAssetId != Source.strEffectAssetId ||
				(!Source.PresentationEvents.empty() &&
				 Visual->hitEffectAssetId.empty()))
			{
				strOutStatus =
					"Local Pattern draft preview rejected a combat-object visual join: " +
					Source.strCombatObjectArchetypeId + ".";
				return false;
			}
			LOCAL_PATTERN_COMBAT_OBJECT_TEMPLATE Template;
			Template.strCombatObjectArchetypeId =
				Source.strCombatObjectArchetypeId;
			Template.strClientVisualId = Source.strClientVisualId;
			Template.strActiveEffectAssetId = Visual->effectAssetId;
			Template.strTerminalEffectAssetId = Visual->hitEffectAssetId;
			Template.iCount = Source.iSpawnValue;
			Template.fRadiusM = Source.fVolleyRadiusM;
			Template.fStartAngleDegrees =
				Source.fVolleyStartAngleDegrees;
			Template.fAngleStepDegrees = Source.fVolleyAngleStepDegrees;
			Template.iFirstSpawnOffsetMs = Source.iFirstSpawnOffsetMs;
			Template.iLifetimeMs = Source.iLifetimeMs;
			for (const Client::VALTAN_COMBAT_OBJECT_PRESENTATION_EVENT_VIEW&
				Event : Source.PresentationEvents)
			{
				if (Event.strPresentationEventId.empty() ||
					Event.iAtMs > Source.iLifetimeMs)
				{
					strOutStatus =
						"Local Pattern draft preview rejected a combat-object presentation clock: " +
						Source.strCombatObjectArchetypeId + ".";
					return false;
				}
				LOCAL_PATTERN_COMBAT_OBJECT_EVENT StagedEvent;
				StagedEvent.strPresentationEventId =
					Event.strPresentationEventId;
				StagedEvent.iAtMs = Event.iAtMs;
				Template.PresentationEvents.push_back(std::move(StagedEvent));
			}
			StagedCombatObjects[Stage.strActionId].push_back(
				std::move(Template));
			iStagedCombatObjectCount += Source.iSpawnValue;
		}
		StagedStageIndices.emplace(
			Stage.strActionId, static_cast<uint32_t>(iStage));
		StagedBindings.emplace(Stage.strActionId, std::move(Clips));
#ifdef _DEBUG
		const bool_t bHasStageHit = !Stage.strHitShape.empty() &&
			"NONE" != Stage.strHitShape &&
			(0u != Stage.iHitCount || Stage.bHasHitActivation);
		if (bHasStageHit || Stage.CounterProxy.has_value())
		{
			PATTERN_HIT_AREA_DEBUG Area{};
			if (bHasStageHit)
			{
				Area.strHitShape = Stage.strHitShape;
				Area.fOuterRadius = Stage.fHitOuterRadius;
				Area.fInnerRadius = Stage.fHitInnerRadius;
				Area.fAngleDegrees = Stage.fHitAngleDegrees;
				Area.fLength = Stage.fHitLength;
				Area.fHalfWidth = Stage.fHitHalfWidth;
				Area.iHitCount = Stage.iHitCount;
				Area.iHitIntervalMs = Stage.iHitIntervalMs;
				Area.iHitDelayMs = Stage.iHitDelayMs;
				Area.HitOffsetsMs = Stage.HitOffsetsMs;
				Area.iStageDurationMs = Stage.iDurationMs;
				Area.bHasActivation = Stage.bHasHitActivation;
				Area.iActivationStartMs = Stage.iHitActivationStartMs;
				Area.iActivationLifetimeMs =
					Stage.iHitActivationLifetimeMs;
			}
			if (Stage.CounterProxy.has_value())
			{
				Area.bHasCounterProxy = true;
				Area.strCounterProxyKind = Stage.CounterProxy->strKind;
				Area.fCounterProxyForwardOffsetM =
					Stage.CounterProxy->fForwardOffsetM;
				Area.fCounterProxyRightOffsetM =
					Stage.CounterProxy->fRightOffsetM;
				Area.fCounterProxyRadiusM =
					Stage.CounterProxy->fRadiusM;
				Area.fCounterProxyArcDegrees =
					Stage.CounterProxy->fArcDegrees;
			}
			StagedHitAreas.emplace(Stage.strActionId, std::move(Area));
		}
#endif
	}
	if (Pattern.ServerMotion.has_value() &&
		"LEAP_TO_ANCHOR" == Pattern.ServerMotion->strKind)
	{
		const auto& Landing = Pattern.ServerMotion->LandingPosition;
		if (!std::isfinite(Landing[0]) || !std::isfinite(Landing[1]) ||
			!std::isfinite(Landing[2]))
		{
			strOutStatus =
				"Local Pattern draft preview rejected a non-finite arena landing anchor.";
			return false;
		}
		StagedArenaCenters.emplace(Pattern.strPatternId,
			float3_t(Landing[0], Landing[1], Landing[2]));
	}
	for (const auto& [ActionId, Cues] : StagedEffectCues)
	{
		for (const Client::VALTAN_PATTERN_EFFECT_CUE& Cue : Cues)
		{
			if (Is_ArenaCenterCueAnchor(Cue.strAnchorSlotId) &&
				!StagedArenaCenters.contains(Pattern.strPatternId))
			{
				strOutStatus =
					"Local Pattern draft preview has no admitted arena anchor for Effect cue: " +
					Cue.strOccurrenceId + ".";
				return false;
			}
		}
	}

	if (m_bLocalPatternAuthoringPreview)
		Reset_LocalPatternPreviewTransport();
	m_LocalPreviewClipByActionId = std::move(StagedBindings);
	m_LocalPreviewEffectCuesByActionId = std::move(StagedEffectCues);
	m_LocalPreviewStageIndexByActionId = std::move(StagedStageIndices);
	m_LocalPreviewArenaCenterAnchors = std::move(StagedArenaCenters);
	m_LocalPreviewCombatObjectsByActionId = std::move(StagedCombatObjects);
	m_strLocalPreviewPatternId = Pattern.strPatternId;
	m_strLocalPreviewActionId.clear();
	m_iLocalPreviewStageIndex = 0u;
	m_iLocalPreviewEffectGeneration =
		m_iLocalPreviewEffectGeneration == (std::numeric_limits<uint32_t>::max)() ?
			1u : m_iLocalPreviewEffectGeneration + 1u;
#ifdef _DEBUG
	m_LocalPreviewHitAreaByActionId = std::move(StagedHitAreas);
#endif
	m_bLocalPatternAuthoringPreview = true;
	m_iPatternPresentationClipOccurrenceIndex =
		(std::numeric_limits<std::size_t>::max)();
	strOutStatus = "Staged effective Pattern draft animation/collider/Effect mirror for local preview: " +
		Pattern.strPatternId + " (" +
		std::to_string(iStagedCombatObjectCount) +
		" local combat-object root(s)).";
	return true;
}

bool_t CValtan::Apply_LocalCombatObjectAuthoringPreviewSample(
	const std::string_view actionId,
	const f32_t fActionAgeSeconds,
	const bool_t bResetTransport,
	std::string& strOutStatus)
{
	strOutStatus.clear();
	const auto Stage = m_LocalPreviewStageIndexByActionId.find(
		std::string(actionId));
	if (m_isServerAuthoritative || !m_bLocalPatternAuthoringPreview ||
		actionId.empty() ||
		m_LocalPreviewStageIndexByActionId.end() == Stage ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f)
	{
		strOutStatus =
			"Local independent combat-object preview sample is invalid.";
		return false;
	}

	if (bResetTransport || m_strLocalPreviewActionId != actionId)
		Reset_LocalPatternPreviewTransport();
	m_strLocalPreviewActionId.assign(actionId);
	m_iLocalPreviewStageIndex = Stage->second;
	return Sync_LocalPatternCombatObjectPreview(
		actionId, fActionAgeSeconds, strOutStatus);
}

void CValtan::Reset_LocalPatternPreviewTransport()
{
	if (m_isServerAuthoritative || !m_bLocalPatternAuthoringPreview)
		return;
	const std::shared_ptr<CValtan> Owner =
		static_pointer_cast<CValtan>(shared_from_this());
	Stop_LocalPatternCombatObjectPreview();
	CEffectPresentationService::Stop_BossOwner(Owner);
	Client::CEffectV2Runtime::Reset_LocalPreviewTarget(
		Client::EFFECT_V2_TARGET::From_Valtan(
			Owner));
	m_AttemptedPatternEffectOccurrenceKeys.clear();
	m_bPatternEffectCueScanAgeValid = false;
	m_fPatternEffectCueScanAgeSeconds = 0.f;
	m_strLocalPreviewActionId.clear();
	m_iLocalPreviewStageIndex = 0u;
	if (nullptr != m_pBodyModelCom)
	{
		m_iPatternPresentationClipOccurrenceIndex =
			(std::numeric_limits<std::size_t>::max)();
	}
}

void CValtan::Reset_LocalPatternPresentationSample()
{
	if (m_isServerAuthoritative || nullptr == m_pBodyModelCom)
		return;
	Reset_LocalPatternPreviewTransport();
	m_iPatternPresentationClipOccurrenceIndex =
		(std::numeric_limits<std::size_t>::max)();
	m_bLocalPatternAuthoringPreview = false;
	m_LocalPreviewClipByActionId.clear();
	m_LocalPreviewEffectCuesByActionId.clear();
	m_LocalPreviewStageIndexByActionId.clear();
	m_LocalPreviewArenaCenterAnchors.clear();
	m_LocalPreviewCombatObjectsByActionId.clear();
	m_LocalPreviewCombatObjectInstances.clear();
	m_strLocalPreviewCombatObjectActionId.clear();
	m_strLocalPreviewCombatObjectStatus.clear();
	m_strLocalPreviewPatternId.clear();
	m_strLocalPreviewActionId.clear();
	m_iLocalPreviewStageIndex = 0u;
#ifdef _DEBUG
	m_LocalPreviewHitAreaByActionId.clear();
#endif
	m_pBodyModelCom->Set_AnimationSpeed(1.f);
	m_pBodyModelCom->Set_AnimPaused(false);
}

void CValtan::Load_PatternEffectCues()
{
	std::string Status;
	if (!Reload_PatternPresentationAuthoring(Status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern Effect cues isolated: " + Status +
			"\n").c_str());
	}
}

bool_t CValtan::Reload_PatternEffectCues_WhileAdmitted(
	std::string& Status)
{
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Document;
	if (!CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
			Document, Status))
	{
		return false;
	}

	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>> Staged;
	std::unordered_map<std::string, float3_t> StagedArenaCenters;
	const bool needsArenaCenters = std::any_of(Document.Cues.begin(), Document.Cues.end(),
		[](const VALTAN_PATTERN_EFFECT_CUE& cue)
		{ return Is_ArenaCenterCueAnchor(cue.strAnchorSlotId); });
	if (needsArenaCenters)
	{
		CEncounterPatternReference encounter;
		if (!encounter.Load(CProjectDataRoot::Resolve(
				std::filesystem::path(L"Encounters") / L"Valtan" /
				L"ValtanEncounter.json"), Status))
		{
			Status = "Valtan arena-center Product admission rejected: " + Status;
			return false;
		}
		for (const ENCOUNTER_PATTERN_REFERENCE& pattern :
			encounter.Get_Patterns())
		{
			if (!pattern.serverMotion.has_value() ||
				pattern.serverMotion->kind != "LEAP_TO_ANCHOR")
				continue;
			const std::array<f32_t, 3u>& landing =
				pattern.serverMotion->landingPosition;
			if (!std::isfinite(landing[0]) || !std::isfinite(landing[1]) ||
				!std::isfinite(landing[2]))
			{
				Status = "Valtan Product arena-center landing is not finite: " +
					pattern.patternId;
				return false;
			}
			StagedArenaCenters.emplace(pattern.patternId,
				float3_t(landing[0], landing[1], landing[2]));
		}
	}
	std::unordered_map<std::string,
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING>> TimingsByAction;
	for (const VALTAN_PATTERN_EFFECT_CUE& Cue : Document.Cues)
	{
		if ((Is_ArenaCenterCueAnchor(Cue.strAnchorSlotId) &&
			!StagedArenaCenters.contains(Cue.strPatternId)) ||
			(Is_PatternTargetSnapshotCueAnchor(Cue.strAnchorSlotId) &&
			 Cue.eFollowPolicy != EFFECT_FOLLOW_POLICY::SNAPSHOT) ||
			(!Is_ArenaCenterCueAnchor(Cue.strAnchorSlotId) &&
			 !Is_PatternTargetSnapshotCueAnchor(Cue.strAnchorSlotId) &&
			 "root" != Cue.strAnchorSlotId &&
			(nullptr == m_pBodyModelCom ||
			 !m_pBodyModelCom->Has_Bone(Cue.strAnchorSlotId.c_str()))))
		{
			Status = "Valtan pattern Effect cue anchor rejected: " +
				Cue.strBindingId;
			return false;
		}
		const auto Binding = m_PatternClipByActionId.find(Cue.strActionId);
		if (m_PatternClipByActionId.end() == Binding)
		{
			Status = "Valtan pattern Effect cue action binding rejected: " +
				Cue.strOccurrenceId;
			return false;
		}
		if (Cue.bUsesStageClock)
		{
			if (!Binding->second.empty() || !Cue.strClipOccurrenceId.empty() ||
				Cue.iStartMs >= Cue.iStageDurationMs)
			{
				Status = "Valtan stage-clock Effect cue NONE binding rejected: " +
					Cue.strOccurrenceId;
				return false;
			}
			Staged[Cue.strActionId].push_back(Cue);
			continue;
		}
		const auto Clip = std::find_if(Binding->second.begin(),
			Binding->second.end(),
			[&Cue](const Client::BOSS_PATTERN_ANIMATION_CLIP& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		if (Binding->second.end() == Clip)
		{
			Status = "Valtan pattern Effect cue clip occurrence rejected: " +
				Cue.strOccurrenceId;
			return false;
		}
		if (!Cue.bUsesLegacyStageWallTime)
		{
			auto Timings = TimingsByAction.find(Cue.strActionId);
			if (TimingsByAction.end() == Timings)
			{
				std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Built;
				if (!Build_PatternTimeline(m_pBodyModelCom,
						std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
							Binding->second.data(), Binding->second.size()),
						Built))
				{
					Status = "Valtan pattern Effect cue timeline rejected: " +
						Cue.strOccurrenceId;
					return false;
				}
				Timings = TimingsByAction.emplace(
					Cue.strActionId, std::move(Built)).first;
			}
			const std::size_t iClipIndex = static_cast<std::size_t>(
				Clip - Binding->second.begin());
			f32_t fSourceDurationSeconds = 0.f;
			f32_t fWallDurationSeconds = 0.f;
			f32_t fCueStartWallSeconds = 0.f;
			if (iClipIndex >= Timings->second.size() ||
				!Client::CActionPresentationTimeline::Resolve_ClipDuration(
					Timings->second[iClipIndex],
					fSourceDurationSeconds, fWallDurationSeconds) ||
				static_cast<f32_t>(Cue.iStartMs) * 0.001f >=
					Timings->second[iClipIndex].fSourceStartSeconds +
					fSourceDurationSeconds ||
				!Client::CActionPresentationTimeline::Resolve_CueWallOffset(
					Timings->second, iClipIndex,
					static_cast<f32_t>(Cue.iStartMs) * 0.001f,
					0u, fCueStartWallSeconds) ||
				fCueStartWallSeconds * 1000.f >=
					static_cast<f32_t>(Cue.iStageDurationMs))
			{
				Status = "Valtan pattern Effect cue source start rejected: " +
					Cue.strOccurrenceId;
				return false;
			}
			if (Cue.bHasSourceEnd)
			{
				f32_t fCueEndWallSeconds = 0.f;
				if (!Client::CActionPresentationTimeline::Resolve_CueEndWallOffset(
						Timings->second, iClipIndex,
						static_cast<f32_t>(Cue.iEndMs) * 0.001f,
						0u, fCueEndWallSeconds) ||
					fCueEndWallSeconds <= fCueStartWallSeconds ||
					(!Timings->second[iClipIndex].bLoop &&
					 fCueEndWallSeconds * 1000.f >
						static_cast<f32_t>(Cue.iStageDurationMs) + 0.01f))
				{
					Status = "Valtan pattern Effect cue source end rejected: " +
						Cue.strOccurrenceId;
					return false;
				}
			}
		}
		Staged[Cue.strActionId].push_back(Cue);
	}
	for (auto& [ActionId, Cues] : Staged)
	{
		std::sort(Cues.begin(), Cues.end(),
			[](const VALTAN_PATTERN_EFFECT_CUE& Left,
				const VALTAN_PATTERN_EFFECT_CUE& Right)
			{
				return std::tie(Left.strClipOccurrenceId,
					Left.iStartMs, Left.strOccurrenceId) <
					std::tie(Right.strClipOccurrenceId,
						Right.iStartMs, Right.strOccurrenceId);
			});
	}
	m_PatternEffectCuesByActionId = std::move(Staged);
	m_PatternArenaCenterAnchors = std::move(StagedArenaCenters);
	m_AttemptedPatternEffectOccurrenceKeys.clear();
	m_bPatternEffectCueScanAgeValid = false;
	m_fPatternEffectCueScanAgeSeconds = 0.f;
	Status = "Reloaded " +
		std::to_string(m_PatternEffectCuesByActionId.size()) +
		" Valtan Pattern Effect action binding(s).";
	return true;
}

void CValtan::Spawn_DuePatternEffectCues(const f32_t fActionAgeSeconds)
{
	const bool_t bLocalPreview =
		!m_isServerAuthoritative && m_bLocalPatternAuthoringPreview;
	const std::string& PatternId = bLocalPreview ?
		m_strLocalPreviewPatternId : m_strServerPatternId;
	const std::string& ActionId = bLocalPreview ?
		m_strLocalPreviewActionId : m_strServerActionId;
	const uint32_t iStageIndex = bLocalPreview ?
		m_iLocalPreviewStageIndex : m_iServerPatternStageIndex;
	if ((!m_isServerAuthoritative && !bLocalPreview) ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f ||
		PatternId.empty() || ActionId.empty() ||
		(!bLocalPreview &&
			(0u == m_iServerActionStartTick || 0u == m_iServerPatternSequence)))
	{
		return;
	}
	const bool_t bHasPreviousActionAge =
		m_bPatternEffectCueScanAgeValid &&
		m_fPatternEffectCueScanAgeSeconds <= fActionAgeSeconds + 0.00001f;
	const f32_t fPreviousActionAgeSeconds =
		m_fPatternEffectCueScanAgeSeconds;
	m_bPatternEffectCueScanAgeValid = true;
	m_fPatternEffectCueScanAgeSeconds = fActionAgeSeconds;
	const auto& EffectCues = bLocalPreview ?
		m_LocalPreviewEffectCuesByActionId : m_PatternEffectCuesByActionId;
	const auto Found = EffectCues.find(ActionId);
	if (EffectCues.end() == Found)
		return;
	const auto& ClipBindings = bLocalPreview ?
		m_LocalPreviewClipByActionId : m_PatternClipByActionId;
	const auto Binding = ClipBindings.find(ActionId);
	if (ClipBindings.end() == Binding)
		return;
	std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Binding->second.empty() && !Build_PatternTimeline(m_pBodyModelCom,
			std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
				Binding->second.data(), Binding->second.size()), Timings))
	{
		return;
	}
	const std::shared_ptr<CValtan> Owner =
		std::static_pointer_cast<CValtan>(shared_from_this());
	for (const VALTAN_PATTERN_EFFECT_CUE& Cue : Found->second)
	{
		if (Cue.strPatternId != PatternId ||
			Cue.iStageIndex != iStageIndex)
		{
			continue;
		}
		std::size_t iClipIndex = 0u;
		if (Cue.bUsesStageClock)
		{
			if (!Binding->second.empty())
				continue;
		}
		else
		{
			const auto Clip = std::find_if(Binding->second.begin(),
				Binding->second.end(),
				[&Cue](const Client::BOSS_PATTERN_ANIMATION_CLIP& Candidate)
				{
					return Candidate.strClipOccurrenceId ==
						Cue.strClipOccurrenceId;
				});
			if (Binding->second.end() == Clip)
				continue;
			iClipIndex = static_cast<std::size_t>(
				Clip - Binding->second.begin());
			if (iClipIndex >= Timings.size())
				continue;
		}
		const std::string* pEffectAssetId = &Cue.strEffectAssetId;
#ifdef _DEBUG
		if (!m_isServerAuthoritative &&
			Is_PatternEffectV1AuditionEnabled() &&
			!Cue.strV1EffectAssetId.empty())
		{
			pEffectAssetId = &Cue.strV1EffectAssetId;
		}
#endif

		f32_t fFirstOccurrenceWallSeconds = 0.f;
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fLoopWallDurationSeconds = 0.f;
		if (Cue.bUsesStageClock || Cue.bUsesLegacyStageWallTime)
		{
			fFirstOccurrenceWallSeconds =
				static_cast<f32_t>(Cue.iStartMs) * 0.001f;
		}
		else
		{
			if (!Client::CActionPresentationTimeline::Resolve_ClipDuration(
					Timings[iClipIndex], fSourceDurationSeconds,
					fLoopWallDurationSeconds) ||
				!Client::CActionPresentationTimeline::Resolve_CueWallOffset(
					Timings, iClipIndex,
					static_cast<f32_t>(Cue.iStartMs) * 0.001f,
					0u, fFirstOccurrenceWallSeconds))
			{
				continue;
			}
			if (VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy && !Timings[iClipIndex].bLoop)
			{
				continue;
			}
		}
		const f32_t fPlaybackRate =
			(Cue.bUsesStageClock || Cue.bUsesLegacyStageWallTime) ?
			1.f : Timings[iClipIndex].fPlayRate;
		f32_t fLiveSourceDurationSeconds = 0.f;
		if (Cue.bHasSourceEnd)
		{
			fLiveSourceDurationSeconds =
				static_cast<f32_t>(Cue.iEndMs - Cue.iStartMs) * 0.001f;
		}
		else if (!CEffectPresentationService::
			Try_Get_PreparedProductDurationSeconds(
				*pEffectAssetId, fLiveSourceDurationSeconds))
		{
			OutputDebugStringA((
				"[Client][Valtan] natural pattern Effect duration is not prepared: " +
				*pEffectAssetId + "\n").c_str());
			continue;
		}

		VALTAN_PATTERN_EFFECT_OCCURRENCE_SCAN_DESC ScanDesc;
		ScanDesc.fPreviousActionAgeSeconds = fPreviousActionAgeSeconds;
		ScanDesc.fCurrentActionAgeSeconds = fActionAgeSeconds;
		ScanDesc.fFirstOccurrenceWallSeconds = fFirstOccurrenceWallSeconds;
		ScanDesc.fLoopWallDurationSeconds = fLoopWallDurationSeconds;
		ScanDesc.fPlaybackRate = fPlaybackRate;
		ScanDesc.fLiveSourceDurationSeconds = fLiveSourceDurationSeconds;
		ScanDesc.bHasPreviousActionAge = bHasPreviousActionAge;
		ScanDesc.bEachLoop =
			VALTAN_PATTERN_EFFECT_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy;
		std::vector<VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE> Samples;
		if (!Resolve_ValtanPatternEffectOccurrenceScan(ScanDesc, Samples))
			continue;

		const uint32_t iCueDurationMs = Cue.bHasSourceEnd ?
			Cue.iEndMs - Cue.iStartMs : 0u;
		for (const VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE& Sample : Samples)
		{
			const std::string AttemptKey = Cue.strOccurrenceId + "/loop:" +
				std::to_string(Sample.iLoopEpoch);
			if (m_AttemptedPatternEffectOccurrenceKeys.contains(AttemptKey))
				continue;

			/* An accepted server occurrence gets one presentation attempt. Retrying
			   an isolated preparation/clone failure on every snapshot could duplicate
			   a request whose pending commit already succeeded. */
			m_AttemptedPatternEffectOccurrenceKeys.insert(AttemptKey);

			EFFECT_SPAWN_DESC Desc;
			Desc.strEffectAssetId = *pEffectAssetId;
			Desc.pBossOwner = Owner;
			Desc.strAnchorSlotId = Cue.strAnchorSlotId;
			Desc.LocalTransform = Cue.LocalTransform;
			Desc.eFollowPolicy = Cue.eFollowPolicy;
			Desc.eScalePolicy = Cue.eScalePolicy;
			Desc.vWorldScale = Cue.vWorldScale;
			Desc.eStopPolicy = Cue.eStopPolicy;
			Desc.iCueDurationMs = iCueDurationMs;
			Desc.iActionStartTick = bLocalPreview ?
				m_iLocalPreviewEffectGeneration : m_iServerActionStartTick;
			Desc.iCueStartMs = Cue.iStartMs;
			Desc.strOccurrenceId = bLocalPreview ?
				("valtan:local-preview:generation:" +
					std::to_string(m_iLocalPreviewEffectGeneration) + "/stage:" +
					std::to_string(iStageIndex) + "/cue:" + Cue.strOccurrenceId +
					"/loop:" + std::to_string(Sample.iLoopEpoch)) :
				("valtan:action-start:" +
					std::to_string(m_iServerActionStartTick) + "/sequence:" +
					std::to_string(m_iServerPatternSequence) + "/stage:" +
					std::to_string(m_iServerPatternStageIndex) + "/cue:" +
					Cue.strOccurrenceId + "/loop:" +
					std::to_string(Sample.iLoopEpoch));
			Desc.fPlaybackRate = fPlaybackRate;
			Desc.fInitialSampleTimeSeconds = Sample.fInitialSampleSeconds;
			std::string Status;
			bool_t spawned = false;
			if (Is_PatternTargetSnapshotCueAnchor(Cue.strAnchorSlotId))
			{
				float4x4_t anchor{}, root{};
				if (!bLocalPreview && m_bServerPatternTargetIdentityStable &&
					m_bHasServerPatternTargetSnapshotPose &&
					m_iServerPatternTargetPoseSequence == m_iServerPatternSequence &&
					m_iServerPatternTargetNetEntityId !=
						LostArk::Shared::INVALID_NET_ENTITY_ID &&
					std::isfinite(m_vServerPatternTargetSnapshotPosition.x) &&
					std::isfinite(m_vServerPatternTargetSnapshotPosition.y) &&
					std::isfinite(m_vServerPatternTargetSnapshotPosition.z) &&
					std::isfinite(m_fServerPatternTargetSnapshotYawDegrees))
				{
					/* The target basis comes from this exact network snapshot.  Its
					   facing is the parent rotation, so authored local yaw composes on
					   top through the existing typed scale-policy root builder. */
					XMStoreFloat4x4(&anchor,
						XMMatrixRotationY(XMConvertToRadians(
							m_fServerPatternTargetSnapshotYawDegrees)) *
						XMMatrixTranslation(
							m_vServerPatternTargetSnapshotPosition.x,
							m_vServerPatternTargetSnapshotPosition.y,
							m_vServerPatternTargetSnapshotPosition.z));
					if (CEffectPresentationService::Build_CueScalePolicyRoot(
							Cue.LocalTransform, Cue.eScalePolicy,
							Cue.vWorldScale, anchor, root))
					{
						EFFECT_WORLD_ROOT_HANDLE handle;
						spawned = CEffectPresentationService::Spawn_WorldRoot(
							Desc, root, handle, Status);
					}
					else
					{
						Status =
							"Pattern-target snapshot cue local transform is invalid.";
					}
				}
				else
				{
					Status =
						"Pattern-target snapshot cue has no admitted finite target pose.";
				}
			}
			else if (Is_ArenaCenterCueAnchor(Cue.strAnchorSlotId))
			{
				const auto& ArenaCenters = bLocalPreview ?
					m_LocalPreviewArenaCenterAnchors : m_PatternArenaCenterAnchors;
				const auto center = ArenaCenters.find(Cue.strPatternId);
				const bool_t bTargetFollow =
					Is_ArenaCenterTargetFollowCueAnchor(Cue.strAnchorSlotId);
				float4x4_t anchor{}, root{};
				const bool_t bHasAdmittedTargetFollowPose = bLocalPreview ||
					(m_bServerPatternTargetIdentityStable &&
					 m_bHasServerPatternTargetSnapshotPose &&
					 m_iServerPatternTargetPoseSequence == m_iServerPatternSequence &&
					 m_iServerPatternTargetNetEntityId !=
						LostArk::Shared::INVALID_NET_ENTITY_ID);
				if (center != ArenaCenters.end() &&
					(!bTargetFollow || bHasAdmittedTargetFollowPose))
				{
					const f32_t yaw = bTargetFollow ?
						(bLocalPreview ? m_fPresentationYawDegrees :
							m_fServerPatternCurrentYawDegrees) +
							VALTAN_TARGET_FOLLOW_ROOT_YAW_OFFSET_DEGREES :
						(Cue.strAnchorSlotId == "arena.center.facing" ?
							(bLocalPreview ? m_fPresentationYawDegrees :
								m_fServerPatternFacingYawDegrees) : 0.f);
					/* Position stays on the authored landing center. Fixed-facing cues
					   use the occurrence lock; target-follow cues use the latest accepted
					   Server tick yaw and retain one mutable composite root handle. */
					if (CValtanPatternEffectCueDocument::Try_BuildArenaCenterAnchor(
							Cue.strAnchorSlotId, center->second, yaw, anchor) &&
						CEffectPresentationService::Build_CueScalePolicyRoot(
						Cue.LocalTransform, Cue.eScalePolicy, Cue.vWorldScale, anchor, root))
					{
						EFFECT_WORLD_ROOT_HANDLE handle;
						spawned = CEffectPresentationService::Spawn_WorldRoot(
							Desc, root, handle, Status);
						if (spawned && bTargetFollow && !bLocalPreview)
						{
							PATTERN_TARGET_FOLLOW_EFFECT_ROOT FollowRoot;
							FollowRoot.iWorldRootHandle = handle.iValue;
							FollowRoot.iPatternSequence = m_iServerPatternSequence;
							FollowRoot.iTargetNetEntityId =
								m_iServerPatternTargetNetEntityId;
							FollowRoot.vArenaCenter = center->second;
							FollowRoot.LocalTransform = Cue.LocalTransform;
							FollowRoot.eScalePolicy = Cue.eScalePolicy;
							FollowRoot.vWorldScale = Cue.vWorldScale;
							m_PatternTargetFollowEffectRoots.push_back(
								std::move(FollowRoot));
						}
					}
					else
						Status = "Arena-center cue local transform is invalid.";
				}
				else if (bTargetFollow && !bHasAdmittedTargetFollowPose)
					Status =
						"Arena-center target-follow cue has no admitted locked target pose.";
				else
					Status = "Arena-center cue has no admitted landing anchor.";
			}
			else
				spawned = CEffectPresentationService::Spawn(Desc, Status);
			if (!spawned)
			{
				OutputDebugStringA((
					"[Client][Valtan] pattern Effect spawn isolated: " + Status +
					"\n").c_str());
			}
		}
	}
}

void CValtan::Update_PatternTargetFollowEffectRoots()
{
	auto FollowRoot = m_PatternTargetFollowEffectRoots.begin();
	while (FollowRoot != m_PatternTargetFollowEffectRoots.end())
	{
		const bool_t bSameAdmittedTarget =
			FollowRoot->iWorldRootHandle != 0u &&
			FollowRoot->iPatternSequence == m_iServerPatternSequence &&
			m_iServerPatternTargetPoseSequence == m_iServerPatternSequence &&
			m_bServerPatternTargetIdentityStable &&
			m_bHasServerPatternTargetSnapshotPose &&
			FollowRoot->iTargetNetEntityId ==
				m_iServerPatternTargetNetEntityId &&
			m_iServerPatternTargetNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID;
		if (!bSameAdmittedTarget)
		{
			/* Detaching freezes the natural Effect on its last valid root.  A
			   missing/changed target never selects a replacement or writes yaw 0. */
			FollowRoot = m_PatternTargetFollowEffectRoots.erase(FollowRoot);
			continue;
		}

		float4x4_t Anchor{}, Root{};
		if (!CValtanPatternEffectCueDocument::Try_BuildArenaCenterAnchor(
				"arena.center.target-follow", FollowRoot->vArenaCenter,
				m_fServerPatternCurrentYawDegrees +
					VALTAN_TARGET_FOLLOW_ROOT_YAW_OFFSET_DEGREES, Anchor) ||
			!CEffectPresentationService::Build_CueScalePolicyRoot(
				FollowRoot->LocalTransform, FollowRoot->eScalePolicy,
				FollowRoot->vWorldScale, Anchor, Root) ||
			!CEffectPresentationService::Update_WorldRoot(
				EFFECT_WORLD_ROOT_HANDLE{ FollowRoot->iWorldRootHandle }, Root))
		{
			/* Build/update is transactional.  The service still owns the last
			   accepted matrix, so dropping only this updater preserves it. */
			FollowRoot = m_PatternTargetFollowEffectRoots.erase(FollowRoot);
			continue;
		}
		++FollowRoot;
	}
}

void CValtan::Detach_PatternTargetFollowEffectRoots()
{
	/* NATURAL documents may still contain delayed elements.  Do not stop the
	   handles here; the final accepted matrix is their terminal root. */
	m_PatternTargetFollowEffectRoots.clear();
}

void CValtan::Load_PatternSoundCues()
{
	std::string Status;
	if (!Reload_PatternPresentationAuthoring(Status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern Sound cues isolated: " + Status +
			"\n").c_str());
	}
}

bool_t CValtan::Reload_PatternSoundCues_WhileAdmitted(
	std::string& Status)
{
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Document;
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT SourceReceipt;
	if (!CValtanPatternSoundCueDocument::Load_Source(
			Document, SourceReceipt, Status))
		return false;

	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_SOUND_CUE>> Staged;
	std::unordered_map<std::string,
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING>> TimingsByAction;
	/* Unlike Load_PatternEffectCues (one bad cue aborts the whole batch, since a
	visual cue can be gameplay-adjacent authoring that deserves a hard stop), a
	Sound cue is pure Client-only presentation with no gameplay authority behind
	it (see CSoundCueCatalog's own header comment) -- one malformed row isolating
	all 286 others' boss voice/impact sound would be a strictly worse outcome
	than just skipping that one row. Rejections below are per-cue (continue),
	not per-document (return). */
	std::size_t iRejectedCueCount = 0u;
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : Document.Cues)
	{
		const auto Binding = m_PatternClipByActionId.find(Cue.strActionId);
		if (m_PatternClipByActionId.end() == Binding)
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Sound cue action binding rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		const auto Clip = std::find_if(Binding->second.begin(),
			Binding->second.end(),
			[&Cue](const Client::BOSS_PATTERN_ANIMATION_CLIP& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		if (Binding->second.end() == Clip)
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Sound cue clip occurrence rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		auto Timings = TimingsByAction.find(Cue.strActionId);
		if (TimingsByAction.end() == Timings)
		{
			std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Built;
			if (!Build_PatternTimeline(m_pBodyModelCom,
					std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
						Binding->second.data(), Binding->second.size()),
					Built))
			{
				OutputDebugStringA((
					"[Client][Valtan] pattern Sound cue timeline rejected: " +
					Cue.strOccurrenceId + "\n").c_str());
				++iRejectedCueCount;
				continue;
			}
			Timings = TimingsByAction.emplace(
				Cue.strActionId, std::move(Built)).first;
		}
		const std::size_t iClipIndex = static_cast<std::size_t>(
			Clip - Binding->second.begin());
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fWallDurationSeconds = 0.f;
		f32_t fCueStartWallSeconds = 0.f;
		if (iClipIndex >= Timings->second.size() ||
			!Client::CActionPresentationTimeline::Resolve_ClipDuration(
				Timings->second[iClipIndex],
				fSourceDurationSeconds, fWallDurationSeconds) ||
			static_cast<f32_t>(Cue.iStartMs) * 0.001f >=
				Timings->second[iClipIndex].fSourceStartSeconds +
				fSourceDurationSeconds ||
			!Client::CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings->second, iClipIndex,
				static_cast<f32_t>(Cue.iStartMs) * 0.001f,
				0u, fCueStartWallSeconds) ||
			fCueStartWallSeconds * 1000.f >=
				static_cast<f32_t>(Cue.iStageDurationMs))
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Sound cue source start rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		if (VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy &&
			!Timings->second[iClipIndex].bLoop)
		{
			OutputDebugStringA((
				"[Client][Valtan] each_loop pattern Sound cue references a non-loop clip: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		Staged[Cue.strActionId].push_back(Cue);
	}
	if (0u != iRejectedCueCount)
	{
		OutputDebugStringA(("[Client][Valtan] pattern Sound cues: " +
			std::to_string(iRejectedCueCount) +
			" cue(s) isolated individually, remaining cues still loaded.\n").c_str());
	}
	for (auto& [ActionId, Cues] : Staged)
	{
		std::sort(Cues.begin(), Cues.end(),
			[](const VALTAN_PATTERN_SOUND_CUE& Left,
				const VALTAN_PATTERN_SOUND_CUE& Right)
			{
				return std::tie(Left.strClipOccurrenceId,
					Left.iStartMs, Left.strOccurrenceId) <
					std::tie(Right.strClipOccurrenceId,
						Right.iStartMs, Right.strOccurrenceId);
			});
	}
	m_PatternSoundCuesByActionId = std::move(Staged);
	m_PatternSoundSourceReceipt = std::move(SourceReceipt);
	m_AttemptedPatternSoundOccurrenceKeys.clear();
	m_bPatternSoundCueScanAgeValid = false;
	m_fPatternSoundCueScanAgeSeconds = 0.f;
	Status = "Reloaded " +
		std::to_string(m_PatternSoundCuesByActionId.size()) +
		" Valtan Pattern Sound action binding(s); isolated " +
		std::to_string(iRejectedCueCount) + " invalid cue(s).";
	return true;
}

void CValtan::Spawn_DuePatternSoundCues(const f32_t fActionAgeSeconds)
{
	/* No prepared-asset duration exists for a wav the way
	CEffectPresentationService::Try_Get_PreparedProductDurationSeconds gives one
	for a Product Effect (CSoundCueCatalog is a plain synchronous path lookup,
	nothing is GPU-prepared ahead of time). A fixed conservative upper bound on
	a boss voice/impact bark's real length is used instead, purely to gate the
	same "already too stale to bother" catch-up case
	Resolve_ValtanPatternEffectOccurrenceScan already handles for Effects --
	every real Voltan1/Voltan2 bark sampled during authoring is well under this. */
	constexpr f32_t ASSUMED_SOUND_DURATION_SECONDS = 4.f;

	if (!m_isServerAuthoritative ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f ||
		m_strServerPatternId.empty() || m_strServerActionId.empty() ||
		0u == m_iServerActionStartTick || 0u == m_iServerPatternSequence)
	{
		return;
	}
	const bool_t bHasPreviousActionAge =
		m_bPatternSoundCueScanAgeValid &&
		m_fPatternSoundCueScanAgeSeconds <= fActionAgeSeconds + 0.00001f;
	const f32_t fPreviousActionAgeSeconds =
		m_fPatternSoundCueScanAgeSeconds;
	m_bPatternSoundCueScanAgeValid = true;
	m_fPatternSoundCueScanAgeSeconds = fActionAgeSeconds;
	const auto Found =
		m_PatternSoundCuesByActionId.find(m_strServerActionId);
	if (m_PatternSoundCuesByActionId.end() == Found)
		return;
	const auto Binding =
		m_PatternClipByActionId.find(m_strServerActionId);
	if (m_PatternClipByActionId.end() == Binding || Binding->second.empty())
		return;
	std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Build_PatternTimeline(m_pBodyModelCom,
			std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
				Binding->second.data(), Binding->second.size()), Timings))
	{
		return;
	}
	for (const VALTAN_PATTERN_SOUND_CUE& Cue : Found->second)
	{
		if (Cue.strPatternId != m_strServerPatternId ||
			Cue.iStageIndex != m_iServerPatternStageIndex)
		{
			continue;
		}
		const auto Clip = std::find_if(Binding->second.begin(),
			Binding->second.end(),
			[&Cue](const Client::BOSS_PATTERN_ANIMATION_CLIP& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		if (Binding->second.end() == Clip)
			continue;
		const std::size_t iClipIndex = static_cast<std::size_t>(
			Clip - Binding->second.begin());
		if (iClipIndex >= Timings.size())
			continue;

		f32_t fFirstOccurrenceWallSeconds = 0.f;
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fLoopWallDurationSeconds = 0.f;
		if (!Client::CActionPresentationTimeline::Resolve_ClipDuration(
				Timings[iClipIndex], fSourceDurationSeconds,
				fLoopWallDurationSeconds) ||
			!Client::CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings, iClipIndex,
				static_cast<f32_t>(Cue.iStartMs) * 0.001f,
				0u, fFirstOccurrenceWallSeconds))
		{
			continue;
		}
		if (VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy &&
			!Timings[iClipIndex].bLoop)
		{
			continue;
		}

		VALTAN_PATTERN_EFFECT_OCCURRENCE_SCAN_DESC ScanDesc;
		ScanDesc.fPreviousActionAgeSeconds = fPreviousActionAgeSeconds;
		ScanDesc.fCurrentActionAgeSeconds = fActionAgeSeconds;
		ScanDesc.fFirstOccurrenceWallSeconds = fFirstOccurrenceWallSeconds;
		ScanDesc.fLoopWallDurationSeconds = fLoopWallDurationSeconds;
		ScanDesc.fPlaybackRate = Timings[iClipIndex].fPlayRate;
		ScanDesc.fLiveSourceDurationSeconds = ASSUMED_SOUND_DURATION_SECONDS;
		ScanDesc.bHasPreviousActionAge = bHasPreviousActionAge;
		ScanDesc.bEachLoop =
			VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy;
		std::vector<VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE> Samples;
		if (!Resolve_ValtanPatternEffectOccurrenceScan(ScanDesc, Samples))
			continue;
		if (Samples.empty())
			continue;

		const std::vector<std::string>& Variants = Cue.ResolvedAssetIds;
		if (Variants.empty())
			continue;

		for (const VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE& Sample : Samples)
		{
			const std::string AttemptKey = "valtan:action-start:" +
				std::to_string(m_iServerActionStartTick) + "/sequence:" +
				std::to_string(m_iServerPatternSequence) + "/stage:" +
				std::to_string(m_iServerPatternStageIndex) + "/cue:" +
				Cue.strOccurrenceId + "/loop:" +
				std::to_string(Sample.iLoopEpoch);
			if (!m_AttemptedPatternSoundOccurrenceKeys.insert(
					AttemptKey).second)
			{
				continue;
			}

			const std::size_t iVariant = Variants.size() == 1u ? 0u :
				(static_cast<std::size_t>(std::rand()) % Variants.size());
			const std::filesystem::path SoundPath =
				CRuntimeAssetRoot::Resolve(Variants[iVariant]);
			CGameInstance::Get().Play_Sound(SoundPath.wstring(), 1.f);
		}
	}
}

void CValtan::Load_CombatObjectSoundCues()
{
	std::string status;
	if (!Reload_PatternPresentationAuthoring(status))
	{
		OutputDebugStringA((
			"[Client][Valtan] combat-object Sound cues isolated: " + status +
			"\n").c_str());
	}
}

bool_t CValtan::Reload_CombatObjectSoundCues_WhileAdmitted(
	std::string& status)
{
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT document;
	if (!CValtanCombatObjectSoundCueDocument::Load_Source(document, status))
		return false;
	std::unordered_map<std::string, VALTAN_COMBAT_OBJECT_SOUND_CUE> staged;
	staged.reserve(document.Cues.size());
	for (const VALTAN_COMBAT_OBJECT_SOUND_CUE& cue : document.Cues)
	{
		const std::string& sourceId = cue.strHitId.empty() ?
			cue.strPresentationEventId : cue.strHitId;
		const std::string key = Make_CombatObjectSoundSourceKey(
			cue.strCombatObjectArchetypeId, sourceId);
		if (!staged.emplace(key, cue).second)
		{
			status = "Duplicate combat-object Sound source: " + cue.strBindingId;
			return false;
		}
	}
	m_CombatObjectSoundCuesBySource = std::move(staged);
	status = "Reloaded " +
		std::to_string(m_CombatObjectSoundCuesBySource.size()) +
		" combat-object Sound binding(s).";
	return true;
}

bool_t CValtan::Apply_CombatObjectPresentationEvent(
	const LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT& event,
	std::string& outStatus)
{
	using namespace LostArk::Shared;
	outStatus.clear();
	if (!m_isServerAuthoritative || 0u == event.iEventSequence ||
		COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE != event.eKind)
	{
		outStatus = "Combat-object presentation event is invalid for this boss.";
		return false;
	}
	if (event.iEventSequence <= m_iLastCombatObjectPresentationEventSequence)
		return true;
	m_iLastCombatObjectPresentationEventSequence = event.iEventSequence;

	/* HIT_PULSE owns a self-contained Server pose so the terminal Effect may
	   still be presented when the combat object despawns in the same room tick.
	   The active/root Effect remains owned by the combat-object projection;
	   this pulse is a separate natural-lifetime occurrence and therefore does
	   not change the existing despawn path. */
	bool_t hitEffectSucceeded = true;
	bool_t hitEffectConfigured = false;
	std::string hitEffectStatus;
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(m_strArchetypeId);
	if (nullptr != actor)
	{
		const auto visual = std::find_if(actor->combatObjectVisuals.begin(),
			actor->combatObjectVisuals.end(),
			[&event](const BOSS_COMBAT_OBJECT_VISUAL_ENTRY& candidate)
			{
				return candidate.combatObjectArchetypeId ==
					event.strCombatObjectArchetypeId;
			});
		/* A Product-owned V2 group owns its complete visual timeline, including
		   any terminal child.  Keep hitEffectAssetId as the V1 authoring-preview
		   fallback, but never layer that fallback over the live V2 group when the
		   Server emits the semantic presentation event used by Sound. */
		if (actor->combatObjectVisuals.end() != visual &&
			BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1 ==
				visual->activeEffectKind &&
			!visual->hitEffectAssetId.empty())
		{
			hitEffectConfigured = true;
			EFFECT_WORLD_ROOT_SPAWN_DESC desc;
			desc.strEffectAssetId = visual->hitEffectAssetId;
			desc.pBossBudgetAndLifetimeOwner =
				std::static_pointer_cast<CValtan>(shared_from_this());
			desc.RootWorld = visual->Make_WorldRoot(
				float3_t(event.fPositionX, event.fPositionY, event.fPositionZ),
				event.fYawDegrees);
			desc.strOccurrenceId = "combatobject.hit." +
				std::to_string(event.iCombatObjectId) + "." +
				std::to_string(event.iEventSequence);
			desc.iSpawnTick = event.iServerTick;
			EFFECT_WORLD_ROOT_HANDLE handle;
			hitEffectSucceeded = CEffectPresentationService::Spawn_WorldRoot(
				desc, handle, hitEffectStatus);
		}
	}

	const std::string key = Make_CombatObjectSoundSourceKey(
		event.strCombatObjectArchetypeId, event.strHitId);
	const auto found = m_CombatObjectSoundCuesBySource.find(key);
	if (m_CombatObjectSoundCuesBySource.end() == found)
	{
		/* Semantic events are broader than the currently required Sound lane.
		An unbound hit is intentionally silent; joined validation decides which
		Product hits are required and reports those gaps in the Workbench. */
		if (!hitEffectSucceeded)
		{
			outStatus = std::move(hitEffectStatus);
			return false;
		}
		if (hitEffectConfigured)
			outStatus = "Played combat-object hit Effect for Server hit " +
				event.strHitId + ".";
		return true;
	}
	const VALTAN_COMBAT_OBJECT_SOUND_CUE& cue = found->second;
	const std::vector<std::string>& variants = cue.ResolvedAssetIds;
	if (variants.empty())
	{
		outStatus = "Combat-object Sound event has no catalog asset: " +
			cue.strSoundEvent;
		return false;
	}
	const std::size_t variantIndex = static_cast<std::size_t>(
		(event.iEventSequence - 1u) % variants.size());
	const std::filesystem::path soundPath =
		CRuntimeAssetRoot::Resolve(variants[variantIndex]);
	if (soundPath.empty() ||
		FAILED(CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f)))
	{
		outStatus = (!hitEffectSucceeded ? hitEffectStatus + " " : "") +
			"Combat-object Sound asset could not play: " + variants[variantIndex];
		return false;
	}
	if (!hitEffectSucceeded)
	{
		outStatus = std::move(hitEffectStatus);
		return false;
	}
	outStatus = "Played " + cue.strBindingId + " for Server event " +
		event.strHitId + ".";
	return true;
}

void CValtan::Load_PatternShakeCues()
{
	std::string Status;
	if (!Reload_PatternPresentationAuthoring(Status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern Shake cues isolated: " + Status +
			"\n").c_str());
	}
}

bool_t CValtan::Reload_PatternShakeCues_WhileAdmitted(
	std::string& Status)
{
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT Document;
	if (!CValtanPatternShakeCueDocument::Load_Source(Document, Status))
		return false;

	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_SHAKE_CUE>> Staged;
	std::unordered_map<std::string,
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING>> TimingsByAction;
	/* Per-cue isolation, same policy as Load_PatternSoundCues. */
	std::size_t iRejectedCueCount = 0u;
	for (const VALTAN_PATTERN_SHAKE_CUE& Cue : Document.Cues)
	{
		const auto Binding = m_PatternClipByActionId.find(Cue.strActionId);
		if (m_PatternClipByActionId.end() == Binding)
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Shake cue action binding rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		const auto Clip = std::find_if(Binding->second.begin(),
			Binding->second.end(),
			[&Cue](const Client::BOSS_PATTERN_ANIMATION_CLIP& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		if (Binding->second.end() == Clip)
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Shake cue clip occurrence rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		auto Timings = TimingsByAction.find(Cue.strActionId);
		if (TimingsByAction.end() == Timings)
		{
			std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Built;
			if (!Build_PatternTimeline(m_pBodyModelCom,
					std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
						Binding->second.data(), Binding->second.size()),
					Built))
			{
				OutputDebugStringA((
					"[Client][Valtan] pattern Shake cue timeline rejected: " +
					Cue.strOccurrenceId + "\n").c_str());
				++iRejectedCueCount;
				continue;
			}
			Timings = TimingsByAction.emplace(
				Cue.strActionId, std::move(Built)).first;
		}
		const std::size_t iClipIndex = static_cast<std::size_t>(
			Clip - Binding->second.begin());
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fWallDurationSeconds = 0.f;
		f32_t fCueStartWallSeconds = 0.f;
		if (iClipIndex >= Timings->second.size() ||
			!Client::CActionPresentationTimeline::Resolve_ClipDuration(
				Timings->second[iClipIndex],
				fSourceDurationSeconds, fWallDurationSeconds) ||
			static_cast<f32_t>(Cue.iStartMs) * 0.001f >=
				Timings->second[iClipIndex].fSourceStartSeconds +
				fSourceDurationSeconds ||
			!Client::CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings->second, iClipIndex,
				static_cast<f32_t>(Cue.iStartMs) * 0.001f,
				0u, fCueStartWallSeconds) ||
			fCueStartWallSeconds * 1000.f >=
				static_cast<f32_t>(Cue.iStageDurationMs))
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Shake cue source start rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		if (VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy &&
			!Timings->second[iClipIndex].bLoop)
		{
			OutputDebugStringA((
				"[Client][Valtan] each_loop pattern Shake cue references a non-loop clip: " +
				Cue.strOccurrenceId + "\n").c_str());
			++iRejectedCueCount;
			continue;
		}
		Staged[Cue.strActionId].push_back(Cue);
	}
	if (0u != iRejectedCueCount)
	{
		OutputDebugStringA(("[Client][Valtan] pattern Shake cues: " +
			std::to_string(iRejectedCueCount) +
			" cue(s) isolated individually, remaining cues still loaded.\n").c_str());
	}
	for (auto& [ActionId, Cues] : Staged)
	{
		std::sort(Cues.begin(), Cues.end(),
			[](const VALTAN_PATTERN_SHAKE_CUE& Left,
				const VALTAN_PATTERN_SHAKE_CUE& Right)
			{
				return std::tie(Left.strClipOccurrenceId,
					Left.iStartMs, Left.strOccurrenceId) <
					std::tie(Right.strClipOccurrenceId,
						Right.iStartMs, Right.strOccurrenceId);
			});
	}
	m_PatternShakeCuesByActionId = std::move(Staged);
	m_AttemptedPatternShakeOccurrenceKeys.clear();
	m_bPatternShakeCueScanAgeValid = false;
	m_fPatternShakeCueScanAgeSeconds = 0.f;
	Status = "Reloaded " +
		std::to_string(m_PatternShakeCuesByActionId.size()) +
		" Valtan Pattern Shake action binding(s); isolated " +
		std::to_string(iRejectedCueCount) + " invalid cue(s).";
	return true;
}

void CValtan::Spawn_DuePatternShakeCues(const f32_t fActionAgeSeconds)
{
	if (!m_isServerAuthoritative ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f ||
		m_strServerPatternId.empty() || m_strServerActionId.empty() ||
		0u == m_iServerActionStartTick || 0u == m_iServerPatternSequence)
	{
		return;
	}
	const bool_t bHasPreviousActionAge =
		m_bPatternShakeCueScanAgeValid &&
		m_fPatternShakeCueScanAgeSeconds <= fActionAgeSeconds + 0.00001f;
	const f32_t fPreviousActionAgeSeconds =
		m_fPatternShakeCueScanAgeSeconds;
	m_bPatternShakeCueScanAgeValid = true;
	m_fPatternShakeCueScanAgeSeconds = fActionAgeSeconds;
	const auto Found =
		m_PatternShakeCuesByActionId.find(m_strServerActionId);
	if (m_PatternShakeCuesByActionId.end() == Found)
		return;
	const auto Binding =
		m_PatternClipByActionId.find(m_strServerActionId);
	if (m_PatternClipByActionId.end() == Binding || Binding->second.empty())
		return;
	std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
	if (!Build_PatternTimeline(m_pBodyModelCom,
			std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
				Binding->second.data(), Binding->second.size()), Timings))
	{
		return;
	}
	for (const VALTAN_PATTERN_SHAKE_CUE& Cue : Found->second)
	{
		if (Cue.strPatternId != m_strServerPatternId ||
			Cue.iStageIndex != m_iServerPatternStageIndex)
		{
			continue;
		}
		const auto Clip = std::find_if(Binding->second.begin(),
			Binding->second.end(),
			[&Cue](const Client::BOSS_PATTERN_ANIMATION_CLIP& Candidate)
			{
				return Candidate.strClipOccurrenceId ==
					Cue.strClipOccurrenceId;
			});
		if (Binding->second.end() == Clip)
			continue;
		const std::size_t iClipIndex = static_cast<std::size_t>(
			Clip - Binding->second.begin());
		if (iClipIndex >= Timings.size())
			continue;

		f32_t fFirstOccurrenceWallSeconds = 0.f;
		f32_t fSourceDurationSeconds = 0.f;
		f32_t fLoopWallDurationSeconds = 0.f;
		if (!Client::CActionPresentationTimeline::Resolve_ClipDuration(
				Timings[iClipIndex], fSourceDurationSeconds,
				fLoopWallDurationSeconds) ||
			!Client::CActionPresentationTimeline::Resolve_CueWallOffset(
				Timings, iClipIndex,
				static_cast<f32_t>(Cue.iStartMs) * 0.001f,
				0u, fFirstOccurrenceWallSeconds))
		{
			continue;
		}
		if (VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy &&
			!Timings[iClipIndex].bLoop)
		{
			continue;
		}

		VALTAN_PATTERN_EFFECT_OCCURRENCE_SCAN_DESC ScanDesc;
		ScanDesc.fPreviousActionAgeSeconds = fPreviousActionAgeSeconds;
		ScanDesc.fCurrentActionAgeSeconds = fActionAgeSeconds;
		ScanDesc.fFirstOccurrenceWallSeconds = fFirstOccurrenceWallSeconds;
		ScanDesc.fLoopWallDurationSeconds = fLoopWallDurationSeconds;
		ScanDesc.fPlaybackRate = Timings[iClipIndex].fPlayRate;
		ScanDesc.fLiveSourceDurationSeconds =
			Cue.Spec.fDurationSeconds * Timings[iClipIndex].fPlayRate;
		ScanDesc.bHasPreviousActionAge = bHasPreviousActionAge;
		ScanDesc.bEachLoop =
			VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP == Cue.eRepeatPolicy;
		std::vector<VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE> Samples;
		if (!Resolve_ValtanPatternEffectOccurrenceScan(ScanDesc, Samples))
			continue;
		if (Samples.empty())
			continue;

		for (const VALTAN_PATTERN_EFFECT_OCCURRENCE_SAMPLE& Sample : Samples)
		{
			const std::string AttemptKey = "valtan:shake:action-start:" +
				std::to_string(m_iServerActionStartTick) + "/sequence:" +
				std::to_string(m_iServerPatternSequence) + "/stage:" +
				std::to_string(m_iServerPatternStageIndex) + "/cue:" +
				Cue.strOccurrenceId + "/loop:" +
				std::to_string(Sample.iLoopEpoch);
			if (!m_AttemptedPatternShakeOccurrenceKeys.insert(
					AttemptKey).second)
			{
				continue;
			}

			/* Shakes run on the wall clock; a late snapshot joins mid-way. */
			Client::CCameraShakeService::Trigger(
				Cue.Spec,
				(std::max)(0.f, fActionAgeSeconds - Sample.fOccurrenceWallSeconds));
		}
	}
}

void CValtan::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CValtan::Trigger_HitFlash()
{
	m_fHitFlashRemainingSeconds = HIT_FLASH_DURATION_SECONDS;
	m_HitFlash.isEnabled = true;
	m_HitFlash.vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY;
	m_HitFlash.usesSurfaceDetailMask = true;
}

void CValtan::Update(f32_t fTimeDelta)
{
	if (m_fHitFlashRemainingSeconds > 0.f)
	{
		m_fHitFlashRemainingSeconds -= fTimeDelta;
		if (m_fHitFlashRemainingSeconds <= 0.f)
		{
			m_fHitFlashRemainingSeconds = 0.f;
			m_HitFlash = {};
		}
		else
		{
			m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY *
				(m_fHitFlashRemainingSeconds / HIT_FLASH_DURATION_SECONDS);
		}
	}
	if (m_isServerAuthoritative)
	{
		if (m_DeathPresentationClock.Has_Started())
			m_DeathPresentationClock.Advance(fTimeDelta);
		else
			Update_NetworkTransform(fTimeDelta);
		__super::Update(fTimeDelta);
		return;
	}
	if (nullptr == m_pNavigationCom)
	{
		m_PathFollower.Cancel();
		m_hasLastPathGoal = false;
		Set_ChaseState(false);
		__super::Update(fTimeDelta);
		return;
	}

	shared_ptr<CTransform> pTargetTransform = m_pTargetTransform.lock();
	if (nullptr == pTargetTransform)
	{
		m_PathFollower.Cancel();
		m_hasLastPathGoal = false;
		Set_ChaseState(false);
		__super::Update(fTimeDelta);
		return;
	}

	const vector_t vPosition = m_pTransformCom->Get_State(STATE::POSITION);
	const vector_t vTargetPosition = pTargetTransform->Get_State(STATE::POSITION);
	const vector_t vHorizontalOffset =
		XMVectorSetY(vTargetPosition - vPosition, 0.f);
	const f32_t fTargetDistance = XMVectorGetX(
		XMVector3Length(vHorizontalOffset));

	if (fTargetDistance <= m_fStopDistance)
	{
		m_PathFollower.Cancel();
		m_hasLastPathGoal = false;
		Set_ChaseState(false);
		__super::Update(fTimeDelta);
		return;
	}

	m_fRepathTime -= fTimeDelta;
	if (m_fRepathTime <= 0.f)
	{
		float3_t vCurrentGoal{};
		XMStoreFloat3(&vCurrentGoal, vTargetPosition);
		const f32_t fGoalDeltaX = vCurrentGoal.x - m_vLastPathGoal.x;
		const f32_t fGoalDeltaZ = vCurrentGoal.z - m_vLastPathGoal.z;
		const bool_t isNewGoal =
			false == m_hasLastPathGoal ||
			fGoalDeltaX * fGoalDeltaX + fGoalDeltaZ * fGoalDeltaZ >= 0.25f ||
			false == m_PathFollower.Has_Path();

		if (isNewGoal)
		{
			const PATH_RESULT_CODE eResult =
				Request_PathToTarget(vTargetPosition);
			if (PATH_RESULT_CODE::SUCCESS == eResult)
			{
				m_vLastPathGoal = vCurrentGoal;
				m_hasLastPathGoal = true;
			}
			else
			{
				m_PathFollower.Cancel();
				m_hasLastPathGoal = false;
			}
		}

		m_fRepathTime = 0.35f;
	}

	m_PathFollower.Update(
		m_pTransformCom,
		m_fMoveSpeed,
		fTimeDelta);
	Set_ChaseState(m_PathFollower.Has_Path());

	__super::Update(fTimeDelta);
}

void CValtan::Late_Update(f32_t fTimeDelta)
{
	/* Container Update still advances the body animation and every Effect clock.
	Only render submission is suppressed for the authoritative relocation edge;
	clearing the snapshot flag therefore restores the same presentation group. */
	if (!m_isGhostPresentationHidden && !m_isPatternBodyHidden)
		__super::Late_Update(fTimeDelta);
	Client::CEffectV2Runtime::Tick(
		Client::EFFECT_V2_TARGET::From_Valtan(
			static_pointer_cast<CValtan>(shared_from_this())),
		m_pDevice, m_pContext);
#ifdef _DEBUG
	if (m_isNavigationDebugVisible && nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
	if (m_isPatternHitPulseDebugVisible ||
		m_isPatternStageGeometryDebugVisible ||
		m_isCounterProxyDebugVisible ||
		!m_strPreviewHitActionId.empty())
		Draw_PatternHitAreaDebug();
#endif
}

HRESULT CValtan::Render()
{
	return S_OK;
}

bool_t CValtan::Try_Get_PresentationRootMatrix(float4x4_t* pOut) const
{
	if (nullptr == pOut || nullptr == m_pBodyVisualRootCom ||
		nullptr == m_pTransformCom)
	{
		return false;
	}

	XMStoreFloat4x4(
		pOut,
		XMLoadFloat4x4(m_pBodyVisualRootCom->Get_WorldMatrixPtr()) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
	return true;
}

HRESULT CValtan::Ready_PartObjects()
{
	CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};

	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;
	bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	bodyDesc.strModelPrototypeTag =
		CValtanPresentationAssetService::Get_BodyModelPrototypeTag(m_strArchetypeId);
	bodyDesc.pEmissiveOverride = &m_HitFlash;

	if (FAILED(__super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Body_Valtan"),
		BODY_PART_TAG,
		&bodyDesc)))
		return E_FAIL;

	m_pBodyModelCom =
		dynamic_pointer_cast<CModel>(
			__super::Get_Component(
				BODY_PART_TAG,
				TEXT("Com_Model")));

	m_pBodyVisualRootCom =
		dynamic_pointer_cast<CTransform>(
			__super::Get_Component(
				BODY_PART_TAG,
				g_strTransformComTag));

	if (nullptr == m_pBodyModelCom ||
		nullptr == m_pBodyVisualRootCom)
		return E_FAIL;

	/* Product Valtan receives X/Y/Z from the Server, so its authored b_root must
	not add a second horizontal or vertical displacement. Tool/non-network
	previews keep local Z because that authored fast jump remains their timing
	oracle. The body owns the skeleton every socketed part follows. */
	m_pBodyModelCom->Enable_RootMotionSuppression(
		ROOT_MOTION_BONE,
		m_isServerAuthoritative ?
			ROOT_MOTION_LOCK_ALL_AXES : ROOT_MOTION_VERTICAL_AXIS);

	CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};

	weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

	weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;

	weaponDesc.strModelTag =
		CValtanPresentationAssetService::Get_WeaponModelPrototypeTag(m_strArchetypeId);

	weaponDesc.strShaderTag = TEXT("Prototype_Component_Shader_VtxMeshBinary");

	weaponDesc.pSkeletonModel =
		m_pBodyModelCom;

	weaponDesc.pSocketBoneName =
		WEAPON_SOCKET_BONE;

	weaponDesc.pSocketRootMatrix =
		m_pBodyVisualRootCom->Get_WorldMatrixPtr();
	weaponDesc.strMaterialProfileId = "material.valtan.monster-base.v1";
	weaponDesc.pEmissiveOverride = &m_HitFlash;

	if (FAILED(__super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Part_Equipment"),
		WEAPON_PART_TAG,
		&weaponDesc)))
		return E_FAIL;

	Ready_ArmorParts();
	return S_OK;
}

bool_t CValtan::Replace_PresentationPartGroup(
	const std::string_view presentationArchetypeId,
	std::string& strOutStatus)
{
	strOutStatus.clear();
	if (presentationArchetypeId.empty())
	{
		strOutStatus = "Valtan presentation part archetype is empty.";
		return false;
	}
	if (m_strPresentationPartArchetypeId == presentationArchetypeId)
		return true;

	const BOSS_ACTOR_ENTRY* pActor =
		CActorCatalog::Find_Boss(presentationArchetypeId);
	if (nullptr == pActor ||
		pActor->clientPresentationId != "boss.valtan.client.v1")
	{
		strOutStatus = "Valtan presentation part catalog entry is unavailable: " +
			std::string(presentationArchetypeId) + ".";
		return false;
	}
	if (!CValtanPresentationAssetService::Is_Ready(
			m_iPrototypeLevelIndex, presentationArchetypeId) &&
		FAILED(CValtanPresentationAssetService::Ensure_Prototypes(
			m_pDevice, m_pContext, m_iPrototypeLevelIndex,
			presentationArchetypeId)))
	{
		strOutStatus = "Valtan presentation part resources are unavailable: " +
			std::string(presentationArchetypeId) + ".";
		return false;
	}

	PART_OBJECT_MAP StagedParts;
	std::unordered_map<uint32_t, wstring_t> StagedArmorPartTags;
	std::string StagedPresentationArchetypeId(presentationArchetypeId);

	CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;
	bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	bodyDesc.strModelPrototypeTag =
		CValtanPresentationAssetService::Get_BodyModelPrototypeTag(
			presentationArchetypeId);
	bodyDesc.pEmissiveOverride = &m_HitFlash;
	shared_ptr<CPartObject> StagedBodyPart;
	if (FAILED(__super::Clone_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Body_Valtan"),
			&bodyDesc, StagedBodyPart)))
	{
		strOutStatus = "Valtan presentation body clone failed: " +
			std::string(presentationArchetypeId) + ".";
		return false;
	}
	const shared_ptr<CModel> StagedBodyModel =
		dynamic_pointer_cast<CModel>(
			StagedBodyPart->Get_Component(TEXT("Com_Model")));
	const shared_ptr<CTransform> StagedBodyVisualRoot =
		dynamic_pointer_cast<CTransform>(
			StagedBodyPart->Get_Component(g_strTransformComTag));
	if (nullptr == StagedBodyModel || nullptr == StagedBodyVisualRoot)
	{
		strOutStatus = "Valtan presentation body components are incomplete: " +
			std::string(presentationArchetypeId) + ".";
		return false;
	}
	StagedBodyModel->Enable_RootMotionSuppression(
		ROOT_MOTION_BONE,
		m_isServerAuthoritative ?
			ROOT_MOTION_LOCK_ALL_AXES : ROOT_MOTION_VERTICAL_AXIS);
	StagedParts.emplace(BODY_PART_TAG, StagedBodyPart);

	CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
	weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	weaponDesc.strModelTag =
		CValtanPresentationAssetService::Get_WeaponModelPrototypeTag(
			presentationArchetypeId);
	weaponDesc.strShaderTag =
		TEXT("Prototype_Component_Shader_VtxMeshBinary");
	weaponDesc.pSkeletonModel = StagedBodyModel;
	weaponDesc.pSocketBoneName = WEAPON_SOCKET_BONE;
	weaponDesc.pSocketRootMatrix =
		StagedBodyVisualRoot->Get_WorldMatrixPtr();
	weaponDesc.strMaterialProfileId = "material.valtan.monster-base.v1";
	weaponDesc.pEmissiveOverride = &m_HitFlash;
	shared_ptr<CPartObject> StagedWeaponPart;
	if (FAILED(__super::Clone_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			&weaponDesc, StagedWeaponPart)) ||
		nullptr == dynamic_pointer_cast<CPart_Equipment>(StagedWeaponPart))
	{
		strOutStatus = "Valtan presentation weapon clone failed: " +
			std::string(presentationArchetypeId) + ".";
		return false;
	}
	StagedParts.emplace(WEAPON_PART_TAG, StagedWeaponPart);

	StagedArmorPartTags.reserve(pActor->armorParts.size());
	for (const BOSS_ARMOR_PART_ENTRY& armorPart : pActor->armorParts)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC armorDesc{};
		armorDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		armorDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		armorDesc.strModelTag = Build_ArmorModelPrototypeTag(
			armorPart.stateMask, presentationArchetypeId);
		armorDesc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		armorDesc.pSkeletonModel = StagedBodyModel;
		armorDesc.pSocketBoneName = nullptr;
		armorDesc.pSocketRootMatrix =
			StagedBodyVisualRoot->Get_WorldMatrixPtr();
		armorDesc.strMaterialProfileId =
			"material.valtan.monster-base.v1";
		armorDesc.pEmissiveOverride = &m_HitFlash;

		const wstring_t armorPartTag =
			Build_ArmorPartTag(armorPart.stateMask);
		shared_ptr<CPartObject> StagedArmorPart;
		if (FAILED(__super::Clone_PartObject(
				m_iPrototypeLevelIndex,
				TEXT("Prototype_GameObject_Part_Equipment"),
				&armorDesc, StagedArmorPart)) ||
			nullptr == dynamic_pointer_cast<CPart_Equipment>(StagedArmorPart) ||
			!StagedParts.emplace(
				armorPartTag, StagedArmorPart).second ||
			!StagedArmorPartTags.emplace(
				armorPart.stateMask, armorPartTag).second)
		{
			strOutStatus = "Valtan presentation armour group clone failed: " +
				armorPart.partId + ".";
			return false;
		}
	}

	/* Nothing above mutates the live part map.  One group swap publishes body,
	   weapon and the complete armour set together; any resource/clone failure
	   has already returned with the previous group and component pointers live. */
	if (FAILED(__super::Replace_PartObjectGroup(
			TEXT("Part_"), std::move(StagedParts))))
	{
		strOutStatus = "Valtan presentation part group commit failed: " +
			std::string(presentationArchetypeId) + ".";
		return false;
	}

	m_pBodyModelCom = StagedBodyModel;
	m_pBodyVisualRootCom = StagedBodyVisualRoot;
	m_ArmorPartTagsByStateMask = std::move(StagedArmorPartTags);
	m_strPresentationPartArchetypeId =
		std::move(StagedPresentationArchetypeId);
	strOutStatus = "Committed Valtan presentation part group: " +
		m_strPresentationPartArchetypeId + ".";
	return true;
}

/* The shoulder and arm plates are authored on the body rig, so they are
skinned parts: no socket bone, and the body model owns the bone palette
they render with. Breaking a plate later only has to hide its part.

A plate is presentation, so a missing or unreadable one is isolated to
itself: the boss still spawns and fights without it. Only the plates that
actually attached are recorded. */
void CValtan::Ready_ArmorParts()
{
	m_ArmorPartTagsByStateMask.clear();
	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss(m_strArchetypeId);
	if (nullptr == pActor)
		return;

	const size_t armorCount = pActor->armorParts.size();
	m_ArmorPartTagsByStateMask.reserve(armorCount);
	for (const BOSS_ARMOR_PART_ENTRY& armorPart : pActor->armorParts)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC armorDesc{};

		armorDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		armorDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		armorDesc.strModelTag =
			Build_ArmorModelPrototypeTag(armorPart.stateMask, m_strArchetypeId);
		armorDesc.strShaderTag =
			TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");
		armorDesc.pSkeletonModel = m_pBodyModelCom;
		/* nullptr keeps the piece skinned to the body skeleton. */
		armorDesc.pSocketBoneName = nullptr;
		armorDesc.pSocketRootMatrix =
			m_pBodyVisualRootCom->Get_WorldMatrixPtr();
		armorDesc.strMaterialProfileId =
			"material.valtan.monster-base.v1";
		armorDesc.pEmissiveOverride = &m_HitFlash;

		wstring_t armorPartTag = Build_ArmorPartTag(armorPart.stateMask);
		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			armorPartTag,
			&armorDesc)))
		{
			/* One unreadable plate must not cost the boss its spawn. */
			OutputDebugStringA(("[Client][Valtan] armour part " +
				armorPart.partId + " failed to attach.\n").c_str());
			continue;
		}
		m_ArmorPartTagsByStateMask.emplace(
			armorPart.stateMask, std::move(armorPartTag));
	}
	OutputDebugStringA(("[Client][Valtan] armour parts attached: " +
		std::to_string(m_ArmorPartTagsByStateMask.size()) + " / catalog " +
		std::to_string(pActor->armorParts.size()) + "\n").c_str());
}

HRESULT CValtan::Ready_Components(const f32_t collisionRadius)
{
	if (collisionRadius > 0.f)
	{
		Engine::CBounding_Sphere::BOUNDING_SPHERE_DESC colliderDesc{};
		colliderDesc.vCenter = float3_t(0.f, collisionRadius, 0.f);
		colliderDesc.fRadius = collisionRadius;
		if (FAILED(__super::Add_Component(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_Component_Collider_WorldEntity"),
			TEXT("Com_CombatCollider"),
			m_pColliderCom,
			&colliderDesc)))
		{
			return E_FAIL;
		}
	}

	if (m_strNavigationPrototypeTag.empty())
		return S_OK;

	return __super::Add_Component(
		m_iPrototypeLevelIndex,
		m_strNavigationPrototypeTag,
		TEXT("Com_Navigation"),
		m_pNavigationCom);
}

PATH_RESULT_CODE CValtan::Request_PathToTarget(fvector_t vGoalPosition)
{
	if (nullptr == m_pNavigationCom || nullptr == m_pTransformCom)
		return PATH_RESULT_CODE::INVALID_GRID;

	return m_PathFollower.Request_Path(
		m_pNavigationCom,
		m_pTransformCom->Get_State(STATE::POSITION),
		vGoalPosition);
}

void CValtan::Set_ChaseState(bool_t isChasing)
{
	const uint32_t iNextState = isChasing ?
		VALTAN_STATE::CHASE :
		VALTAN_STATE::IDLE;
	if (m_iState == iNextState)
		return;

	m_iState = iNextState;
	if (nullptr != m_pBodyModelCom)
	{
		m_pBodyModelCom->Set_Animation(
			isChasing ? "mesh_run_battle_1" : "mesh_idle_battle_1",
			true);
		Client::CEffectV2Runtime::Notify_Clip(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			isChasing ? "mesh_run_battle_1" : "mesh_idle_battle_1");
	}
}

void CValtan::Queue_NetworkTransformSample(
	const float3_t& position,
	const f32_t yawDegrees,
	const uint32_t iServerTick)
{
	bool_t reset =
		/* The relocation snapshot is admitted before its BossCombat flags in
		   CClientReplication.  While the previous accepted snapshot is hidden,
		   every queued pose therefore belongs to the teleport edge and must not
		   retain the two-tick interpolation history.  This also covers the first
		   visible snapshot: m_isGhostPresentationHidden is cleared only after
		   Apply_NetworkState has committed this sample. */
		m_isGhostPresentationHidden ||
		!m_hasNetworkTransformState || 0u == m_iNetworkSampleCount;
	if (!reset)
	{
		const NETWORK_TRANSFORM_SAMPLE& newest =
			m_NetworkSamples[m_iNetworkSampleCount - 1u];
		const f32_t dx = position.x - newest.vPosition.x;
		const f32_t dy = position.y - newest.vPosition.y;
		const f32_t dz = position.z - newest.vPosition.z;
		if (dx * dx + dy * dy + dz * dz >
				NETWORK_TELEPORT_DISTANCE_SQ ||
			(iServerTick < newest.iServerTick &&
			 CActionPresentationTimeline::Is_ForwardTick(
				iServerTick, newest.iServerTick)))
		{
			reset = true;
		}
	}

	if (reset)
	{
		m_iNetworkSampleCount = 0u;
		m_fPresentationYawDegrees = yawDegrees;
		m_fPlaybackServerTick = static_cast<f32_t>(iServerTick) -
			NETWORK_INTERPOLATION_DELAY_TICKS;
	}

	if (m_iNetworkSampleCount > 0u &&
		m_NetworkSamples[m_iNetworkSampleCount - 1u].iServerTick ==
			iServerTick)
	{
		NETWORK_TRANSFORM_SAMPLE& newest =
			m_NetworkSamples[m_iNetworkSampleCount - 1u];
		newest.vPosition = position;
		newest.fYawDegrees = yawDegrees;
	}
	else
	{
		if (NETWORK_SAMPLE_CAPACITY == m_iNetworkSampleCount)
		{
			for (std::size_t i = 1u; i < NETWORK_SAMPLE_CAPACITY; ++i)
				m_NetworkSamples[i - 1u] = m_NetworkSamples[i];
			--m_iNetworkSampleCount;
		}
		NETWORK_TRANSFORM_SAMPLE& sample =
			m_NetworkSamples[m_iNetworkSampleCount++];
		sample.iServerTick = iServerTick;
		sample.vPosition = position;
		sample.fYawDegrees = yawDegrees;
	}
	m_hasNetworkTransformState = true;
}

void CValtan::Update_NetworkTransform(const f32_t fTimeDelta)
{
	if (!m_hasNetworkTransformState || nullptr == m_pTransformCom ||
		0u == m_iNetworkSampleCount || !std::isfinite(fTimeDelta) ||
		fTimeDelta < 0.f)
	{
		return;
	}

	const f32_t oldestTick =
		static_cast<f32_t>(m_NetworkSamples[0].iServerTick);
	const f32_t newestTick = static_cast<f32_t>(
		m_NetworkSamples[m_iNetworkSampleCount - 1u].iServerTick);
	m_fPlaybackServerTick += fTimeDelta * VALTAN_SERVER_TICK_HZ;

	const f32_t targetTick =
		newestTick - NETWORK_INTERPOLATION_DELAY_TICKS;
	const f32_t drift = targetTick - m_fPlaybackServerTick;
	if (fabsf(drift) > NETWORK_PLAYBACK_SNAP_TICKS)
	{
		m_fPlaybackServerTick = targetTick;
	}
	else
	{
		m_fPlaybackServerTick += drift * (std::min)(
			1.f, NETWORK_PLAYBACK_DRIFT_GAIN * fTimeDelta);
	}
	m_fPlaybackServerTick = (std::max)(
		oldestTick,
		(std::min)(newestTick, m_fPlaybackServerTick));

	std::size_t older = m_iNetworkSampleCount - 1u;
	for (std::size_t i = 0u; i + 1u < m_iNetworkSampleCount; ++i)
	{
		if (m_fPlaybackServerTick <=
			static_cast<f32_t>(m_NetworkSamples[i + 1u].iServerTick))
		{
			older = i;
			break;
		}
	}

	const NETWORK_TRANSFORM_SAMPLE& from = m_NetworkSamples[older];
	const NETWORK_TRANSFORM_SAMPLE& to = m_NetworkSamples[
		(std::min)(older + 1u, m_iNetworkSampleCount - 1u)];
	vector_t nextPosition = XMVectorSet(
		to.vPosition.x, to.vPosition.y, to.vPosition.z, 1.f);
	f32_t targetYawDegrees = to.fYawDegrees;
	if (to.iServerTick > from.iServerTick)
	{
		const f32_t ratio =
			(m_fPlaybackServerTick - static_cast<f32_t>(from.iServerTick)) /
			static_cast<f32_t>(to.iServerTick - from.iServerTick);
		const vector_t fromPosition = XMVectorSet(
			from.vPosition.x, from.vPosition.y, from.vPosition.z, 1.f);
		nextPosition = XMVectorLerp(fromPosition, nextPosition, ratio);

		f32_t yawSpan = to.fYawDegrees - from.fYawDegrees;
		while (yawSpan > 180.f)
			yawSpan -= 360.f;
		while (yawSpan < -180.f)
			yawSpan += 360.f;
		targetYawDegrees = from.fYawDegrees + yawSpan * ratio;
	}

	m_pTransformCom->Set_State(
		STATE::POSITION, XMVectorSetW(nextPosition, 1.f));
	f32_t yawDifference = targetYawDegrees - m_fPresentationYawDegrees;
	while (yawDifference > 180.f)
		yawDifference -= 360.f;
	while (yawDifference < -180.f)
		yawDifference += 360.f;
	const f32_t yawStep = NETWORK_TURN_DEGREES_PER_SECOND * fTimeDelta;
	if (fabsf(yawDifference) <= yawStep)
		m_fPresentationYawDegrees = targetYawDegrees;
	else
		m_fPresentationYawDegrees +=
			yawDifference > 0.f ? yawStep : -yawStep;
	m_pTransformCom->Rotation(0.f, m_fPresentationYawDegrees, 0.f);

	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}
}

void CValtan::Set_ArmorPartVisible(
	const uint32_t iStateMask,
	const bool_t isVisible)
{
	const auto tag = m_ArmorPartTagsByStateMask.find(iStateMask);
	if (tag == m_ArmorPartTagsByStateMask.end())
		return;
	const auto pPart = dynamic_cast<CPart_Equipment*>(
		__super::Find_PartObject(tag->second));
	if (nullptr != pPart)
		pPart->Set_Visible(isVisible);
}

bool_t CValtan::Apply_BossCombatState(
	const LostArk::Shared::BOSS_COMBAT_SNAPSHOT& state)
{
	if (!m_isServerAuthoritative || !Is_ValidBossCombatState(state))
		return false;
	const bool_t isGhostHidden = LostArk::Shared::Has_BossCombatFlag(
		state.iFlags,
		LostArk::Shared::BOSS_COMBAT_STATE_FLAG::GHOST_HIDDEN);
	if (isGhostHidden &&
		("BOSS_VALTAN" != m_strArchetypeId ||
		 LostArk::Shared::INVALID_NET_ENTITY_ID != m_iOwnerBossNetEntityId))
	{
		return false;
	}
	if (m_hasBossCombatState)
	{
		if (state.iStateRevision < m_BossCombatState.iStateRevision)
			return false;
		if (state.iStateRevision == m_BossCombatState.iStateRevision)
			return Is_SameBossCombatState(state, m_BossCombatState);
	}
	/* Phase 3 changes only the primary boss's presentation group.  Its
	   archetype/NetEntityId, HP and Server damage authority remain untouched;
	   dependent BOSS_VALTAN_GHOST entities keep their own ownership contract. */
	if ("BOSS_VALTAN" == m_strArchetypeId &&
		LostArk::Shared::INVALID_NET_ENTITY_ID == m_iOwnerBossNetEntityId)
	{
		const std::string_view DesiredPresentationArchetype =
			state.iGameplayPhase >= 3u ?
			"BOSS_VALTAN_GHOST" : "BOSS_VALTAN";
		if (m_strPresentationPartArchetypeId != DesiredPresentationArchetype)
		{
			std::string PresentationStatus;
			if (!Replace_PresentationPartGroup(
					DesiredPresentationArchetype, PresentationStatus))
			{
				/* Presentation admission is isolated from the valid authoritative
				   combat snapshot.  The old complete group remains drawable and a
				   later state revision may retry after the Drive asset is restored. */
				OutputDebugStringA((
					"[Client][Valtan] phase presentation swap isolated: " +
					PresentationStatus + "\n").c_str());
			}
		}
	}

	if (m_isGhostPresentationHidden != isGhostHidden)
	{
		/* Relocation hides the whole presentation occurrence, not only the
		   skinned parts.  Retire target-owned V2 objects on the hidden edge and
		   re-admit the target on the visible edge; the following authoritative
		   stage snapshot rebuilds only effects that still belong to that stage. */
		CEffectV2Runtime::Set_Ignored(
			EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			isGhostHidden);
	}
	m_BossCombatState = state;
	m_hasBossCombatState = true;
	m_isGhostPresentationHidden = isGhostHidden;
	Refresh_ArmorPartVisibility();
	return true;
}

bool_t CValtan::Apply_BrokenArmorMask(const uint8_t iBrokenArmorMask)
{
	if (!m_isServerAuthoritative)
		return false;
	if (iBrokenArmorMask == m_iBrokenArmorMask)
		return true;

	m_iBrokenArmorMask = iBrokenArmorMask;
	Refresh_ArmorPartVisibility();
	return true;
}

void CValtan::Refresh_ArmorPartVisibility()
{
	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss(m_strArchetypeId);
	if (nullptr == pActor)
		return;

	/* Two server systems can retire the same plate: the world entity armour
	   durability reports it as a broken bit by authored plate index, and the
	   boss combat state clears its alive mask. The plate is worn only while
	   both still say so, so whichever system breaks it first hides the part
	   and neither can put it back on behind the other. */
	uint32_t plateIndex = 0u;
	for (const BOSS_ARMOR_PART_ENTRY& armorPart : pActor->armorParts)
	{
		const bool_t brokenByDurability =
			0u != (m_iBrokenArmorMask & (1u << plateIndex));
		const bool_t aliveInCombatState = !m_hasBossCombatState ||
			0u != (m_BossCombatState.iAlivePartMask & armorPart.stateMask);
		Set_ArmorPartVisible(armorPart.stateMask,
			aliveInCombatState && !brokenByDurability);
		++plateIndex;
	}
}

bool_t CValtan::Apply_BossCombatEvent(
	const LostArk::Shared::BOSS_COMBAT_EVENT& event)
{
	if (!m_isServerAuthoritative || 0u == event.iEventSequence ||
		0u == event.iEventTick || 0u == event.iPartMask ||
		LostArk::Shared::BOSS_COMBAT_EVENT_KIND::PART_BROKEN != event.eKind)
	{
		return false;
	}
	if (event.iEventSequence <= m_iLastBossCombatEventSequence)
		return true;

	for (const auto& [stateMask, partTag] : m_ArmorPartTagsByStateMask)
	{
		(void)partTag;
		if (0u != (event.iPartMask & stateMask))
			Set_ArmorPartVisible(stateMask, false);
	}
	m_iLastBossCombatEventSequence = event.iEventSequence;
	return true;
}

void CValtan::Transition_RaidBgm(const RAID_BGM_STATE nextState)
{
	if (!m_isRaidBgmEnabled || m_eRaidBgmState == nextState)
		return;

	const wchar_t* pAssetId = nullptr;
	bool_t bLoop = false;
	switch (nextState)
	{
	case RAID_BGM_STATE::M05_INTRO:
		pAssetId = VALTAN_BGM_M05_ASSET_ID;
		break;
	case RAID_BGM_STATE::M06_PHASE_ONE:
		pAssetId = VALTAN_BGM_M06_ASSET_ID;
		bLoop = true;
		break;
	case RAID_BGM_STATE::M07_GHOST_TRANSITION:
		pAssetId = VALTAN_BGM_M07_ASSET_ID;
		break;
	case RAID_BGM_STATE::M08_GHOST_PHASE:
		pAssetId = VALTAN_BGM_M08_ASSET_ID;
		bLoop = true;
		break;
	case RAID_BGM_STATE::M09_DEATH:
		pAssetId = VALTAN_BGM_M09_ASSET_ID;
		break;
	case RAID_BGM_STATE::NONE:
		CGameInstance::Get().Stop_Music();
		m_eRaidBgmState = nextState;
		return;
	default:
		return;
	}

	// An older death tail must not stop music claimed by a newly spawned boss.
	if (0u == ++g_iRaidBgmOwnershipGeneration)
		++g_iRaidBgmOwnershipGeneration;
	m_iRaidBgmOwnershipGeneration = g_iRaidBgmOwnershipGeneration;
	/* Commit the presentation edge even when the local resource is missing.
	The Server snapshot remains authoritative, and a missing WAV must not turn
	every following snapshot into a 30 Hz load retry. */
	m_eRaidBgmState = nextState;
	const filesystem::path MusicPath = CRuntimeAssetRoot::Resolve(pAssetId);
	if (MusicPath.empty() || !filesystem::is_regular_file(MusicPath) ||
		FAILED(CGameInstance::Get().Play_Music(
			MusicPath.wstring(), 1.f, bLoop)))
	{
#ifdef _DEBUG
		OutputDebugStringA(
			"[Client][Valtan] raid BGM transition was isolated because the "
			"runtime WAV could not be played.\n");
#endif
	}
}

void CValtan::Update_RaidBgm(
	const LostArk::Shared::WORLD_ENTITY_ACTION action,
	const std::string_view patternId,
	const std::string_view actionId)
{
	if (!m_isRaidBgmEnabled)
		return;

	const bool_t isEntrancePattern =
		VALTAN_ENTRANCE_PATTERN_ID == patternId;
	const bool_t isCinematicEntrancePattern =
		VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID == patternId ||
		VALTAN_IDLE_CINEMATIC_ENTRANCE_PATTERN_ID == patternId;
	const bool_t isGhostTransition =
		VALTAN_GHOST_TRANSITION_PATTERN_ID == patternId;
	const bool_t isGhostPhaseEdge = isGhostTransition &&
		VALTAN_GHOST_PHASE_ACTION_ID == actionId;

	if (LostArk::Shared::WORLD_ENTITY_ACTION::DEAD == action)
	{
		Transition_RaidBgm(RAID_BGM_STATE::M09_DEATH);
		return;
	}
	if (isGhostPhaseEdge)
	{
		Transition_RaidBgm(RAID_BGM_STATE::M08_GHOST_PHASE);
		return;
	}
	if (isGhostTransition)
	{
		if (RAID_BGM_STATE::M08_GHOST_PHASE != m_eRaidBgmState)
			Transition_RaidBgm(RAID_BGM_STATE::M07_GHOST_TRANSITION);
		return;
	}
	/* A packet can skip the final transition stage. Leaving M07 still proves
	the Server completed the 14-bar transition, so recover directly into M08. */
	if (RAID_BGM_STATE::M07_GHOST_TRANSITION == m_eRaidBgmState)
	{
		Transition_RaidBgm(RAID_BGM_STATE::M08_GHOST_PHASE);
		return;
	}
	/* These camera-only entrance gates deliberately add no BGM edge. Keep
	   the Level-owned M04 track until VALTAN_ENTRANCE_WHIRLWIND starts M05,
	   and do not classify its non-idle snapshots as a late join. */
	if (isCinematicEntrancePattern)
		return;
	if (isEntrancePattern)
	{
		m_hasObservedEntrancePattern = true;
		Transition_RaidBgm(RAID_BGM_STATE::M05_INTRO);
		return;
	}

	if (RAID_BGM_STATE::NONE == m_eRaidBgmState ||
		RAID_BGM_STATE::M09_DEATH == m_eRaidBgmState)
	{
		/* Do not start M05 while the authored entrance boss is still waiting
		in IDLE. The exact cutscene edge is the entrance pattern handled
		above. A non-IDLE first snapshot means this Client joined after the
		intro or an audition deliberately started inside phase one. */
		if (LostArk::Shared::WORLD_ENTITY_ACTION::IDLE != action)
		{
			m_hasObservedEntrancePattern = false;
			Transition_RaidBgm(RAID_BGM_STATE::M06_PHASE_ONE);
		}
		return;
	}

	/* M05 stays intact through the full entrance whirlwind. M06 begins on
	the first accepted post-intro snapshot, including an IDLE boundary before
	the Server selects the first normal pattern. */
	if (RAID_BGM_STATE::M05_INTRO == m_eRaidBgmState &&
		(m_hasObservedEntrancePattern ||
		 LostArk::Shared::WORLD_ENTITY_ACTION::IDLE != action))
	{
		Transition_RaidBgm(RAID_BGM_STATE::M06_PHASE_ONE);
	}
}

bool_t CValtan::Begin_NetworkDeathPresentation()
{
	if (!m_isServerAuthoritative)
		return false;
	if (m_DeathPresentationClock.Has_Started())
		return !Is_NetworkDeathPresentationComplete();
	m_iState = VALTAN_STATE::DEAD;
	m_strServerPatternId.clear();
	m_strServerActionId.clear();
	m_iServerActionStartTick = 0u;
	m_iServerPatternStageIndex = 0u;
	m_iNetworkSampleCount = 0u;
	m_hasNetworkTransformState = false;
	m_fHitFlashRemainingSeconds = 0.f;
	m_HitFlash = {};
	const auto owner = std::static_pointer_cast<CValtan>(shared_from_this());
	Detach_PatternTargetFollowEffectRoots();
	CEffectPresentationService::Stop_BossOwner(owner);
	Client::CEffectV2Runtime::Sync_Stage(
		Client::EFFECT_V2_TARGET::From_Valtan(owner), "", 0.f, m_pDevice, m_pContext);
	bool_t hasPlayableClip = false;
	f32_t durationSeconds = 0.f;
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(m_strArchetypeId);
	if (nullptr != actor && nullptr != m_pBodyModelCom &&
		!actor->presentationClips.dead.empty())
	{
		for (uint32_t index = 0u; index < m_pBodyModelCom->Get_NumAnimations(); ++index)
		{
			const char* name = m_pBodyModelCom->Get_AnimationName(index);
			if (nullptr == name || actor->presentationClips.dead != name)
				continue;
			f32_t position = 0.f, durationTicks = 0.f;
			const f32_t ticksPerSecond = m_pBodyModelCom->Get_AnimationTickPerSecond(index);
			if (!m_pBodyModelCom->Get_AnimationProgress(index, position, durationTicks) ||
				!std::isfinite(durationTicks) || durationTicks <= 0.f ||
				!std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f)
				break;
			durationSeconds = durationTicks / ticksPerSecond;
			if (!std::isfinite(durationSeconds) || durationSeconds <= 0.f)
				break;
			m_pBodyModelCom->Set_AnimPaused(false);
			m_pBodyModelCom->Set_AnimationSpeed(1.f);
			hasPlayableClip = m_pBodyModelCom->Start_Animation(index, false);
			if (hasPlayableClip)
				m_iDeathAnimationIndex = index;
			break;
		}
	}
	m_DeathPresentationClock.Start(hasPlayableClip, durationSeconds);
	if (!hasPlayableClip)
		OutputDebugStringA(("[Client][Valtan] no playable death clip; retire immediately: " +
			m_strArchetypeId + "\n").c_str());
	Update_RaidBgm(LostArk::Shared::WORLD_ENTITY_ACTION::DEAD, {}, {});
	return !Is_NetworkDeathPresentationComplete();
}

bool_t CValtan::Is_NetworkDeathPresentationComplete() const
{
	if (!m_DeathPresentationClock.Has_Started() || m_DeathPresentationClock.Is_Complete() ||
		nullptr == m_pBodyModelCom ||
		m_pBodyModelCom->Get_CurrentAnimIndex() != m_iDeathAnimationIndex ||
		m_pBodyModelCom->Is_AnimLoop() || m_pBodyModelCom->Is_AnimPaused())
		return true;
	f32_t position = 0.f, duration = 0.f;
	return !m_pBodyModelCom->Get_AnimationProgress(m_iDeathAnimationIndex, position, duration) ||
		!std::isfinite(position) || !std::isfinite(duration) ||
		duration <= 0.f || position >= duration;
}

bool_t CValtan::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees,
	const LostArk::Shared::WORLD_ENTITY_ACTION action,
	const std::string_view patternId,
	const std::string_view actionId,
	const uint32_t iServerTick,
	const uint32_t iActionStartTick,
	const uint32_t iPatternSequence,
	const uint32_t iPatternStageIndex,
	const PATTERN_TARGET_SNAPSHOT_POSE& PatternTargetPose)
{
	if (!m_isServerAuthoritative || nullptr == m_pTransformCom ||
		!std::isfinite(position.x) || !std::isfinite(position.y) ||
		!std::isfinite(position.z) || !std::isfinite(yawDegrees))
	{
		return false;
	}

	uint32_t nextState = VALTAN_STATE::IDLE;
	switch (action)
	{
	case LostArk::Shared::WORLD_ENTITY_ACTION::IDLE:
		nextState = VALTAN_STATE::IDLE;
		break;
	case LostArk::Shared::WORLD_ENTITY_ACTION::CHASE:
		nextState = VALTAN_STATE::CHASE;
		break;
	case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_WINDUP:
		nextState = VALTAN_STATE::PATTERN_WINDUP;
		break;
	case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_ACTIVE:
		nextState = VALTAN_STATE::PATTERN_ACTIVE;
		break;
	case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_RECOVERY:
		nextState = VALTAN_STATE::PATTERN_RECOVERY;
		break;
	case LostArk::Shared::WORLD_ENTITY_ACTION::DEAD:
		nextState = VALTAN_STATE::DEAD;
		break;
	default:
		return false;
	}

	const bool_t isPatternState =
		VALTAN_STATE::PATTERN_WINDUP == nextState ||
		VALTAN_STATE::PATTERN_ACTIVE == nextState ||
		VALTAN_STATE::PATTERN_RECOVERY == nextState;
	if (0u != m_iLastServerTick &&
		(0u == iServerTick ||
		 (iServerTick != m_iLastServerTick &&
		  !Client::CActionPresentationTimeline::Is_ForwardTick(
			  iServerTick, m_iLastServerTick))))
	{
		return false;
	}
	if (VALTAN_STATE::DEAD == nextState)
	{
		Detach_PatternTargetFollowEffectRoots();
		m_iServerPatternTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		m_iServerPatternTargetPoseSequence = 0u;
		m_bHasServerPatternTargetSnapshotPose = false;
		m_bServerPatternTargetIdentityStable = false;
		if (!m_DeathPresentationClock.Has_Started())
		{
			m_pTransformCom->Set_State(STATE::POSITION,
				XMVectorSet(position.x, position.y, position.z, 1.f));
			m_pTransformCom->Rotation(0.f, yawDegrees, 0.f);
			m_fPresentationYawDegrees = yawDegrees;
			Begin_NetworkDeathPresentation();
		}
		if (0u != iServerTick)
			m_iLastServerTick = iServerTick;
		return true;
	}
	if (m_DeathPresentationClock.Has_Started())
		return false;
	f32_t fActionAgeSeconds = 0.f;
	if (isPatternState)
	{
		if (patternId.empty() || actionId.empty() || 0u == iServerTick ||
			0u == iActionStartTick || 0u == iPatternSequence ||
			!Client::CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				iServerTick, iActionStartTick, VALTAN_SERVER_TICK_HZ,
				fActionAgeSeconds))
		{
			return false;
		}
		if (0u != m_iServerPatternSequence)
		{
			if (iPatternSequence != m_iServerPatternSequence &&
				!Client::CActionPresentationTimeline::Is_ForwardTick(
					iPatternSequence, m_iServerPatternSequence))
			{
				return false;
			}
			if (iPatternSequence == m_iServerPatternSequence &&
				(m_strServerPatternId != patternId ||
				 iPatternStageIndex < m_iServerPatternStageIndex ||
				 (iPatternStageIndex == m_iServerPatternStageIndex &&
				  m_iServerActionStartTick != 0u &&
				  iActionStartTick != m_iServerActionStartTick)))
			{
				return false;
			}
		}
	}

	const bool_t actionIdChanged = m_strServerActionId != actionId;
	const bool_t patternIdChanged = m_strServerPatternId != patternId;
	const bool_t patternEdgeChanged = isPatternState &&
		(patternIdChanged || actionIdChanged ||
		 iPatternSequence != m_iServerPatternSequence ||
		 iPatternStageIndex != m_iServerPatternStageIndex ||
		 iActionStartTick != m_iServerActionStartTick);
	/* A pattern's stages can share one entity action kind (two ACTIVE stages in
	a row), so the stage's actionId is what marks a clip change there. */
	const bool_t bAnimationEdgeChanged =
		m_iState != nextState || patternEdgeChanged;
	std::size_t iAcceptedPatternClipOccurrenceIndex =
		m_iPatternPresentationClipOccurrenceIndex;
	bool_t bCommitPatternClipOccurrenceIndex = false;
	const bool_t bEnteredDead =
		VALTAN_STATE::DEAD == nextState && m_iState != nextState;
	const uint32_t iPreviousActionStartTick = m_iServerActionStartTick;
	if (nullptr != m_pBodyModelCom &&
		(bAnimationEdgeChanged ||
		 (isPatternState && fActionAgeSeconds >
			m_fServerActionAgeSeconds +
				VALTAN_PRESENTATION_SEEK_EPSILON_SECONDS)))
	{
		const BOSS_ACTOR_ENTRY* pActor =
			CActorCatalog::Find_Boss(m_strArchetypeId);
		if (nullptr == pActor)
			return false;
		const std::string* pClip =
			Resolve_ValtanPresentationClip(*pActor, action);
		if (nullptr == pClip)
			return false;
		if (!isPatternState)
		{
			m_pBodyModelCom->Set_AnimPaused(false);
			if (bAnimationEdgeChanged &&
				!m_pBodyModelCom->Start_Animation(pClip->c_str(), true))
			{
				return false;
			}
			if (bAnimationEdgeChanged)
			{
				Client::CEffectV2Runtime::Notify_Clip(
					Client::EFFECT_V2_TARGET::From_Valtan(
						static_pointer_cast<CValtan>(shared_from_this())),
					pClip->c_str());
			}
			m_pBodyModelCom->Set_AnimationSpeed(1.f);
		}
		else
		{
			if (!Apply_PatternPresentationSample(
					actionId,
					*pClip,
					fActionAgeSeconds,
					bAnimationEdgeChanged,
					m_iPatternPresentationClipOccurrenceIndex,
					iAcceptedPatternClipOccurrenceIndex))
			{
				return false;
			}
			bCommitPatternClipOccurrenceIndex = true;
		}
	}

	/* Every earlier validation/animation seek has succeeded, so this snapshot is
	   accepted.  Retire the previous authoritative action before queuing any cue
	   for the replacement edge.  This also removes A from the deferred queue when
	   an A -> B backlog is consumed in one frame. */
	if (0u != iPreviousActionStartTick &&
		(patternEdgeChanged || !isPatternState))
	{
		CEffectPresentationService::Stop_BossAction(
			std::static_pointer_cast<CValtan>(shared_from_this()),
			 iPreviousActionStartTick);
	}
	if (!isPatternState || iPatternSequence != m_iServerPatternSequence)
		Detach_PatternTargetFollowEffectRoots();

	Queue_NetworkTransformSample(position, yawDegrees, iServerTick);
	m_strServerActionId.assign(actionId);
	m_iState = nextState;
	if (0u != iServerTick)
		m_iLastServerTick = iServerTick;
	if (isPatternState)
	{
		if (iPatternSequence != m_iServerPatternSequence)
			m_fServerPatternFacingYawDegrees = yawDegrees;
		m_fServerPatternCurrentYawDegrees = yawDegrees;
		m_strServerPatternId.assign(patternId);
		m_iServerActionStartTick = iActionStartTick;
		m_iServerPatternSequence = iPatternSequence;
		m_iServerPatternStageIndex = iPatternStageIndex;
		m_fServerActionAgeSeconds = fActionAgeSeconds;
		m_isPatternBodyHidden = false;
		const auto BodyVisibility =
			m_PatternBodyVisibilityByActionId.find(m_strServerActionId);
		if (m_PatternBodyVisibilityByActionId.end() != BodyVisibility)
		{
			const f32_t fActionAgeMs = fActionAgeSeconds * 1000.f;
			m_isPatternBodyHidden =
				fActionAgeMs >= static_cast<f32_t>(
					BodyVisibility->second.iHiddenFromMs) &&
				fActionAgeMs < static_cast<f32_t>(
					BodyVisibility->second.iHiddenToMs);
		}
		const bool_t bIncomingTargetPoseFinite =
			PatternTargetPose.bHasFinitePose &&
			PatternTargetPose.iNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			std::isfinite(PatternTargetPose.vPosition.x) &&
			std::isfinite(PatternTargetPose.vPosition.y) &&
			std::isfinite(PatternTargetPose.vPosition.z) &&
			std::isfinite(PatternTargetPose.fYawDegrees);
		if (m_iServerPatternTargetPoseSequence != iPatternSequence)
		{
			m_iServerPatternTargetPoseSequence = iPatternSequence;
			m_iServerPatternTargetNetEntityId =
				PatternTargetPose.iNetEntityId;
			m_bServerPatternTargetIdentityStable = true;
			m_bHasServerPatternTargetSnapshotPose =
				bIncomingTargetPoseFinite;
		}
		else if (m_bServerPatternTargetIdentityStable &&
			PatternTargetPose.iNetEntityId !=
				m_iServerPatternTargetNetEntityId)
		{
			/* A virtual snapshot anchor is permitted only for a target locked
			   to the pattern occurrence.  Never recover a changed identity in
			   the same sequence by silently selecting a different player. */
			m_bServerPatternTargetIdentityStable = false;
			m_bHasServerPatternTargetSnapshotPose = false;
		}
		else if (m_bServerPatternTargetIdentityStable)
		{
			m_bHasServerPatternTargetSnapshotPose =
				bIncomingTargetPoseFinite;
		}
		if (m_bHasServerPatternTargetSnapshotPose)
		{
			m_vServerPatternTargetSnapshotPosition =
				PatternTargetPose.vPosition;
			m_fServerPatternTargetSnapshotYawDegrees =
				PatternTargetPose.fYawDegrees;
		}
		if (bCommitPatternClipOccurrenceIndex)
		{
			m_iPatternPresentationClipOccurrenceIndex =
				 iAcceptedPatternClipOccurrenceIndex;
		}
		Update_PatternTargetFollowEffectRoots();
	}
	else
	{
		m_isPatternBodyHidden = false;
		m_iServerActionStartTick = 0u;
		m_iServerPatternStageIndex = 0u;
		m_fServerActionAgeSeconds = 0.f;
		m_iServerPatternTargetNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		m_iServerPatternTargetPoseSequence = 0u;
		m_fServerPatternCurrentYawDegrees = 0.f;
		m_bHasServerPatternTargetSnapshotPose = false;
		m_bServerPatternTargetIdentityStable = false;
		m_iPatternPresentationClipOccurrenceIndex =
			(std::numeric_limits<std::size_t>::max)();
		m_bPatternEffectCueScanAgeValid = false;
		m_fPatternEffectCueScanAgeSeconds = 0.f;
	}
	if (isPatternState)
	{
		if (patternEdgeChanged)
		{
			m_AttemptedPatternEffectOccurrenceKeys.clear();
			m_bPatternEffectCueScanAgeValid = false;
			m_fPatternEffectCueScanAgeSeconds = 0.f;
			m_AttemptedPatternSoundOccurrenceKeys.clear();
			m_bPatternSoundCueScanAgeValid = false;
			m_fPatternSoundCueScanAgeSeconds = 0.f;
			m_AttemptedPatternShakeOccurrenceKeys.clear();
			m_bPatternShakeCueScanAgeValid = false;
			m_fPatternShakeCueScanAgeSeconds = 0.f;
		}
		Spawn_DuePatternEffectCues(fActionAgeSeconds);
		Spawn_DuePatternSoundCues(fActionAgeSeconds);
		Spawn_DuePatternShakeCues(fActionAgeSeconds);
		std::vector<Client::EFFECT_V2_CLIP_OCCURRENCE_CLOCK> EffectV2Clocks;
		if (!Build_EffectV2OccurrenceClocksForAction(
				m_pBodyModelCom, m_PatternClipByActionId,
				m_strServerActionId, EffectV2Clocks))
		{
			OutputDebugStringA((
				"[Client][Valtan] Effect V2 occurrence clock rejected for action: " +
				m_strServerActionId + "\n").c_str());
			EffectV2Clocks.clear();
		}
		Client::CEffectV2Runtime::Sync_Stage(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			m_strServerActionId.c_str(), fActionAgeSeconds,
			EffectV2Clocks,
			m_pDevice, m_pContext);
	}
	else
	{
		Client::CEffectV2Runtime::Sync_Stage(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			"", 0.f, m_pDevice, m_pContext);
	}
	if (!isPatternState && bEnteredDead)
	{
		CEffectPresentationService::Stop_BossOwner(
			std::static_pointer_cast<CValtan>(shared_from_this()));
	}
	Update_RaidBgm(action, patternId, actionId);
	return true;
}

unique_ptr<CValtan> CValtan::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CValtan>(new CValtan(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][Valtan] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CValtan::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CValtan>(new CValtan(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][Valtan] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
