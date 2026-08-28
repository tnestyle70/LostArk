#include "Valtan.h"

#include "ActionPresentationTimeline.h"
#include "ActorCatalog.h"
#include "AnimationSkillBindingDocument.h"
#include "Body_Valtan.h"
#include "Collider.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "Model.h"
#include "Navigation.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"
#include <filesystem>
#ifdef _DEBUG
#include "EncounterPatternReference.h"
#include "HitAreaWire.h"
#include "ProjectDataRoot.h"
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
	constexpr f32_t VALTAN_SERVER_TICK_HZ = 30.f;
	constexpr f32_t VALTAN_PRESENTATION_SEEK_EPSILON_SECONDS = 1.f / 120.f;
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
	constexpr f32_t NETWORK_INTERPOLATION_DELAY_TICKS = 2.f;
	constexpr f32_t NETWORK_PLAYBACK_SNAP_TICKS = 6.f;
	constexpr f32_t NETWORK_PLAYBACK_DRIFT_GAIN = 4.f;
	constexpr f32_t NETWORK_TELEPORT_DISTANCE_SQ = 100.f;
	constexpr f32_t NETWORK_TURN_DEGREES_PER_SECOND = 720.f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
	constexpr int32_t ROOT_MOTION_LOCK_ALL_AXES = -1;
	constexpr const char_t* VALTAN_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_WHIRLWIND";
	constexpr const char_t* VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID =
		"VALTAN_ENTRANCE_CINEMATIC";
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
		return 0u != state.iStateRevision &&
			0u == (state.iFlags & static_cast<std::uint16_t>(
				~LostArk::Shared::BOSS_COMBAT_STATE_KNOWN_FLAG_MASK)) &&
			state.iCurrentStagger <= state.iMaximumStagger &&
			state.iCurrentShield <= state.iMaximumShield &&
			(hasShield == (0u != state.iCurrentShield)) &&
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
			left.iGameplayPhase) ==
			std::tie(
				right.iStateRevision,
				right.iAlivePartMask,
				right.iFlags,
				right.iCurrentStagger,
				right.iMaximumStagger,
				right.iCurrentShield,
				right.iMaximumShield,
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

wstring_t CValtan::Build_ArmorModelPrototypeTag(const uint32_t iStateMask)
{
	return TEXT("Prototype_Component_Model_ValtanArmorMask_") +
		std::to_wstring(iStateMask);
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
	if (m_isRaidBgmEnabled && RAID_BGM_STATE::NONE != m_eRaidBgmState)
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
	m_isRaidBgmEnabled = m_isServerAuthoritative &&
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
	Load_PatternBindings();
	Load_PatternEffectCues();
	Load_PatternSoundCues();
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
			if (stage.actionId.empty() || stage.hitShape.empty() ||
				"NONE" == stage.hitShape || 0u == stage.iHitCount)
			{
				continue;
			}
			PATTERN_HIT_AREA_DEBUG area{};
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
	const auto iter = m_PatternHitAreaByActionId.find(strActionId);
	if (m_PatternHitAreaByActionId.end() == iter)
		return;
	const PATTERN_HIT_AREA_DEBUG& area = iter->second;

	/* Mirror the Server schedule exactly. Non-uniform source contacts own an
	   ordered stage-relative offset vector; an empty vector retains the legacy
	   delay + k * interval schedule. */
	constexpr f32_t MIN_VISIBLE_HIT_WINDOW_MS = 300.f;
	const f32_t fAgeMs = (isPreviewDriven ?
		m_fPreviewHitAgeSeconds : m_fServerActionAgeSeconds) * 1000.f;
	bool_t isHitWindow = false;
	for (uint32_t iTick = 0u; iTick < area.iHitCount; ++iTick)
	{
		const uint64_t iTickMs = area.HitOffsetsMs.empty() ?
			static_cast<uint64_t>(area.iHitDelayMs) +
				static_cast<uint64_t>(iTick) * area.iHitIntervalMs :
			area.HitOffsetsMs[iTick];
		const f32_t fTickMs = static_cast<f32_t>(iTickMs);
		if (fAgeMs >= fTickMs && fAgeMs <= fTickMs + MIN_VISIBLE_HIT_WINDOW_MS)
		{
			isHitWindow = true;
			break;
		}
	}
	if (!isHitWindow)
		return;

	constexpr uint32_t PATTERN_HIT_COLOR_RGBA =
		255u | (60u << 8) | (200u << 16) | (255u << 24);
	constexpr f32_t METERS_TO_UNITS = 100.f;
	const matrix_t World =
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	const vector_t vPosition = World.r[3];
	const vector_t vLook = XMVector3Normalize(XMVectorSetY(World.r[2], 0.f));

	const auto ToUnits = [](const f32_t fMeters)
	{
		return static_cast<int32_t>(fMeters * METERS_TO_UNITS + 0.5f);
	};
	const auto Draw_WithYawOffset = [&](const f32_t fYawOffsetDegrees,
		const HIT_AREA_SHAPE& Shape)
	{
		const vector_t vRotatedLook = XMVector3Rotate(vLook,
			XMQuaternionRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f),
				XMConvertToRadians(fYawOffsetDegrees)));
		const vector_t vRight = XMVector3Normalize(XMVector3Cross(
			XMVectorSet(0.f, 1.f, 0.f, 0.f), vRotatedLook));
		float4x4_t Root{};
		XMStoreFloat4x4(&Root, XMMatrixSet(
			XMVectorGetX(vRight), 0.f, XMVectorGetZ(vRight), 0.f,
			0.f, 1.f, 0.f, 0.f,
			XMVectorGetX(vRotatedLook), 0.f, XMVectorGetZ(vRotatedLook), 0.f,
			XMVectorGetX(vPosition), XMVectorGetY(vPosition),
			XMVectorGetZ(vPosition), 1.f));
		CHitAreaWire::Draw(Root, Shape, PATTERN_HIT_COLOR_RGBA);
	};

	HIT_AREA_SHAPE Shape{};
	if ("CIRCLE" == area.strHitShape || "RING" == area.strHitShape)
	{
		Shape.iAreaType = 1;
		Shape.iAreaRange = ToUnits(area.fOuterRadius);
		Shape.iAreaInner = ToUnits(area.fInnerRadius);
		Draw_WithYawOffset(0.f, Shape);
	}
	else if ("CONE" == area.strHitShape)
	{
		Shape.iAreaType = 3;
		Shape.iAreaRange = ToUnits(area.fLength);
		Shape.iAreaAngle = static_cast<int32_t>(area.fAngleDegrees + 0.5f);
		Draw_WithYawOffset(0.f, Shape);
	}
	else if ("BOX" == area.strHitShape)
	{
		Shape.iAreaType = 2;
		Shape.iAreaRange = ToUnits(area.fLength);
		Shape.iAreaAngle = ToUnits(area.fHalfWidth * 2.f);
		Draw_WithYawOffset(0.f, Shape);
	}
	else if ("CROSS" == area.strHitShape ||
		"SIX_DIRECTIONS" == area.strHitShape)
	{
		/* The server tests centered strips spanning [-length, +length] along
		   each strip axis and +-halfWidth across: forward and right for CROSS,
		   forward and the two 60-degree diagonals for SIX_DIRECTIONS. */
		Shape.iAreaType = 2;
		Shape.iAreaOffsetX = -ToUnits(area.fLength);
		Shape.iAreaRange = ToUnits(area.fLength * 2.f);
		Shape.iAreaAngle = ToUnits(area.fHalfWidth * 2.f);
		Draw_WithYawOffset(0.f, Shape);
		if ("CROSS" == area.strHitShape)
			Draw_WithYawOffset(90.f, Shape);
		else
		{
			Draw_WithYawOffset(60.f, Shape);
			Draw_WithYawOffset(-60.f, Shape);
		}
	}
}

void CValtan::Set_PatternHitAreaPreview(
	const std::string& stageActionId,
	const f32_t fStageAgeSeconds)
{
	/* A Development preview boss never took the server-authoritative load in
	   Initialize, so the display copy is admitted on first use here. */
	if (!m_isPatternHitAreaDebugLoadAttempted)
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
	if (nullptr == m_pBodyModelCom)
		return;
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
	std::string status;
	if (!Client::CValtanPatternAnimationBindingDocument::Load(
			"Valtan", "BOSS_VALTAN", availableClips, document, status))
	{
		const std::string message =
			"[Client][Valtan] pattern bindings rejected; catalog clips remain: " +
			status + "\n";
		OutputDebugStringA(message.c_str());
		return;
	}
	std::unordered_map<std::string,
		std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP>> staged;
	for (const Client::BOSS_PATTERN_ANIMATION_BINDING& binding :
		document.Bindings)
	{
		if (binding.bSuppressAnimation)
		{
			staged.emplace(binding.strActionId,
				std::vector<Client::BOSS_PATTERN_ANIMATION_CLIP>{});
			continue;
		}
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING> Timings;
		if (!Build_PatternTimeline(m_pBodyModelCom,
				std::span<const Client::BOSS_PATTERN_ANIMATION_CLIP>(
					binding.Clips.data(), binding.Clips.size()), Timings))
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern binding source segment rejected: " +
				binding.strActionId + "\n").c_str());
			return;
		}
		staged.emplace(binding.strActionId, binding.Clips);
	}
	m_PatternClipByActionId = std::move(staged);
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
		const auto Bound = m_PatternClipByActionId.find(std::string(actionId));
		if (m_PatternClipByActionId.end() != Bound)
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
	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss("BOSS_VALTAN");
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
	Client::CEffectV2Runtime::Sync_Stage(
		Client::EFFECT_V2_TARGET::From_Valtan(
			static_pointer_cast<CValtan>(shared_from_this())),
		std::string(actionId).c_str(), fActionAgeSeconds,
		m_pDevice, m_pContext);
	return true;
}

void CValtan::Reset_LocalPatternPresentationSample()
{
	if (m_isServerAuthoritative || nullptr == m_pBodyModelCom)
		return;
	Client::CEffectV2Runtime::Sync_Stage(
		Client::EFFECT_V2_TARGET::From_Valtan(
			static_pointer_cast<CValtan>(shared_from_this())),
		"", 0.f, m_pDevice, m_pContext);
	m_iPatternPresentationClipOccurrenceIndex =
		(std::numeric_limits<std::size_t>::max)();
	m_pBodyModelCom->Set_AnimationSpeed(1.f);
	m_pBodyModelCom->Set_AnimPaused(false);
}

void CValtan::Load_PatternEffectCues()
{
	VALTAN_PATTERN_EFFECT_CUE_DOCUMENT Document;
	std::string Status;
	if (!CValtanPatternEffectCueDocument::Load_ForProductPrewarm(
			Document, Status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern Effect cues isolated: " + Status +
			"\n").c_str());
		return;
	}

	std::unordered_map<std::string,
		std::vector<VALTAN_PATTERN_EFFECT_CUE>> Staged;
	std::unordered_map<std::string,
		std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING>> TimingsByAction;
	for (const VALTAN_PATTERN_EFFECT_CUE& Cue : Document.Cues)
	{
		if ("root" != Cue.strAnchorSlotId &&
			(nullptr == m_pBodyModelCom ||
			 !m_pBodyModelCom->Has_Bone(Cue.strAnchorSlotId.c_str())))
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Effect cue anchor rejected: " +
				Cue.strBindingId + "\n").c_str());
			return;
		}
		const auto Binding = m_PatternClipByActionId.find(Cue.strActionId);
		if (m_PatternClipByActionId.end() == Binding)
		{
			OutputDebugStringA((
				"[Client][Valtan] pattern Effect cue action binding rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			return;
		}
		if (Cue.bUsesStageClock)
		{
			if (!Binding->second.empty() || !Cue.strClipOccurrenceId.empty() ||
				Cue.iStartMs >= Cue.iStageDurationMs)
			{
				OutputDebugStringA((
					"[Client][Valtan] stage-clock Effect cue NONE binding rejected: " +
					Cue.strOccurrenceId + "\n").c_str());
				return;
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
			OutputDebugStringA((
				"[Client][Valtan] pattern Effect cue clip occurrence rejected: " +
				Cue.strOccurrenceId + "\n").c_str());
			return;
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
					OutputDebugStringA((
						"[Client][Valtan] pattern Effect cue timeline rejected: " +
						Cue.strOccurrenceId + "\n").c_str());
					return;
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
					"[Client][Valtan] pattern Effect cue source start rejected: " +
					Cue.strOccurrenceId + "\n").c_str());
				return;
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
					OutputDebugStringA((
						"[Client][Valtan] pattern Effect cue source end rejected: " +
						Cue.strOccurrenceId + "\n").c_str());
					return;
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
	m_AttemptedPatternEffectOccurrenceKeys.clear();
	m_bPatternEffectCueScanAgeValid = false;
	m_fPatternEffectCueScanAgeSeconds = 0.f;
}

void CValtan::Spawn_DuePatternEffectCues(const f32_t fActionAgeSeconds)
{
	if (!m_isServerAuthoritative ||
		!std::isfinite(fActionAgeSeconds) || fActionAgeSeconds < 0.f ||
		m_strServerPatternId.empty() || m_strServerActionId.empty() ||
		0u == m_iServerActionStartTick || 0u == m_iServerPatternSequence)
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
	const auto Found =
		m_PatternEffectCuesByActionId.find(m_strServerActionId);
	if (m_PatternEffectCuesByActionId.end() == Found)
		return;
	const auto Binding =
		m_PatternClipByActionId.find(m_strServerActionId);
	if (m_PatternClipByActionId.end() == Binding)
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
		if (Cue.strPatternId != m_strServerPatternId ||
			Cue.iStageIndex != m_iServerPatternStageIndex)
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
		if (Is_PatternEffectV1AuditionEnabled() &&
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
			Desc.iActionStartTick = m_iServerActionStartTick;
			Desc.iCueStartMs = Cue.iStartMs;
			Desc.strOccurrenceId = "valtan:action-start:" +
				std::to_string(m_iServerActionStartTick) + "/sequence:" +
				std::to_string(m_iServerPatternSequence) + "/stage:" +
				std::to_string(m_iServerPatternStageIndex) + "/cue:" +
				Cue.strOccurrenceId + "/loop:" +
				std::to_string(Sample.iLoopEpoch);
			Desc.fPlaybackRate = fPlaybackRate;
			Desc.fInitialSampleTimeSeconds = Sample.fInitialSampleSeconds;
			std::string Status;
			if (!CEffectPresentationService::Spawn(Desc, Status))
			{
				OutputDebugStringA((
					"[Client][Valtan] pattern Effect spawn isolated: " + Status +
					"\n").c_str());
			}
		}
	}
}

void CValtan::Load_PatternSoundCues()
{
	VALTAN_PATTERN_SOUND_CUE_DOCUMENT Document;
	std::string Status;
	if (!CValtanPatternSoundCueDocument::Load_Source(Document, Status))
	{
		OutputDebugStringA((
			"[Client][Valtan] pattern Sound cues isolated: " + Status +
			"\n").c_str());
		return;
	}

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
	m_AttemptedPatternSoundOccurrenceKeys.clear();
	m_bPatternSoundCueScanAgeValid = false;
	m_fPatternSoundCueScanAgeSeconds = 0.f;
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

		const std::vector<std::string>& Variants =
			CSoundCueCatalog::Find_Variants("Valtan", Cue.strSoundEvent);
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
	if (m_isPatternHitAreaDebugVisible || !m_strPreviewHitActionId.empty())
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

	weaponDesc.strModelTag = TEXT("Prototype_Component_Model_ValtanWeapon");

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

/* The shoulder and arm plates are authored on the body rig, so they are
skinned parts: no socket bone, and the body model owns the bone palette
they render with. Breaking a plate later only has to hide its part.

A plate is presentation, so a missing or unreadable one is isolated to
itself: the boss still spawns and fights without it. Only the plates that
actually attached are recorded. */
void CValtan::Ready_ArmorParts()
{
	m_ArmorPartTagsByStateMask.clear();
	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss("BOSS_VALTAN");
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
			Build_ArmorModelPrototypeTag(armorPart.stateMask);
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
	if (m_hasBossCombatState)
	{
		if (state.iStateRevision < m_BossCombatState.iStateRevision)
			return false;
		if (state.iStateRevision == m_BossCombatState.iStateRevision)
			return Is_SameBossCombatState(state, m_BossCombatState);
	}

	m_BossCombatState = state;
	m_hasBossCombatState = true;
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
	const BOSS_ACTOR_ENTRY* pActor = CActorCatalog::Find_Boss("BOSS_VALTAN");
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
		VALTAN_CINEMATIC_ENTRANCE_PATTERN_ID == patternId;
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
	/* This camera-only entrance gate deliberately adds no BGM edge. Keep
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

bool_t CValtan::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees,
	const LostArk::Shared::WORLD_ENTITY_ACTION action,
	const std::string_view patternId,
	const std::string_view actionId,
	const uint32_t iServerTick,
	const uint32_t iActionStartTick,
	const uint32_t iPatternSequence,
	const uint32_t iPatternStageIndex)
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
			CActorCatalog::Find_Boss("BOSS_VALTAN");
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

	Queue_NetworkTransformSample(position, yawDegrees, iServerTick);
	m_strServerActionId.assign(actionId);
	m_iState = nextState;
	if (0u != iServerTick)
		m_iLastServerTick = iServerTick;
	if (isPatternState)
	{
		m_strServerPatternId.assign(patternId);
		m_iServerActionStartTick = iActionStartTick;
		m_iServerPatternSequence = iPatternSequence;
		m_iServerPatternStageIndex = iPatternStageIndex;
		m_fServerActionAgeSeconds = fActionAgeSeconds;
		if (bCommitPatternClipOccurrenceIndex)
		{
			m_iPatternPresentationClipOccurrenceIndex =
				iAcceptedPatternClipOccurrenceIndex;
		}
	}
	else
	{
		m_iServerActionStartTick = 0u;
		m_iServerPatternStageIndex = 0u;
		m_fServerActionAgeSeconds = 0.f;
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
		}
		Spawn_DuePatternEffectCues(fActionAgeSeconds);
		Spawn_DuePatternSoundCues(fActionAgeSeconds);
		Client::CEffectV2Runtime::Sync_Stage(
			Client::EFFECT_V2_TARGET::From_Valtan(
				static_pointer_cast<CValtan>(shared_from_this())),
			m_strServerActionId.c_str(), fActionAgeSeconds,
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
