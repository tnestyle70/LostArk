# 쇼타임 카메라·베른 보행 바닥 인수 수정

## G01. 실제 작업 경계

실행 저장소는 `C:/Users/USER/source/졸업팀폴/LostArk`다. 별도 Codex worktree의 오래된 C++를 이 저장소에 덮어쓰지 않는다. 기존 dirty 변경을 보존한다. 조명, NPC, 발탄, 전투 패턴과 보스 크기는 변경 대상이 아니다.

## G02. 쇼타임 카메라

`KAKULSAYDON_G1_PATTERN_76`의 네 CAMERA 참조와 `kouku.gate3.showtime.camera.1`~`.4`의 키를 추적한다. 원본 배우 drawscale 1.2000000476837158과 현재 BossCatalog bodyModelPreScale 0.017의 차이는 카메라의 배우 상대 거리에도 적용해야 한다. 원본 키 시각·FOV·roll·컷 경계는 보존하고 eye/lookAt만 같은 기준점에서 등비 변환한다. 기존 미완료 배우 변경은 카메라 변경과 구분한다.

카메라만 게시하는 범위를 기존 Map publisher의 검증·transaction에 추가해 조명 등 다른 문서의 동반 게시를 방지한다. 게시 전후 비대상 파일의 바이트 동일성을 확인한다. 숫자 검증은 사용자 화면 판정을 대체하지 않는다.

## G03. 베른

기존 coverage2.py는 submesh-local index에 vertexOffset을 반영하지 않아 복수 submesh의 삼각형을 잘못 해석한다. 기존 커버리지 비율과 missing_clusters를 삭제·복구 목록으로 사용하지 않는다. 실제 WMeshReader 계약대로 읽어 원본과 설치 모델을 다시 대조한다.

실제 보행로의 바닥·계단을 확인한 뒤 해당 배치만 복구한다. 수면, 충돌 전용 박스, 장식 상면과 원래 낭떠러지는 보행 바닥으로 승격하지 않는다. 바닥 복구 후 서버 navigation의 높이·연결·기존 차단 영역을 검사한다. 미니맵은 위치 대조 자료이지 지오메트리 증거가 아니다.

## G04. 검증과 실행

수정 블록과 재현 가능한 검증 코드는 적용 전에 이 문서에 추가한다. 데이터 변경은 관련 publisher Validate/Check와 비대상 보존 검사를 수행한다. Client는 에이전트가 실행하지 않는다. 최종 화면과 실제 이동 확인은 사용자에게 정확한 실행 경로를 전달한다.

## G05. 쇼타임 중복 배우·바닥 관통 수정 코드 정본

2026-09-18 사용자 1548ms 재현에 대한 적용 코드다. CNpc 표시 억제는 서버 상태가 아닌 렌더만 제어하며 commit 이후 획득하고 Stop/실패/교체에서 해제한다. 카메라 2번은 실제 바닥 교차 측정에 따라 Y만 0.4m 올린다. 나머지 프레이밍 보정과 원본 시간은 유지한다. 신규 C++ 파일과 project/filter 변경은 없다. 검증 명령·실측·사용자 확인 경계는 같은 제목 RESULT G03~G05에 기록한다.

### Client/Public/Npc.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameObject.h"
#include "PlayerHandGripTransform.h"
#include "KoukuSaydonCompositionDocument.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <string>
#include <optional>

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)

enum class ANIMATION_BONE_TARGET : uint8_t;
struct ANIMATION_MODEL_TARGET_VIEW;

struct NPC_NETWORK_TRANSFORM_FRAME final
{
	float3_t vPosition = {};
	f32_t fYawDegrees = 0.f;
};

/* Pure fixed-tick sample buffer used directly by CNpc and by the focused
client harness. It owns interpolation delay and shortest-arc yaw smoothing;
the GameObject owns only applying the resulting frame to CTransform. */
class CNpcNetworkTransformInterpolator final
{
private:
	struct SAMPLE final
	{
		std::uint32_t iServerTick = 0u;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
	};

	static constexpr size_t SAMPLE_CAPACITY = 8u;
	static constexpr f32_t SERVER_TICK_HZ = 30.f;
	static constexpr f32_t INTERPOLATION_DELAY_TICKS = 2.f;
	static constexpr f32_t PLAYBACK_SNAP_TICKS = 6.f;
	static constexpr f32_t PLAYBACK_DRIFT_GAIN = 4.f;
	static constexpr f32_t TELEPORT_DISTANCE_SQ = 100.f;
	static constexpr f32_t TURN_DEGREES_PER_SECOND = 720.f;

public:
	void Reset()
	{
		m_iSampleCount = 0u;
		m_fPlaybackServerTick = 0.f;
		m_fPresentationYawDegrees = 0.f;
		m_hasState = false;
	}

	bool_t Push(
		const float3_t& position,
		const f32_t yawDegrees,
		const std::uint32_t iServerTick)
	{
		if (0u == iServerTick || !std::isfinite(position.x) ||
			!std::isfinite(position.y) || !std::isfinite(position.z) ||
			!std::isfinite(yawDegrees))
		{
			return false;
		}

		bool_t reset = !m_hasState;
		if (!reset && m_iSampleCount > 0u)
		{
			const SAMPLE& newest = m_Samples[m_iSampleCount - 1u];
			const f32_t dx = position.x - newest.vPosition.x;
			const f32_t dy = position.y - newest.vPosition.y;
			const f32_t dz = position.z - newest.vPosition.z;
			reset = dx * dx + dy * dy + dz * dz > TELEPORT_DISTANCE_SQ;
		}
		if (reset)
		{
			m_iSampleCount = 0u;
			m_fPresentationYawDegrees = yawDegrees;
			m_fPlaybackServerTick = static_cast<f32_t>(iServerTick) -
				INTERPOLATION_DELAY_TICKS;
		}

		if (m_iSampleCount > 0u &&
			m_Samples[m_iSampleCount - 1u].iServerTick >= iServerTick)
		{
			SAMPLE& newest = m_Samples[m_iSampleCount - 1u];
			newest.vPosition = position;
			newest.fYawDegrees = yawDegrees;
		}
		else
		{
			if (SAMPLE_CAPACITY == m_iSampleCount)
			{
				for (size_t i = 1u; i < SAMPLE_CAPACITY; ++i)
					m_Samples[i - 1u] = m_Samples[i];
				--m_iSampleCount;
			}
			SAMPLE& sample = m_Samples[m_iSampleCount++];
			sample.iServerTick = iServerTick;
			sample.vPosition = position;
			sample.fYawDegrees = yawDegrees;
		}
		m_hasState = true;
		return true;
	}

	bool_t Advance(
		const f32_t fTimeDelta,
		NPC_NETWORK_TRANSFORM_FRAME& outFrame)
	{
		if (!m_hasState || 0u == m_iSampleCount ||
			!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		{
			return false;
		}

		const f32_t oldestTick = static_cast<f32_t>(
			m_Samples[0u].iServerTick);
		const f32_t newestTick = static_cast<f32_t>(
			m_Samples[m_iSampleCount - 1u].iServerTick);
		m_fPlaybackServerTick += fTimeDelta * SERVER_TICK_HZ;
		const f32_t targetTick = newestTick - INTERPOLATION_DELAY_TICKS;
		const f32_t drift = targetTick - m_fPlaybackServerTick;
		if (std::fabs(drift) > PLAYBACK_SNAP_TICKS)
			m_fPlaybackServerTick = targetTick;
		else
		{
			m_fPlaybackServerTick += drift * (std::min)(
				1.f, PLAYBACK_DRIFT_GAIN * fTimeDelta);
		}
		m_fPlaybackServerTick = (std::max)(oldestTick,
			(std::min)(newestTick, m_fPlaybackServerTick));

		size_t older = m_iSampleCount - 1u;
		for (size_t i = 0u; i + 1u < m_iSampleCount; ++i)
		{
			if (m_fPlaybackServerTick <= static_cast<f32_t>(
					m_Samples[i + 1u].iServerTick))
			{
				older = i;
				break;
			}
		}
		const SAMPLE& from = m_Samples[older];
		const SAMPLE& to = m_Samples[
			(std::min)(older + 1u, m_iSampleCount - 1u)];
		outFrame.vPosition = to.vPosition;
		f32_t targetYawDegrees = to.fYawDegrees;
		if (to.iServerTick > from.iServerTick)
		{
			const f32_t ratio =
				(m_fPlaybackServerTick -
					static_cast<f32_t>(from.iServerTick)) /
				static_cast<f32_t>(to.iServerTick - from.iServerTick);
			outFrame.vPosition.x = from.vPosition.x +
				(to.vPosition.x - from.vPosition.x) * ratio;
			outFrame.vPosition.y = from.vPosition.y +
				(to.vPosition.y - from.vPosition.y) * ratio;
			outFrame.vPosition.z = from.vPosition.z +
				(to.vPosition.z - from.vPosition.z) * ratio;
			f32_t yawSpan = to.fYawDegrees - from.fYawDegrees;
			while (yawSpan > 180.f)
				yawSpan -= 360.f;
			while (yawSpan < -180.f)
				yawSpan += 360.f;
			targetYawDegrees = from.fYawDegrees + yawSpan * ratio;
		}

		f32_t yawDifference =
			targetYawDegrees - m_fPresentationYawDegrees;
		while (yawDifference > 180.f)
			yawDifference -= 360.f;
		while (yawDifference < -180.f)
			yawDifference += 360.f;
		const f32_t yawStep = TURN_DEGREES_PER_SECOND * fTimeDelta;
		if (std::fabs(yawDifference) <= yawStep)
			m_fPresentationYawDegrees = targetYawDegrees;
		else
		{
			m_fPresentationYawDegrees +=
				yawDifference > 0.f ? yawStep : -yawStep;
		}
		outFrame.fYawDegrees = m_fPresentationYawDegrees;
		return true;
	}

private:
	SAMPLE m_Samples[SAMPLE_CAPACITY] = {};
	size_t m_iSampleCount = 0u;
	f32_t m_fPlaybackServerTick = 0.f;
	f32_t m_fPresentationYawDegrees = 0.f;
	bool_t m_hasState = false;
};

/* A town NPC: one skinned model that presents a Server-owned transform and
semantic action as a model clip.

Deliberately not a CCharacter. That type assembles equipment parts, weapon
sockets and a class logic from a CHARACTER_SPEC, none of which an NPC has -- the
cook already merges an NPC's body and head into a single mesh, so there is one
model and nothing to assemble.

Everything an instance starts with is in NPC_DESC, so the placement tool can
spawn the same prototype many times; product movement and later action edges are
then supplied by Client replication. */
class CNpc final : public CGameObject, public IPlayerHandGripSocketSource
{
public:
	typedef struct tagNpcDesc : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;
		// Optional stable binding owner for a catalog boss using the NPC renderer.
		std::string strEffectV2BindingOwner;

		/* Clip to stand in. Every NPC is cooked under the same "npc" armature
		name, so the clip names all carry that prefix -- "npc_idle_normal_1",
		"npc_sc_talk_1" -- and one name works across every NPC that shares an
		archetype. An unknown name falls back to the model's first clip. */
		const char_t* pIdleClip = { nullptr };
		bool_t isLoop = { true };

		float3_t vPosition = {};
		/* Degrees about Y. Town NPCs face doors and counters, not always north. */
		f32_t fYawDegree = {};
		/* Zero for non-combat NPCs; Server-replicated radius for monsters. */
		f32_t fCollisionRadius = {};
		/* Product town behavior and MapTool previews are transform-authoritative
		outside the model. Esther summons leave this false because their authored
		action chains intentionally carry root motion. */
		bool_t bSuppressRootMotion = false;
		/* Independent from root-motion policy. Server-owned town NPCs and
		monsters interpolate snapshot transforms; local previews and Esther keep
		their existing immediate-transform behavior. */
		bool_t bInterpolateNetworkTransform = false;
		/* Inverted-hull outline in world metres; 0 disables. Only shaders that
		expose an Outline pass (esther) honour it. */
		f32_t fOutlineWidth = {};
		float4_t vOutlineColor = { 1.f, 1.f, 1.f, 1.f };
		/* Optional socketed weapon: a second animated CModel prototype drawn in
		its rest pose from one bone of the body skeleton. Both fields are set
		together or neither is; a socket the body lacks fails the clone instead
		of drawing the weapon at the origin. Unit conversion is baked into the
		weapon prototype's pre-transform, so no scale travels here. */
		wstring_t strWeaponModelTag;
		const char_t* pWeaponSocketBone = { nullptr };
	} NPC_DESC;

	/* Esther summons (Sillian / Wei / Bahuntur) draw with a white silhouette
	like the original. Width is world metres along the skinned normal. */
	static constexpr f32_t ESTHER_OUTLINE_WIDTH = 0.04f;

private:
	CNpc(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	virtual ~CNpc();

public:
	shared_ptr<Engine::CModel> Get_Model() const {
		return m_pModelCom;
	}
	shared_ptr<Engine::CTransform> Get_Transform() const {
		return m_pTransformCom;
	}
	const wstring_t& Get_ModelTag() const {
		return m_strModelTag;
	}
	const std::string& Get_EffectV2BindingOwner() const { return m_strEffectV2BindingOwner; }
	bool_t Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET target, ANIMATION_MODEL_TARGET_VIEW& outView) const;
	bool_t Set_PlayerHandGripLocalOffset(const PLAYER_HAND_GRIP_LOCAL_OFFSET& offset);
	void Clear_PlayerHandGripLocalOffset() { m_PlayerHandGripLocalOffset.reset(); }
	bool_t Try_Get_PlayerHandGripSocketView(LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const override;
	bool_t Try_Get_PlayerHandGripLocalOffset(LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const override;
	void Synchronize_WeaponPose();
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	/* Restarts the selected clip even when the previous action used the same
	clip. The network action edge owns restart timing; the model only owns how
	the authored clip is blended and played. */
	// Stage age owns the delay; the hold deadline starts at animation time zero.
	// A zero hold deadline preserves the existing whole-stage action lifetime.
	bool_t Set_NetworkAnimationWindow(f32_t ageSeconds, f32_t holdSeconds,
		f32_t startOffsetSeconds = 0.f, uint32_t sourceStartMs = 0u, uint32_t sourceEndMs = 0u);
	bool_t Set_NetworkAnimationBlendWindows(
        const std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& windows,
        std::string_view semanticOccurrenceId, f32_t patternAgeSeconds);
	bool_t Apply_NetworkAnimationTransition(const char_t* sourceClip, f32_t sourceMs,
		f32_t durationMs, f32_t ageSeconds, f32_t playRate);
	bool_t Play_NetworkAction(
		const char_t* pClipName,
		bool_t isLoop,
		f32_t fPlaybackRate,
		f32_t fBlendSeconds,
		f32_t fRootVerticalScale = 1.f);
	bool_t Play_TransientNetworkAction(
		const char_t* pClipName,
		f32_t fPlaybackRate,
		f32_t fDurationSeconds,
		const char_t* pReturnClip,
		bool_t isReturnLoop,
		f32_t fReturnPlaybackRate = 1.f,
		f32_t fBlendSeconds = 0.05f);
	bool_t Play_DefaultIdle(f32_t fBlendSeconds = 0.12f);
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees,
		std::uint32_t iServerTick = 0u,
		bool_t snapToSnapshot = false);
	void Trigger_HitFlash();
    void Set_PresentationVisible(bool visible) { m_bPresentationVisible = visible; }
    // Preview owns only a render suppression; network state and base visibility keep updating.
    void Acquire_CompositionPreviewSuppression() { ++m_iCompositionPreviewSuppressions; }
    void Release_CompositionPreviewSuppression() { if (m_iCompositionPreviewSuppressions) --m_iCompositionPreviewSuppressions; }
    bool Is_PresentationVisible() const { return m_bPresentationVisible && m_iCompositionPreviewSuppressions == 0u; }
#ifdef _DEBUG
	void Set_CombatColliderDebugVisible(bool_t isVisible) {
		m_isCombatColliderDebugVisible = isVisible;
	}
	/* F1 tuning only. The scale multiplies the drawn body transform, the
	offset shifts the drawn body from its replicated position, and the weapon
	multiplier scales the socketed weapon. None of them reaches the Server or
	the collider authority; they exist so a catalog value can be chosen by eye
	before it is saved. */
	void Set_DebugPresentationScale(f32_t fScale);
	void Set_DebugPresentationOffset(const float3_t& vOffset);
	/* Added to the replicated yaw of every drawn pose; the collider keeps the
	Server yaw. Lets a placement yawDegrees be chosen by eye before saving. */
	void Set_DebugPresentationYawOffset(f32_t fYawOffsetDegrees);
	void Set_DebugWeaponScale(f32_t fScale);
	/* Match the saved Euler rotation despite the catalog rotation already
	baked into this Client's weapon prototype. */
	void Set_DebugWeaponRotation(
		const float3_t& vCatalogDegrees, const float3_t& vTargetDegrees);
	f32_t Get_DebugPresentationScale() const { return m_fDebugPresentationScale; }
	const float3_t& Get_DebugPresentationOffset() const { return m_vDebugPresentationOffset; }
	const float3_t& Get_DebugUnadjustedPosition() const { return m_vDebugUnadjustedPosition; }
	f32_t Get_DebugUnadjustedYawDegrees() const { return m_fDebugUnadjustedYawDegrees; }
	f32_t Get_DebugPresentationYawOffset() const { return m_fDebugPresentationYawOffset; }
	f32_t Get_DebugWeaponScale() const { return m_fDebugWeaponScale; }
	const float4x4_t& Get_DebugWeaponRotation() const { return m_DebugWeaponRotation; }
#endif

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	shared_ptr<Engine::CShader> m_pShaderCom = { nullptr };
	bool_t m_bNativeBinaryBasePass = false;
	shared_ptr<Engine::CModel> m_pModelCom = { nullptr };
	wstring_t m_strModelTag;
	std::string m_strEffectV2BindingOwner;
	/* Socketed weapon with body-clock pose synchronization; null when the
	desc declared none. It never starts a clip of its own. */
	shared_ptr<Engine::CModel> m_pWeaponModelCom = { nullptr };
	std::vector<float4x4_t> m_WeaponRestPose;
	std::string m_strWeaponSocketBone;
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
	std::string m_strDefaultIdleClip;
	std::optional<PLAYER_HAND_GRIP_LOCAL_OFFSET> m_PlayerHandGripLocalOffset;
	bool_t Try_SampleNetworkAnimationTicks(f32_t animationAgeSeconds, uint32_t clip,
		f32_t playRate, f32_t& outTicks) const;
	// CModel's blended target may precede the actual Server action edge.
    // Preserve the semantic action clip independently from the displayed pose.
    uint32_t m_iNetworkSemanticClip = UINT32_MAX;
    uint32_t m_iNetworkSemanticAbsoluteStartMs = UINT32_MAX;
    std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> m_NetworkAnimationBlendWindows;
    double m_fNetworkPatternAgeSeconds = 0.0;
	bool_t m_bNetworkAnimationWindow = false;
	bool_t m_bNetworkAnimationTransition = false;
	bool_t m_isNetworkAnimationLoop = false;
	f32_t m_fNetworkAnimationAgeSeconds = 0.f, m_fNetworkAnimationHoldSeconds = 0.f;
	f32_t m_fNetworkAnimationStartOffsetSeconds = 0.f;
	uint32_t m_iNetworkAnimationSourceStartMs = 0u, m_iNetworkAnimationSourceEndMs = 0u;
	f32_t m_fNetworkAnimationPlayRate = 1.f;
	uint32_t m_iTransitionSourceIndex = UINT32_MAX;
	f32_t m_fTransitionSourceTicks = 0.f, m_fTransitionDurationSeconds = 0.f;
	f32_t m_fTransitionAgeSeconds = 0.f, m_fTransitionPlayRate = 1.f;
	CNpcNetworkTransformInterpolator m_NetworkTransformInterpolator;
	bool_t m_bSuppressRootMotion = false;
    bool m_bPresentationVisible = true;
    std::uint32_t m_iCompositionPreviewSuppressions = 0u;
	bool_t m_bInterpolateNetworkTransform = false;
	f32_t m_fTransientActionRemainingSeconds = 0.f;
	std::string m_strTransientReturnClip;
	f32_t m_fTransientReturnPlaybackRate = 1.f;
	bool_t m_isTransientReturnLoop = true;
	f32_t m_fOutlineWidth = { 0.f };
	float4_t m_vOutlineColor = { 1.f, 1.f, 1.f, 1.f };
#ifdef _DEBUG
	bool_t m_isCombatColliderDebugVisible = { false };
	f32_t m_fDebugPresentationScale = 1.f;
	float3_t m_vDebugPresentationOffset = {};
	float3_t m_vDebugUnadjustedPosition = {};
	f32_t m_fDebugPresentationYawOffset = 0.f;
	f32_t m_fDebugUnadjustedYawDegrees = 0.f;
	f32_t m_fDebugWeaponScale = 1.f;
	float4x4_t m_DebugWeaponRotation = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };
#endif

private:
	HRESULT Ready_Components(const NPC_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	void Apply_ImmediateTransform(
		const float3_t& position,
		f32_t yawDegrees);
	void Update_NetworkTransform(f32_t fTimeDelta);
	void Update_CombatCollider();

public:
	static unique_ptr<CNpc> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
```

### Client/Private/Npc.cpp

```cpp
#include "Npc.h"
#include "KoukuSaydonAnimationBlend.h"
#include "KoukuSaydonCompositionDocument.h"
#include "EffectV2_Runtime.h"
#include "AnimationTargetService.h"
#include "NpcPresentationAssetService.h"

#include "Collider.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
	constexpr const char_t* PLAYER_LEFT_HAND_BONE = "bip001-l-hand";
}

CNpc::CNpc(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{
}

CNpc::~CNpc()
{
}

HRESULT CNpc::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNpc::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const NPC_DESC* pDesc = static_cast<const NPC_DESC*>(pArg);
	if (!std::isfinite(pDesc->fCollisionRadius) ||
		pDesc->fCollisionRadius < 0.f)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	Apply_ImmediateTransform(pDesc->vPosition, pDesc->fYawDegree);

	m_fOutlineWidth = pDesc->fOutlineWidth;
	m_vOutlineColor = pDesc->vOutlineColor;
	m_bSuppressRootMotion = pDesc->bSuppressRootMotion;
	m_bInterpolateNetworkTransform = pDesc->bInterpolateNetworkTransform;

	/* With no animation set the bone palette is never filled, so every vertex
	collapses onto the origin and the NPC simply vanishes -- a wrong clip name
	looks exactly like a failed load. Fall back to the model's first clip so a
	bad name is visible as a wrong pose instead of nothing at all. */
	if (nullptr == pDesc->pIdleClip ||
		!m_pModelCom->Set_Animation(pDesc->pIdleClip, pDesc->isLoop))
	{
		if (0 == m_pModelCom->Get_NumAnimations())
			return E_FAIL;
		m_pModelCom->Set_Animation(0u, pDesc->isLoop);
	}
	const char_t* pResolvedIdle = m_pModelCom->Get_AnimationName(
		m_pModelCom->Get_CurrentAnimIndex());
	if (nullptr == pResolvedIdle || '\0' == pResolvedIdle[0])
		return E_FAIL;
	m_strDefaultIdleClip = pResolvedIdle;
	m_pModelCom->Set_AnimationSpeed(1.f);
	/* Authored town placements are Server-transform authoritative. Their clips
	may pose translated root keys but must not move presentation away from the
	replicated transform. Esther leaves suppression disabled because its existing
	action chains intentionally use authored root motion. */
	if (m_bSuppressRootMotion)
	{
		m_pModelCom->Enable_RootMotionSuppression(
			ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);
	}

	return S_OK;
}

void CNpc::Synchronize_WeaponPose()
{
	CNpcPresentationAssetService::Synchronize_SaydonHammerPose(m_pModelCom, m_pWeaponModelCom, m_WeaponRestPose);
}

bool_t CNpc::Try_GetAnimationModelTarget(const ANIMATION_BONE_TARGET target,
	ANIMATION_MODEL_TARGET_VIEW& outView) const
{
	if (!m_pModelCom || !m_pTransformCom) return false;
	ANIMATION_MODEL_TARGET_VIEW staged;
	staged.TargetRoot = *m_pTransformCom->Get_WorldMatrixPtr();
	if (target == ANIMATION_BONE_TARGET::BODY)
	{
		staged.Model = m_pModelCom;
		staged.BoneRoot = staged.TargetRoot;
	}
	else if (target == ANIMATION_BONE_TARGET::WEAPON)
	{
		if (!m_pWeaponModelCom || !m_pModelCom->Has_Bone(m_strWeaponSocketBone.c_str())) return false;
		matrix_t weaponLocal = XMMatrixIdentity();
#ifdef _DEBUG
		weaponLocal = XMLoadFloat4x4(&m_DebugWeaponRotation) * XMMatrixScaling(m_fDebugWeaponScale, m_fDebugWeaponScale, m_fDebugWeaponScale);
#endif
		staged.Model = m_pWeaponModelCom;
		XMStoreFloat4x4(&staged.BoneRoot, weaponLocal *
			m_pModelCom->Get_BoneMatrix(m_strWeaponSocketBone.c_str()) * XMLoadFloat4x4(&staged.TargetRoot));
	}
	else return false;
	outView = std::move(staged);
	return true;
}

bool_t CNpc::Set_PlayerHandGripLocalOffset(const PLAYER_HAND_GRIP_LOCAL_OFFSET& offset)
{
    if (!m_pModelCom || !m_pModelCom->Has_Bone(PLAYER_LEFT_HAND_BONE) ||
        !CPlayerHandGripTransform::Is_ValidGripLocalOffset(offset)) return false;
    m_PlayerHandGripLocalOffset = offset;
    return true;
}

bool_t CNpc::Try_Get_PlayerHandGripLocalOffset(const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
    PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const
{
    if (slot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
        !m_PlayerHandGripLocalOffset ||
        !CPlayerHandGripTransform::Is_ValidGripLocalOffset(*m_PlayerHandGripLocalOffset)) return false;
    outOffset = *m_PlayerHandGripLocalOffset;
    return true;
}

bool_t CNpc::Try_Get_PlayerHandGripSocketView(const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
    PLAYER_HAND_GRIP_SOCKET_VIEW& outView) const
{
    if (slot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND ||
        !m_PlayerHandGripLocalOffset || !m_pModelCom || !m_pTransformCom ||
        !m_pModelCom->Has_Bone(PLAYER_LEFT_HAND_BONE)) return false;
    PLAYER_HAND_GRIP_SOCKET_VIEW staged{};
    const auto* root = m_pTransformCom->Get_WorldMatrixPtr();
    XMStoreFloat4x4(&staged.SocketWorld,
        m_pModelCom->Get_BoneMatrix(PLAYER_LEFT_HAND_BONE) * XMLoadFloat4x4(root));
    staged.OwnerYawBasis = *root;
    // The same composition validates matrices and metre offsets for every owner.
    float3_t position{};
    if (!CPlayerHandGripTransform::Compose_WorldPosition(staged, *m_PlayerHandGripLocalOffset, position)) return false;
    outView = staged;
    return true;
}

bool_t CNpc::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName || nullptr == m_pModelCom)
		return false;
	m_bNetworkAnimationWindow = false;
	m_bNetworkAnimationTransition = false;
    m_NetworkAnimationBlendWindows.clear();
    m_iNetworkSemanticClip = UINT32_MAX;
    m_iNetworkSemanticAbsoluteStartMs = UINT32_MAX;
    m_fNetworkPatternAgeSeconds = 0.0;
	m_iNetworkAnimationSourceStartMs = m_iNetworkAnimationSourceEndMs = 0u;
	m_fNetworkAnimationHoldSeconds = 0.f;
	m_fNetworkAnimationAgeSeconds = 0.f;
	m_isNetworkAnimationLoop = isLoop;
	m_fNetworkAnimationStartOffsetSeconds = 0.f;
	m_pModelCom->Set_AnimationSpeed(1.f);
	if (!m_pModelCom->Set_Animation(pClipName, isLoop))
		return false;
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
	return true;
}

bool_t CNpc::Try_SampleNetworkAnimationTicks(const f32_t animationAgeSeconds,
    const uint32_t clip, const f32_t playRate, f32_t& outTicks) const
{
    if (!m_pModelCom || !std::isfinite(animationAgeSeconds) || animationAgeSeconds < 0.f) return false;
    float cursor = 0.f, duration = 0.f;
    const float ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(clip);
    if (!m_pModelCom->Get_AnimationProgress(clip, cursor, duration) ||
        !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f) return false;
    const float sampleAge = m_fNetworkAnimationHoldSeconds > 0.f ?
        (std::min)(animationAgeSeconds, m_fNetworkAnimationHoldSeconds) : animationAgeSeconds;
    double sourceMs = 0.0;
    if (!Client::CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
        m_iNetworkAnimationSourceStartMs, m_iNetworkAnimationSourceEndMs,
        double(sampleAge) * 1000.0, playRate, double(duration) * 1000.0 / ticksPerSecond,
        m_isNetworkAnimationLoop, sourceMs)) return false;
    outTicks = (std::min)(duration, float(sourceMs * .001 * ticksPerSecond));
    return true;
}

bool_t CNpc::Set_NetworkAnimationWindow(const f32_t ageSeconds, const f32_t holdSeconds,
    const f32_t startOffsetSeconds, const uint32_t sourceStartMs, const uint32_t sourceEndMs)
{
    if (!m_pModelCom || !std::isfinite(ageSeconds) || ageSeconds < 0.f ||
        !std::isfinite(holdSeconds) || holdSeconds < 0.f || holdSeconds > 600.f ||
        !std::isfinite(startOffsetSeconds) || startOffsetSeconds < 0.f || startOffsetSeconds > 600.f) return false;
    const auto clip = m_pModelCom->Get_CurrentAnimIndex();
    float cursor = 0.f, duration = 0.f;
    const float ticksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(clip);
    if (!m_pModelCom->Get_AnimationProgress(clip, cursor, duration) ||
        !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f) return false;
    const float animationAge = (std::max)(0.f, ageSeconds - startOffsetSeconds);
    const float sampleAge = holdSeconds > 0.f ? (std::min)(animationAge, holdSeconds) : animationAge;
    double sourceMs = 0.0;
    if (!Client::CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
        sourceStartMs, sourceEndMs, double(sampleAge) * 1000.0, m_fNetworkAnimationPlayRate,
        double(duration) * 1000.0 / ticksPerSecond, m_isNetworkAnimationLoop, sourceMs)) return false;
    const float ticks = (std::min)(duration, float(sourceMs * .001 * ticksPerSecond));
    // The source window owns wrapping. Disable the model's full-clip loop so
    // its endpoint cannot wrap behind the cropped-range sampler.
    if (!m_pModelCom->Start_Animation(clip, false) ||
        !m_pModelCom->Set_AnimTrackPosition(clip, ticks)) return false;
    m_fNetworkAnimationAgeSeconds = ageSeconds;
    m_fNetworkAnimationHoldSeconds = holdSeconds;
    m_fNetworkAnimationStartOffsetSeconds = startOffsetSeconds;
    m_iNetworkAnimationSourceStartMs = sourceStartMs;
    m_iNetworkAnimationSourceEndMs = sourceEndMs;
    m_bNetworkAnimationWindow = true;
    m_iNetworkSemanticClip = clip;
    m_pModelCom->Skip_Blend();
    m_pModelCom->Set_AnimPaused(true);
    m_pModelCom->Update_Animation(0.f);
    Synchronize_WeaponPose();
    return true;
}

bool_t CNpc::Set_NetworkAnimationBlendWindows(
    const std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW>& windows,
    const std::string_view semanticOccurrenceId, const f32_t patternAgeSeconds)
{
    if (!m_pModelCom || !m_bNetworkAnimationWindow || !std::isfinite(patternAgeSeconds) || patternAgeSeconds < 0.f)
        return false;
    std::string status;
    if (!CKoukuSaydonAnimationBlend::Validate_ModelWindows(*m_pModelCom, windows, status)) return false;
    const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* semantic = nullptr;
    for (const auto& window : windows)
        for (const auto* row : {&window.Source, &window.Target})
            if (row->strOccurrenceId == semanticOccurrenceId)
            {
                if (semantic && *semantic != *row) return false;
                semantic = row;
            }
    const char* clipName = m_pModelCom->Get_AnimationName(m_iNetworkSemanticClip);
    if (!semantic || !clipName || semantic->strRuntimeClip != clipName) return false;
    CModel::ANIMATION_TRANSITION_POSE pose;
    bool active = false;
    if (!CKoukuSaydonAnimationBlend::Sample_Pose(*m_pModelCom, windows, double(patternAgeSeconds) * 1000.0,
        pose, active, status) || (active && !m_pModelCom->Set_AnimationTransitionPose(pose))) return false;
    const float semanticAge = (std::max)(0.f, patternAgeSeconds - semantic->iStartOffsetMs * .001f);
    if (!active)
    {
        float ticks = 0.f;
        if (!Try_SampleNetworkAnimationTicks(semanticAge, m_iNetworkSemanticClip, m_fNetworkAnimationPlayRate, ticks)) return false;
        if (m_bNetworkAnimationTransition)
        {
            pose.sourceIndex = m_iTransitionSourceIndex; pose.sourceTicks = m_fTransitionSourceTicks;
            pose.targetIndex = m_iNetworkSemanticClip; pose.targetTicks = ticks;
            pose.durationSeconds = m_fTransitionDurationSeconds; pose.elapsedSeconds = semanticAge;
            pose.playRate = m_fTransitionPlayRate;
            if (!m_pModelCom->Set_AnimationTransitionPose(pose)) return false;
        }
        else
        {
            m_pModelCom->Clear_AnimationTransitionPose();
            m_pModelCom->Set_Animation(m_iNetworkSemanticClip, false, 0.f);
            if (!m_pModelCom->Set_AnimTrackPosition(m_iNetworkSemanticClip, ticks)) return false;
            m_pModelCom->Skip_Blend();
            m_pModelCom->Update_Animation(0.f);
        }
    }
    m_NetworkAnimationBlendWindows = windows;
    m_iNetworkSemanticAbsoluteStartMs = semantic->iStartOffsetMs;
    m_fNetworkPatternAgeSeconds = patternAgeSeconds;
    m_pModelCom->Set_AnimPaused(true);
    Synchronize_WeaponPose();
    return true;
}

bool_t CNpc::Apply_NetworkAnimationTransition(const char_t* sourceClip, const f32_t sourceMs,
    const f32_t durationMs, const f32_t ageSeconds, const f32_t playRate)
{
    if (!m_pModelCom || !m_bNetworkAnimationWindow || !sourceClip ||
        !std::isfinite(sourceMs) || sourceMs < 0.f ||
        !std::isfinite(durationMs) || durationMs <= 0.f || durationMs > 1000.f ||
        !std::isfinite(ageSeconds) || ageSeconds < 0.f || !std::isfinite(playRate) || playRate <= 0.f) return false;
    uint32_t sourceIndex = UINT32_MAX;
    for (uint32_t i = 0; i < m_pModelCom->Get_NumAnimations(); ++i)
    {
        const char_t* name = m_pModelCom->Get_AnimationName(i);
        if (name && std::string_view(sourceClip) == name) { sourceIndex = i; break; }
    }
    if (sourceIndex == UINT32_MAX) return false;
    const uint32_t targetIndex = m_pModelCom->Get_CurrentAnimIndex();
    float cursor = 0.f, sourceEnd = 0.f;
    const float sourceTicksPerSecond = m_pModelCom->Get_AnimationTickPerSecond(sourceIndex);
    if (!m_pModelCom->Get_AnimationProgress(sourceIndex, cursor, sourceEnd) ||
        !std::isfinite(sourceTicksPerSecond) || sourceTicksPerSecond <= 0.f ||
        !std::isfinite(sourceEnd) || sourceEnd <= 0.f) return false;
    CModel::ANIMATION_TRANSITION_POSE pose;
    pose.sourceIndex = sourceIndex; pose.targetIndex = targetIndex;
    pose.sourceTicks = (std::min)(sourceEnd, sourceMs * .001f * sourceTicksPerSecond);
    if (!Try_SampleNetworkAnimationTicks(ageSeconds, targetIndex, playRate, pose.targetTicks)) return false;
    pose.durationSeconds = durationMs * .001f; pose.elapsedSeconds = ageSeconds; pose.playRate = playRate;
    const bool waitingForAnimation = m_fNetworkAnimationAgeSeconds < m_fNetworkAnimationStartOffsetSeconds;
    if (!waitingForAnimation && !m_pModelCom->Set_AnimationTransitionPose(pose)) return false;
    m_iTransitionSourceIndex = sourceIndex; m_fTransitionSourceTicks = pose.sourceTicks;
    m_fTransitionDurationSeconds = pose.durationSeconds; m_fTransitionAgeSeconds = ageSeconds;
    m_fTransitionPlayRate = playRate; m_bNetworkAnimationTransition = true;
    m_pModelCom->Set_AnimPaused(true);
    Synchronize_WeaponPose();
    return true;
}

bool_t CNpc::Play_NetworkAction(
	const char_t* pClipName,
	const bool_t isLoop,
	const f32_t fPlaybackRate,
	const f32_t fBlendSeconds,
	const f32_t fRootVerticalScale)
{
	m_bNetworkAnimationWindow = false;
	m_bNetworkAnimationTransition = false;
    m_NetworkAnimationBlendWindows.clear();
    m_iNetworkSemanticClip = UINT32_MAX;
    m_iNetworkSemanticAbsoluteStartMs = UINT32_MAX;
    m_fNetworkPatternAgeSeconds = 0.0;
	m_iNetworkAnimationSourceStartMs = m_iNetworkAnimationSourceEndMs = 0u;
	m_fNetworkAnimationHoldSeconds = 0.f;
	m_fNetworkAnimationAgeSeconds = 0.f;
	m_isNetworkAnimationLoop = isLoop;
	m_fNetworkAnimationStartOffsetSeconds = 0.f;
	m_fTransientActionRemainingSeconds = 0.f;
	m_strTransientReturnClip.clear();
	if (nullptr == pClipName || '\0' == pClipName[0] ||
		nullptr == m_pModelCom || !std::isfinite(fPlaybackRate) ||
		fPlaybackRate < 0.1f || fPlaybackRate > 4.f ||
		!std::isfinite(fBlendSeconds) ||
		fBlendSeconds < 0.f || fBlendSeconds > 2.f ||
		!std::isfinite(fRootVerticalScale) || fRootVerticalScale < 0.f || fRootVerticalScale > 1.f ||
		(fRootVerticalScale != 1.f && !m_bSuppressRootMotion) ||
		!m_pModelCom->Set_Animation(pClipName, isLoop, fBlendSeconds))
	{
		return false;
	}
	if (!m_pModelCom->Set_RootMotionVerticalScale(fRootVerticalScale)) return false;
	m_pModelCom->Set_AnimationSpeed(fPlaybackRate);
	m_fNetworkAnimationPlayRate = fPlaybackRate;
	if (!m_pModelCom->Start_Animation(
			m_pModelCom->Get_CurrentAnimIndex(), isLoop))
	{
		return false;
	}
	/* Keep the existing NPC effect/cutin hook on every semantic action edge,
	including a restart that resolves to the same clip name. */
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
	return true;
}

bool_t CNpc::Play_TransientNetworkAction(
	const char_t* pClipName,
	const f32_t fPlaybackRate,
	const f32_t fDurationSeconds,
	const char_t* pReturnClip,
	const bool_t isReturnLoop,
	const f32_t fReturnPlaybackRate,
	const f32_t fBlendSeconds)
{
	if (nullptr == pReturnClip || '\0' == pReturnClip[0] ||
		!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f ||
		fDurationSeconds > 5.f || !std::isfinite(fReturnPlaybackRate) ||
		fReturnPlaybackRate < 0.1f || fReturnPlaybackRate > 4.f ||
		!Play_NetworkAction(
			pClipName, false, fPlaybackRate, fBlendSeconds))
	{
		return false;
	}
	m_fTransientActionRemainingSeconds = fDurationSeconds;
	m_strTransientReturnClip = pReturnClip;
	m_fTransientReturnPlaybackRate = fReturnPlaybackRate;
	m_isTransientReturnLoop = isReturnLoop;
	return true;
}

bool_t CNpc::Play_DefaultIdle(const f32_t fBlendSeconds)
{
	return !m_strDefaultIdleClip.empty() && Play_NetworkAction(
		m_strDefaultIdleClip.c_str(), true, 1.f, fBlendSeconds);
}

bool_t CNpc::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees,
	const std::uint32_t iServerTick,
	const bool_t snapToSnapshot)
{
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	if (!m_bInterpolateNetworkTransform)
	{
		Apply_ImmediateTransform(position, yawDegrees);
		return true;
	}
	/* Spawn messages carry no simulation tick. They place the object exactly;
	interpolation begins with the first world snapshot that has a tick. */
	if (0u == iServerTick)
	{
		m_NetworkTransformInterpolator.Reset();
		Apply_ImmediateTransform(position, yawDegrees);
		return true;
	}
	// A new Server pattern can commit a discontinuous position and facing.
	// Seed both the rendered root and interpolation history before its first cue.
	if (snapToSnapshot)
		m_NetworkTransformInterpolator.Reset();
	if (!m_NetworkTransformInterpolator.Push(position, yawDegrees, iServerTick))
		return false;
	if (snapToSnapshot)
		Apply_ImmediateTransform(position, yawDegrees);
	return true;
}

void CNpc::Trigger_HitFlash()
{
	m_fHitFlashRemainingSeconds = HIT_FLASH_DURATION_SECONDS;
	m_HitFlash.isEnabled = true;
	m_HitFlash.vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY;
	m_HitFlash.usesSurfaceDetailMask = true;
}

void CNpc::Priority_Update(f32_t fTimeDelta)
{
}

void CNpc::Update(f32_t fTimeDelta)
{
	if (m_bInterpolateNetworkTransform)
		Update_NetworkTransform(fTimeDelta);
	if (m_fTransientActionRemainingSeconds > 0.f)
	{
		m_fTransientActionRemainingSeconds -= fTimeDelta;
		if (m_fTransientActionRemainingSeconds <= 0.f)
		{
			const std::string returnClip = m_strTransientReturnClip;
			const f32_t returnRate = m_fTransientReturnPlaybackRate;
			const bool_t returnLoop = m_isTransientReturnLoop;
			m_fTransientActionRemainingSeconds = 0.f;
			m_strTransientReturnClip.clear();
			if (!returnClip.empty())
			{
				(void)Play_NetworkAction(
					returnClip.c_str(), returnLoop, returnRate, 0.05f);
			}
		}
	}
    const float frameDelta = (std::max)(0.f, fTimeDelta);
    m_fNetworkAnimationAgeSeconds += frameDelta;
    m_fNetworkPatternAgeSeconds += frameDelta;
    const float animationAge = m_bNetworkAnimationWindow && m_iNetworkSemanticAbsoluteStartMs != UINT32_MAX ?
        float((std::max)(0.0, m_fNetworkPatternAgeSeconds - double(m_iNetworkSemanticAbsoluteStartMs) * .001)) :
        (std::max)(0.f, m_fNetworkAnimationAgeSeconds - m_fNetworkAnimationStartOffsetSeconds);
    if (m_bNetworkAnimationWindow)
    {
        const auto clip = m_iNetworkSemanticClip;
        float ticks = 0.f;
        bool sampled = Try_SampleNetworkAnimationTicks(animationAge, clip,
            m_fNetworkAnimationPlayRate, ticks);
        CModel::ANIMATION_TRANSITION_POSE logicPose;
        bool logicActive = false;
        std::string blendStatus;
        if (sampled) sampled = CKoukuSaydonAnimationBlend::Sample_Pose(*m_pModelCom,
            m_NetworkAnimationBlendWindows, m_fNetworkPatternAgeSeconds * 1000.0, logicPose, logicActive, blendStatus);
        if (sampled && logicActive) sampled = m_pModelCom->Set_AnimationTransitionPose(logicPose);
        else if (sampled && m_bNetworkAnimationTransition &&
            m_fNetworkAnimationAgeSeconds >= m_fNetworkAnimationStartOffsetSeconds)
        {
            CModel::ANIMATION_TRANSITION_POSE pose;
            pose.sourceIndex = m_iTransitionSourceIndex; pose.sourceTicks = m_fTransitionSourceTicks;
            pose.targetIndex = clip; pose.targetTicks = ticks;
            pose.durationSeconds = m_fTransitionDurationSeconds; pose.elapsedSeconds = animationAge;
            pose.playRate = m_fTransitionPlayRate;
            sampled = m_pModelCom->Set_AnimationTransitionPose(pose);
            m_fTransitionAgeSeconds = animationAge;
        }
        else if (sampled)
        {
            m_pModelCom->Clear_AnimationTransitionPose();
            m_pModelCom->Set_Animation(clip, false, 0.f);
            sampled = m_pModelCom->Set_AnimTrackPosition(clip, ticks);
            if (sampled) { m_pModelCom->Skip_Blend(); m_pModelCom->Update_Animation(0.f); }
        }
        if (!sampled)
        {
            // A failed source sample must hold the last valid palette rather
            // than silently resume the uncropped full clip.
            m_pModelCom->Set_AnimPaused(true);
            m_bNetworkAnimationWindow = false; m_bNetworkAnimationTransition = false;
            OutputDebugStringA("[Npc] Network animation source window could not be sampled.\n");
        }
    }
    else if (m_bSuppressRootMotion)
    {
        m_pModelCom->Update_Animation(frameDelta);
    }
    else
    {
        m_pModelCom->Play_Animation(frameDelta);
    }
	Synchronize_WeaponPose();
	Update_CombatCollider();
	CEffectV2Runtime::Tick(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		m_pDevice, m_pContext);
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
}

void CNpc::Apply_ImmediateTransform(
	const float3_t& position,
	const f32_t yawDegrees)
{
	float3_t drawn = position;
	f32_t drawnYaw = yawDegrees;
#ifdef _DEBUG
	m_vDebugUnadjustedPosition = position;
	m_fDebugUnadjustedYawDegrees = yawDegrees;
	drawn.x += m_vDebugPresentationOffset.x;
	drawn.y += m_vDebugPresentationOffset.y;
	drawn.z += m_vDebugPresentationOffset.z;
	drawnYaw += m_fDebugPresentationYawOffset;
#endif
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(drawn.x, drawn.y, drawn.z, 1.f));
	// Rotation keeps the transform's current scale, so a Debug scale set once
	// through Set_DebugPresentationScale survives every replicated pose.
	m_pTransformCom->Rotation(0.f, drawnYaw, 0.f);
	Update_CombatCollider();
}

void CNpc::Update_NetworkTransform(const f32_t fTimeDelta)
{
	if (nullptr == m_pTransformCom)
		return;
	NPC_NETWORK_TRANSFORM_FRAME frame{};
	if (!m_NetworkTransformInterpolator.Advance(fTimeDelta, frame))
		return;
	float3_t drawn = frame.vPosition;
	f32_t drawnYaw = frame.fYawDegrees;
#ifdef _DEBUG
	m_vDebugUnadjustedPosition = frame.vPosition;
	m_fDebugUnadjustedYawDegrees = frame.fYawDegrees;
	drawn.x += m_vDebugPresentationOffset.x;
	drawn.y += m_vDebugPresentationOffset.y;
	drawn.z += m_vDebugPresentationOffset.z;
	drawnYaw += m_fDebugPresentationYawOffset;
#endif
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
		drawn.x,
		drawn.y,
		drawn.z,
		1.f));
	m_pTransformCom->Rotation(0.f, drawnYaw, 0.f);
}

void CNpc::Update_CombatCollider()
{
	if (nullptr == m_pColliderCom || nullptr == m_pTransformCom)
		return;
	matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
#ifdef _DEBUG
	// Presentation tuning must not move or resize the Server-radius mirror.
	if (m_fDebugPresentationScale != 1.f ||
		m_vDebugPresentationOffset.x != 0.f || m_vDebugPresentationOffset.y != 0.f ||
		m_vDebugPresentationOffset.z != 0.f)
	{
		for (size_t axis = 0u; axis < 3u; ++axis)
			world.r[axis] = XMVector3Normalize(world.r[axis]);
		world.r[3] = XMVectorSet(m_vDebugUnadjustedPosition.x,
			m_vDebugUnadjustedPosition.y, m_vDebugUnadjustedPosition.z, 1.f);
	}
#endif
	m_pColliderCom->Update(world);
}

#ifdef _DEBUG
void CNpc::Set_DebugWeaponScale(const f32_t fScale)
{
	if (std::isfinite(fScale) && fScale > 0.f && fScale <= 10000.f)
		m_fDebugWeaponScale = fScale;
}

void CNpc::Set_DebugWeaponRotation(
	const float3_t& vCatalogDegrees, const float3_t& vTargetDegrees)
{
	if (!std::isfinite(vCatalogDegrees.x) || !std::isfinite(vCatalogDegrees.y) ||
		!std::isfinite(vCatalogDegrees.z) || !std::isfinite(vTargetDegrees.x) ||
		!std::isfinite(vTargetDegrees.y) || !std::isfinite(vTargetDegrees.z))
	{
		return;
	}
	const matrix_t catalogRotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vCatalogDegrees.x), XMConvertToRadians(vCatalogDegrees.y),
		XMConvertToRadians(vCatalogDegrees.z));
	const matrix_t targetRotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vTargetDegrees.x), XMConvertToRadians(vTargetDegrees.y),
		XMConvertToRadians(vTargetDegrees.z));
	// A pure rotation's transpose is its inverse. R(catalog) * delta then
	// equals R(saved Euler), including mixed axes and Save/Reload in one run.
	XMStoreFloat4x4(&m_DebugWeaponRotation,
		XMMatrixTranspose(catalogRotation) * targetRotation);
}

void CNpc::Set_DebugPresentationScale(const f32_t fScale)
{
	if (!std::isfinite(fScale) || fScale <= 0.f || nullptr == m_pTransformCom)
		return;
	m_fDebugPresentationScale = fScale;
	m_pTransformCom->Scale(fScale, fScale, fScale);
}

void CNpc::Set_DebugPresentationOffset(const float3_t& vOffset)
{
	if (!std::isfinite(vOffset.x) || !std::isfinite(vOffset.y) ||
		!std::isfinite(vOffset.z))
	{
		return;
	}
	m_vDebugPresentationOffset = vOffset;
	if (nullptr != m_pTransformCom)
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
			m_vDebugUnadjustedPosition.x + vOffset.x,
			m_vDebugUnadjustedPosition.y + vOffset.y,
			m_vDebugUnadjustedPosition.z + vOffset.z, 1.f));
	Update_CombatCollider();
}

void CNpc::Set_DebugPresentationYawOffset(const f32_t fYawOffsetDegrees)
{
	if (!std::isfinite(fYawOffsetDegrees))
		return;
	m_fDebugPresentationYawOffset = fYawOffsetDegrees;
	// Rotation keeps the current scale; the collider re-reads the Server yaw.
	if (nullptr != m_pTransformCom)
		m_pTransformCom->Rotation(0.f,
			m_fDebugUnadjustedYawDegrees + fYawOffsetDegrees, 0.f);
	Update_CombatCollider();
}
#endif

void CNpc::Late_Update(f32_t fTimeDelta)
{
    if (!Is_PresentationVisible()) return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
#ifdef _DEBUG
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CNpc::Render()
{
    if (!Is_PresentationVisible()) return S_OK;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const uint32_t iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (uint32_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash,
				nullptr, m_bNativeBinaryBasePass)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	if (nullptr != m_pWeaponModelCom && !CNpcPresentationAssetService::Is_SaydonHammerSuppressed(m_pModelCom))
	{
		ANIMATION_MODEL_TARGET_VIEW weaponView;
		if (!Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView) ||
			FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &weaponView.BoneRoot)))
			return E_FAIL;
		const uint32_t iNumWeaponMeshes = m_pWeaponModelCom->Get_NumMeshes();
		for (uint32_t i = 0; i < iNumWeaponMeshes; ++i)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					*m_pWeaponModelCom, m_pShaderCom, i, {}, &m_HitFlash,
					nullptr, m_bNativeBinaryBasePass)) ||
				FAILED(m_pWeaponModelCom->Bind_BoneMatrices(
					m_pShaderCom, "g_BoneMatrices", i)) ||
				FAILED(m_pShaderCom->Begin(0)) ||
				FAILED(m_pWeaponModelCom->Render(i)))
				return E_FAIL;
		}
		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	if (m_fOutlineWidth > 0.f)
	{
		/* Pass 3 of the esther shader: front-culled hull pushed along the
		skinned normal, stencil-tested against the body drawn just above. */
		if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_OutlineWidth", &m_fOutlineWidth, sizeof(m_fOutlineWidth))) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_OutlineColor", &m_vOutlineColor, sizeof(m_vOutlineColor))))
			return E_FAIL;
		for (uint32_t i = 0; i < iNumMeshes; ++i)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
				FAILED(m_pModelCom->Bind_BoneMatrices(
					m_pShaderCom, "g_BoneMatrices", i)) ||
				FAILED(m_pShaderCom->Begin(3)) ||
				FAILED(m_pModelCom->Render(i)))
				return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CNpc::Ready_Components(const NPC_DESC* pDesc)
{
	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strShaderTag,
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;
	m_bNativeBinaryBasePass =
		pDesc->strShaderTag == TEXT("Prototype_Component_Shader_VtxAnimMeshBinary");

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;
	m_strModelTag = pDesc->strModelTag;
	m_strEffectV2BindingOwner = pDesc->strEffectV2BindingOwner;

	if (!pDesc->strWeaponModelTag.empty() || nullptr != pDesc->pWeaponSocketBone)
	{
		if (pDesc->strWeaponModelTag.empty() ||
			nullptr == pDesc->pWeaponSocketBone ||
			'\0' == pDesc->pWeaponSocketBone[0] ||
			!m_pModelCom->Has_Bone(pDesc->pWeaponSocketBone))
		{
			return E_INVALIDARG;
		}
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			pDesc->strWeaponModelTag,
			TEXT("Com_WeaponModel"),
			m_pWeaponModelCom)))
			return E_FAIL;
		m_strWeaponSocketBone = pDesc->pWeaponSocketBone;
		// Keep the actual loaded rest pose for body clips without a hammer counterpart.
		matrix_t local;
		for (uint32_t i = 0u; m_pWeaponModelCom->Get_BoneRestLocalMatrix(i, local); ++i)
		{
			float4x4_t stored;
			XMStoreFloat4x4(&stored, local);
			m_WeaponRestPose.push_back(stored);
		}
		m_pWeaponModelCom->Set_AnimPaused(true);
		m_pWeaponModelCom->Refresh_BoneCombinedMatrices();
	}

	if (pDesc->fCollisionRadius > 0.f)
	{
		Engine::CBounding_Sphere::BOUNDING_SPHERE_DESC colliderDesc{};
		colliderDesc.vCenter = float3_t(
			0.f, pDesc->fCollisionRadius, 0.f);
		colliderDesc.fRadius = pDesc->fCollisionRadius;
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			TEXT("Prototype_Component_Collider_WorldEntity"),
			TEXT("Com_CombatCollider"),
			m_pColliderCom,
			&colliderDesc)))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CNpc::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CNpc> CNpc::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CNpc>(new CNpc(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CNpc");

	return move(pInstance);
}

shared_ptr<CPrototype> CNpc::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CNpc>(new CNpc(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CNpc");

	return pInstance;
}
```

### Client/Public/KoukuSaydonPresentationPlayer.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "CardMazeVisualPolicy.h"
#include "KoukuSaydonCompositionDocument.h"
#include "KoukuSaydonPreviewRootMotion.h"
#include "Network/PacketMessages.h"
#include "HitAreaWire.h"
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace Engine { class CModel; }
namespace Client
{
class CNpc;
class CWorldSequencePlayer;
class CWorldSequenceDocument;
class CCharacter;
class CRenderingProfileService;
class CLightResourceCatalog;
class EFFECT_V2_CATALOG_SNAPSHOT;
class EFFECT_V2_PIVOT_HISTORY;
struct EFFECT_V2_TARGET;
struct EFFECT_V2_TARGET_VIEW;
struct ANIMATION_MODEL_TARGET_VIEW;
struct EFFECT_DOCUMENT_DESC;
struct EFFECT_SOURCE_MODEL_PREVIEW;
class DATA_JSON_VALUE;

struct KOUKU_BOSS_PRESENTATION_VIEW final
{
    std::weak_ptr<CNpc> pNpc;
    LostArk::Shared::WORLD_ENTITY_SNAPSHOT Snapshot;
    std::uint32_t iServerTick = 0;
    LostArk::Shared::NET_ENTITY_ID iOwnerBossNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
    std::string strArchetypeId;
};
struct KOUKU_CARD_PRESENTATION_VIEW final
{
    std::weak_ptr<CCharacter> pCharacter;
    LostArk::Shared::PLAYER_SNAPSHOT Snapshot;
};
struct KOUKU_MAZE_TARGET_VIEW final
{
    std::weak_ptr<CNpc> npc;
    std::uint32_t entityId = 0u;
    std::string archetypeId;
};

// The Server supplies identity and time. This owner only samples presentation
// resources and releases its own effects, audio and temporary scene/camera state.
class CKoukuSaydonPresentationPlayer final
{
public:
    CKoukuSaydonPresentationPlayer(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, CRenderingProfileService& profiles);
    ~CKoukuSaydonPresentationPlayer();
    bool Reload_Product(std::string& status, std::uint32_t expectedSourceRevision = 0u);
    static bool Is_TargetedCombatObjectArchetype(std::string_view archetypeId) noexcept;
    bool Start_TargetedCombatVisual(const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawn,
        std::string& status);
    bool Update_TargetedCombatVisual(const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot,
        std::uint32_t serverTick, std::string& status);
    void Stop_TargetedCombatVisual(LostArk::Shared::COMBAT_OBJECT_ID objectId);
    using WORLD_EMISSION_ANCHOR = std::function<bool_t(f32_t, float4x4_t&)>;
    static WORLD_EMISSION_ANCHOR Make_WorldEmissionAnchor(
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
        const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& occurrence);
    bool Resolve_ProductWorldEmissionAnchor(std::uint32_t sourceRevision, std::string_view patternId,
        std::string_view occurrenceId, WORLD_EMISSION_ANCHOR& out) const;
    void Set_LightResources(const CLightResourceCatalog* catalog) { m_pLightResources = catalog; }
    std::size_t Light_SkippedByBudget() const;
    void Update(float dt, const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
        const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    bool Begin_Preview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
        std::uint32_t clockMs, bool paused, std::string& status);
    bool Begin_BundlePreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const std::string& bundleId, std::uint32_t clockMs, bool paused, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr, bool automaticRootMotion = true,
        bool externalWorldPreview = false);
    // Optional Effect Workbench reference: existing actors and model sampler,
    // with Pattern presentation disabled and an external master clock. Only an
    // explicit source document enables validated actor-bound WORLD props.
    bool Begin_ModelReferencePreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const std::string& selectionId, bool bundle, std::uint32_t clockMs, bool paused, std::string& status,
        const CWorldSequenceDocument* propSequences = nullptr);
    void Sample_ModelReferencePreview(std::uint32_t clockMs, bool paused);
    bool Place_ModelReferenceRoot(const float4x4_t& root);
    bool Resolve_ModelReferenceWorldPivot(const std::string& memberId,
        const std::string& occurrenceId, float4x4_t& out) const;
    bool Resolve_ModelReferenceTarget(const std::string& memberId,
        EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const;
    // Source sockets use the same CNpc/CModel as the selected animation target.
    static bool Resolve_SourceAnchorWorlds(const EFFECT_DOCUMENT_DESC& document,
        const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root,
        std::unordered_map<std::string, float4x4_t>& anchors, std::string& error);
    static bool Sample_SourceAnchorWorlds(const EFFECT_DOCUMENT_DESC& document,
        const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root, float seconds,
        std::unordered_map<std::string, float4x4_t>& anchors, std::string& error,
        const EFFECT_SOURCE_MODEL_PREVIEW* sourceOverride = nullptr);
    using V1_SOURCE_ANCHOR_SAMPLER = std::function<bool(float, const float4x4_t&,
        std::unordered_map<std::string, float4x4_t>&, std::string&)>;
    bool Preview_IsModelReference() const { return m_bModelReferencePreview; }
    std::uint64_t Preview_Generation() const { return m_iPreviewGeneration; }
    bool Preview_IsBundle() const { return !m_PreviewBundleId.empty(); }
    // MainApp resolves this before WORLD/model sampling, for either clock owner.
    bool Resolve_PreviewCaptureClock(std::uint32_t requestedMs, std::uint32_t& effectiveMs);
    void Sample_Preview(std::uint32_t clockMs, bool playing, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model);
    void Set_PreviewPivot(const float4x4_t& pivot,
        const std::shared_ptr<Engine::CModel>& model);
    void Pause_Preview(bool paused);
    void Seek_Preview(std::uint32_t clockMs);
    void Stop_Preview();
    void Reset();
    // Called after ImGui NewFrame; sampling never draws from a loader/update thread.
    void Render_Debug() const;
    void Refresh_ColliderAuthoring(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::uint64_t generation);
    // Only the selected Collider/Effect placement changes; clocks and unrelated cues remain live.
    bool Preview_PresentationGeometry(const std::string& patternId,
        const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& occurrence);
    bool Consume_CompletedPreview(std::string& patternId);
    // Terminal WORLD failure is separate from natural completion and consumed once.
    bool Consume_FailedPreview(std::string& patternId, std::string& status);
    bool Preview_OwnsClock() const { return m_bOwnPreviewClock; }
    bool Preview_IsColliderResource() const { return m_bColliderResourcePreview; }
    bool Preview_Playing() const { return m_bPreviewPlaying; }
    bool Preview_HasActiveWorldBox(std::string_view occurrenceId) const;
    bool Preview_Paused() const { return m_bPreviewPaused; }
    std::uint32_t Preview_ClockMs() const { return static_cast<std::uint32_t>(m_fPreviewClockMs); }
    std::uint32_t Preview_DurationMs() const { return m_iPreviewDurationMs; }
    const std::string& Preview_PatternId() const { return m_PreviewPattern.strPatternId; }
    const std::string& Status() const { return m_strStatus; }
private:
    struct PLAYING_ROW final
    {
        KOUKU_SAYDON_PRESENTATION_KIND kind = KOUKU_SAYDON_PRESENTATION_KIND::EFFECT;
        std::uint32_t effectHandle = 0;
        std::uint64_t v1EffectHandle = 0;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> effectPivotHistory;
        V1_SOURCE_ANCHOR_SAMPLER sourceAnchorSampler;
        std::uint64_t soundHandle = 0;
        float lastAge = -1.f;
        float startMs = 0.f;
        std::uint32_t cameraDurationMs = 0u;
        float4x4_t pivot{};
        std::string assetId;
        float3_t cameraOffset{};
        HIT_AREA_SHAPE wire{};
        float4x4_t placementAnchor{};
        std::array<double, 3u> placementAnchorScale{1.0, 1.0, 1.0};
        bool hasPlacementAnchor = false;
        KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE lightBox;
        float lightWeight = 1.f;
        bool failed = false;
        std::string failureStatus;
        bool waitingForAnchor = false;
        bool debugRender = true;
    };
    struct EFFECT_ANCHOR_HISTORY final
    {
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> samples;
        float recordedSeconds = -1.f;
        float4x4_t recordedPivot{};
        bool missingSinceSample = false;
    };
    struct SESSION final
    {
        std::string key;
        float lastClockMs = -1.f;
        std::map<std::string, PLAYING_ROW> rows;
        std::uint32_t runEpoch = 0;
        std::string memberId;
        std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> rootHistory;
        float rootRecordedSeconds = -1.f;
        float4x4_t rootRecordedPivot{};
        // Pattern time, recorded before each following bone/WORLD cue starts.
        std::map<std::string, EFFECT_ANCHOR_HISTORY> effectAnchorHistories;
        std::map<std::string, std::shared_ptr<CWorldSequencePlayer>> previewWorlds;
    };
    struct PRODUCT_PATTERN final
    {
        KOUKU_SAYDON_COMPOSITION_DOCUMENT document;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::uint32_t durationMs = 0;
        std::map<std::string, WORLD_EMISSION_ANCHOR> worldEmissionAnchors;
    };
    struct LOGIC_PREVIEW_SPAWN final
    {
        SESSION session;
        float4x4_t pivot{};
    };
    struct LOGIC_PREVIEW_TRIGGER final
    {
        PRODUCT_PATTERN presentation;
        std::vector<LOGIC_PREVIEW_SPAWN> spawns;
        bool captured = false;
    };
    std::map<std::string, std::map<std::string, LOGIC_PREVIEW_TRIGGER>> m_LogicPreviewTriggers;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> m_LogicPreviewPlayers;
    void Sample_LogicPreview(SESSION& session, const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused);
    void Clear_LogicPreview();
    bool Collect_LogicPreviewEffects(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::set<std::string>& targets);
    struct PRODUCT_BUNDLE final
    {
        PRODUCT_PATTERN common;
        std::vector<std::string> patternIds;
    };
    struct TARGETED_COMBAT_VISUAL final
    {
        PRODUCT_PATTERN presentation;
        PRODUCT_PATTERN sourceBossPresentation;
        std::string archetypeId;
        bool loop = false;
    };
    struct TARGETED_COMBAT_SESSION final
    {
        std::shared_ptr<const TARGETED_COMBAT_VISUAL> definition;
        SESSION playback;
        SESSION sourceBossPlayback;
        LostArk::Shared::NET_ENTITY_ID sourceId = LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::GameplayDataRevision pinnedRevision{};
        std::uint32_t spawnTick = 0u, serverTick = 0u;
        double elapsedMs = 0.0;
        std::uint64_t cycle = 0u;
        float4x4_t root{};
        bool finished = false;
        std::string failure;
    };
    using TARGETED_COMBAT_VISUALS = std::map<std::string, std::shared_ptr<const TARGETED_COMBAT_VISUAL>>;
    static TARGETED_COMBAT_VISUALS Read_TargetedCombatVisuals(const DATA_JSON_VALUE& root);
    bool Sample_TargetedCombatVisual(TARGETED_COMBAT_SESSION& session,
        const KOUKU_BOSS_PRESENTATION_VIEW* sourceBoss = nullptr);
    void Update_TargetedCombatVisuals(float dt,
        const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses);
    struct BUNDLE_PREVIEW_MEMBER final
    {
        std::string memberId;
        std::uint32_t offsetTicks = 0, durationMs = 0;
        KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
        std::shared_ptr<CNpc> actor;
        std::string sourceArchetypeId;
        std::weak_ptr<CNpc> suppressedSourceActor;
        SESSION session;
        std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> animations;
        std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> facingStages;
        bool finiteActorLifetime = false;
        bool spatialLogicPreview = false;
        std::map<std::string, std::pair<uint32_t, uint32_t>> cloneAnimationWindows;
        std::map<std::string, float3_t> worldOffsets;
        std::uint32_t initialAnimation = 0;
        float initialTicks = 0.f;
        float initialYawDegrees = 0.f;
        float3_t initialPosition{};
        std::unique_ptr<CKoukuSaydonPreviewRootMotion> rootMotion;
        struct TARGET_TRACKING_WINDOW final
        {
            std::string occurrenceId;
            uint32_t startMs = 0u, durationMs = 0u, targetEntityId = 0u;
            bool immediate = false;
            // Each first-visited fixed tick pins the then-current replicated target
            // position. Replaying those inputs reproduces facing on any later seek.
            std::map<uint32_t, std::optional<float3_t>> targetSamples;
        };
        std::vector<TARGET_TRACKING_WINDOW> targetTracking;
        std::map<std::string, float> stageFacingYawDegrees;
        std::map<std::string, std::pair<uint32_t, float3_t>> airborneSelections;
        std::map<std::string, float3_t> airborneAppearancePositions;
        uint32_t airborneSelectionSeed = 0u;
    };
    bool Prepare_CloneSplitPreview(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        std::vector<BUNDLE_PREVIEW_MEMBER>& members, std::string& status);
    static const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* Resolve_PreviewAnimation(
        const BUNDLE_PREVIEW_MEMBER& member, double localMs,
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE*& previous);
    void Sample_BundlePreview();
    void Sync_PreviewSourceVisibility();
    bool Prepare_PreviewEffects();
    void Fail_Preview(std::string status);
    bool Sample_BundlePreviewFacing(BUNDLE_PREVIEW_MEMBER& member, double localMs);
    bool Sample_BundlePreviewPose(BUNDLE_PREVIEW_MEMBER& member, double localMs,
        float3_t& position, float& yaw, bool recordTargets);
    void Refresh_WorldPlacementAuthoring(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document);
    void Release_BundlePreviewMembers(std::vector<BUNDLE_PREVIEW_MEMBER>& members);
    struct CARD final
    {
        std::string assetId;
        std::uint32_t handle = 0;
        // Used only by the eight cardmaze mark/exit groups.
        CARD_MAZE_MARK_RETRY mazeRetry;
    };
    void Sync_MazeMark(CARD& mark, const std::string& asset, const float4x4_t& pivot);
    void Update_FearPresentation(float dt, const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    void Update_MazeMarks(const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players);
    void Sample(SESSION& session, const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
        const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
        const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
        const ANIMATION_MODEL_TARGET_VIEW* weaponView = nullptr);
    void Stop_Session(SESSION& session);
    bool Ensure_EffectResource(const std::string& kind, const std::string& asset,
        std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot);
    void Restore_Scene();
    void Refresh_SharedPresentation();
    void Collect_FrameLights();
    struct FRAME_LIGHT_PROVIDER;
    std::shared_ptr<FRAME_LIGHT_PROVIDER> m_LightProvider;
    const CLightResourceCatalog* m_pLightResources = nullptr;
    std::vector<float4x4_t> m_LightPlayerPivots;
    struct LIGHT_BOSS_FOLLOWER final
    {
        std::uint32_t entityId = 0u;
        std::weak_ptr<CNpc> npc;
    };
    std::map<std::uint32_t, std::vector<LIGHT_BOSS_FOLLOWER>> m_LightBossFollowers;
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    CRenderingProfileService& m_Profiles;
    std::map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> m_EffectResources;
    std::map<std::string, std::string> m_EffectResourceFailures;
    std::uint64_t m_iEffectCacheGeneration = 0u;
    std::set<std::string> m_QueuedV1Effects;
    std::uint64_t m_iV1CatalogRevision = 0u;
    TARGETED_COMBAT_VISUALS m_TargetedCombatVisuals;
    std::map<LostArk::Shared::COMBAT_OBJECT_ID, TARGETED_COMBAT_SESSION> m_TargetedCombatSessions;
    std::map<std::string, PRODUCT_PATTERN> m_FearPresentations;
    SESSION m_FearSession;
    std::string m_strCompletedFearKey;
    std::map<std::string, PRODUCT_PATTERN> m_Product;
    std::map<std::string, PRODUCT_BUNDLE> m_ProductBundles;
    SESSION m_ProductBundleSession;
    std::uint32_t m_iProductSourceRevision = 0u;
    std::uint32_t m_iProductReloadRunEpoch = 0u;
    std::set<std::string> m_MissingProductPatterns;
    std::map<std::uint32_t, SESSION> m_BossSessions;
    std::map<std::uint32_t, SESSION> m_ChildBossSessions;
    std::map<std::uint32_t, SESSION> m_MarioEntrySessions;
    std::map<std::uint32_t, CARD> m_Cards;
    std::map<std::uint32_t, CARD> m_MazeExits;
    std::map<std::uint32_t, CARD> m_MazePlayerMarks;
    std::map<std::uint32_t, CARD> m_MazeTargetMarks;
    /* One floor decal per painted bingo cell, keyed by cell index. */
    std::map<std::int32_t, CARD> m_BingoMarks;
    /* Keyed by the Server's bomb slot, so a mark turning into a planted
    bomb replaces the same entry instead of leaving two on screen. */
    std::map<std::int32_t, CARD> m_BingoBombs;
    std::map<std::string, bool> m_ColliderDebugOverrides;
    std::uint64_t m_iColliderAuthoringGeneration = UINT64_MAX;
    bool m_bProductLoaded = false, m_bProductAttempted = false;
    std::string m_strScenePrevious, m_strSceneOwner, m_strSceneApplied;
    bool m_bSceneUsed = false, m_bCameraUsed = false;
    SESSION m_PreviewSession;
    std::string m_PreviewBundleId;
    std::vector<BUNDLE_PREVIEW_MEMBER> m_BundlePreviewMembers;
    KOUKU_SAYDON_COMPOSITION_DOCUMENT m_PreviewDocument;
    KOUKU_SAYDON_COMPOSITION_PATTERN m_PreviewPattern;
    float4x4_t m_PreviewPivot{};
    std::weak_ptr<Engine::CModel> m_PreviewModel;
    std::string m_strCompletedPreviewPatternId;
    std::string m_strFailedPreviewPatternId, m_strFailedPreviewStatus;
    bool m_bOwnPreviewClock = false, m_bPreviewPlaying = false, m_bPreviewPaused = false;
    bool m_bPreviewClockAwaitingFirstUpdate = false;
    bool m_bPreviewPreparationQueued = false;
    std::vector<std::string> m_PreviewPreparationTargets;
    bool m_bPreviewCaptureClockHeld = false;
    bool m_bPreviewCaptureBoundarySampled = false, m_bPreviewCaptureAllowed = true;
    std::uint32_t m_iPreviewCaptureResumeMs = 0u, m_iPreviewCaptureBoundaryMs = 0u;
    std::uint32_t m_iPreviewCaptureWaitFrames = 0u;
    bool m_bPreviewPivotReady = false;
    bool m_bColliderResourcePreview = false;
    bool m_bModelReferencePreview = false;
    bool m_bBundleWorldExternal = false;
    std::uint64_t m_iPreviewGeneration = 0u;
    double m_fPreviewClockMs = 0;
    std::uint32_t m_iPreviewDurationMs = 0;
    std::string m_strStatus;
};
}
```

### Client/Private/KoukuSaydonPresentationPlayer.cpp

```cpp
#include <WinSock2.h>
#include "imgui.h"
#include "Engine_RenderTypes.h"
#include "KoukuSaydonPresentationPlayer.h"
#include <random>
#include "KoukuSaydonAnimationBlend.h"

#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"
#include "Character.h"
#include "DataJson.h"
#include "EffectV2_Catalog.h"
#include "Effect_PresentationService.h"
#include "Effect_Catalog.h"
#include "EffectCompositionModelPreview.h"
#include "Effect_Playback.h"
#include "NetworkManager.h"
#include "EffectV2_Object.h"
#include "EffectV2_Runtime.h"
#include "GameInstance.h"
#include "CombatHUDViewModel.h"
#include "Level_KakulSaydonArena.h"
#include "LightResourceCatalog.h"
#include "Presentation_Manager.h"
#include "Model.h"
#include "Npc.h"
#include "WorldSequencePlayer.h"
#include "WorldGameplayDocument.h"
#include "ProjectDataRoot.h"
#include "RenderingProfileService.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>

namespace
{
using namespace Client;
using RESOURCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE;
using OCCURRENCE = KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE;
using KIND = KOUKU_SAYDON_PRESENTATION_KIND;
constexpr std::uint32_t MAX_TIMELINE_MS = 600000u;

bool Validate_EffectAnchor(const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const OCCURRENCE& box, std::string& status)
{
    const auto reject = [&](const char* reason) {
        status = std::string(reason) + ": " + box.strOccurrenceId;
        return false;
    };
    const auto stableId = [](const std::string& value) {
        return !value.empty() && value.size() <= 128u && value != "." && value != ".." &&
            std::all_of(value.begin(), value.end(), [](const unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
            });
    };
    if ((box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" && box.strAnchorKind != "MAP") ||
        (box.strBoneTarget != "BODY" && box.strBoneTarget != "WEAPON") ||
        (!box.strBone.empty() && !stableId(box.strBone)) || box.iWorldEmissionIndex > 127u)
        return reject("Invalid Effect anchor, bone or World emission index");
    if (box.strAnchorKind == "MAP" && (box.bFollowBoss || !box.strBone.empty() ||
        box.strBoneTarget != "BODY" || !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty()))
        return reject("MAP Effect requires a fixed position without a bone or World dependency");
    if (box.strAnchorKind == "WORLD")
    {
        if (box.strWorldId.empty() || std::none_of(document.Worlds.begin(), document.Worlds.end(),
            [&](const auto& world) { return world.strWorldId == box.strWorldId; }))
            return reject("Effect needs an existing World anchor");
    }
    else if (!box.strWorldId.empty())
        return reject("Only a WORLD Effect can name a World anchor");
    if (box.strBoneTarget == "WEAPON" && (box.strAnchorKind != "BOSS" || box.strBone.empty()))
        return reject("WEAPON Effect requires a boss anchor and an explicit weapon bone");
    if (!box.strWorldOccurrenceId.empty())
    {
        const auto owner = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
            [&](const auto& world) { return world.strOccurrenceId == box.strWorldOccurrenceId; });
        const auto world = owner == pattern.WorldOccurrences.end() ? document.Worlds.end() :
            std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == owner->strWorldId; });
        if (!stableId(box.strWorldOccurrenceId) || world == document.Worlds.end())
            return reject("Effect needs an existing World occurrence in the same Pattern");
        if (box.strAnchorKind == "WORLD")
        {
            if (box.strWorldId != owner->strWorldId)
                return reject("Effect World occurrence must match its World anchor");
        }
        else if (world->strCompanionEffectResourceId != box.strResourceId ||
            std::any_of(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                [&](const auto& other) {
                    if (other.strOccurrenceId == box.strOccurrenceId || other.strAnchorKind == "WORLD" ||
                        other.strWorldOccurrenceId != box.strWorldOccurrenceId) return false;
                    return std::any_of(document.PresentationResources.begin(), document.PresentationResources.end(),
                        [&](const auto& resource) {
                            return resource.strResourceId == other.strResourceId && resource.eKind == KIND::EFFECT;
                        });
                }))
            return reject("Effect companion needs one matching World box/resource in the same Pattern");
    }
    return true;
}

std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> WorldPlacementFromOccurrence(
    const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& box)
{
    if (!box.Placement) return {};
    const auto& value = *box.Placement;
    return CWorldSequencePlayer::OBJECT_PLACEMENT{
        {float(value.Position[0]), float(value.Position[1]), float(value.Position[2])},
        {float(value.RotationDegrees[0]), float(value.RotationDegrees[1]), float(value.RotationDegrees[2])},
        {float(value.Scale[0]), float(value.Scale[1]), float(value.Scale[2])}};
}

std::uint32_t Camera_ReturnMs(const RESOURCE& resource, const bool authoring)
{
    if (resource.eKind != KIND::CAMERA) return 0u;
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level) return 0u;
    if (authoring) { std::string status; if (!level->Ensure_CameraShotAuthoring(status)) return 0u; }
    const auto& shots = authoring ? level->Get_CameraShots() : level->Get_PublishedCameraShots();
    const auto found = std::find_if(shots.begin(), shots.end(), [&](const auto& shot) { return shot.strShotId == resource.strAssetId; });
    return found == shots.end() ? 0u : found->iBlendOutMs;
}


const DATA_JSON_VALUE& Field(const DATA_JSON_VALUE& row, const char* key)
{
    const auto* value = row.Find(key);
    if (!value) throw std::runtime_error(std::string("Missing presentation field: ") + key);
    return *value;
}

std::string Text(const DATA_JSON_VALUE& row, const char* key, bool allowEmpty = false)
{
    const auto& value = Field(row, key);
    if (!value.Is_String() || value.Get_String().size() > 512u ||
        (!allowEmpty && value.Get_String().empty()) ||
        value.Get_String().find('\0') != std::string::npos)
        throw std::runtime_error(std::string("Invalid presentation string: ") + key);
    return value.Get_String();
}

double Number(const DATA_JSON_VALUE& row, const char* key, double low, double high)
{
    const auto& value = Field(row, key);
    if (!value.Is_Number() || !std::isfinite(value.Get_Number()) ||
        value.Get_Number() < low || value.Get_Number() > high)
        throw std::runtime_error(std::string("Invalid presentation number: ") + key);
    return value.Get_Number();
}

std::uint32_t UInt(const DATA_JSON_VALUE& row, const char* key,
    std::uint32_t low, std::uint32_t high)
{
    const double value = Number(row, key, low, high);
    if (std::floor(value) != value)
        throw std::runtime_error(std::string("Presentation integer required: ") + key);
    return static_cast<std::uint32_t>(value);
}

std::array<double, 3u> Vector(const DATA_JSON_VALUE& row, const char* key,
    double low, double high)
{
    const auto& value = Field(row, key);
    if (!value.Is_Array() || value.Get_Array().size() != 3u)
        throw std::runtime_error(std::string("Presentation vector required: ") + key);
    std::array<double, 3u> result{};
    for (size_t index = 0; index < result.size(); ++index)
    {
        const auto& part = value.Get_Array()[index];
        if (!part.Is_Number() || !std::isfinite(part.Get_Number()) ||
            part.Get_Number() < low || part.Get_Number() > high)
            throw std::runtime_error(std::string("Invalid presentation vector: ") + key);
        result[index] = part.Get_Number();
    }
    return result;
}

KIND Read_Kind(const std::string& kind)
{
    if (kind == "EFFECT") return KIND::EFFECT;
    if (kind == "SOUND") return KIND::SOUND;
    if (kind == "CAMERA") return KIND::CAMERA;
    if (kind == "COLLIDER") return KIND::COLLIDER;
    if (kind == "LIGHT") return KIND::LIGHT;
    if (kind == "SCENE_PROFILE") return KIND::SCENE_PROFILE;
    throw std::runtime_error("Unsupported presentation kind: " + kind);
}

RESOURCE Read_Resource(const DATA_JSON_VALUE& row)
{
    RESOURCE resource;
    resource.strResourceId = Text(row, "resourceId");
    resource.strDisplayName = resource.strResourceId;
    resource.eKind = Read_Kind(Text(row, "kind"));
    resource.strAssetId = Text(row, "assetId", resource.eKind == KIND::COLLIDER);
    resource.strResourceKind = Text(row, "resourceKind", resource.eKind != KIND::EFFECT);
    if (row.Find("elementId")) resource.strElementId = Text(row, "elementId", true);
    resource.iDurationMs = UInt(row, "resourceDurationMs", 1u, MAX_TIMELINE_MS);
    resource.strShape = Text(row, "shape");
    resource.HalfExtents = Vector(row, "halfExtents", 0.001, 100000.0);
    resource.fRadiusM = Number(row, "radiusM", 0.001, 100000.0);
    resource.fHalfAngleDegrees = Number(row, "halfAngleDegrees", resource.strShape == "REVERSE_SECTOR" ? 0.0 : 0.001, 180.0);
    const bool v1 = resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT";
    if ((resource.eKind == KIND::EFFECT && resource.strResourceKind != "GROUP" && resource.strResourceKind != "LEAF" && !v1) ||
        (resource.strShape != "BOX" && resource.strShape != "SECTOR" && resource.strShape != "REVERSE_SECTOR" && resource.strShape != "CIRCLE"))
        throw std::runtime_error("Invalid presentation resource type: " + resource.strResourceId);
    if (resource.eKind == KIND::EFFECT &&
        !CEffectV2Document::Is_ValidEffectId(resource.strAssetId))
        throw std::runtime_error("Invalid Effect identity: " + resource.strAssetId);
    const auto stableLightId = [](const std::string& id)
    {
        return !id.empty() && id.size() <= 128u && id != "." && id != ".." &&
            std::all_of(id.begin(), id.end(), [](unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'; });
    };
    if ((resource.strResourceKind == "V1_ELEMENT" && !stableLightId(resource.strElementId)) ||
        (resource.strResourceKind != "V1_ELEMENT" && !resource.strElementId.empty()))
        throw std::runtime_error("Invalid Effect element identity: " + resource.strElementId);
    if (resource.eKind == KIND::LIGHT && (!resource.strResourceKind.empty() ||
        !stableLightId(resource.strAssetId)))
        throw std::runtime_error("Invalid Light resource identity: " + resource.strAssetId);
    if (resource.eKind == KIND::SOUND &&
        (resource.strAssetId.rfind("Sound/", 0u) != 0u ||
         CRuntimeAssetRoot::Resolve(resource.strAssetId).empty()))
        throw std::runtime_error("Invalid Sound asset ID: " + resource.strAssetId);
    if (resource.eKind == KIND::COLLIDER && !resource.strAssetId.empty())
        throw std::runtime_error("Collider presentation cannot name an asset.");
    return resource;
}

OCCURRENCE Read_Occurrence(const DATA_JSON_VALUE& row, std::uint32_t durationMs, KIND kind)
{
    OCCURRENCE box;
    box.strOccurrenceId = Text(row, "occurrenceId");
    box.strResourceId = Text(row, "resourceId");
    box.iStartMs = UInt(row, "startMs", 0u, durationMs);
    box.iDurationMs = UInt(row, "durationMs", 1u, durationMs);
    if (box.iDurationMs > durationMs - box.iStartMs)
        throw std::runtime_error("Presentation occurrence exceeds its pattern.");
    box.PositionOffset = Vector(row, "positionOffset", -100000.0, 100000.0);
    box.RotationDegrees = Vector(row, "rotationDegrees", -100000.0, 100000.0);
    box.Scale = Vector(row, "scale", 0.001, 100000.0);
    // SCENE_PROFILE projects its reserved blendMs metadata into fadeInMs.
    // Its authoring range is the whole timeline, independent of this box's
    // lifetime; applying Effect envelope bounds here rejects every Product.
    box.iFadeInMs = UInt(row, "fadeInMs", 0u,
        kind == KIND::SCENE_PROFILE ? MAX_TIMELINE_MS : box.iDurationMs);
    box.iFadeOutMs = UInt(row, "fadeOutMs", 0u, box.iDurationMs);
    if (kind != KIND::SCENE_PROFILE &&
        box.iFadeInMs + box.iFadeOutMs > box.iDurationMs)
        throw std::runtime_error("Presentation fades exceed the occurrence.");
    box.fDissolveStart = Number(row, "dissolveStart", 0.0, 1.0);
    box.fDissolveEnd = Number(row, "dissolveEnd", 0.0, 1.0);
    if (box.fDissolveStart > box.fDissolveEnd)
        throw std::runtime_error("Presentation dissolve interval is reversed.");
    box.fVolume = Number(row, "volume", 0.0, 1.0);
    if (row.Find("brightnessMultiplier")) box.fBrightnessMultiplier = Number(row, "brightnessMultiplier", 0.0, 16.0);
    const auto& follow = Field(row, "followBoss");
    if (!follow.Is_Boolean()) throw std::runtime_error("followBoss must be Boolean.");
    box.bFollowBoss = follow.Get_Boolean();
    if (const auto* fit = row.Find("fitEffectToDuration"))
    {
        if (!fit->Is_Boolean()) throw std::runtime_error("fitEffectToDuration must be Boolean.");
        box.bFitEffectToDuration = fit->Get_Boolean();
        if (box.bFitEffectToDuration && (kind != KIND::EFFECT ||
            (Field(row, "resourceKind").Get_String() != "V1_EFFECT" && Field(row, "resourceKind").Get_String() != "V1_ELEMENT")))
            throw std::runtime_error("Fit Effect lifetime requires a V1 Effect resource.");
    }
    if (const auto* loop = row.Find("loopEffectToDuration"))
    {
        if (!loop->Is_Boolean()) throw std::runtime_error("loopEffectToDuration must be Boolean.");
        box.bLoopEffectToDuration = loop->Get_Boolean();
        if (box.bLoopEffectToDuration && (box.bFitEffectToDuration || kind != KIND::EFFECT ||
            (Field(row, "resourceKind").Get_String() != "V1_EFFECT" && Field(row, "resourceKind").Get_String() != "V1_ELEMENT")))
            throw std::runtime_error("Loop Effect lifetime requires a V1 Effect without time stretching.");
    }
    if (const auto* debug = row.Find("debugRender"))
    {
        if (!debug->Is_Boolean()) throw std::runtime_error("debugRender must be Boolean.");
        box.bDebugRender = debug->Get_Boolean();
    }
    box.strBone = Text(row, "bone", true);
    if (row.Find("boneTarget")) box.strBoneTarget = Text(row, "boneTarget");
    if (box.strBoneTarget != "BODY" && box.strBoneTarget != "WEAPON")
        throw std::runtime_error("Presentation boneTarget is unsupported.");
    if (box.strBone.size() > 128u) throw std::runtime_error("Presentation bone is too long.");
    if (row.Find("anchorKind")) box.strAnchorKind = Text(row, "anchorKind");
    if (row.Find("worldId")) box.strWorldId = Text(row, "worldId", true);
    if (row.Find("worldOccurrenceId")) box.strWorldOccurrenceId = Text(row, "worldOccurrenceId", true);
    if (row.Find("worldEmissionIndex")) box.iWorldEmissionIndex = UInt(row, "worldEmissionIndex", 0u, 127u);
    if (box.strAnchorKind != "BOSS" && box.strAnchorKind != "WORLD" &&
        !(kind == KIND::LIGHT && (box.strAnchorKind == "MAP" || box.strAnchorKind == "PLAYER")) &&
        !((kind == KIND::EFFECT || kind == KIND::COLLIDER) && box.strAnchorKind == "MAP"))
        throw std::runtime_error("Presentation anchorKind is unsupported.");
    if (kind == KIND::LIGHT && ((box.strAnchorKind != "WORLD" && !box.strWorldId.empty()) ||
        box.Scale != std::array<double, 3u>{1.0, 1.0, 1.0} ||
        (box.strAnchorKind != "BOSS" && !box.strBone.empty()) ||
        (box.strAnchorKind == "PLAYER" && !box.bFollowBoss)))
        throw std::runtime_error("Invalid Light anchor, scale or bone.");
    if ((kind == KIND::EFFECT || kind == KIND::COLLIDER) && box.strAnchorKind == "MAP" &&
        (box.bFollowBoss || !box.strBone.empty() || box.strBoneTarget != "BODY" ||
            !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() ||
            (kind == KIND::COLLIDER && box.iWorldEmissionIndex != 0u)))
        throw std::runtime_error("MAP Effect/Collider requires a fixed position without a bone or World occurrence.");
    if ((kind == KIND::EFFECT || kind == KIND::LIGHT || kind == KIND::COLLIDER) &&
        box.strAnchorKind == "WORLD" && box.strWorldId.empty())
        throw std::runtime_error("World Object anchor needs a worldId; fixed world coordinates use MAP: " + box.strOccurrenceId);
    if (box.strBoneTarget == "WEAPON" && ((kind != KIND::COLLIDER && kind != KIND::EFFECT) || box.strAnchorKind != "BOSS" || box.strBone.empty()))
        throw std::runtime_error("WEAPON bone target needs a Boss Collider/Effect and a named weapon bone.");
    return box;
}

DATA_JSON_VALUE Read_ProductPresentationRoot()
{
        const auto path = CProjectDataRoot::Resolve(
            "Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json");
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (error || bytes > 16u * 1024u * 1024u)
            throw std::runtime_error("KoukuSaydon Product presentation is missing or oversized.");
        std::ifstream input(path, std::ios::binary);
        const std::string text{ std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
        if (!input || input.bad() || text.size() != bytes)
            throw std::runtime_error("KoukuSaydon Product presentation read failed.");
        DATA_JSON_VALUE root;
        std::string parseStatus;
        if (!CDataJson::Parse(text, root, parseStatus) || !root.Is_Object())
            throw std::runtime_error("KoukuSaydon Product parse failed: " + parseStatus);
        if (Text(root, "schema") != "lostark.kouku-saydon-pattern-bindings" ||
            UInt(root, "formatVersion", 1u, 1u) != 1u ||
            Text(root, "bossArchetypeId") != "BOSS_KAKULSAYDON_G1_KOUKU")
            throw std::runtime_error("KoukuSaydon Product presentation header is incompatible.");
    return root;
}

struct PRESENTATION_WINDOW final
{
    KIND kind;
    std::int64_t startSubtick = 0, endSubtick = 0;
    std::string owner;
};
bool Admit_PresentationWindow(std::vector<PRESENTATION_WINDOW>& windows,
    KIND kind, double startMs, double endMs, const std::string& owner)
{
    const auto start = static_cast<std::int64_t>(std::llround(startMs * 30.0));
    const auto end = static_cast<std::int64_t>(std::llround(endMs * 30.0));
    for (const auto& window : windows)
        if (window.kind == kind && window.owner != owner &&
            start < window.endSubtick && window.startSubtick < end) return false;
    windows.push_back({ kind, start, end, owner });
    return true;
}

std::uint32_t Pattern_Duration(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern)
{
    std::uint64_t stageDuration = 0;
    for (const auto& stage : pattern.Stages) stageDuration += stage.iDurationMs;
    std::uint64_t duration = (std::max)(stageDuration, std::uint64_t(pattern.iDurationMs));
    for (const auto& row : pattern.LogicOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.SummonOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.PresentationOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.SceneProfileOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    for (const auto& row : pattern.WorldOccurrences)
        duration = (std::max)(duration, std::uint64_t(row.iStartMs) + row.iDurationMs);
    return static_cast<std::uint32_t>((std::min)(duration, std::uint64_t(MAX_TIMELINE_MS)));
}

bool Make_Pivot(const OCCURRENCE& box, const float4x4_t& root,
    const std::shared_ptr<Engine::CModel>& model, float4x4_t& result,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView = nullptr, float4x4_t* sampledBasis = nullptr)
{
    float4x4_t anchor = root;
    if (!box.strBone.empty())
    {
        EFFECT_V2_TARGET_VIEW view;
        if (box.strBoneTarget == "WEAPON")
        {
            if (!weaponView) return false;
            view.pModel = weaponView->Model;
            view.BoneRoot = weaponView->BoneRoot;
            view.YawBasis = weaponView->TargetRoot;
        }
        else if (box.strBoneTarget == "BODY")
        {
            view.pModel = model;
            view.BoneRoot = root;
            view.YawBasis = root;
        }
        else return false;
        if (!view.pModel || !view.pModel->Has_Bone(box.strBone.c_str())) return false;
        if (!CEffectV2Object::Resolve_TargetPivot(view, box.strBone,
            CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, anchor)) return false;
    }
    matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0; axis < 3u; ++axis)
    {
        if (XMVectorGetX(XMVector3LengthSq(basis.r[axis])) < 0.000001f) return false;
        basis.r[axis] = XMVector3Normalize(basis.r[axis]);
    }
    if (sampledBasis) XMStoreFloat4x4(sampledBasis, basis);
    XMStoreFloat4x4(&result,
        XMMatrixScaling(float(box.Scale[0]), float(box.Scale[1]), float(box.Scale[2])) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(float(box.RotationDegrees[0])),
            XMConvertToRadians(float(box.RotationDegrees[1])), XMConvertToRadians(float(box.RotationDegrees[2]))) *
        XMMatrixTranslation(float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2])) * basis);
    return true;
}

// Recorded resolved anchors contain WORLD scale but no editable occurrence geometry.
bool Make_ResolvedEffectPivot(const OCCURRENCE& box, const float4x4_t& anchor,
    float4x4_t& output)
{
    auto placed = box;
    placed.strBone.clear();
    const matrix_t basis = XMLoadFloat4x4(&anchor);
    for (size_t axis = 0u; axis < 3u; ++axis)
    {
        const float scale = XMVectorGetX(XMVector3Length(basis.r[axis]));
        placed.PositionOffset[axis] *= scale;
        placed.Scale[axis] *= scale;
    }
    return Make_Pivot(placed, anchor, {}, output);
}

CEffectV2Object::PIVOT_SAMPLER Effect_PivotSampler(const OCCURRENCE& box,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory,
    CEffectV2Object::PIVOT_SAMPLER exactRoot = {})
{
    if (box.strAnchorKind == "MAP" || !box.bFollowBoss) return {};
    if (exactRoot && box.strAnchorKind == "BOSS" && box.strBone.empty())
        return [box, exactRoot = std::move(exactRoot)](float seconds, float4x4_t& output, std::string& error) {
            float4x4_t root;
            return exactRoot(box.iStartMs / 1000.f + seconds, root, error) && Make_Pivot(box, root, {}, output);
        };
    const bool resolved = box.strAnchorKind == "WORLD" || !box.strBone.empty();
    return [box, history = resolved ? anchorHistory : rootHistory, resolved]
        (float seconds, float4x4_t& output, std::string& error)
    {
        if (!history) { error = "Effect anchor history is unavailable."; return false; }
        float4x4_t recorded;
        if (!history->Sample(box.iStartMs / 1000.f + seconds,
            recorded, error)) return false;
        if (resolved ? Make_ResolvedEffectPivot(box, recorded, output) :
            Make_Pivot(box, recorded, {}, output)) return true;
        error = "Recorded Effect anchor cannot form its authored pivot.";
        return false;
    };
}

using SOURCE_ATTACHMENTS = std::vector<EFFECT_ACTION_CUE_ATTACHMENT_DESC>;
using SOURCE_BONES = std::unordered_map<std::string, float4x4_t>;
using ANCHOR_ANIMATION = KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE;

SOURCE_ATTACHMENTS Source_Attachments(const EFFECT_DOCUMENT_DESC& document, const std::string& elementId = {})
{
    SOURCE_ATTACHMENTS result;
    for (const auto& element : document.Elements)
    {
        const auto& attachment = element.ActionCueAttachment;
        if (element.bVisible && (elementId.empty() || element.strElementId == elementId) &&
            attachment.bEnabled && attachment.bFollow && attachment.strModelCueId.empty())
            result.push_back(attachment);
    }
    return result;
}

bool Valid_SourceMatrix(const matrix_t& value)
{
    const float determinant = XMVectorGetX(XMMatrixDeterminant(value));
    return !XMMatrixIsNaN(value) && !XMMatrixIsInfinite(value) &&
        std::isfinite(determinant) && std::abs(determinant) > 1.e-12f;
}

bool Build_SourceAnchorWorlds(const SOURCE_ATTACHMENTS& attachments,
    const EFFECT_V2_TARGET_VIEW& view, const float4x4_t& root, const SOURCE_BONES& bones,
    SOURCE_BONES& anchors, std::string& error)
{
    SOURCE_BONES staged;
    float4x4_t ownerPivot;
    const bool needsBones = std::any_of(attachments.begin(), attachments.end(), [](const auto& value)
        { return value.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; });
    matrix_t delta = XMMatrixIdentity();
    if (needsBones)
    {
        if (!CEffectV2Object::Resolve_TargetPivot(view, "", CEffectV2Object::PIVOT_ROTATION::TARGET_YAW, ownerPivot) ||
            !Valid_SourceMatrix(XMLoadFloat4x4(&ownerPivot)) || !Valid_SourceMatrix(XMLoadFloat4x4(&root)))
        { error = "Kouku source attachment owner root is unavailable or singular."; return false; }
        delta = XMMatrixInverse(nullptr, XMLoadFloat4x4(&ownerPivot)) * XMLoadFloat4x4(&root);
    }
    for (const auto& attachment : attachments)
    {
        if (attachment.strRuntimeAnchorSlotId.empty())
        { error = "Kouku source attachment has no runtime slot."; return false; }
        matrix_t anchor;
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
        {
            const auto* camera = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
            if (!camera) { error = "Kouku source camera anchor is unavailable."; return false; }
            anchor = XMLoadFloat4x4(camera);
        }
        else
        {
            const auto bone = bones.find(attachment.strRuntimeBoneName);
            if (bone == bones.end())
            { error = "Kouku source bone is unavailable: " + attachment.strRuntimeBoneName; return false; }
            matrix_t raw = XMLoadFloat4x4(&bone->second);
            // Both Kouku/Saydon cooked rigs already carry a 100x root basis.
            // CModel applies the actor pre-scale (G1 0.017 -> 1.7, G2 Kouku
            // 0.012053 -> 1.2053). Metre-based offsets consume that basis
            // directly; preserve animated scale/translation without another x100.
            if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::BONE)
                anchor = raw * XMLoadFloat4x4(&view.BoneRoot);
            else if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
            {
                float4x4_t normalizedBone, yawAnchor;
                XMStoreFloat4x4(&normalizedBone, raw);
                if (!CEffectPlayback::Build_OwnerYawBoneAnchorWorld(normalizedBone, view.BoneRoot, view.YawBasis, yawAnchor))
                { error = "Kouku source owner-yaw anchor is invalid."; return false; }
                anchor = XMLoadFloat4x4(&yawAnchor);
            }
            else { error = "Unsupported Kouku source attachment orientation."; return false; }
        }
        const auto& local = attachment.SocketLocalTransform;
        matrix_t world = XMMatrixScaling(local.vScale.x, local.vScale.y, local.vScale.z) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(local.vRotationDegrees.x),
                XMConvertToRadians(local.vRotationDegrees.y), XMConvertToRadians(local.vRotationDegrees.z)) *
            XMMatrixTranslation(local.vPosition.x, local.vPosition.y, local.vPosition.z) * anchor;
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW) world *= delta;
        if (!Valid_SourceMatrix(world))
        { error = "Kouku source attachment matrix is invalid: " + attachment.strRuntimeAnchorSlotId; return false; }
        float4x4_t value;
        XMStoreFloat4x4(&value, world);
        const auto [found, inserted] = staged.emplace(attachment.strRuntimeAnchorSlotId, value);
        if (!inserted)
            for (size_t r = 0u; r < 4u; ++r) for (size_t c = 0u; c < 4u; ++c)
                if (std::abs(found->second.m[r][c] - value.m[r][c]) > 0.0001f)
                { error = "Conflicting Kouku source attachment slot: " + attachment.strRuntimeAnchorSlotId; return false; }
    }
    anchors = std::move(staged);
    error.clear();
    return true;
}

bool Sample_SourceBones(const std::shared_ptr<CModel>& model,
    const std::vector<ANCHOR_ANIMATION>& animations,
    std::span<const KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> blendWindows, float sampleMs,
    const std::vector<std::string>& names, SOURCE_BONES& bones, std::string& error)
{
    if (names.empty()) return true;
    if (!model) { error = "Kouku source animation model was released."; return false; }
    const ANCHOR_ANIMATION* animation = nullptr;
    const ANCHOR_ANIMATION* previous = nullptr;
    for (const auto& value : animations)
        if (sampleMs >= (value.iPoseStartMs == UINT32_MAX ? value.iStartOffsetMs : value.iPoseStartMs))
        { previous = animation; animation = &value; }
    // A leading Effect samples the scheduled clip's first pose before playback starts.
    if (!animation && !animations.empty()) animation = &animations.front();
    if (!animation) { error = "Kouku source attachment has no animation at its requested time."; return false; }
    const auto clipIndex = [&](const std::string& name) {
        uint32_t found = UINT32_MAX;
        for (uint32_t index = 0u; index < model->Get_NumAnimations(); ++index)
            if (const auto* value = model->Get_AnimationName(index); value && name == value)
            { if (found != UINT32_MAX) return UINT32_MAX; found = index; }
        return found;
    };
    const uint32_t index = clipIndex(animation->strRuntimeClip);
    float cursor = 0.f, duration = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, cursor, duration))
    { error = "Kouku source animation clip is unavailable: " + animation->strRuntimeClip; return false; }
    const float age = (std::max)(0.f, sampleMs - animation->iStartOffsetMs);
    const bool loop = animation->strEndPolicy == "LOOP_TO_WINDOW";
    const float elapsed = (std::min)(age, float(animation->iPlayMs));
    const float tps = model->Get_AnimationTickPerSecond(index);
    double sourceMs = 0.0;
    if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(animation->iSourceStartMs,
        animation->iSourceEndMs, elapsed, animation->fPlayRate, duration * 1000.0 / tps, loop, sourceMs))
    { error = "Kouku source animation range is outside the native clip."; return false; }
    const float ticks = float(sourceMs * tps / 1000.0);
    std::vector<uint32_t> indices;
    for (const auto& name : names)
    {
        const auto bone = model->Find_BoneIndex(name.c_str());
        if (bone < 0) { error = "Kouku source bone is unavailable: " + name; return false; }
        indices.push_back(static_cast<uint32_t>(bone));
    }
    std::vector<float4x4_t> sampled(indices.size());
    bool sampledPose = false;
    CModel::ANIMATION_TRANSITION_POSE logicPose;
    bool logicActive = false;
    if (!CKoukuSaydonAnimationBlend::Sample_Pose(*model, blendWindows, sampleMs, logicPose, logicActive, error)) return false;
    if (logicActive) sampledPose = model->Sample_AnimationTransitionBoneCombinedMatrices(logicPose, indices, sampled);
    else if (previous && animation->iBlendInMs && age < animation->iBlendInMs)
    {
        CModel::ANIMATION_TRANSITION_POSE pose;
        pose.sourceIndex = clipIndex(previous->strRuntimeClip);
        pose.targetIndex = index; pose.targetTicks = ticks;
        pose.durationSeconds = animation->iBlendInMs * .001f;
        pose.elapsedSeconds = age * .001f; pose.playRate = animation->fPlayRate;
        float previousDuration = 0.f;
        if (pose.sourceIndex == UINT32_MAX || !model->Get_AnimationProgress(pose.sourceIndex, cursor, previousDuration))
        { error = "Kouku source animation blend clip is unavailable."; return false; }
        const float previousTps = model->Get_AnimationTickPerSecond(pose.sourceIndex);
        double previousMs = 0.0;
        if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(previous->iSourceStartMs,
            previous->iSourceEndMs, previous->iPlayMs, previous->fPlayRate,
            previousDuration * 1000.0 / previousTps, previous->strEndPolicy == "LOOP_TO_WINDOW", previousMs))
        { error = "Kouku source animation blend range is outside the native clip."; return false; }
        pose.sourceTicks = float(previousMs * previousTps / 1000.0);
        sampledPose = model->Sample_AnimationTransitionBoneCombinedMatrices(pose, indices, sampled);
    }
    else sampledPose = model->Sample_AnimationBoneCombinedMatrices(animation->strRuntimeClip.c_str(), ticks, indices, sampled);
    if (!sampledPose) { error = "Kouku source animation pose sample failed."; return false; }
    for (size_t i = 0u; i < names.size(); ++i) bones.emplace(names[i], sampled[i]);
    return true;
}

CKoukuSaydonPresentationPlayer::V1_SOURCE_ANCHOR_SAMPLER Make_SourceAnchorSampler(
    const RESOURCE& resource, const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const std::shared_ptr<CModel>& model, const float4x4_t& ownerRoot, uint32_t startMs,
    CEffectV2Object::PIVOT_SAMPLER exactRoot = {})
{
    const auto document = CEffectCatalog::Find_Loaded(resource.strAssetId);
    if (!document) return [](float, const float4x4_t&, SOURCE_BONES&, std::string& error)
        { error = "Prepared Kouku source Effect document is unavailable."; return false; };
    auto attachments = Source_Attachments(*document, resource.strElementId);
    if (attachments.empty()) return {};
    std::vector<ANCHOR_ANIMATION> animations;
    uint32_t stageStart = 0u;
    for (const auto& stage : pattern.Stages)
    {
        for (auto animation : stage.AnimationOccurrences)
        {
            if (animation.iPoseStartMs == UINT32_MAX)
                animation.iPoseStartMs = stageStart + (animation.strOccurrenceId == std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; })->strOccurrenceId ? 0u : animation.iStartOffsetMs);
            animation.iStartOffsetMs += stageStart; animations.push_back(std::move(animation));
        }
        stageStart += stage.iDurationMs;
    }
    std::stable_sort(animations.begin(), animations.end(), [](const auto& a, const auto& b)
        { return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
    std::vector<std::string> names;
    for (const auto& attachment : attachments)
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
            std::find(names.begin(), names.end(), attachment.strRuntimeBoneName) == names.end())
            names.push_back(attachment.strRuntimeBoneName);
    // A resource with no saved model animation previews the selected pose.
    // Product/Pattern timelines still require their authored animation history.
    SOURCE_BONES frozenBones;
    std::string frozenError;
    const bool freezeResourcePose = animations.empty() && (pattern.strPatternId == "preview.kouku.resource" ||
        pattern.strPatternId == "preview.kouku.resource.actor");
    if (freezeResourcePose)
        for (const auto& name : names)
        {
            if (!model || !model->Has_Bone(name.c_str()))
            { frozenError = "Selected Resource Preview model has no source bone: " + name; break; }
            float4x4_t value;
            XMStoreFloat4x4(&value, model->Get_BoneMatrix(name.c_str()));
            frozenBones.emplace(name, value);
        }
    return [attachments = std::move(attachments), animations = std::move(animations), names = std::move(names),
        frozenBones = std::move(frozenBones), frozenError = std::move(frozenError), freezeResourcePose,
        blendWindows = pattern.AnimationBlendWindows, weakModel = std::weak_ptr<CModel>(model), ownerRoot, startMs, exactRoot = std::move(exactRoot)]
        (float seconds, const float4x4_t& root, SOURCE_BONES& anchors, std::string& error)
    {
        if (!std::isfinite(seconds) || seconds < 0.f) { error = "Invalid Kouku source anchor sample time."; return false; }
        if (std::any_of(attachments.begin(), attachments.end(), [](const auto& value)
            { return value.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW; }))
        { error = "Kouku Product camera-view source attachment requires recorded camera history."; return false; }
        EFFECT_V2_TARGET_VIEW view;
        view.pModel = weakModel.lock(); view.BoneRoot = ownerRoot; view.YawBasis = ownerRoot;
        if (exactRoot)
        {
            if (!exactRoot(startMs / 1000.f + seconds, view.BoneRoot, error)) return false;
            view.YawBasis = view.BoneRoot;
        }
        if (freezeResourcePose)
        {
            if (!frozenError.empty()) { error = frozenError; return false; }
            return Build_SourceAnchorWorlds(attachments, view, root, frozenBones, anchors, error);
        }
        SOURCE_BONES bones;
        return Sample_SourceBones(view.pModel, animations, blendWindows, startMs + seconds * 1000.f, names, bones, error) &&
            Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
    };
}

float Effect_SourceClockRate(const RESOURCE& resource, const OCCURRENCE& box)
{
    return box.bFitEffectToDuration ? float(resource.iDurationMs) / float(box.iDurationMs) : 1.f;
}

EFFECT_FIXED_STEP_TRANSFORM_PROVIDER Effect_V1TransformProvider(const OCCURRENCE& box,
    const float4x4_t& frozenPivot, const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& rootHistory,
    const std::shared_ptr<EFFECT_V2_PIVOT_HISTORY>& anchorHistory,
    CKoukuSaydonPresentationPlayer::V1_SOURCE_ANCHOR_SAMPLER sourceAnchors,
    float sourceSecondsPerBoxSecond = 1.f, CEffectV2Object::PIVOT_SAMPLER exactRoot = {})
{
    return [sampler = Effect_PivotSampler(box, rootHistory, anchorHistory, std::move(exactRoot)), frozenPivot,
        sourceAnchors = std::move(sourceAnchors), sourceSecondsPerBoxSecond]
        (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error)
    {
        // Source particles keep their own clock; owner/bone history keeps the box clock.
        seconds /= sourceSecondsPerBoxSecond;
        output.RootWorld = frozenPivot;
        output.SourceAnchorWorlds.clear();
        if (sampler && !sampler(seconds, output.RootWorld, error)) return false;
        if (sourceAnchors && !sourceAnchors(seconds, output.RootWorld, output.SourceAnchorWorlds, error)) return false;
        error.clear();
        return true;
    };
}

// A centered WORLD circle is a ground-plane radius proxy. Uniform model
// scale changes its radius; mesh roll/pitch and self-spin do not tilt the proxy.
bool Is_CenteredWorldCircle(const RESOURCE& resource, const OCCURRENCE& box)
{
    return resource.eKind == KIND::COLLIDER && resource.strShape == "CIRCLE" &&
        box.strAnchorKind == "WORLD" && box.strBone.empty() &&
        std::all_of(box.PositionOffset.begin(), box.PositionOffset.end(), [](const double v) { return v == 0.0; }) &&
        std::abs(box.Scale[0] - box.Scale[1]) <= .0001 && std::abs(box.Scale[0] - box.Scale[2]) <= .0001;
}

void Flatten_CenteredWorldCircle(const RESOURCE& resource, const OCCURRENCE& box, float4x4_t& anchor)
{
    if (!Is_CenteredWorldCircle(resource, box)) return;
    const matrix_t world = XMLoadFloat4x4(&anchor);
    const float sx = XMVectorGetX(XMVector3Length(world.r[0]));
    const float sy = XMVectorGetX(XMVector3Length(world.r[1]));
    const float sz = XMVectorGetX(XMVector3Length(world.r[2]));
    // Keep legacy nonuniform upright colliders on their existing path. The
    // gameplay publisher rejects nonuniform spinning circle sources.
    if (!std::isfinite(sx) || sx <= 0.f || std::abs(sx - sy) > .0001f || std::abs(sx - sz) > .0001f) return;
    matrix_t upright = XMMatrixIdentity();
    upright.r[3] = world.r[3];
    XMStoreFloat4x4(&anchor, upright);
}

HIT_AREA_SHAPE Collider_Wire(const RESOURCE& resource, const OCCURRENCE& box)
{
    HIT_AREA_SHAPE shape;
    if (resource.strShape == "BOX")
    {
        const double halfWidth = resource.HalfExtents[0] * box.Scale[0];
        const double halfLength = resource.HalfExtents[2] * box.Scale[2];
        shape.fBoxHalfHeightM = static_cast<float>(resource.HalfExtents[1] * box.Scale[1]);
        shape.iAreaType = 2;
        shape.iAreaRange = static_cast<int32_t>((std::min)(halfLength * 200.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>((std::min)(halfWidth * 200.0, 1000000000.0));
        shape.iAreaOffsetX = -shape.iAreaRange / 2;
    }
    else
    {
        shape.iAreaType = resource.strShape == "CIRCLE" ? 1 : 3;
        shape.iAreaRange = static_cast<int32_t>((std::min)(
            resource.fRadiusM * (std::max)(box.Scale[0], box.Scale[2]) * 100.0, 1000000000.0));
        shape.iAreaAngle = static_cast<int32_t>(resource.fHalfAngleDegrees * 2.0);
        if (shape.iAreaType == 3)
        {
            shape.fSectorRadiusXM = static_cast<float>(resource.fRadiusM * box.Scale[0]);
            shape.fSectorRadiusZM = static_cast<float>(resource.fRadiusM * box.Scale[2]);
            shape.fSectorAngleDegrees = static_cast<float>(resource.fHalfAngleDegrees * 2.0);
            shape.bReverseSector = resource.strShape == "REVERSE_SECTOR";
        }
    }
    return shape;
}

#ifdef _DEBUG
void Draw_LightWire(const LIGHT_DESC& light)
{
    if (light.eType == LIGHT::DIRECTIONAL) return;
    auto& game = CGameInstance::Get();
    const matrix_t view = XMLoadFloat4x4(game.Get_Transform(D3DTS::VIEW));
    const matrix_t projection = XMLoadFloat4x4(game.Get_Transform(D3DTS::PROJ));
    auto* viewport = ImGui::GetMainViewport();
    auto* draw = ImGui::GetBackgroundDrawList(viewport);
    const auto project = [&](fvector_t world, ImVec2& out)
    {
        const vector_t v = XMVector3TransformCoord(world, view);
        if (XMVectorGetZ(v) <= .1f) return false;
        const vector_t p = XMVector3TransformCoord(v, projection);
        out = {viewport->Pos.x + (XMVectorGetX(p) * .5f + .5f) * viewport->Size.x,
            viewport->Pos.y + (.5f - XMVectorGetY(p) * .5f) * viewport->Size.y};
        return std::isfinite(out.x) && std::isfinite(out.y);
    };
    const auto line = [&](fvector_t a, fvector_t b)
    { ImVec2 pa{}, pb{}; if (project(a, pa) && project(b, pb)) draw->AddLine(pa, pb, IM_COL32(255, 224, 80, 220), 1.5f); };
    const vector_t origin = XMLoadFloat4(&light.vPosition);
    const bool spot = light.eType == LIGHT::SPOT;
    for (int ring = 0; ring < (spot ? 1 : 3); ++ring)
    {
        const vector_t direction = spot ? XMVector3Normalize(XMLoadFloat4(&light.vDirection)) :
            ring == 0 ? XMVectorSet(0.f, 1.f, 0.f, 0.f) : ring == 1 ? XMVectorSet(1.f, 0.f, 0.f, 0.f) : XMVectorSet(0.f, 0.f, 1.f, 0.f);
        const vector_t up = std::abs(XMVectorGetY(direction)) > .99f ? XMVectorSet(0.f, 0.f, 1.f, 0.f) : XMVectorSet(0.f, 1.f, 0.f, 0.f);
        const vector_t right = XMVector3Normalize(XMVector3Cross(direction, up));
        const vector_t forward = XMVector3Normalize(XMVector3Cross(right, direction));
        const float angle = spot ? std::acos(std::clamp(light.fSpotOuterCos, -1.f, 1.f)) : 0.f;
        const float radius = spot ? light.fRange * std::sin(angle) : light.fRange;
        const vector_t center = spot ? origin + direction * (light.fRange * std::cos(angle)) : origin;
        for (int i = 0; i < 48; ++i)
        {
            const float a = XM_2PI * float(i) / 48.f, b = XM_2PI * float(i + 1) / 48.f;
            const vector_t p = center + (right * std::cos(a) + forward * std::sin(a)) * radius;
            line(p, center + (right * std::cos(b) + forward * std::sin(b)) * radius);
            if (spot && i % 12 == 0) line(origin, p);
        }
    }
}
#endif

std::string Card_Asset(const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
{
    using namespace LostArk::Shared;
    // Roulette retains its overhead card; maze assignment now lives on the floor.
    const MECHANIC_CARD_SYMBOL cardSymbol = snapshot.eMechanicCardSymbol;
    const MECHANIC_CARD_COLOR cardColor = snapshot.eMechanicCardColor;
    const char* symbol = nullptr;
    switch (cardSymbol)
    {
    case MECHANIC_CARD_SYMBOL::HEART: symbol = "heart"; break;
    case MECHANIC_CARD_SYMBOL::SPADE: symbol = "spade"; break;
    case MECHANIC_CARD_SYMBOL::CLUB: symbol = "clober"; break;
    case MECHANIC_CARD_SYMBOL::DIAMOND: symbol = "dia"; break;
    default: return {};
    }
    const char* color = nullptr;
    switch (cardColor)
    {
    case MECHANIC_CARD_COLOR::RED: color = "red"; break;
    case MECHANIC_CARD_COLOR::BLACK: color = "black"; break;
    default: return {};
    }
    return std::string("boss.kouku.card.") + symbol + "." + color;
}
}

struct Client::CKoukuSaydonPresentationPlayer::FRAME_LIGHT_PROVIDER final : Engine::IPresentationProvider
{
    struct FRAME_LIGHT final { LIGHT_DESC desc; bool debugRender = false; };
    std::vector<FRAME_LIGHT> lights;
    std::size_t skippedByBudget = 0u;
    HRESULT Submit_Presentation() override
    {
        auto& presentation = CPresentation_Manager::Get();
        const auto used = presentation.Get_TransientLights().size();
        constexpr auto capacity = CPresentation_Manager::TRANSIENT_LIGHT_CAPACITY;
        const std::size_t remaining = used < capacity ? capacity - used : 0u;
        const auto count = (std::min)(remaining, lights.size());
        skippedByBudget = lights.size() - count;
        // Validation is finished before this provider joins the frame transaction.
        presentation.Register_ProviderSubmissionExpectation(lights.size(), count, 0u, 0u);
        for (std::size_t i = 0u; i < count; ++i)
            if (FAILED(presentation.Add_TransientLight(lights[i].desc))) return E_FAIL;
        return S_OK;
    }
};

std::size_t Client::CKoukuSaydonPresentationPlayer::Light_SkippedByBudget() const
{
    return m_LightProvider ? m_LightProvider->skippedByBudget : 0u;
}

void Client::CKoukuSaydonPresentationPlayer::Collect_FrameLights()
{
    if (!m_LightProvider) m_LightProvider = std::make_shared<FRAME_LIGHT_PROVIDER>();
    m_LightProvider->lights.clear();
    if (!m_pLightResources) return;
    const auto collect = [&](const SESSION& session, bool preview, std::uint32_t bossId = 0u)
    {
        for (const auto& [id, row] : session.rows)
        {
            if (row.kind != KIND::LIGHT || row.failed || row.waitingForAnchor || row.lightWeight <= 0.f) continue;
            if (!m_FearSession.rows.empty() && &session != &m_FearSession) continue;
            const auto* resource = preview ? m_pLightResources->Find_Resource(row.assetId) :
                m_pLightResources->Find_RuntimeResource(row.assetId);
            if (!resource)
            {
                m_strStatus = "Light resource unavailable: " + row.assetId + "; other rows preserved.";
                continue;
            }
            const auto append = [&](const float4x4_t& pivot, const float anchorHeight)
            {
                LIGHT_DESC desc{};
                std::string status;
                if (!CLightResourceCatalog::Try_BuildAnchoredLightDesc(*resource, row.lightBox.strAnchorKind,
                    pivot, anchorHeight, row.lightWeight, desc, status))
                { m_strStatus = "Light occurrence isolated: " + id + "; " + status; return; }
                m_LightProvider->lights.push_back({desc, row.debugRender});
            };
            if (row.lightBox.strAnchorKind == "PLAYER")
            {
                // No present character is a temporary empty target set, never a sticky row failure.
                for (const auto& root : m_LightPlayerPivots)
                {
                    float4x4_t pivot;
                    if (Make_Pivot(row.lightBox, root, nullptr, pivot)) append(pivot, root._42);
                }
            }
            else
            {
                append(row.pivot, row.placementAnchor._42);
                // A Server-owned same-body actor shares its owner's following
                // spotlight. Gaze clones retain their own transform and animation;
                // the original occurrence still owns timing, fade and brightness.
                if (preview || resource->eType != LIGHT::SPOT ||
                    row.lightBox.strAnchorKind != "BOSS" || !row.lightBox.bFollowBoss) continue;
                const auto followers = m_LightBossFollowers.find(bossId);
                if (followers == m_LightBossFollowers.end()) continue;
                for (const auto& follower : followers->second)
                {
                    const auto own = m_BossSessions.find(follower.entityId);
                    const bool hasOwnLight = own != m_BossSessions.end() &&
                        std::any_of(own->second.rows.begin(), own->second.rows.end(),
                            [&](const auto& entry)
                            {
                                const auto& candidate = entry.second;
                                return candidate.kind == KIND::LIGHT && candidate.assetId == row.assetId &&
                                    candidate.lightBox.strAnchorKind == "BOSS" && !candidate.failed &&
                                    !candidate.waitingForAnchor && candidate.lightWeight > 0.f;
                            });
                    if (hasOwnLight) continue;
                    const auto npc = follower.npc.lock();
                    float4x4_t pivot;
                    if (npc && npc->Get_Transform() && Make_Pivot(row.lightBox,
                        *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), pivot))
                        append(pivot, npc->Get_Transform()->Get_WorldMatrixPtr()->_42);
                }
            }
        }
    };
    for (const auto& [id, session] : m_BossSessions) collect(session, false, id);
    for (const auto& [id, session] : m_ChildBossSessions) collect(session, false, id);
    for (const auto& [id, session] : m_MarioEntrySessions) collect(session, false, id);
    if (m_bPreviewPlaying) collect(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) collect(member.session, true);
    collect(m_FearSession, false);
    if (!m_LightProvider->lights.empty())
        if (FAILED(CPresentation_Manager::Get().Add_FrameProvider(m_LightProvider)))
            m_strStatus = "Light frame provider registration failed.";
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_SourceAnchorWorlds(
    const EFFECT_DOCUMENT_DESC& document, const EFFECT_V2_TARGET_VIEW& view,
    const float4x4_t& root, std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    const auto attachments = Source_Attachments(document);
    SOURCE_BONES bones;
    for (const auto& attachment : attachments)
    {
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW ||
            bones.contains(attachment.strRuntimeBoneName)) continue;
        if (!view.pModel || !view.pModel->Has_Bone(attachment.strRuntimeBoneName.c_str()))
        { error = "Selected Kouku model has no source bone: " + attachment.strRuntimeBoneName; return false; }
        float4x4_t value;
        XMStoreFloat4x4(&value, view.pModel->Get_BoneMatrix(attachment.strRuntimeBoneName.c_str()));
        bones.emplace(attachment.strRuntimeBoneName, value);
    }
    return Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_SourceAnchorWorlds(
    const EFFECT_DOCUMENT_DESC& document, const EFFECT_V2_TARGET_VIEW& view,
    const float4x4_t& root, const float seconds, SOURCE_BONES& anchors, std::string& error,
    const EFFECT_SOURCE_MODEL_PREVIEW* sourceOverride)
{
    const auto* sourceModel = sourceOverride ? sourceOverride :
        (document.SourceModelPreview ? &*document.SourceModelPreview : nullptr);
    if (!sourceModel) return Resolve_SourceAnchorWorlds(document, view, root, anchors, error);
    if (!std::isfinite(seconds) || seconds < 0.f)
    { error = "Source model animation time must be finite and nonnegative."; return false; }
    const auto attachments = Source_Attachments(document);
    std::vector<std::string> names;
    for (const auto& attachment : attachments)
        if (attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
            std::find(names.begin(), names.end(), attachment.strRuntimeBoneName) == names.end())
            names.push_back(attachment.strRuntimeBoneName);
    std::vector<ANCHOR_ANIMATION> animations;
    for (const auto& source : sourceModel->Animations)
    {
        ANCHOR_ANIMATION animation;
        animation.strRuntimeClip = source.strRuntimeClip;
        animation.iStartOffsetMs = source.iStartOffsetMs; animation.iSourceStartMs = source.iSourceStartMs;
        animation.iPoseStartMs = source.iStartOffsetMs;
        animation.iPlayMs = source.iPlayMs; animation.fPlayRate = source.fPlayRate; animation.strEndPolicy = source.strEndPolicy;
        animations.push_back(std::move(animation));
    }
    SOURCE_BONES bones;
    return Sample_SourceBones(view.pModel, animations, {}, seconds * 1000.f, names, bones, error) &&
        Build_SourceAnchorWorlds(attachments, view, root, bones, anchors, error);
}

Client::CKoukuSaydonPresentationPlayer::CKoukuSaydonPresentationPlayer(
    ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context,
    CRenderingProfileService& profiles)
    : m_Device(std::move(device)), m_Context(std::move(context)), m_Profiles(profiles)
{
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
}

Client::CKoukuSaydonPresentationPlayer::~CKoukuSaydonPresentationPlayer()
{
    Reset();
}

Client::CKoukuSaydonPresentationPlayer::WORLD_EMISSION_ANCHOR
Client::CKoukuSaydonPresentationPlayer::Make_WorldEmissionAnchor(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
    const KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION& world,
    const KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE& occurrence)
{
    if (!pattern.BossMotion || occurrence.Placement || world.strAnchorKind != "BOSS_SPAWN") return {};
    KOUKU_SAYDON_COMPOSITION_PATTERN sampled;
    sampled.BossMotion = pattern.BossMotion;
    return [sampled = std::move(sampled), world, startMs = occurrence.iStartMs](
        const f32_t birthMs, float4x4_t& out)
    {
        std::array<double, 3u> position{};
        double yaw = 0.0;
        if (!Sample_KoukuSaydonBossMotion(sampled, double(startMs) + birthMs, position, yaw)) return false;
        for (std::size_t axis = 0; axis < position.size(); ++axis)
            position[axis] += world.PositionOffset[axis] - world.AnchorPosition[axis];
        XMStoreFloat4x4(&out, XMMatrixRotationY(XMConvertToRadians(float(yaw))) *
            XMMatrixTranslation(float(position[0]), float(position[1]), float(position[2])));
        return true;
    };
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ProductWorldEmissionAnchor(
    const std::uint32_t sourceRevision, const std::string_view patternId,
    const std::string_view occurrenceId, WORLD_EMISSION_ANCHOR& out) const
{
    if (sourceRevision != m_iProductSourceRevision) return false;
    const auto pattern = m_Product.find(std::string(patternId));
    if (pattern == m_Product.end()) return false;
    const auto anchor = pattern->second.worldEmissionAnchors.find(std::string(occurrenceId));
    out = anchor == pattern->second.worldEmissionAnchors.end() ? WORLD_EMISSION_ANCHOR{} : anchor->second;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Is_TargetedCombatObjectArchetype(
    const std::string_view archetypeId) noexcept
{
    return archetypeId == "combatobject.kouku.showtime.fixed" ||
        archetypeId == "combatobject.kouku.showtime.tracking";
}

Client::CKoukuSaydonPresentationPlayer::TARGETED_COMBAT_VISUALS
Client::CKoukuSaydonPresentationPlayer::Read_TargetedCombatVisuals(const DATA_JSON_VALUE& root)
{
    TARGETED_COMBAT_VISUALS staged;
    const auto* definitions = root.Find("targetedCombatVisuals");
    if (!definitions) return staged;
    if (!definitions->Is_Array() || definitions->Get_Array().size() > 8192u)
        throw std::runtime_error("Targeted combat visuals require a bounded array.");
    const auto stableId = [](const std::string& id) {
        return !id.empty() && id.size() <= 128u && id != "." && id != ".." &&
            std::all_of(id.begin(), id.end(), [](const unsigned char c) {
                return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_'; });
    };
    for (const auto& row : definitions->Get_Array())
    {
        auto definition = std::make_shared<TARGETED_COMBAT_VISUAL>();
        const auto id = Text(row, "clientVisualId");
        definition->archetypeId = Text(row, "combatObjectArchetypeId");
        const auto& loop = Field(row, "loop");
        if (!stableId(id) || !Is_TargetedCombatObjectArchetype(definition->archetypeId) || !loop.Is_Boolean())
            throw std::runtime_error("Invalid targeted combat visual identity or loop.");
        definition->loop = loop.Get_Boolean();
        if (definition->loop != (definition->archetypeId == "combatobject.kouku.showtime.tracking"))
            throw std::runtime_error("Targeted visual loop must match its fixed/tracking archetype.");
        auto& presentation = definition->presentation;
        presentation.pattern.strPatternId = id;
        presentation.durationMs = UInt(row, "durationMs", 1u, MAX_TIMELINE_MS);
        auto& sourceBossPresentation = definition->sourceBossPresentation;
        sourceBossPresentation.pattern.strPatternId = id;
        sourceBossPresentation.durationMs = presentation.durationMs;
        const auto& resources = Field(row, "resources");
        const auto& occurrences = Field(row, "occurrences");
        if (!resources.Is_Array() || resources.Get_Array().empty() || resources.Get_Array().size() > 1024u ||
            !occurrences.Is_Array() || occurrences.Get_Array().empty() || occurrences.Get_Array().size() > 1024u)
            throw std::runtime_error("Targeted visual resources/occurrences require bounded nonempty arrays.");
        std::map<std::string, KIND> kinds;
        for (const auto& value : resources.Get_Array())
        {
            auto resource = Read_Resource(value);
            if (resource.eKind != KIND::EFFECT || !stableId(resource.strResourceId) ||
                !kinds.emplace(resource.strResourceId, resource.eKind).second)
                throw std::runtime_error("Targeted visual requires unique Effect resources.");
            presentation.document.PresentationResources.push_back(std::move(resource));
        }
        sourceBossPresentation.document = presentation.document;
        std::set<std::string> ids;
        for (const auto& value : occurrences.Get_Array())
        {
            const auto resource = kinds.find(Text(value, "resourceId"));
            if (resource == kinds.end()) throw std::runtime_error("Targeted occurrence has no resource.");
            auto box = Read_Occurrence(value, presentation.durationMs, resource->second);
            const bool sourceBoss = box.strAnchorKind == "BOSS" && box.bFollowBoss && !definition->loop;
            const bool map = box.strAnchorKind == "MAP" && !box.bFollowBoss;
            if (!stableId(box.strOccurrenceId) || !ids.insert(box.strOccurrenceId).second ||
                (!map && !sourceBoss) || !box.strBone.empty() || box.strBoneTarget != "BODY" ||
                !box.strWorldId.empty() || !box.strWorldOccurrenceId.empty() ||
                !box.strLogicOccurrenceId.empty() || box.iWorldEmissionIndex != 0u)
                throw std::runtime_error("Targeted occurrence requires a relative MAP or following source BOSS placement.");
            if (sourceBoss)
            {
                sourceBossPresentation.pattern.PresentationOccurrences.push_back(std::move(box));
                continue;
            }
            // Private execution copy: the existing root/follow sampler consumes the
            // authoritative CombatObject translation, without a boss model or yaw.
            // Published MAP offsets and the ordinary fixed-MAP path are unchanged.
            box.strAnchorKind = "BOSS";
            box.bFollowBoss = definition->loop;
            presentation.pattern.PresentationOccurrences.push_back(std::move(box));
        }
        if (presentation.pattern.PresentationOccurrences.empty())
            throw std::runtime_error("Targeted visual requires at least one MAP occurrence.");
        if (!staged.emplace(id, std::move(definition)).second)
            throw std::runtime_error("Duplicate targeted combat visual identity.");
    }
    return staged;
}

bool Client::CKoukuSaydonPresentationPlayer::Start_TargetedCombatVisual(
    const LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED& spawn, std::string& status)
{
    if (!Is_TargetedCombatObjectArchetype(spawn.strCombatObjectArchetypeId) ||
        !spawn.iCombatObjectId || !spawn.iSourceNetEntityId || !spawn.PinnedDefinitionRevision.Is_Valid() ||
        !std::isfinite(spawn.fPositionX) || !std::isfinite(spawn.fPositionY) || !std::isfinite(spawn.fPositionZ))
    { status = "Targeted combat visual spawn identity or position is invalid."; return false; }
    float age = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(spawn.iServerTick, spawn.iSpawnTick, 30.f, age))
    { status = "Targeted combat visual spawn clock is invalid."; return false; }
    if (m_TargetedCombatSessions.contains(spawn.iCombatObjectId))
    { status = "Targeted combat visual already has a live owner."; return false; }
    auto definition = m_TargetedCombatVisuals.find(spawn.strClientVisualId);
    if (definition == m_TargetedCombatVisuals.end())
    {
        try
        {
            // A content-addressed new visual can arrive before MainApp observes
            // the new Product run. Refresh this registry only; old sessions pin
            // their immutable definitions and keep their existing child clocks.
            auto staged = Read_TargetedCombatVisuals(Read_ProductPresentationRoot());
            m_TargetedCombatVisuals = std::move(staged);
        }
        catch (const std::exception& error) { status = error.what(); return false; }
        definition = m_TargetedCombatVisuals.find(spawn.strClientVisualId);
    }
    if (definition == m_TargetedCombatVisuals.end() ||
        definition->second->archetypeId != spawn.strCombatObjectArchetypeId)
    { status = "Targeted combat visual has no exact published ID/archetype join."; return false; }
    TARGETED_COMBAT_SESSION candidate;
    candidate.definition = definition->second;
    candidate.sourceId = spawn.iSourceNetEntityId;
    candidate.pinnedRevision = spawn.PinnedDefinitionRevision;
    candidate.spawnTick = spawn.iSpawnTick;
    candidate.serverTick = spawn.iServerTick;
    candidate.elapsedMs = double(age) * 1000.0;
    candidate.playback.key = "targeted:" + std::to_string(spawn.iCombatObjectId) + ":" +
        std::to_string(spawn.iSpawnTick);
    candidate.sourceBossPlayback.key = candidate.playback.key + ":source-boss";
    XMStoreFloat4x4(&candidate.root, XMMatrixTranslation(spawn.fPositionX, spawn.fPositionY, spawn.fPositionZ));
    const auto [entry, inserted] = m_TargetedCombatSessions.emplace(spawn.iCombatObjectId, std::move(candidate));
    if (!inserted) { status = "Targeted combat visual identity collided."; return false; }
    if (!Sample_TargetedCombatVisual(entry->second))
    {
        status = entry->second.failure;
        Stop_TargetedCombatVisual(spawn.iCombatObjectId);
        return false;
    }
    status = "Started targeted combat visual from the Server clock.";
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Update_TargetedCombatVisual(
    const LostArk::Shared::COMBAT_OBJECT_SNAPSHOT& snapshot, const std::uint32_t serverTick,
    std::string& status)
{
    const auto found = m_TargetedCombatSessions.find(snapshot.iCombatObjectId);
    if (found == m_TargetedCombatSessions.end())
    { status = "Targeted combat visual has no live session."; return false; }
    auto& session = found->second;
    float age = 0.f;
    if (session.sourceId != snapshot.iSourceNetEntityId || session.pinnedRevision != snapshot.PinnedDefinitionRevision ||
        !std::isfinite(snapshot.fPositionX) || !std::isfinite(snapshot.fPositionY) || !std::isfinite(snapshot.fPositionZ) ||
        !CActionPresentationTimeline::Try_ResolveActionAgeSeconds(serverTick, session.spawnTick, 30.f, age))
    { status = "Targeted combat visual snapshot identity, pose or clock changed."; return false; }
    if (!session.failure.empty()) { status = session.failure; return false; }
    session.serverTick = serverTick;
    session.elapsedMs = (std::max)(session.elapsedMs, double(age) * 1000.0);
    if (session.definition->loop)
        XMStoreFloat4x4(&session.root, XMMatrixTranslation(snapshot.fPositionX, snapshot.fPositionY, snapshot.fPositionZ));
    // MainApp samples all child groups once per presentation frame after the
    // complete snapshot batch. No Client tracking velocity is synthesized.
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_TargetedCombatVisual(
    TARGETED_COMBAT_SESSION& session, const KOUKU_BOSS_PRESENTATION_VIEW* sourceBoss)
{
    if (session.finished) return true;
    if (!session.failure.empty()) return false;
    const auto& presentation = session.definition->presentation;
    if (!session.definition->loop && session.elapsedMs >= presentation.durationMs)
    {
        Stop_Session(session.playback);
        Stop_Session(session.sourceBossPlayback);
        session.finished = true;
        return true;
    }
    const auto cycle = session.definition->loop ?
        static_cast<std::uint64_t>(session.elapsedMs / presentation.durationMs) : 0u;
    if (cycle != session.cycle)
    {
        Stop_Session(session.playback);
        Stop_Session(session.sourceBossPlayback);
        session.cycle = cycle;
    }
    const float clock = static_cast<float>(session.definition->loop ?
        std::fmod(session.elapsedMs, double(presentation.durationMs)) : session.elapsedMs);
    if (session.definition->loop && !session.playback.rootHistory)
    {
        // A late join has no earlier tracking snapshots. Bind pre-join births
        // to the first authoritative pose, then record only observed movement.
        // This seeds the existing sampler without inventing a tracking path.
        session.playback.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
        std::string historyStatus;
        if (!session.playback.rootHistory->Record(0.f, session.root, false, historyStatus))
        { session.failure = std::move(historyStatus); return false; }
        session.playback.rootRecordedSeconds = 0.f;
        session.playback.rootRecordedPivot = session.root;
    }
    Sample(session.playback, presentation.document, presentation.pattern, clock, false, session.root, nullptr);
    const auto& sourcePresentation = session.definition->sourceBossPresentation;
    bool sampledSourceBoss = false;
    if (!sourcePresentation.pattern.PresentationOccurrences.empty() && sourceBoss &&
        sourceBoss->Snapshot.iNetEntityId == session.sourceId)
    {
        if (const auto npc = sourceBoss->pNpc.lock(); npc && npc->Get_Transform())
        {
            const auto& pivot = *npc->Get_Transform()->Get_WorldMatrixPtr();
            if (!session.sourceBossPlayback.rootHistory)
            {
                // Spawns can precede the first actor view. As with tracking late
                // joins, bind earlier particle births to the first observed pose.
                session.sourceBossPlayback.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
                std::string historyStatus;
                if (!session.sourceBossPlayback.rootHistory->Record(0.f, pivot, false, historyStatus))
                {
                    session.failure = std::move(historyStatus);
                    Stop_Session(session.playback);
                    Stop_Session(session.sourceBossPlayback);
                    return false;
                }
                session.sourceBossPlayback.rootRecordedSeconds = 0.f;
                session.sourceBossPlayback.rootRecordedPivot = pivot;
            }
            // Both placements use the same Server birth clock. The muzzle follows
            // its actual source actor; only ground children use the random root.
            Sample(session.sourceBossPlayback, sourcePresentation.document, sourcePresentation.pattern,
                clock, false, pivot, npc->Get_Model());
            sampledSourceBoss = true;
        }
    }
    if (!sampledSourceBoss && (session.sourceBossPlayback.rootHistory || !session.sourceBossPlayback.rows.empty()))
        Stop_Session(session.sourceBossPlayback);
    for (const auto* playback : { &session.playback, &session.sourceBossPlayback })
        for (const auto& [id, row] : playback->rows)
            if (row.failed)
            {
                session.failure = "Targeted combat visual " + id + ": " +
                    (row.failureStatus.empty() ? m_strStatus : row.failureStatus);
                break;
            }
    if (session.failure.empty()) return true;
    Stop_Session(session.playback);
    Stop_Session(session.sourceBossPlayback);
    return false;
}

void Client::CKoukuSaydonPresentationPlayer::Update_TargetedCombatVisuals(
    const float dt, const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses)
{
    for (auto& [id, session] : m_TargetedCombatSessions)
    {
        (void)id;
        if (!session.finished && session.failure.empty()) session.elapsedMs += double(dt) * 1000.0;
        const auto source = std::find_if(bosses.begin(), bosses.end(), [&](const auto& boss) {
            return boss.Snapshot.iNetEntityId == session.sourceId;
        });
        if (!Sample_TargetedCombatVisual(session, source == bosses.end() ? nullptr : &*source))
            m_strStatus = session.failure;
    }
}

void Client::CKoukuSaydonPresentationPlayer::Stop_TargetedCombatVisual(
    const LostArk::Shared::COMBAT_OBJECT_ID objectId)
{
    const auto found = m_TargetedCombatSessions.find(objectId);
    if (found == m_TargetedCombatSessions.end()) return;
    Stop_Session(found->second.playback);
    Stop_Session(found->second.sourceBossPlayback);
    m_TargetedCombatSessions.erase(found);
}

bool Client::CKoukuSaydonPresentationPlayer::Reload_Product(
    std::string& status, const std::uint32_t expectedSourceRevision)
{
    // Restart keeps its admitted in-memory Product even after another publish.
    if (expectedSourceRevision && m_bProductLoaded && expectedSourceRevision == m_iProductSourceRevision)
    { status = "KoukuSaydon presentation already matches the admitted source revision."; return true; }
    m_bProductAttempted = true;
    try
    {
        const auto root = Read_ProductPresentationRoot();
        std::string parseStatus;
        const auto sourceRevision = UInt(root, "sourceRevision", 1u, UINT32_MAX);
        if (expectedSourceRevision && sourceRevision != expectedSourceRevision)
            throw std::runtime_error("KoukuSaydon presentation source revision mismatch: requested " +
                std::to_string(expectedSourceRevision) + ", published " + std::to_string(sourceRevision) +
                ". Previous presentation is preserved; publish the matching Product on this Client.");
        const auto& patterns = Field(root, "patterns");
        if (!patterns.Is_Array() || patterns.Get_Array().size() > 4096u)
            throw std::runtime_error("Product patterns must be a bounded array.");
        std::map<std::string, PRODUCT_PATTERN> staged;
        std::size_t isolatedLights = 0u;
        std::size_t isolatedSceneProfiles = 0u;
        std::string isolatedSceneStatus;
        for (const auto& value : patterns.Get_Array())
        {
            PRODUCT_PATTERN item;
            item.pattern.strPatternId = Text(value, "patternId");
            if (value.Find("gateId")) item.pattern.strGateId = Text(value, "gateId");
            if (value.Find("targetBossPlacementId")) item.pattern.strTargetBossPlacementId = Text(value, "targetBossPlacementId");
            if (value.Find("actorProfileId")) item.pattern.strActorProfileId = Text(value, "actorProfileId");
            item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
            if (const auto* windows = value.Find("animationBlendWindows"))
                if (!CKoukuSaydonAnimationBlend::Read_ProductWindows(*windows, item.pattern.AnimationBlendWindows, parseStatus))
                    throw std::runtime_error(parseStatus);
            if (const auto* animations = value.Find("sourceAnchorAnimations"))
            {
                if (!animations->Is_Array() || animations->Get_Array().empty() || animations->Get_Array().size() > 4096u)
                    throw std::runtime_error("Product source anchor animations require a bounded nonempty array.");
                KOUKU_SAYDON_COMPOSITION_STAGE stage;
                stage.iDurationMs = item.durationMs;
                for (const auto& source : animations->Get_Array())
                {
                    ANCHOR_ANIMATION animation;
                    animation.strRuntimeClip = Text(source, "runtimeClip");
                    animation.iStartOffsetMs = UInt(source, "startOffsetMs", 0u, item.durationMs - 1u);
                    animation.iPoseStartMs = source.Find("stageStartMs") ? UInt(source, "stageStartMs", 0u, animation.iStartOffsetMs) : animation.iStartOffsetMs;
                    animation.iSourceStartMs = UInt(source, "sourceStartMs", 0u, MAX_TIMELINE_MS);
                    if (source.Find("sourceEndMs")) animation.iSourceEndMs = UInt(source, "sourceEndMs", 0u, MAX_TIMELINE_MS);
                    animation.iPlayMs = UInt(source, "playMs", 1u, MAX_TIMELINE_MS);
                    animation.iBlendInMs = UInt(source, "blendInMs", 0u, 1000u);
                    animation.fPlayRate = float(Number(source, "playRate", 0.01, 100.0));
                    animation.strEndPolicy = Text(source, "endPolicy");
                    if (animation.strEndPolicy != "EXACT" && animation.strEndPolicy != "HOLD_LAST_POSE" &&
                        animation.strEndPolicy != "LOOP_TO_WINDOW")
                        throw std::runtime_error("Unsupported source anchor animation end policy.");
                    if (!stage.AnimationOccurrences.empty() &&
                        animation.iStartOffsetMs <= stage.AnimationOccurrences.back().iStartOffsetMs)
                        throw std::runtime_error("Source anchor animations must have distinct increasing pattern times.");
                    stage.AnimationOccurrences.push_back(std::move(animation));
                }
                item.pattern.Stages.push_back(std::move(stage));
            }
            if (value.Find("animationRootVerticalScale"))
                item.pattern.fAnimationRootVerticalScale = Number(value, "animationRootVerticalScale", 0.0, 1.0);
            if (const auto* source = value.Find("bossMotion"))
            {
                KOUKU_SAYDON_BOSS_MOTION motion;
                motion.iStartMs = UInt(*source, "startMs", 0u, item.durationMs - 1u);
                motion.iEndMs = UInt(*source, "endMs", motion.iStartMs + 1u, item.durationMs);
                motion.StartPosition = Vector(*source, "startPosition", -100000.0, 100000.0);
                motion.EndPosition = Vector(*source, "endPosition", -100000.0, 100000.0);
                motion.fYawDegrees = Number(*source, "yawDegrees", -360.0, 360.0);
                if (std::abs(motion.StartPosition[1] - motion.EndPosition[1]) > 0.0001)
                    throw std::runtime_error("Product bossMotion must keep its ground height.");
                item.pattern.BossMotion = motion;
            }
            if (const auto* anchors = value.Find("worldEmissionAnchors"))
            {
                if (!item.pattern.BossMotion || !anchors->Is_Array() || anchors->Get_Array().size() > 4096u)
                    throw std::runtime_error("Product WORLD emission anchors require a bounded bossMotion owner.");
                for (const auto& anchor : anchors->Get_Array())
                {
                    KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
                    world.strAnchorKind = "BOSS_SPAWN";
                    world.PositionOffset = Vector(anchor, "positionOffset", -100000.0, 100000.0);
                    world.AnchorPosition = Vector(anchor, "anchorPosition", -100000.0, 100000.0);
                    KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE occurrence;
                    occurrence.strOccurrenceId = Text(anchor, "occurrenceId");
                    occurrence.iStartMs = UInt(anchor, "startMs", 0u, item.durationMs - 1u);
                    if (!item.worldEmissionAnchors.emplace(occurrence.strOccurrenceId,
                        Make_WorldEmissionAnchor(item.pattern, world, occurrence)).second)
                        throw std::runtime_error("Duplicate Product WORLD emission anchor.");
                }
            }
            const auto& boxes = Field(value, "presentationOccurrences");
            if (!boxes.Is_Array() || boxes.Get_Array().size() > 4096u)
                throw std::runtime_error("Product presentationOccurrences must be a bounded array.");
            std::set<std::string> ids;
            for (const auto& box : boxes.Get_Array())
            {
                try
                {
                RESOURCE resource = Read_Resource(box);
                OCCURRENCE occurrence = Read_Occurrence(box, item.durationMs, resource.eKind);
                if (occurrence.strAnchorKind == "WORLD")
                {
                    // The Product pins the published sequence identity; it never
                    // reopens the editable source composition during gameplay.
                    const std::string sequenceId = Text(box, "worldSequenceInstanceId");
                    const auto world = std::find_if(item.document.Worlds.begin(), item.document.Worlds.end(),
                        [&occurrence](const auto& value) { return value.strWorldId == occurrence.strWorldId; });
                    if (world == item.document.Worlds.end())
                    {
                        KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION definition;
                        definition.strWorldId = occurrence.strWorldId;
                        definition.strSequenceInstanceId = sequenceId;
                        item.document.Worlds.push_back(std::move(definition));
                    }
                    else if (world->strSequenceInstanceId != sequenceId)
                        throw std::runtime_error("Conflicting WORLD presentation anchor identity.");
                }
                if (!ids.insert(occurrence.strOccurrenceId).second)
                    throw std::runtime_error("Duplicate Product presentation occurrence.");
                auto old = std::find_if(item.document.PresentationResources.begin(),
                    item.document.PresentationResources.end(), [&resource](const auto& row)
                    { return row.strResourceId == resource.strResourceId; });
                if (old == item.document.PresentationResources.end())
                    item.document.PresentationResources.push_back(std::move(resource));
                else
                {
                    // Scene resourceDurationMs is the individual projected box duration.
                    resource.iDurationMs = old->iDurationMs;
                    if (resource != *old) throw std::runtime_error("Conflicting Product resource identity.");
                }
                item.pattern.PresentationOccurrences.push_back(std::move(occurrence));
                }
                catch (const std::exception& error)
                {
                    const auto* kind = box.Find("kind");
                    if (!kind || !kind->Is_String()) throw;
                    if (kind->Get_String() == "LIGHT")
                    {
                        ++isolatedLights;
                        OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated LIGHT row: ") + error.what() + "\n").c_str());
                    }
                    else if (kind->Get_String() == "SCENE_PROFILE")
                    {
                        ++isolatedSceneProfiles;
                        const auto* id = box.Find("occurrenceId");
                        isolatedSceneStatus = item.pattern.strPatternId + "/" +
                            (id && id->Is_String() ? id->Get_String() : "<missing occurrenceId>") +
                            ": " + error.what();
                        OutputDebugStringA((std::string("[KoukuSaydonPresentationPlayer] Isolated SCENE_PROFILE row: ") +
                            isolatedSceneStatus + "\n").c_str());
                    }
                    else throw;
                }
            }
            const std::string key = item.pattern.strPatternId;
            if (!staged.emplace(key, std::move(item)).second)
                throw std::runtime_error("Duplicate Product pattern identity.");
        }

        std::map<std::string, PRODUCT_BUNDLE> stagedBundles;
        if (const auto* bundles = root.Find("bundles"))
        {
            if (!bundles->Is_Array() || bundles->Get_Array().size() > 4096u)
                throw std::runtime_error("Product bundles must be a bounded array.");
            for (const auto& value : bundles->Get_Array())
            {
                PRODUCT_BUNDLE item;
                auto& common = item.common;
                common.pattern.strPatternId = Text(value, "bundleId");
                common.pattern.strGateId = Text(value, "gateId");
                common.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
                const auto& members = Field(value, "members");
                const auto& boxes = Field(value, "presentationOccurrences");
                if (!members.Is_Array() || members.Get_Array().empty() ||
                    members.Get_Array().size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS ||
                    !boxes.Is_Array() || boxes.Get_Array().size() > 4096u)
                    throw std::runtime_error("Product bundle members/common rows are invalid.");
                std::set<std::string> memberIds, targets, boxIds;
                std::vector<PRESENTATION_WINDOW> globalWindows;
                for (const auto& box : boxes.Get_Array())
                {
                    auto resource = Read_Resource(box);
                    if (resource.eKind != KIND::CAMERA && resource.eKind != KIND::SCENE_PROFILE)
                        throw std::runtime_error("Product bundle common lane has a non-global resource.");
                    auto occurrence = Read_Occurrence(box, common.durationMs, resource.eKind);
                    if (!boxIds.insert(occurrence.strOccurrenceId).second)
                        throw std::runtime_error("Product bundle has duplicate common occurrences.");
                    (void)Admit_PresentationWindow(globalWindows, resource.eKind, occurrence.iStartMs,
                        double(occurrence.iStartMs) + occurrence.iDurationMs + Camera_ReturnMs(resource, false), "common");
                    const auto previous = std::find_if(common.document.PresentationResources.begin(),
                        common.document.PresentationResources.end(), [&](const auto& row)
                        { return row.strResourceId == resource.strResourceId; });
                    if (previous == common.document.PresentationResources.end())
                        common.document.PresentationResources.push_back(std::move(resource));
                    else
                    {
                        resource.iDurationMs = previous->iDurationMs;
                        if (resource != *previous) throw std::runtime_error("Conflicting bundle resource identity.");
                    }
                    common.pattern.PresentationOccurrences.push_back(std::move(occurrence));
                }
                for (const auto& member : members.Get_Array())
                {
                    const auto memberId = Text(member, "memberId");
                    const auto patternId = Text(member, "patternId");
                    const auto targetId = Text(member, "targetBossPlacementId");
                    const auto actorId = Text(member, "actorProfileId");
                    const auto offsetMs = UInt(member, "startOffsetMs", 0u, MAX_TIMELINE_MS);
                    const double offset = std::ceil(double(offsetMs) * 30.0 / 1000.0) * 1000.0 / 30.0;
                    const auto child = staged.find(patternId);
                    if (!memberIds.insert(memberId).second || !targets.insert(targetId).second ||
                        child == staged.end() || child->second.pattern.strGateId != common.pattern.strGateId ||
                        child->second.pattern.strTargetBossPlacementId != targetId || child->second.pattern.strActorProfileId != actorId)
                        throw std::runtime_error("Product bundle has duplicate or inconsistent child targets.");
                    for (const auto& row : child->second.pattern.PresentationOccurrences)
                        for (const auto& resource : child->second.document.PresentationResources)
                            if (resource.strResourceId == row.strResourceId &&
                                (resource.eKind == KIND::CAMERA || resource.eKind == KIND::SCENE_PROFILE) &&
                                !Admit_PresentationWindow(globalWindows, resource.eKind, offset + row.iStartMs,
                                    offset + row.iStartMs + row.iDurationMs + Camera_ReturnMs(resource, false), memberId))
                                throw std::runtime_error("Product bundle global presentation windows overlap.");
                    item.patternIds.push_back(patternId);
                }
                const auto key = common.pattern.strPatternId;
                if (!stagedBundles.emplace(key, std::move(item)).second)
                    throw std::runtime_error("Duplicate Product bundle identity.");
            }
        }

        std::map<std::string, PRODUCT_PATTERN> stagedFear;
        if (const auto* presentations = root.Find("fearPresentations"))
        {
            if (!presentations->Is_Array() || presentations->Get_Array().size() > 4096u)
                throw std::runtime_error("Fear presentations must be a bounded array.");
            for (const auto& value : presentations->Get_Array())
            {
                PRODUCT_PATTERN item;
                item.pattern.strPatternId = Text(value, "presentationId");
                item.durationMs = UInt(value, "durationMs", 1u, MAX_TIMELINE_MS);
                const auto append = [&](RESOURCE resource, std::uint32_t startMs, const std::string& anchor)
                {
                    OCCURRENCE box;
                    box.strOccurrenceId = item.pattern.strPatternId + ":" + resource.strResourceId;
                    box.strResourceId = resource.strResourceId;
                    box.iStartMs = startMs;
                    box.iDurationMs = item.durationMs - startMs;
                    box.strAnchorKind = anchor;
                    box.bDebugRender = false;
                    item.document.PresentationResources.push_back(std::move(resource));
                    item.pattern.PresentationOccurrences.push_back(std::move(box));
                };
                const auto sceneId = Text(value, "sceneProfileId", true);
                if (!sceneId.empty())
                {
                    RESOURCE scene;
                    scene.strResourceId = "fear.scene";
                    scene.eKind = KIND::SCENE_PROFILE;
                    scene.strResourceKind.clear();
                    scene.strAssetId = sceneId;
                    scene.iDurationMs = item.durationMs;
                    append(std::move(scene), 0u, "BOSS");
                }
                const auto delayMs = UInt(value, "effectDelayMs", 0u, item.durationMs - 1u);
                if (const auto* effect = value.Find("effectResource"); effect && !effect->Is_Null())
                {
                    auto resource = Read_Resource(*effect);
                    if (resource.eKind != KIND::EFFECT) throw std::runtime_error("Fear effectResource must be EFFECT.");
                    append(std::move(resource), delayMs, "BOSS");
                }
                else if (delayMs) throw std::runtime_error("Fear delay requires an Effect resource.");
                if (const auto* light = value.Find("lightResource"); light && !light->Is_Null())
                {
                    auto resource = Read_Resource(*light);
                    if (resource.eKind != KIND::LIGHT) throw std::runtime_error("Fear lightResource must be LIGHT.");
                    append(std::move(resource), 0u, "PLAYER");
                }
                const auto key = item.pattern.strPatternId;
                if (!stagedFear.emplace(key, std::move(item)).second)
                    throw std::runtime_error("Duplicate Fear presentation identity.");
            }
        }

        auto stagedTargeted = Read_TargetedCombatVisuals(root);

        // Only a fully staged replacement may stop the old running presentation.
        for (auto& [id, session] : m_BossSessions) Stop_Session(session);
        for (auto& [id, session] : m_ChildBossSessions) Stop_Session(session);
        m_BossSessions.clear();
        m_ChildBossSessions.clear();
        for (auto& [id, session] : m_MarioEntrySessions) Stop_Session(session);
        m_MarioEntrySessions.clear();
        m_Product = std::move(staged);
        m_TargetedCombatVisuals = std::move(stagedTargeted);
        Stop_Session(m_FearSession);
        m_FearSession.key.clear();
        m_strCompletedFearKey.clear();
        m_FearPresentations = std::move(stagedFear);
        Stop_Session(m_ProductBundleSession);
        m_ProductBundles = std::move(stagedBundles);
        m_iProductSourceRevision = sourceRevision;
        m_MissingProductPatterns.clear();
        m_bProductLoaded = true;
        Refresh_SharedPresentation();
        status = "Loaded KoukuSaydon presentation for " + std::to_string(m_Product.size()) + " Product patterns.";
        if (isolatedLights) status += " Isolated invalid LIGHT rows: " + std::to_string(isolatedLights);
        if (isolatedSceneProfiles) status += " Isolated invalid SCENE_PROFILE rows: " +
            std::to_string(isolatedSceneProfiles) + ". " + isolatedSceneStatus;
        m_strStatus = status;
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        m_strStatus = status;
        OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + status + "\n").c_str());
        return false;
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Ensure_EffectResource(
    const std::string& kind, const std::string& asset,
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>& snapshot)
{
    const auto generation = CEffectV2Runtime::Cache_Generation();
    if (generation != m_iEffectCacheGeneration)
    {
        m_EffectResources.clear();
        m_EffectResourceFailures.clear();
        m_iEffectCacheGeneration = generation;
    }
    const std::string key = kind + ":" + asset;
    if (const auto found = m_EffectResources.find(key); found != m_EffectResources.end())
    { snapshot = found->second; return true; }
    if (const auto failed = m_EffectResourceFailures.find(key); failed != m_EffectResourceFailures.end())
    { m_strStatus = failed->second; return false; }
    EFFECT_V2_RESOURCE_KIND resourceKind;
    if (kind == "GROUP") resourceKind = EFFECT_V2_RESOURCE_KIND::GROUP;
    else if (kind == "LEAF") resourceKind = EFFECT_V2_RESOURCE_KIND::LEAF;
    else { m_strStatus = "Unsupported Effect owner: " + kind; return false; }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> staged;
    if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(resourceKind, asset, staged, m_strStatus))
    {
        m_strStatus = "Effect resource " + asset + ": " + m_strStatus;
        m_EffectResourceFailures.emplace(key, m_strStatus);
        return false;
    }
    m_EffectResources.emplace(key, staged);
    snapshot = std::move(staged);
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Session(SESSION& session)
{
    for (auto& [id, row] : session.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Stop_Group(row.effectHandle);
        if (row.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
        if (row.soundHandle) CGameInstance::Get().Stop_SoundCue(row.soundHandle);
    }
    session.rows.clear();
    session.rootHistory.reset();
    session.effectAnchorHistories.clear();
    session.rootRecordedSeconds = -1.f;
    session.lastClockMs = -1.f;
}

void Client::CKoukuSaydonPresentationPlayer::Sample(SESSION& session,
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, float clockMs, bool paused,
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model,
    const ANIMATION_MODEL_TARGET_VIEW* weaponView)
{
    if (!std::isfinite(clockMs) || clockMs < 0.f) return;
    const bool previewSession = &session == &m_PreviewSession || std::any_of(
        m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& member) { return &session == &member.session; });
    CEffectV2Object::PIVOT_SAMPLER exactRoot;
    const auto previewMember = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& member) { return &session == &member.session && member.spatialLogicPreview; });
    if (previewMember != m_BundlePreviewMembers.end())
        exactRoot = [this, memberId = previewMember->memberId, generation = m_iPreviewGeneration]
            (float seconds, float4x4_t& output, std::string& error) {
            const auto member = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
                [&](const auto& candidate) { return candidate.memberId == memberId; });
            float3_t position;
            float yaw = 0.f;
            if (generation != m_iPreviewGeneration || member == m_BundlePreviewMembers.end() ||
                !Sample_BundlePreviewPose(*member, seconds * 1000.f, position, yaw, false))
            { error = "Spatial preview root has no recorded target input at this source clock."; return false; }
            const matrix_t root = XMLoadFloat4x4(member->actor->Get_Transform()->Get_WorldMatrixPtr());
            XMStoreFloat4x4(&output, XMMatrixScaling(XMVectorGetX(XMVector3Length(root.r[0])),
                XMVectorGetX(XMVector3Length(root.r[1])), XMVectorGetX(XMVector3Length(root.r[2]))) *
                XMMatrixRotationY(XMConvertToRadians(yaw)) * XMMatrixTranslation(position.x, position.y, position.z));
            error.clear(); return true;
        };
    // Preview's external clocks already handle forward/rewind samples. A slow
    // frame must not destroy a live occurrence and its frozen screen capture,
    // or the capture resolver repeatedly returns to the same boundary.
    if (!previewSession && session.lastClockMs >= 0.f &&
        (clockMs < session.lastClockMs - 0.5f || clockMs > session.lastClockMs + 150.f))
    {
        // Same occurrence seek retains observed actor history; a new Server run
        // calls Stop_Session separately and never borrows the previous run.
        auto history = session.rootHistory;
        auto anchorHistories = std::move(session.effectAnchorHistories);
        const auto recordedSeconds = session.rootRecordedSeconds;
        const auto recordedPivot = session.rootRecordedPivot;
        Stop_Session(session);
        session.rootHistory = std::move(history);
        session.effectAnchorHistories = std::move(anchorHistories);
        session.rootRecordedSeconds = recordedSeconds;
        session.rootRecordedPivot = recordedPivot;
    }
    if (!session.rootHistory) session.rootHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
    const float rootSeconds = clockMs / 1000.f;
    if (rootSeconds >= session.rootRecordedSeconds)
    {
        const float dx = pivot._41 - session.rootRecordedPivot._41;
        const float dy = pivot._42 - session.rootRecordedPivot._42;
        const float dz = pivot._43 - session.rootRecordedPivot._43;
        const bool jump = session.rootRecordedSeconds >= 0.f && dx*dx + dy*dy + dz*dz > 2500.f;
        std::string historyStatus;
        if (session.rootHistory->Record(rootSeconds, pivot, jump, historyStatus))
        { session.rootRecordedSeconds = rootSeconds; session.rootRecordedPivot = pivot; }
        else m_strStatus = std::move(historyStatus);
    }
    const auto resolveWorldPivot = [&](const OCCURRENCE& box, float4x4_t& anchor) {
        const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
            [&box](const auto& value) { return value.strWorldId == box.strWorldId; });
        if (world == document.Worlds.end()) return false;
        if (!session.previewWorlds.empty())
        {
            const CWorldSequencePlayer* selected = nullptr;
            for (const auto& [id, player] : session.previewWorlds)
                if ((box.strWorldOccurrenceId.empty() || id == box.strWorldOccurrenceId) &&
                    player->Is_Playing(world->strSequenceInstanceId))
                { if (selected) return false; selected = player.get(); }
            return selected && selected->Try_GetSequencePivot(world->strSequenceInstanceId, anchor, box.iWorldEmissionIndex);
        }
        const auto* level = CLevel_KakulSaydonArena::Get_Active();
        if (!level) return false;
        if (session.runEpoch)
            return level->Try_GetOwnedCompositionWorldPivot(session.runEpoch, session.memberId,
                world->strSequenceInstanceId, box.strWorldOccurrenceId, anchor, box.iWorldEmissionIndex);
        return level->Try_GetCompositionWorldPivot(world->strSequenceInstanceId, anchor, box.strWorldOccurrenceId, box.iWorldEmissionIndex);
    };
    // Observe future anchored cues as well as active ones. Their first emission
    // can then interpolate the real samples bracketing the box start even when
    // a render frame crosses that start by more than one fixed step.
    for (const auto& box : pattern.PresentationOccurrences)
    {
        if (!box.bFollowBoss || (box.strAnchorKind != "WORLD" && box.strBone.empty()) ||
            clockMs > double(box.iStartMs) + box.iDurationMs) continue;
        const auto resource = std::find_if(document.PresentationResources.begin(),
            document.PresentationResources.end(), [&box](const auto& row)
            { return row.strResourceId == box.strResourceId && row.eKind == KIND::EFFECT; });
        if (resource == document.PresentationResources.end()) continue;
        auto& history = session.effectAnchorHistories[box.strOccurrenceId];
        if (!history.samples) history.samples = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
        // Rewinding retains observed poses; it never appends fabricated history.
        if (rootSeconds < history.recordedSeconds) continue;
        auto sampledBox = box;
        float4x4_t anchor = pivot;
        auto anchorModel = model;
        float3_t anchorScale{1.f, 1.f, 1.f};
        if (box.strAnchorKind == "WORLD")
        {
            if (!resolveWorldPivot(box, anchor)) { history.missingSinceSample = true; continue; }
            const matrix_t matrix = XMLoadFloat4x4(&anchor);
            anchorScale = {XMVectorGetX(XMVector3Length(matrix.r[0])),
                XMVectorGetX(XMVector3Length(matrix.r[1])), XMVectorGetX(XMVector3Length(matrix.r[2]))};
            sampledBox.strBone.clear();
            anchorModel.reset();
        }
        float4x4_t unusedPivot, basis;
        if (!Make_Pivot(sampledBox, anchor, anchorModel, unusedPivot, weaponView, &basis))
        { history.missingSinceSample = true; continue; }
        float4x4_t recorded;
        XMStoreFloat4x4(&recorded, XMMatrixScaling(anchorScale.x, anchorScale.y, anchorScale.z) *
            XMLoadFloat4x4(&basis));
        const float dx = recorded._41 - history.recordedPivot._41;
        const float dy = recorded._42 - history.recordedPivot._42;
        const float dz = recorded._43 - history.recordedPivot._43;
        const bool discontinuity = history.recordedSeconds >= 0.f &&
            (history.missingSinceSample || dx*dx + dy*dy + dz*dz > 2500.f);
        std::string historyStatus;
        if (history.samples->Record(rootSeconds, recorded, discontinuity, historyStatus))
        {
            history.recordedSeconds = rootSeconds;
            history.recordedPivot = recorded;
            history.missingSinceSample = false;
        }
        else m_strStatus = "Effect anchor history " + box.strOccurrenceId + ": " + historyStatus;
    }
    std::set<std::string> active;
    const auto sampleOne = [&](const RESOURCE& resource, const OCCURRENCE& box)
    {
        if (clockMs < box.iStartMs || clockMs >= double(box.iStartMs) + box.iDurationMs) return;
        active.insert(box.strOccurrenceId);
        auto [found, inserted] = session.rows.try_emplace(box.strOccurrenceId);
        PLAYING_ROW& row = found->second;
        if (row.failed) return;
        struct ROW_FAILURE_GUARD
        {
            PLAYING_ROW& row;
            const std::string& status;
            ~ROW_FAILURE_GUARD() { if (row.failed && row.failureStatus.empty()) row.failureStatus = status; }
        } failureGuard{row, m_strStatus};
        const float age = (clockMs - box.iStartMs) / 1000.f;
        const float effectRate = Effect_SourceClockRate(resource, box);
        const float effectAge = age * effectRate;
        row.kind = resource.eKind;
        row.debugRender = box.bDebugRender;
        row.startMs = float(box.iStartMs);
        row.cameraDurationMs = box.iDurationMs;
        row.assetId = resource.strAssetId;
        row.cameraOffset = { float(box.PositionOffset[0]), float(box.PositionOffset[1]), float(box.PositionOffset[2]) };
        if (resource.eKind == KIND::LIGHT)
        {
            row.lightBox = box;
            row.lightWeight = float(box.fBrightnessMultiplier);
            if (box.iFadeInMs) row.lightWeight *= (std::min)(1.f, (clockMs - box.iStartMs) / box.iFadeInMs);
            if (box.iFadeOutMs) row.lightWeight *= (std::min)(1.f, (box.iStartMs + box.iDurationMs - clockMs) / box.iFadeOutMs);
        }
        if ((resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT || resource.eKind == KIND::COLLIDER) &&
            (inserted || box.bFollowBoss || row.waitingForAnchor) &&
            !(resource.eKind == KIND::LIGHT && box.strAnchorKind == "PLAYER"))
        {
            OCCURRENCE placedBox = box;
            float4x4_t anchor = pivot;
            if (exactRoot && !box.bFollowBoss && box.strAnchorKind == "BOSS" && box.strBone.empty() &&
                !exactRoot(box.iStartMs / 1000.f, anchor, m_strStatus))
            { row.failed = true; return; }
            auto anchorModel = model;
            if ((resource.eKind == KIND::LIGHT || resource.eKind == KIND::EFFECT || resource.eKind == KIND::COLLIDER) && box.strAnchorKind == "MAP")
            { XMStoreFloat4x4(&anchor, XMMatrixIdentity()); anchorModel.reset(); }
            if (box.strAnchorKind == "WORLD")
            {
                if (!resolveWorldPivot(box, anchor))
                {
                    // A WORLD may not have its first sampled pose yet. Retry
                    // presentation anchors next frame instead of hiding this box forever.
                    row.waitingForAnchor = resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT;
                    row.failed = !row.waitingForAnchor;
                    m_strStatus = "WORLD presentation anchor unavailable: " + box.strWorldId;
                    return;
                }
                const matrix_t worldMatrix = XMLoadFloat4x4(&anchor);
                // Lights use metre offsets and retain their source shape, independent of model scale.
                for (size_t axis = 0; resource.eKind != KIND::LIGHT && axis < 3u; ++axis)
                {
                    const float scale = XMVectorGetX(XMVector3Length(worldMatrix.r[axis]));
                    placedBox.Scale[axis] *= scale;
                    placedBox.PositionOffset[axis] *= scale;
                }
                Flatten_CenteredWorldCircle(resource, box, anchor);
                placedBox.strBone.clear();
                anchorModel.reset();
            }
            if (!Make_Pivot(placedBox, anchor, anchorModel, row.pivot, weaponView,
                (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT || resource.eKind == KIND::LIGHT) ? &row.placementAnchor : nullptr))
            {
                row.waitingForAnchor = resource.eKind == KIND::LIGHT;
                row.failed = !row.waitingForAnchor;
                m_strStatus = "Presentation bone/pivot is unavailable: " + box.strOccurrenceId;
                return;
            }
            if (row.waitingForAnchor)
                m_strStatus = "Presentation WORLD anchor ready: " + box.strWorldId;
            row.waitingForAnchor = false;
            if (resource.eKind == KIND::COLLIDER)
                row.wire = Collider_Wire(resource, placedBox);
            if (resource.eKind == KIND::COLLIDER || resource.eKind == KIND::EFFECT)
            {
                for (size_t axis = 0u; axis < 3u; ++axis)
                    row.placementAnchorScale[axis] = placedBox.Scale[axis] / box.Scale[axis];
                row.hasPlacementAnchor = true;
            }
        }
        if (resource.eKind == KIND::EFFECT && box.bFollowBoss &&
            (box.strAnchorKind == "WORLD" || !box.strBone.empty()))
        {
            const auto history = session.effectAnchorHistories.find(box.strOccurrenceId);
            if (history != session.effectAnchorHistories.end()) row.effectPivotHistory = history->second.samples;
        }
        if (inserted || (resource.eKind == KIND::EFFECT && !row.effectHandle && !row.v1EffectHandle))
        {
            switch (resource.eKind)
            {
            case KIND::EFFECT:
            {
                if (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT")
                {
                    const auto catalogRevision = CEffectCatalog::Get_RuntimeRevision();
                    if (catalogRevision != m_iV1CatalogRevision)
                    { m_QueuedV1Effects.clear(); m_iV1CatalogRevision = catalogRevision; }
                    std::vector<std::string> targets{resource.strAssetId};
                    if (m_QueuedV1Effects.insert(resource.strAssetId).second)
                    {
                        std::vector<std::string> admitted;
                        if (!CEffectPresentationService::Queue_ProductTargets_Priority(targets, admitted, m_strStatus))
                        { m_QueuedV1Effects.erase(resource.strAssetId); row.failed = true; break; }
                    }
                    const auto preparation = CEffectPresentationService::Get_ProductCuePreparationProbe(targets);
                    if (preparation.iFailedCount || preparation.iUnavailableCount)
                    {
                        row.failed = true;
                        m_strStatus = "V1 Effect preparation failed: " + resource.strAssetId + "; " +
                            CEffectPresentationService::Get_ProductCuePreparationFailure(resource.strAssetId);
                        break;
                    }
                    if (!preparation.bCatalogRevisionCurrent || !preparation.bSettled) break;
                    row.sourceAnchorSampler = Make_SourceAnchorSampler(resource, pattern, model, pivot, box.iStartMs, exactRoot);
                    EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                    spawn.iLevelIndex = CGameInstance::Get().Get_CurrentLevelID();
                    spawn.strPlacementId = "kouku:" + session.key + ":" + box.strOccurrenceId;
                    spawn.strEffectAssetId = resource.strAssetId;
                    spawn.strElementId = resource.strElementId;
                    spawn.RootWorld = row.pivot;
                    spawn.fInitialSampleTimeSeconds = effectAge;
                    spawn.bExternallySampled = true;
                    spawn.fSourceLoopEndSeconds = box.bLoopEffectToDuration ? box.iDurationMs / 1000.f : 0.f;
                    EFFECT_WORLD_ROOT_HANDLE handle;
                    if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_strStatus))
                    {
                        m_strStatus = "V1 Effect spawn rejected: " + resource.strAssetId + " | " + box.strOccurrenceId + "; " + m_strStatus;
                        row.failed = true;
                    }
                    else row.v1EffectHandle = handle.iValue;
                    break;
                }
                std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
                if (!Ensure_EffectResource(resource.strResourceKind, resource.strAssetId, effects))
                { row.failed = true; break; }
                EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                playback.PivotWorld = row.pivot;
                playback.fInitialAgeSeconds = age;
                playback.fDurationSeconds = box.iDurationMs / 1000.f;
                // Workbench Fade 0 retains the leaf envelope via the runtime's -1 sentinel.
                playback.fFadeInSeconds = box.iFadeInMs > 0u ? box.iFadeInMs / 1000.f : -1.f;
                playback.fFadeOutSeconds = box.iFadeOutMs > 0u ? box.iFadeOutMs / 1000.f : -1.f;
                playback.fDissolveOutStart = box.iFadeOutMs > 0u ? float(box.fDissolveStart) : -1.f;
                playback.fDissolveOutEnd = box.iFadeOutMs > 0u ? float(box.fDissolveEnd) : -1.f;
                // Server/preview occurrence age owns this lane. Layer Update and
                // MainApp's general Effect clock cannot advance it a second time.
                playback.bProductOwned = true;
                playback.bExternalClock = true;
                playback.PivotSampler = Effect_PivotSampler(box, session.rootHistory, row.effectPivotHistory, exactRoot);
                if (resource.strResourceKind == "GROUP")
                {
                    const auto* group = effects->Find_Group(resource.strAssetId);
                    if (group)
                    {
                        // A particle composition owns emission intervals and tail;
                        // stretching it to the box would manufacture extra births.
                        if (std::any_of(effects->Get_Documents().begin(), effects->Get_Documents().end(),
                            [](const auto& leaf) { return leaf.eType == EFFECT_V2_TYPE::PARTICLE; }))
                            playback.fDurationSeconds = -1.f;
                        row.effectHandle = CEffectV2Runtime::Play_Group(*group,
                            effects, playback, m_Device, m_Context);
                    }
                }
                else if (resource.strResourceKind == "LEAF")
                {
                    const auto* leaf = effects->Find_Document(resource.strAssetId);
                    // Particle boxes include the living particles after emission ends.
                    // Keep the source emitter lifetime/loop; the box still owns final cleanup.
                    if (leaf && leaf->Desc.eShape == CEffectV2Object::SHAPE::PARTICLE)
                        playback.fDurationSeconds = -1.f;
                    row.effectHandle = CEffectV2Runtime::Play_Leaf(resource.strAssetId,
                        effects, playback, m_Device, m_Context);
                }
                if (!row.effectHandle)
                {
                    row.failed = true;
                    m_strStatus = "Effect occurrence unavailable: " + resource.strAssetId + "; " + CEffectV2Runtime::Last_Error();
                }
                break;
            }
            case KIND::SOUND:
            {
                const auto path = CRuntimeAssetRoot::Resolve(resource.strAssetId);
                if (!path.empty()) row.soundHandle = CGameInstance::Get().Play_SoundCue(
                    path.wstring(), float(box.fVolume), static_cast<std::uint32_t>(age * 1000.f));
                if (!row.soundHandle)
                {
                    row.failed = true;
                    m_strStatus = "Sound occurrence unavailable: " + resource.strAssetId;
                }
                break;
            }
            case KIND::LIGHT: break;
            case KIND::COLLIDER: break;
            case KIND::CAMERA: break;
            case KIND::SCENE_PROFILE:
                if (!m_Profiles.Has_Profile(resource.strAssetId))
                {
                    row.failed = true;
                    m_strStatus = "Scene profile unavailable: " + resource.strAssetId;
                }
                break;
            default:
                row.failed = true;
                m_strStatus = "Unsupported occurrence resource: " + resource.strResourceId;
                break;
            }
        }
        if (row.v1EffectHandle)
        {
            const bool captureSample = previewSession && m_bPreviewCaptureClockHeld;
            CEffectPresentationService::Set_ScreenPostCaptureAllowed({row.v1EffectHandle},
                !captureSample || m_bPreviewCaptureAllowed);
            // A capture boundary continues the same fixed-step history. Commit
            // it before render without replaying every active particle from birth.
            if (!CEffectPresentationService::Update_WorldRoot({row.v1EffectHandle}, row.pivot) ||
                !CEffectPresentationService::Seek_WorldRoot({row.v1EffectHandle}, effectAge,
                    Effect_V1TransformProvider(box, row.pivot, session.rootHistory, row.effectPivotHistory, row.sourceAnchorSampler, effectRate, exactRoot),
                    false, static_cast<float>(box.iDurationMs) * effectRate / 1000.f) || (captureSample && FAILED(
                        CEffectPresentationService::Commit_WorldRootCaptureSample({row.v1EffectHandle}))))
            {
                CEffectPresentationService::Stop_WorldRoot({row.v1EffectHandle});
                row.v1EffectHandle = 0u;
                row.failed = true;
                m_strStatus = "V1 Effect occurrence lost its admitted handle after spawn/attach or rendering failure: " +
                    box.strOccurrenceId + ". " + CEffectPresentationService::Get_Status() +
                    " Correct the resource and restart or seek Preview to retry.";
                return;
            }
        }
        if (row.effectHandle)
        {
            CEffectV2Runtime::Set_GroupPivot(row.effectHandle, row.pivot);
            (void)CEffectV2Runtime::Sample_Group(row.effectHandle, age, paused, m_Device, m_Context);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(row.effectHandle, failure))
            {
                CEffectV2Runtime::Stop_Group(row.effectHandle);
                row.effectHandle = 0;
                row.failed = true;
                m_strStatus = std::move(failure);
            }
        }
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
        row.lastAge = age;
    };
    for (const auto& box : pattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(),
            document.PresentationResources.end(), [&box](const auto& row)
            { return row.strResourceId == box.strResourceId; });
        if (resource != document.PresentationResources.end()) sampleOne(*resource, box);
    }
    for (const auto& sceneBox : pattern.SceneProfileOccurrences)
    {
        const auto profile = std::find_if(document.SceneProfiles.begin(), document.SceneProfiles.end(),
            [&sceneBox](const auto& row) { return row.strSceneProfileId == sceneBox.strSceneProfileId; });
        if (profile == document.SceneProfiles.end()) continue;
        RESOURCE resource;
        resource.eKind = KIND::SCENE_PROFILE;
        resource.strResourceId = profile->strSceneProfileId;
        resource.strAssetId = profile->strRenderingProfileId;
        OCCURRENCE box;
        box.strOccurrenceId = sceneBox.strOccurrenceId;
        box.strResourceId = sceneBox.strSceneProfileId;
        box.iStartMs = sceneBox.iStartMs;
        box.iDurationMs = sceneBox.iDurationMs;
        box.iFadeInMs = sceneBox.iBlendMs;
        sampleOne(resource, box);
    }
    for (auto row = session.rows.begin(); row != session.rows.end();)
    {
        if (active.contains(row->first)) { ++row; continue; }
        if (row->second.effectHandle) CEffectV2Runtime::Stop_Group(row->second.effectHandle);
        if (row->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row->second.v1EffectHandle});
        if (row->second.soundHandle) CGameInstance::Get().Stop_SoundCue(row->second.soundHandle);
        row = session.rows.erase(row);
    }
    session.lastClockMs = clockMs;
    Sample_LogicPreview(session, document, pattern, clockMs, paused);
}

void Client::CKoukuSaydonPresentationPlayer::Restore_Scene()
{
    if (m_bSceneUsed && !m_strScenePrevious.empty() &&
        m_Profiles.Get_ActiveProfileId() == m_strSceneApplied)
    {
        std::string status;
        if (!m_Profiles.Activate_Profile(m_strScenePrevious, status))
        {
            // Keep the owned previous profile until restoration succeeds or an
            // external profile supersedes it; a transient failure is not a release.
            m_strStatus = status;
            return;
        }
    }
    m_bSceneUsed = false;
    m_strScenePrevious.clear();
    m_strSceneOwner.clear();
    m_strSceneApplied.clear();
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_SharedPresentation()
{
    Collect_FrameLights();
    const PLAYING_ROW* scene = nullptr;
    const PLAYING_ROW* camera = nullptr;
    std::string sceneOwner;
    std::string cameraOwner;
    bool cameraPreview = false;
    std::set<KIND> conflictingGlobals;
    std::map<KIND, std::string> globalOwners;
    const auto inspectGlobals = [&](const SESSION& session)
    {
        if (!m_ProductBundleSession.runEpoch || session.runEpoch != m_ProductBundleSession.runEpoch) return;
        for (const auto& [id, row] : session.rows)
            if (!row.failed && (row.kind == KIND::CAMERA || row.kind == KIND::SCENE_PROFILE))
            {
                const auto [owner, inserted] = globalOwners.emplace(row.kind, session.key);
                if (!inserted && owner->second != session.key) conflictingGlobals.insert(row.kind);
            }
    };
    inspectGlobals(m_ProductBundleSession);
    for (const auto& [id, session] : m_BossSessions) inspectGlobals(session);
    for (const auto& [id, session] : m_ChildBossSessions) inspectGlobals(session);
    if (!conflictingGlobals.empty())
        m_strStatus = "Bundle global presentation owner conflict; overlapping Camera/Scene rows isolated.";
    const auto choose = [&](const SESSION& session, bool product)
    {
        const PLAYING_ROW* localScene = nullptr;
        const PLAYING_ROW* localCamera = nullptr;
        std::string localOwner;
        std::string localCameraOwner;
        for (const auto& [id, row] : session.rows)
        {
            if (row.failed || (product && conflictingGlobals.contains(row.kind))) continue;
            if (row.kind == KIND::SCENE_PROFILE && (!localScene || row.startMs >= localScene->startMs))
            {
                localScene = &row;
                localOwner = session.key + ":" + id;
            }
            if (row.kind == KIND::CAMERA && (!localCamera || row.startMs >= localCamera->startMs))
            {
                localCamera = &row;
                localCameraOwner = session.key + ":" + id + ":" +
                    std::to_string(product ? session.runEpoch : m_iPreviewGeneration);
            }
        }
        if (localScene) { scene = localScene; sceneOwner = std::move(localOwner); }
        if (localCamera) { camera = localCamera; cameraOwner = std::move(localCameraOwner); cameraPreview = !product; }
    };
    // Stable entity/id order resolves overlapping presentation; preview owns the final choice.
    for (const auto& [id, session] : m_BossSessions) choose(session, true);
    for (const auto& [id, session] : m_ChildBossSessions) choose(session, true);
    choose(m_ProductBundleSession, true);
    for (const auto& member : m_BundlePreviewMembers) choose(member.session, false);
    if (m_bPreviewPlaying) choose(m_PreviewSession, false);
    // A local player's temporary fear owns the scene until its replicated end.
    choose(m_FearSession, false);
    if (!scene) Restore_Scene();
    else if (m_strSceneOwner != sceneOwner || m_strSceneApplied != scene->assetId)
    {
        const std::string previous = m_Profiles.Get_ActiveProfileId();
        std::string status;
        if (m_Profiles.Activate_Profile(scene->assetId, status))
        {
            if (!m_bSceneUsed) m_strScenePrevious = previous;
            m_bSceneUsed = true;
            m_strSceneOwner = std::move(sceneOwner);
            m_strSceneApplied = scene->assetId;
        }
        else m_strStatus = status;
    }
    if (auto* level = CLevel_KakulSaydonArena::Get_Active())
    {
        const bool cameraEnabled = level->Is_CompositionCameraEnabled();
        if (camera && cameraEnabled)
        {
            if (level->Sample_CompositionCamera(camera->assetId, camera->lastAge, camera->cameraOffset,
                cameraOwner, camera->cameraDurationMs, cameraPreview))
                m_bCameraUsed = true;
            else
            {
                m_strStatus = "Camera shot unavailable or interrupted: " + camera->assetId;
                if (m_bCameraUsed) level->Stop_CompositionCamera();
                m_bCameraUsed = false;
            }
        }
        else if (m_bCameraUsed)
        {
            level->Stop_CompositionCamera(!cameraEnabled);
            m_bCameraUsed = false;
        }
    }
    else m_bCameraUsed = false;
}

void Client::CKoukuSaydonPresentationPlayer::Update_FearPresentation(float dt,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    const auto stopFear = [this]() { Stop_Session(m_FearSession); m_FearSession.key.clear(); };
    const auto localId = CNetworkManager::Get().Get_LocalEntityId();
    const auto player = std::find_if(players.begin(), players.end(), [&](const auto& view)
        { return view.Snapshot.iNetEntityId == localId; });
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    if (!arena || !localId || player == players.end() || !player->Snapshot.iCurrentHp ||
        player->Snapshot.eAction != LostArk::Shared::PLAYER_ACTION_STATE::FEAR)
    { stopFear(); m_strCompletedFearKey.clear(); return; }
    const auto& snapshot = player->Snapshot;
    const auto key = "fear:" + snapshot.strFearPresentationId + ":" + std::to_string(snapshot.iActionStartTick);
    // A locally completed clock must not restart from the last, slightly older
    // Server snapshot while the expiry snapshot is still in flight.
    if (m_strCompletedFearKey == key) { stopFear(); return; }
    const auto rejectFear = [&](const std::string& reason)
    { stopFear(); m_strCompletedFearKey = key; m_strStatus = reason; };
    const auto presentation = m_FearPresentations.find(snapshot.strFearPresentationId);
    if (presentation == m_FearPresentations.end())
    { rejectFear("Fear presentation unavailable: " + snapshot.strFearPresentationId); return; }
    float seconds = 0.f;
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(arena->Get_PresentationServerTick(),
        snapshot.iActionStartTick, 30.f, seconds))
    { stopFear(); return; }
    if (m_FearSession.key != key) { stopFear(); m_FearSession.key = key; }
    const float clockMs = m_FearSession.lastClockMs < 0.f ? seconds * 1000.f :
        (std::max)(seconds * 1000.f, m_FearSession.lastClockMs + dt * 1000.f);
    const auto durationTicks = snapshot.iFearEndTick - snapshot.iActionStartTick;
    const float durationMs = (std::min)(float(presentation->second.durationMs), durationTicks * (1000.f / 30.f));
    if (clockMs >= durationMs) { stopFear(); m_strCompletedFearKey = key; return; }
    // Do not darken the scene until its declared character lights are usable.
    // Gameplay FEAR remains Server-owned even if this presentation is isolated.
    for (const auto& resource : presentation->second.document.PresentationResources)
    {
        if (resource.eKind == KIND::SCENE_PROFILE && !m_Profiles.Has_Profile(resource.strAssetId))
        { rejectFear("Fear scene profile unavailable: " + resource.strAssetId); return; }
        if (resource.eKind != KIND::LIGHT) continue;
        const auto* light = m_pLightResources ? m_pLightResources->Find_RuntimeResource(resource.strAssetId) : nullptr;
        if (!light) { rejectFear("Fear character light unavailable: " + resource.strAssetId); return; }
        if (m_LightPlayerPivots.empty()) { stopFear(); return; }
        for (const auto& pivot : m_LightPlayerPivots)
        {
            LIGHT_DESC desc{}; std::string reason;
            if (!CLightResourceCatalog::Try_BuildLightDesc(*light, pivot, 1.f, desc, reason))
            { rejectFear("Fear character light rejected: " + resource.strAssetId + "; " + reason); return; }
        }
    }
    float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
    Sample(m_FearSession, presentation->second.document, presentation->second.pattern,
        clockMs, false, pivot, nullptr);
}

void Client::CKoukuSaydonPresentationPlayer::Update(float dt,
    const std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    m_LogicPreviewPlayers = players;
    if (!std::isfinite(dt) || dt < 0.f) return;
    Sync_PreviewSourceVisibility();
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    for (const auto& view : bosses)
    {
        if (!view.iOwnerBossNetEntityId || !view.Snapshot.iCurrentHp || view.pNpc.expired() || view.strArchetypeId.empty()) continue;
        const auto owner = std::find_if(bosses.begin(), bosses.end(), [&](const auto& candidate)
        {
            return candidate.Snapshot.iNetEntityId == view.iOwnerBossNetEntityId &&
                !candidate.iOwnerBossNetEntityId && candidate.Snapshot.iCurrentHp && !candidate.pNpc.expired() &&
                candidate.strArchetypeId == view.strArchetypeId;
        });
        if (owner != bosses.end()) m_LightBossFollowers[view.iOwnerBossNetEntityId].push_back(
            {view.Snapshot.iNetEntityId, view.pNpc});
    }
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        if (character && character->Get_Transform())
            m_LightPlayerPivots.push_back(*character->Get_Transform()->Get_WorldMatrixPtr());
    }
    if (!m_bProductAttempted) { std::string status; (void)Reload_Product(status); }
    Update_TargetedCombatVisuals(dt, bosses);
    const auto* arena = CLevel_KakulSaydonArena::Get_Active();
    const auto* run = arena ? &arena->Get_KoukuBundleState() : nullptr;
    using RUN_STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
    const bool runLive = run && run->iRunEpoch &&
        (run->eState == RUN_STATE::PENDING || run->eState == RUN_STATE::ACTIVE || run->eState == RUN_STATE::PATTERN_COMPLETED);
    m_ProductBundleSession.runEpoch = runLive && !run->strBundleId.empty() ? run->iRunEpoch : 0u;
    if (runLive && m_iProductReloadRunEpoch != run->iRunEpoch)
    {
        m_iProductReloadRunEpoch = run->iRunEpoch;
        if (auto* level = CLevel_KakulSaydonArena::Get_Active())
        { std::string cameraStatus; if (!level->Reload_PublishedCameraShots(cameraStatus)) m_strStatus = cameraStatus; }
        if (run->iPinnedSourceRevision != m_iProductSourceRevision)
        { std::string status; (void)Reload_Product(status, run->iPinnedSourceRevision); }
    }
    if (runLive && !run->strBundleId.empty())
    {
        const auto product = m_ProductBundles.find(run->strBundleId);
        float seconds = 0.f;
        if (product == m_ProductBundles.end() || run->iPinnedSourceRevision != m_iProductSourceRevision)
        {
            Stop_Session(m_ProductBundleSession);
            m_strStatus = "Replicated bundle Product/source revision is unavailable: " + run->strBundleId;
        }
        else if (CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), run->iCommonStartTick, 30.f, seconds))
        {
            const auto key = "bundle-product:" + std::to_string(run->iRunEpoch) + ":" + run->strBundleId;
            if (m_ProductBundleSession.key != key)
            { Stop_Session(m_ProductBundleSession); m_ProductBundleSession.key = key; }
            const float clock = m_ProductBundleSession.lastClockMs < 0.f ? seconds * 1000.f :
                (std::max)(seconds * 1000.f, m_ProductBundleSession.lastClockMs + dt * 1000.f);
            float4x4_t pivot; XMStoreFloat4x4(&pivot, XMMatrixIdentity());
            Sample(m_ProductBundleSession, product->second.common.document, product->second.common.pattern,
                clock, false, pivot, nullptr);
        }
        else Stop_Session(m_ProductBundleSession);
    }
    else Stop_Session(m_ProductBundleSession);
    std::set<std::uint32_t> liveMarioEntries;
    if (runLive && run->iPinnedSourceRevision == m_iProductSourceRevision)
        for (const auto& member : run->Members)
        {
            if (member.strMarioEntryPatternId.empty()) continue;
            const auto root = m_Product.find(member.strMarioEntryPatternId);
            if (root == m_Product.end()) continue;
            float seconds = 0.f;
            if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
                (std::max)(run->iServerTick, arena->Get_PresentationServerTick()), member.iMarioEntryStartTick, 30.f, seconds)) continue;
            liveMarioEntries.insert(member.iBossNetEntityId);
            auto& session = m_MarioEntrySessions[member.iBossNetEntityId];
            const auto key = std::to_string(run->iRunEpoch) + ":" + member.strMemberId + ":" + std::to_string(member.iMarioEntryStartTick);
            if (session.key != key) { Stop_Session(session); session.key = key; }
            session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId;
            const auto pivot = XMMatrixRotationY(XMConvertToRadians(member.fMarioEntryYawDegrees)) *
                XMMatrixTranslation(member.fMarioEntryX, member.fMarioEntryY, member.fMarioEntryZ);
            float4x4_t storedPivot; XMStoreFloat4x4(&storedPivot, pivot);
            // Keep the authored entry frame while child animations run. Its
            // lifetime and terminal stop come only from persistent Server state.
            const float clock = (std::min)(seconds * 1000.f, float(member.iMarioEntryHoldMs));
            Sample(session, root->second.document, root->second.pattern, clock,
                seconds * 1000.f >= member.iMarioEntryHoldMs, storedPivot, nullptr);
        }
    for (auto it = m_MarioEntrySessions.begin(); it != m_MarioEntrySessions.end();)
        if (liveMarioEntries.contains(it->first)) ++it;
        else { Stop_Session(it->second); it = m_MarioEntrySessions.erase(it); }
    std::set<std::uint32_t> liveBosses;
    for (const auto& view : bosses)
    {
        if (runLive && run->iPinnedSourceRevision != m_iProductSourceRevision) continue;
        const auto npc = view.pNpc.lock();
        const auto product = m_Product.find(view.Snapshot.strPatternId);
        if (m_bProductLoaded && !view.Snapshot.strPatternId.empty() && product == m_Product.end() &&
            m_MissingProductPatterns.insert(view.Snapshot.strPatternId).second)
        {
            m_strStatus = "Snapshot pattern has no Product presentation: " + view.Snapshot.strPatternId;
            OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + m_strStatus + "\n").c_str());
        }
        if (!npc || !npc->Get_Transform() || product == m_Product.end() || !view.Snapshot.iCurrentHp) continue;
        float seconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(view.iServerTick,
            view.Snapshot.iPatternStartTick, 30.f, seconds)) continue;
        const auto id = view.Snapshot.iNetEntityId;
        if (liveMarioEntries.contains(id) && std::any_of(run->Members.begin(), run->Members.end(), [&](const auto& member) {
            return member.iBossNetEntityId == id && member.strMarioEntryPatternId == view.Snapshot.strPatternId;
        })) continue;
        liveBosses.insert(id);
        SESSION& session = m_BossSessions[id];
        session.runEpoch = 0u; session.memberId.clear();
        if (runLive)
            for (const auto& member : run->Members)
                if (member.iBossNetEntityId == id)
                { session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId; break; }
        const std::string key = std::to_string(id) + ":" + view.Snapshot.strPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPatternStartTick);
        if (session.key != key) { Stop_Session(session); session.key = key; }
        // Interpolate between snapshots without rewinding/restarting effects every network tick.
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        Sample(session, product->second.document, product->second.pattern,
            (std::min)(clock, float(product->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weaponView : nullptr);
    }
    for (auto session = m_BossSessions.begin(); session != m_BossSessions.end();)
    {
        if (liveBosses.contains(session->first)) { ++session; continue; }
        Stop_Session(session->second);
        session = m_BossSessions.erase(session);
    }
    // Child effects use their own Server clock and the same live body. Keeping
    // a separate session preserves every common row on the parent timeline.
    std::set<std::uint32_t> liveChildren;
    for (const auto& view : bosses)
    {
        if (runLive && run->iPinnedSourceRevision != m_iProductSourceRevision) continue;
        if (view.Snapshot.strPresentationPatternId.empty() || !view.Snapshot.iCurrentHp) continue;
        const auto npc = view.pNpc.lock();
        const auto child = m_Product.find(view.Snapshot.strPresentationPatternId);
        if (m_bProductLoaded && child == m_Product.end() &&
            m_MissingProductPatterns.insert(view.Snapshot.strPresentationPatternId).second)
        {
            m_strStatus = "Snapshot child pattern has no Product presentation: " + view.Snapshot.strPresentationPatternId;
            OutputDebugStringA(("[KoukuSaydonPresentationPlayer] " + m_strStatus + "\n").c_str());
        }
        if (!npc || !npc->Get_Transform() || child == m_Product.end()) continue;
        float seconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(view.iServerTick,
            view.Snapshot.iPresentationPatternStartTick, 30.f, seconds)) continue;
        const auto id = view.Snapshot.iNetEntityId;
        liveChildren.insert(id);
        SESSION& session = m_ChildBossSessions[id];
        const auto key = std::to_string(id) + ":child:" + view.Snapshot.strPresentationPatternId + ":" +
            std::to_string(view.Snapshot.iPatternSequence) + ":" + std::to_string(view.Snapshot.iPresentationPatternStartTick);
        if (session.key != key) { Stop_Session(session); session.key = key; }
        session.runEpoch = 0u; session.memberId.clear();
        if (runLive)
            for (const auto& member : run->Members)
                if (member.iBossNetEntityId == id)
                { session.runEpoch = run->iRunEpoch; session.memberId = member.strMemberId; break; }
        const float clock = session.lastClockMs < 0.f ? seconds * 1000.f :
            (std::max)(seconds * 1000.f, session.lastClockMs + dt * 1000.f);
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = npc->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        Sample(session, child->second.document, child->second.pattern,
            (std::min)(clock, float(child->second.durationMs)), false,
            *npc->Get_Transform()->Get_WorldMatrixPtr(), npc->Get_Model(), hasWeapon ? &weaponView : nullptr);
    }
    for (auto session = m_ChildBossSessions.begin(); session != m_ChildBossSessions.end();)
    {
        if (liveChildren.contains(session->first)) { ++session; continue; }
        Stop_Session(session->second);
        session = m_ChildBossSessions.erase(session);
    }
    std::set<std::uint32_t> liveCards;
    for (const auto& view : players)
    {
        const auto character = view.pCharacter.lock();
        const std::string asset = Card_Asset(view.Snapshot);
        if (!character || !character->Get_Transform() || asset.empty()) continue;
        const auto id = view.Snapshot.iNetEntityId;
        liveCards.insert(id);
        CARD& card = m_Cards[id];
        if (card.assetId != asset)
        {
            if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
            card = {};
            card.assetId = asset;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
            if (Ensure_EffectResource("GROUP", asset, effects))
            {
                if (const auto* source = effects->Find_Group(asset))
                {
                    EFFECT_V2_GROUP group = *source;
                    group.iDurationMs = 0u;
                    for (auto& child : group.Children)
                    {
                        child.vOffset.z = 0.f;
                        child.LocalTransform.vTranslation.z = 0.f;
                    }
                    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                    XMStoreFloat4x4(&playback.PivotWorld, XMMatrixIdentity());
                    playback.fDurationSeconds = 0.f;
                    playback.bProductOwned = true;
                    card.handle = CEffectV2Runtime::Play_Group(group, effects,
                        playback, m_Device, m_Context);
                }
                if (!card.handle) m_strStatus = "Assigned card effect unavailable: " + asset;
            }
        }
        if (card.handle)
        {
            const matrix_t world = XMLoadFloat4x4(character->Get_Transform()->Get_WorldMatrixPtr());
            vector_t position = world.r[3];
            const auto body = character->Get_BodyModel();
            if (body && body->Has_Bone("bip001-head"))
            {
                position = (body->Get_BoneMatrix("bip001-head") * world).r[3];
                position = XMVectorSetY(position, XMVectorGetY(position) + 0.65f - 2.f);
            }
            // Both card leaves already contribute +2m Y. Keep their billboard vertical.
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslationFromVector(position));
            CEffectV2Runtime::Set_GroupPivot(card.handle, pivot);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(card.handle, failure))
            {
                CEffectV2Runtime::Stop_Group(card.handle);
                card.handle = 0;
                m_strStatus = std::move(failure);
            }
        }
    }
    for (auto card = m_Cards.begin(); card != m_Cards.end();)
    {
        if (liveCards.contains(card->first)) { ++card; continue; }
        if (card->second.handle) CEffectV2Runtime::Stop_Group(card->second.handle);
        card = m_Cards.erase(card);
    }
    Update_MazeMarks(players);
    Update_FearPresentation(dt, players);
    if (m_bPreviewPlaying && m_bOwnPreviewClock && m_bPreviewPivotReady)
    {
        // Begin runs after this Update; the next delta includes synchronous WORLD
        // preparation. Arm the new clock once without charging that setup time.
        if (!Prepare_PreviewEffects())
        { m_bPreviewClockAwaitingFirstUpdate = true; Refresh_SharedPresentation(); return; }
        const bool advanceClock = !m_bPreviewClockAwaitingFirstUpdate;
        m_bPreviewClockAwaitingFirstUpdate = false;
        if (advanceClock && !m_bPreviewPaused && !m_bModelReferencePreview && !m_bPreviewCaptureClockHeld)
            m_fPreviewClockMs += double(dt) * 1000.0;
        if (!m_bPreviewPaused && !m_bModelReferencePreview && m_fPreviewClockMs >= m_iPreviewDurationMs)
        {
            if (m_bColliderResourcePreview)
            {
                m_fPreviewClockMs = m_iPreviewDurationMs - 1u;
                Pause_Preview(true);
            }
            else
            {
                // MainApp applies this frame's Pause/Seek/Stop before consuming completion.
                // Keep the real clock and borrowed actors alive until that decision.
                m_fPreviewClockMs = m_iPreviewDurationMs;
                m_strCompletedPreviewPatternId = Preview_IsBundle() ? m_PreviewBundleId : m_PreviewPattern.strPatternId;
            }
        }
        // Bundle members own their independent WORLD players, sampled before effects.
        if (Preview_IsBundle() && m_strCompletedPreviewPatternId.empty()) Sample_BundlePreview();
        // MainApp samples single-pattern WORLD first, then its presentation.
    }
    Refresh_SharedPresentation();
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_Preview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, bool ownClock,
    std::uint32_t clockMs, bool paused, std::string& status)
{
    const auto duration = Pattern_Duration(pattern);
    if (!pattern.strLoadError.empty() || !duration)
    {
        status = "Presentation preview needs a valid finite pattern.";
        return false;
    }
    for (const auto& box : pattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource == document.PresentationResources.end())
        {
            status = "Preview names an unknown presentation resource: " + box.strResourceId;
            return false;
        }
        if (resource->eKind == KIND::EFFECT && !Validate_EffectAnchor(document, pattern, box, status)) return false;
    }
    if (pattern.strPatternId == "preview.kouku.resource" && pattern.WorldOccurrences.empty() &&
        pattern.PresentationOccurrences.size() == 1u)
    {
        const auto& box = pattern.PresentationOccurrences.front();
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource->eKind == KIND::EFFECT &&
            (resource->strResourceKind == "V1_EFFECT" || resource->strResourceKind == "V1_ELEMENT"))
        {
            // A Resource has its own animation origin. Prepare its saved actor
            // through the existing single-member model/presentation owner.
            const auto effect = CEffectCatalog::Find(resource->strAssetId);
            if (!effect) { status = CEffectCatalog::Get_Status(); return false; }
            if (effect->SourceModelPreview)
            {
                CEffectCompositionModelPreview source;
                if (!source.Select_SourceEffect(*effect)) { status = source.Status(); return false; }
                auto stagedDocument = document;
                auto sourcePattern = source.Get_Document().Patterns.front();
                sourcePattern.strPatternId = pattern.strPatternId + ".actor";
                sourcePattern.PresentationOccurrences = pattern.PresentationOccurrences;
                // The Effect window owns lifetime; the native final pose is held
                // across particle tails by the same CModel source sampler.
                sourcePattern.iDurationMs = duration;
                sourcePattern.Stages.front().iDurationMs = duration;
                auto& animations = sourcePattern.Stages.front().AnimationOccurrences;
                std::erase_if(animations, [&](const auto& animation) { return animation.iStartOffsetMs >= duration; });
                for (auto& animation : animations)
                {
                    animation.iPoseStartMs = animation.iStartOffsetMs;
                    animation.iPlayMs = (std::min)(animation.iPlayMs, duration - animation.iStartOffsetMs);
                }
                const auto sourceId = sourcePattern.strPatternId;
                const auto gateId = sourcePattern.strGateId;
                stagedDocument.Patterns.push_back(std::move(sourcePattern));
                KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
                bundle.strBundleId = pattern.strPatternId; bundle.strGateId = gateId;
                bundle.Members.push_back({pattern.strPatternId + ".member", sourceId, 0u});
                stagedDocument.Bundles.push_back(std::move(bundle));
                if (!Begin_BundlePreview(stagedDocument, pattern.strPatternId, clockMs, paused, status)) return false;
                status = m_strStatus = "Effect Resource source model and animation ready: " + resource->strAssetId;
                return true;
            }
        }
        const bool bossLight = resource->eKind == KIND::LIGHT && box.strAnchorKind == "BOSS";
        if (bossLight && (pattern.strActorProfileId.empty() || pattern.strGateId.empty() || pattern.strTargetBossPlacementId.empty()))
        { status = "Boss Light Preview requires a selected Pattern with an exact Gate and boss target."; return false; }
        if ((resource->eKind == KIND::EFFECT || bossLight) && !pattern.strActorProfileId.empty() &&
            !pattern.strGateId.empty() && !pattern.strTargetBossPlacementId.empty())
        {
            // Reuse the selected boss actor owner for a Resource without source
            // animation metadata. Its existing idle pose needs no invented clip.
            auto stagedDocument = document;
            auto sourcePattern = pattern;
            sourcePattern.strPatternId = pattern.strPatternId + ".actor";
            sourcePattern.iDurationMs = duration;
            const auto sourceId = sourcePattern.strPatternId;
            stagedDocument.Patterns.push_back(std::move(sourcePattern));
            KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
            bundle.strBundleId = pattern.strPatternId; bundle.strGateId = pattern.strGateId;
            bundle.Members.push_back({pattern.strPatternId + ".member", sourceId, 0u});
            stagedDocument.Bundles.push_back(std::move(bundle));
            if (!Begin_BundlePreview(stagedDocument, pattern.strPatternId, clockMs, paused, status)) return false;
            status = m_strStatus = "Resource selected boss ready: " + resource->strAssetId +
                " | " + pattern.strGateId + " | " + pattern.strTargetBossPlacementId;
            return true;
        }
    }
    // Stage first. A rejected preview request preserves the active session.
    auto stagedDocument = document;
    auto stagedPattern = pattern;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(stagedPattern);
    m_PreviewSession.key = "preview:" + pattern.strPatternId;
    m_iPreviewDurationMs = duration;
    m_fPreviewClockMs = (std::min)(clockMs, duration);
    m_bOwnPreviewClock = ownClock;
    m_bPreviewClockAwaitingFirstUpdate = ownClock;
    m_bPreviewPlaying = true;
    m_bPreviewPaused = paused;
    m_bColliderResourcePreview = pattern.strPatternId == "preview.kouku.resource" &&
        pattern.PresentationOccurrences.size() == 1u &&
        std::any_of(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&pattern](const auto& resource) { return resource.eKind == KIND::COLLIDER &&
                resource.strResourceId == pattern.PresentationOccurrences.front().strResourceId; });
    m_EffectResources.clear();
    m_EffectResourceFailures.clear();
    status = "Presentation preview ready: " + pattern.strPatternId;
    m_strStatus = status;
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Release_BundlePreviewMembers(
    std::vector<BUNDLE_PREVIEW_MEMBER>& members)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    for (auto& member : members)
    {
        if (const auto source = member.suppressedSourceActor.lock())
            source->Release_CompositionPreviewSuppression();
        member.suppressedSourceActor.reset();
        if (member.rootMotion)
        {
            member.rootMotion.reset();
            if (member.actor) (void)member.actor->Apply_NetworkState(member.initialPosition, member.initialYawDegrees);
        }
        else if (member.actor && member.actor->Get_Model())
            (void)member.actor->Get_Model()->Set_RootMotionVerticalScale(1.f);
        Stop_Session(member.session);
        if (level)
        {
            const auto targets = level->Get_CompositionWorldTargets();
            for (auto& [id, player] : member.session.previewWorlds) player->Stop_All(targets, true);
            level->Release_CompositionPreviewActor(member.actor);
        }
        else if (member.actor)
            CGameInstance::Get().Remove_GameObject_from_Layer(ETOUI(LEVEL::KAKULSAYDON_ARENA),
                L"Layer_KoukuCompositionPreview", member.actor);
    }
    members.clear();
}

bool Client::CKoukuSaydonPresentationPlayer::Prepare_CloneSplitPreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document,
    std::vector<BUNDLE_PREVIEW_MEMBER>& members, std::string& status)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    std::vector<BUNDLE_PREVIEW_MEMBER> clones;
    const auto fail = [&](const std::string& reason) {
        Release_BundlePreviewMembers(clones); status = reason; return false;
    };
    const auto flatten = [](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern) {
        std::vector<KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE> rows;
        uint32_t start = 0u;
        for (const auto& stage : pattern.Stages)
        {
            for (auto row : stage.AnimationOccurrences)
            {
                const auto first = std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                    [](const auto& a, const auto& b) { return a.iStartOffsetMs < b.iStartOffsetMs; });
                row.iPoseStartMs = start + (row.strOccurrenceId == first->strOccurrenceId ? 0u : row.iStartOffsetMs);
                row.iStartOffsetMs += start;
                rows.push_back(std::move(row));
            }
            start += stage.iDurationMs;
        }
        std::stable_sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
            return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId;
        });
        return rows;
    };
    const auto findPattern = [&](const std::string& id) -> const KOUKU_SAYDON_COMPOSITION_PATTERN* {
        const auto found = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& pattern) { return pattern.strPatternId == id; });
        return found == document.Patterns.end() ? nullptr : &*found;
    };
    const auto prepareClone = [&](const KOUKU_SAYDON_COMPOSITION_PATTERN& source, const std::string& id,
        uint32_t startTicks, uint32_t duration, const float3_t& position, float yaw) {
        clones.emplace_back();
        auto& clone = clones.back();
        clone.memberId = id; clone.offsetTicks = startTicks; clone.durationMs = duration;
        clone.pattern = source; clone.facingStages = source.Stages; clone.finiteActorLifetime = true;
        clone.animations = flatten(source);
        if (!level || !duration || clone.animations.empty() ||
            !level->Create_CompositionPreviewActor(source, clone.actor, status)) return false;
        clone.actor->Set_PresentationVisible(false);
        if (!clone.actor->Apply_NetworkState(position, yaw)) return false;
        clone.initialPosition = position; clone.initialYawDegrees = yaw;
        const auto model = clone.actor->Get_Model();
        clone.initialAnimation = model->Get_CurrentAnimIndex();
        float ignored = 0.f;
        model->Get_AnimationProgress(clone.initialAnimation, clone.initialTicks, ignored);
        clone.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
        return clone.rootMotion->Prepare(model, clone.animations, float(source.fAnimationRootVerticalScale), status) &&
            clone.rootMotion->Begin_Suppression();
    };
    for (auto& member : members)
    {
        const auto authoredPattern = member.pattern;
        std::vector<KOUKU_SAYDON_COMPOSITION_CROSS_DIRECTION_WINDOW> splitBoxes;
        if (!CKoukuSaydonCompositionDocument::Try_ResolveCrossDirectionWindows(
            document, authoredPattern, splitBoxes, status)) return fail(status);
        for (const auto& box : splitBoxes)
        {
            if (box.DirectionPatternIds.size() != 4u || !box.iDurationMs)
                return fail("Cross-direction preview needs four Patterns and a finite duration.");
            CWorldGameplayDocument world;
            if (!world.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") / document.strAreaId / "Gameplay.world.json"),
                document.strAreaId, status)) return fail(status);
            const auto* spawn = world.Find(member.pattern.strTargetBossPlacementId);
            if (!spawn || !Sample_BundlePreviewFacing(member, box.iStartMs))
                return fail("Cross-direction preview cannot resolve its boss spawn or starting transform.");
            const auto root = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
            const float3_t position{root._41, root._42, root._43};
            const float yaw = XMConvertToDegrees(std::atan2(root._31, root._33));
            std::array<const KOUKU_SAYDON_COMPOSITION_PATTERN*, 4u> candidates{};
            std::array<uint32_t, 4u> cutoffs{};
            size_t selected = 0u;
            double bestDistance = (std::numeric_limits<double>::max)();
            for (size_t i = 0u; i < candidates.size(); ++i)
            {
                const auto* candidate = findPattern(box.DirectionPatternIds[i]);
                if (!candidate || candidate->strTargetBossPlacementId != member.pattern.strTargetBossPlacementId)
                    return fail("Cross-direction preview names an unavailable boss Pattern.");
                bool cutoffFound = false;
                for (const auto& stage : candidate->Stages)
                {
                    cutoffs[i] += stage.iDurationMs;
                    if (stage.strStageId == box.strCloneEndStageId) { cutoffFound = true; break; }
                }
                if (!cutoffFound || Pattern_Duration(*candidate) > box.iDurationMs)
                    return fail("Cross-direction preview has an invalid cutoff or child duration.");
                const auto rows = flatten(*candidate);
                CKoukuSaydonPreviewRootMotion motion;
                float3_t endpoint;
                const std::vector<float> yaws(rows.size(), yaw);
                if (!motion.Prepare(member.actor->Get_Model(), rows, float(candidate->fAnimationRootVerticalScale), status) ||
                    !motion.Sample_Displacement(cutoffs[i], yaws, endpoint)) return fail(status);
                const double dx = position.x + endpoint.x - spawn->position.x;
                const double dz = position.z + endpoint.z - spawn->position.z;
                const double distance = dx * dx + dz * dz;
                if (distance < bestDistance - 0.00001) { selected = i; bestDistance = distance; }
                candidates[i] = candidate;
            }
            const uint32_t startTicks = (uint64_t(box.iStartMs) * 30u + 999u) / 1000u;
            for (size_t i = 0u; i < candidates.size(); ++i)
            {
                if (i == selected) continue;
                auto fake = *candidates[i];
                fake.PresentationOccurrences.clear();
                if (!prepareClone(fake, member.memberId + ":" + box.strOccurrenceId + ":" + std::to_string(i),
                    member.offsetTicks + startTicks, cutoffs[i], position, yaw)) return fail(status);
            }
            const auto prefix = box.strOccurrenceId + ".selected.";
            for (auto row : flatten(*candidates[selected]))
            {
                row.strOccurrenceId = prefix + row.strOccurrenceId;
                row.iStartOffsetMs += box.iStartMs; row.iPoseStartMs += box.iStartMs;
                member.cloneAnimationWindows.emplace(row.strOccurrenceId,
                    std::pair{box.iStartMs, box.iStartMs + box.iDurationMs});
                member.animations.push_back(std::move(row));
            }
            for (auto effect : candidates[selected]->PresentationOccurrences)
            {
                effect.strOccurrenceId = prefix + effect.strOccurrenceId;
                effect.iStartMs += box.iStartMs;
                if (!effect.strSelectionGroupId.empty()) effect.strSelectionGroupId = prefix + effect.strSelectionGroupId;
                if (!effect.strLogicOccurrenceId.empty()) effect.strLogicOccurrenceId = prefix + effect.strLogicOccurrenceId;
                member.pattern.PresentationOccurrences.push_back(std::move(effect));
            }
            std::stable_sort(member.animations.begin(), member.animations.end(), [](const auto& a, const auto& b) {
                return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId;
            });
            KOUKU_SAYDON_COMPOSITION_STAGE previewStage;
            previewStage.strStageId = "preview.clone-split";
            previewStage.iDurationMs = member.durationMs;
            previewStage.AnimationOccurrences = member.animations;
            member.pattern.Stages = {std::move(previewStage)};
            member.rootMotion.reset();
            member.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
            if (!member.rootMotion->Prepare(member.actor->Get_Model(), member.animations,
                float(member.pattern.fAnimationRootVerticalScale), status) ||
                !member.rootMotion->Begin_Suppression()) return fail(status);
        }
        for (const auto& summon : authoredPattern.SummonOccurrences)
        {
            if (summon.PatternSpawns.empty()) continue;
            if (!Sample_BundlePreviewFacing(member, summon.iStartMs)) return fail("Summon preview cannot sample its owner.");
            const auto root = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
            const float yaw = XMConvertToDegrees(std::atan2(root._31, root._33));
            const auto rotation = XMMatrixRotationY(XMConvertToRadians(yaw));
            for (const auto& spawn : summon.PatternSpawns)
            {
                const auto* candidate = findPattern(spawn.strPatternId);
                if (!candidate) return fail("Summon preview child Pattern is unavailable: " + spawn.strPatternId);
                float3_t offset;
                XMStoreFloat3(&offset, XMVector3TransformNormal(XMVectorSet(float(spawn.PositionOffset[0]),
                    float(spawn.PositionOffset[1]), float(spawn.PositionOffset[2]), 0.f), rotation));
                const float3_t position{root._41 + offset.x, root._42 + offset.y, root._43 + offset.z};
                const uint32_t startTicks = (uint64_t(summon.iStartMs) * 30u + 999u) / 1000u;
                if (!prepareClone(*candidate, member.memberId + ":" + summon.strOccurrenceId + ":" + spawn.strSpawnId,
                    member.offsetTicks + startTicks, (std::min)(summon.iDurationMs, Pattern_Duration(*candidate)),
                    position, yaw + float(spawn.fYawOffsetDegrees))) return fail(status);
            }
        }
        (void)member.actor->Apply_NetworkState(member.initialPosition, member.initialYawDegrees);
    }
    for (auto& clone : clones) members.push_back(std::move(clone));
    return true;
}
bool Client::CKoukuSaydonPresentationPlayer::Begin_BundlePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& bundleId,
    std::uint32_t clockMs, bool paused, std::string& status, const CWorldSequenceDocument* sourceDocument,
    const bool automaticRootMotion, bool externalWorldPreview)
{
    const auto bundle = std::find_if(document.Bundles.begin(), document.Bundles.end(),
        [&](const auto& value) { return value.strBundleId == bundleId; });
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || bundle == document.Bundles.end() || !bundle->strLoadError.empty() ||
        bundle->Members.empty() || bundle->Members.size() > LostArk::Shared::MAX_KOUKUSAYDON_BUNDLE_MEMBERS)
    { status = "Bundle preview requires an active arena and a valid nonempty bundle."; return false; }
    if (externalWorldPreview && (bundle->Members.size() != 1u || bundle->Members.front().iStartOffsetMs))
    { status = "Level WORLD preview requires one Pattern on the common clock."; return false; }
    CWorldGameplayDocument previewWorld;
    if (!previewWorld.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
        document.strAreaId / "Gameplay.world.json"), document.strAreaId, status)) return false;
    std::vector<BUNDLE_PREVIEW_MEMBER> staged;
    std::vector<CWorldSequencePlayer*> stagedWorldPlayers;
    KOUKU_SAYDON_COMPOSITION_PATTERN common;
    common.strPatternId = bundleId;
    common.PresentationOccurrences = bundle->PresentationOccurrences;
    common.SceneProfileOccurrences = bundle->SceneProfileOccurrences;
    std::uint64_t duration = Pattern_Duration(common);
    std::set<std::string> targetsUsed, memberIds, placementBindings;
    const auto targets = level->Get_CompositionWorldTargets();
    const auto& sequences = sourceDocument ? *sourceDocument : level->Get_WorldSequenceDocument();
    const auto fail = [&](const std::string& reason)
    { Release_BundlePreviewMembers(staged); status = reason; return false; };
    std::vector<PRESENTATION_WINDOW> globalWindows;
    for (const auto& row : common.SceneProfileOccurrences)
        (void)Admit_PresentationWindow(globalWindows, KIND::SCENE_PROFILE, row.iStartMs,
            double(row.iStartMs) + row.iDurationMs, "common");
    for (const auto& box : common.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (resource == document.PresentationResources.end() ||
            (resource->eKind != KIND::CAMERA && resource->eKind != KIND::SCENE_PROFILE))
            return fail("Bundle common lane only accepts Camera or Scene Profile resources.");
        (void)Admit_PresentationWindow(globalWindows, resource->eKind, box.iStartMs,
            double(box.iStartMs) + box.iDurationMs + Camera_ReturnMs(*resource, true), "common");
    }
    for (const auto& sourceMember : bundle->Members)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == sourceMember.strPatternId; });
        if (source == document.Patterns.end() || !source->strLoadError.empty() ||
            source->strGateId != bundle->strGateId || source->strTargetBossPlacementId.empty() ||
            !targetsUsed.insert(source->strTargetBossPlacementId).second ||
            !memberIds.insert(sourceMember.strMemberId).second || sourceMember.strMemberId.empty())
            return fail("Bundle has an invalid, duplicate, or cross-Gate target/member.");
        const double offset = std::ceil(double(sourceMember.iStartOffsetMs) * 30.0 / 1000.0) * 1000.0 / 30.0;
        for (const auto& row : source->SceneProfileOccurrences)
            if (!Admit_PresentationWindow(globalWindows, KIND::SCENE_PROFILE, offset + row.iStartMs,
                offset + row.iStartMs + row.iDurationMs, sourceMember.strMemberId))
                return fail("Bundle global Scene Profile windows overlap.");
        for (const auto& box : source->PresentationOccurrences)
        {
            const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
                [&](const auto& value) { return value.strResourceId == box.strResourceId; });
            if (resource == document.PresentationResources.end()) return fail("Unknown child presentation resource.");
            if (resource->eKind == KIND::EFFECT && !Validate_EffectAnchor(document, *source, box, status))
                return fail(status);
            if ((resource->eKind == KIND::CAMERA || resource->eKind == KIND::SCENE_PROFILE) &&
                !Admit_PresentationWindow(globalWindows, resource->eKind, offset + box.iStartMs,
                    offset + box.iStartMs + box.iDurationMs + Camera_ReturnMs(*resource, true), sourceMember.strMemberId))
                return fail("Bundle global presentation windows overlap.");
        }
        staged.emplace_back();
        auto& member = staged.back();
        member.memberId = sourceMember.strMemberId;
        member.offsetTicks = static_cast<std::uint32_t>((std::uint64_t(sourceMember.iStartOffsetMs) * 30u + 999u) / 1000u);
        member.pattern = *source;
        const auto* sourcePlacement = previewWorld.Find(source->strTargetBossPlacementId);
        if (!sourcePlacement || sourcePlacement->eKind != WORLD_PLACEMENT_KIND::BOSS)
            return fail("Preview source boss placement is unavailable: " + source->strTargetBossPlacementId);
        member.sourceArchetypeId = sourcePlacement->archetypeId;
        member.facingStages = source->Stages;
        if (!CKoukuSaydonCompositionDocument::Try_ResolveAnimationBlendWindows(document, *source,
            member.pattern.AnimationBlendWindows, status)) return fail(status);
        member.durationMs = Pattern_Duration(*source);
        if (!member.durationMs) return fail("Empty child pattern cannot be previewed.");
        duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u + member.durationMs);
        if (duration > MAX_TIMELINE_MS) return fail("Bundle preview exceeds the timeline duration limit.");
        if (!level->Create_CompositionPreviewActor(*source, member.actor, status)) return fail(status);
        const auto model = member.actor->Get_Model();
        if (!CKoukuSaydonAnimationBlend::Validate_ModelWindows(*model, member.pattern.AnimationBlendWindows, status))
            return fail(status);
        member.initialAnimation = model->Get_CurrentAnimIndex();
        const auto& initialRoot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
        member.initialYawDegrees = XMConvertToDegrees(std::atan2(initialRoot._31, initialRoot._33));
        member.initialPosition = {initialRoot._41, initialRoot._42, initialRoot._43};
        float ignored = 0.f;
        model->Get_AnimationProgress(member.initialAnimation, member.initialTicks, ignored);
        std::uint32_t stageStart = 0u;
        for (const auto& stage : source->Stages)
        {
            for (auto box : stage.AnimationOccurrences)
            {
                bool found = false;
                for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                    if (const auto* name = model->Get_AnimationName(i); name && box.strRuntimeClip == name)
                    { found = true; break; }
                if (!found || !box.iPlayMs || !std::isfinite(box.fPlayRate) || box.fPlayRate <= 0.f)
                    return fail("Bundle child animation is unavailable: " + box.strRuntimeClip);
                box.iPoseStartMs = stageStart + (box.strOccurrenceId == std::min_element(stage.AnimationOccurrences.begin(), stage.AnimationOccurrences.end(),
                [](const auto& a, const auto& b) { return a.iStartOffsetMs != b.iStartOffsetMs ?
                    a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; })->strOccurrenceId ? 0u : box.iStartOffsetMs);
                box.iStartOffsetMs += stageStart;
                member.animations.push_back(std::move(box));
            }
            stageStart += stage.iDurationMs;
        }
        std::sort(member.animations.begin(), member.animations.end(), [](const auto& a, const auto& b)
            { return a.iStartOffsetMs != b.iStartOffsetMs ? a.iStartOffsetMs < b.iStartOffsetMs : a.strOccurrenceId < b.strOccurrenceId; });
        bool airborne = false, spatialLogic = false;
        for (const auto& box : source->LogicOccurrences)
        {
            if (!box.bEnabled) continue;
            const auto definition = std::find_if(document.Logics.begin(), document.Logics.end(),
                [&](const auto& logic) { return logic.strLogicId == box.strLogicId; });
            if (definition == document.Logics.end()) continue;
            airborne |= definition->strTriggerKind == "ALBION_AIRBORNE";
            spatialLogic |= definition->strTriggerKind == "BOSS_TELEPORT_XZ";
            if (definition->strJudgementKind == "BOSS_TRACK_TARGET" ||
                definition->strJudgementKind == "SHOWTIME_PLAYER_TARGETS")
            {
                auto& tracking = member.targetTracking.emplace_back();
                tracking.occurrenceId = box.strOccurrenceId;
                tracking.startMs = box.iStartMs; tracking.durationMs = box.iDurationMs;
                tracking.immediate = definition->strJudgementKind == "SHOWTIME_PLAYER_TARGETS";
                spatialLogic = true;
            }
        }
        member.spatialLogicPreview = airborne || spatialLogic;
        if ((airborne || spatialLogic) && !CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion(document, *source))
            return fail("Spatial Logic preview requires no competing boss motion owner.");
        if (airborne && member.animations.empty())
            return fail("Albion airborne preview requires its original source animations.");
        if (((automaticRootMotion && !member.animations.empty()) || airborne || spatialLogic) &&
            CKoukuSaydonPreviewRootMotion::Allows_AutomaticMotion(document, *source))
        {
            member.rootMotion = std::make_unique<CKoukuSaydonPreviewRootMotion>();
            if (!member.rootMotion->Prepare(model, member.animations,
                    float(source->fAnimationRootVerticalScale), status) ||
                !member.rootMotion->Prepare_Airborne(document, *source, status) ||
                !member.rootMotion->Begin_Suppression())
                return fail("Bundle child root motion: " + status);
            if (airborne) member.airborneSelectionSeed = std::random_device{}();
        }
        for (const auto& box : source->WorldOccurrences)
        {
            if (externalWorldPreview) continue;
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            if (world == document.Worlds.end()) return fail("Bundle child WORLD resource is missing.");
            const auto* instance = sequences.Find_Instance(world->strSequenceInstanceId);
            if (!instance) return fail("Bundle WORLD sequence is missing: " + world->strSequenceInstanceId);
            if (!level->Can_StartCompositionWorld(world->strSequenceInstanceId, status, &sequences)) return fail(status);
            for (const auto& binding : instance->bindings)
                if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
                    !placementBindings.insert(std::to_string(static_cast<int>(binding.targetKind)) + ":" + binding.targetId).second)
                    return fail("Bundle WORLD members share a mutable map/deploy target.");
            auto player = std::make_shared<CWorldSequencePlayer>();
            const auto [entry, inserted] = member.session.previewWorlds.emplace(box.strOccurrenceId, player);
            if (!inserted) return fail("Bundle WORLD occurrence is duplicated: " + box.strOccurrenceId);
            stagedWorldPlayers.push_back(entry->second.get());
            float3_t offset(float(world->PositionOffset[0]), float(world->PositionOffset[1]), float(world->PositionOffset[2]));
            if (!box.Placement && world->strAnchorKind == "BOSS_SPAWN")
            {
                const auto& pivot = *member.actor->Get_Transform()->Get_WorldMatrixPtr();
                offset.x += pivot._41 - float(world->AnchorPosition[0]);
                offset.y += pivot._42 - float(world->AnchorPosition[1]);
                offset.z += pivot._43 - float(world->AnchorPosition[2]);
            }
            member.worldOffsets.emplace(box.strOccurrenceId, offset);
        }
        member.session.key = "bundle-preview:" + bundleId + ":" + member.memberId;
    }
    if (!stagedWorldPlayers.empty() &&
        !CWorldSequencePlayer::Set_DocumentBatch(sequences, targets, stagedWorldPlayers, status)) return fail(status);
    for (auto& member : staged)
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            if (externalWorldPreview) continue;
            const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = *member.session.previewWorlds.at(box.strOccurrenceId);
            if (!player.Prepare_InstanceResources(world->strSequenceInstanceId, targets))
                return fail("Bundle WORLD " + box.strOccurrenceId + ": " + player.Get_Status());
            if (!player.Validate_ObjectPlacement(world->strSequenceInstanceId, WorldPlacementFromOccurrence(box), status))
                return fail(status);
            const auto worldSpan = player.Get_InstanceElapsedSpanMs(
                world->strSequenceInstanceId, box.fPlaybackSpeed, box.iDurationMs);
            if (worldSpan <= 0.f) return fail("Bundle WORLD has no finite presentation span.");
            duration = (std::max)(duration, (std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u +
                box.iStartMs + static_cast<std::uint64_t>(std::ceil(worldSpan)));
            if (duration > MAX_TIMELINE_MS) return fail("Bundle WORLD tail exceeds the timeline duration limit.");
        }
    if (!Prepare_CloneSplitPreview(document, staged, status)) return fail(status);
    // All models, clips and WORLD inputs are prepared before replacing the live preview.
    auto stagedDocument = document;
    Stop_Preview();
    m_PreviewDocument = std::move(stagedDocument);
    m_PreviewPattern = std::move(common);
    m_PreviewBundleId = bundleId;
    m_bBundleWorldExternal = externalWorldPreview;
    m_BundlePreviewMembers = std::move(staged);
    m_PreviewSession.key = "bundle-preview:" + bundleId + ":common";
    m_iPreviewDurationMs = static_cast<std::uint32_t>(duration);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bOwnPreviewClock = m_bPreviewPlaying = m_bPreviewPivotReady = true;
    m_bPreviewClockAwaitingFirstUpdate = true;
    m_bPreviewPaused = paused;
    XMStoreFloat4x4(&m_PreviewPivot, XMMatrixIdentity());
    Sync_PreviewSourceVisibility();
    if (!externalWorldPreview) Sample_BundlePreview();
    if (!m_bPreviewPlaying) { status = m_strStatus; return false; }
    Refresh_SharedPresentation();
    status = m_strStatus = "Bundle preview ready: " + bundleId;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Begin_ModelReferencePreview(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, const std::string& selectionId,
    const bool isBundle, const std::uint32_t clockMs, const bool paused, std::string& status,
    const CWorldSequenceDocument* propSequences)
{
    KOUKU_SAYDON_COMPOSITION_DOCUMENT reference;
    reference.iRevision = document.iRevision;
    reference.strAreaId = document.strAreaId;
    KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
    if (isBundle)
    {
        const auto source = std::find_if(document.Bundles.begin(), document.Bundles.end(),
            [&](const auto& value) { return value.strBundleId == selectionId; });
        if (source == document.Bundles.end() || !source->strLoadError.empty())
        { status = "Saved model-reference Bundle is missing or invalid."; return false; }
        bundle = *source;
    }
    else
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == selectionId; });
        if (source == document.Patterns.end())
        { status = "Saved model-reference Pattern is missing."; return false; }
        // Runtime-only grouping lets one Pattern use the same staged actor path.
        // It is neither a second saved Pattern nor a writable authoring document.
        bundle.strBundleId = "effect.model.reference:" + selectionId;
        bundle.strGateId = source->strGateId;
        bundle.Members.push_back({selectionId, selectionId, 0u});
    }
    bundle.PresentationOccurrences.clear();
    bundle.SceneProfileOccurrences.clear();
    for (const auto& member : bundle.Members)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == member.strPatternId; });
        if (source == document.Patterns.end() || !source->strLoadError.empty())
        {
            status = "Model-reference child is missing or invalid: " + member.strPatternId;
            if (source != document.Patterns.end()) status += ". " + source->strLoadError;
            return false;
        }
        auto& pattern = reference.Patterns.emplace_back(*source);
        pattern.BossMotion.reset();
        pattern.fAnimationRootVerticalScale = 1.0;
        for (auto& stage : pattern.Stages) stage.bRetargetOnEnter = false;
        pattern.LogicOccurrences.clear();
        pattern.SummonOccurrences.clear();
        if (!propSequences) pattern.WorldOccurrences.clear();
        else
        {
            // A model reference may borrow actor props only. It never starts
            // map/deploy choreography, projectile transitions or nested Effects.
            std::vector<KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE> props;
            if (!CEffectCompositionModelPreview::Stage_ActorWorldProps(document, pattern, *propSequences, props, status))
                return false;
            for (const auto& box : props)
            {
                const auto world = std::find_if(document.Worlds.begin(), document.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == box.strWorldId; });
                if (std::none_of(reference.Worlds.begin(), reference.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == world->strWorldId; }))
                    reference.Worlds.push_back(*world);
            }
            pattern.WorldOccurrences = std::move(props);
        }
        pattern.SceneProfileOccurrences.clear();
        pattern.PresentationOccurrences.clear();
    }
    const std::string bundleId = bundle.strBundleId;
    reference.Bundles.push_back(std::move(bundle));
    if (!Begin_BundlePreview(reference, bundleId, clockMs, paused, status, propSequences, false)) return false;
    m_bModelReferencePreview = true;
    status = m_strStatus = "Model reference ready. Master cursor owns time; actors stay at authored spawn (no Server movement replay).";
    if (propSequences) status = m_strStatus += " Saved actor-bound WORLD props are enabled.";
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_ModelReferencePreview(
    const std::uint32_t clockMs, const bool paused)
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying) return;
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    Sample_BundlePreview();
}

bool Client::CKoukuSaydonPresentationPlayer::Place_ModelReferenceRoot(const float4x4_t& root)
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying || m_BundlePreviewMembers.size() != 1u) return false;
    for (const auto& row : root.m) for (const float value : row) if (!std::isfinite(value)) return false;
    auto& member = m_BundlePreviewMembers.front();
    const float3_t position{root._41, root._42, root._43};
    const float yaw = XMConvertToDegrees(std::atan2(root._31, root._33));
    if (!member.actor || !member.actor->Apply_NetworkState(position, yaw)) return false;
    // Model-reference stages never retarget. Their facing seed must be the
    // explicit preview placement, so resampling props cannot restore spawn yaw.
    member.initialPosition = position;
    member.initialYawDegrees = yaw;
    if (!member.session.previewWorlds.empty()) Sample_BundlePreview();
    return m_bPreviewPlaying;
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ModelReferenceWorldPivot(
    const std::string& memberId, const std::string& occurrenceId, float4x4_t& out) const
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying || occurrenceId.empty()) return false;
    const auto member = std::find_if(m_BundlePreviewMembers.begin(), m_BundlePreviewMembers.end(),
        [&](const auto& value) { return value.memberId == memberId; });
    if (member == m_BundlePreviewMembers.end()) return false;
    const auto box = std::find_if(member->pattern.WorldOccurrences.begin(), member->pattern.WorldOccurrences.end(),
        [&](const auto& value) { return value.strOccurrenceId == occurrenceId; });
    if (box == member->pattern.WorldOccurrences.end()) return false;
    const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
        [&](const auto& value) { return value.strWorldId == box->strWorldId; });
    const auto player = member->session.previewWorlds.find(occurrenceId);
    return world != m_PreviewDocument.Worlds.end() && player != member->session.previewWorlds.end() &&
        player->second->Try_GetSequencePivot(world->strSequenceInstanceId, out);
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_ModelReferenceTarget(
    const std::string& memberId, EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const
{
    if (!m_bModelReferencePreview || !m_bPreviewPlaying) return false;
    const BUNDLE_PREVIEW_MEMBER* selected = nullptr;
    if (memberId.empty() && m_BundlePreviewMembers.size() == 1u)
        selected = &m_BundlePreviewMembers.front();
    else
        for (const auto& member : m_BundlePreviewMembers)
            if (member.memberId == memberId) { selected = &member; break; }
    if (!selected || !selected->actor) return false;
    auto stagedTarget = EFFECT_V2_TARGET::From_Npc(selected->actor);
    stagedTarget.strArchetypeId = std::string(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
        selected->pattern.strTargetBossPlacementId));
    EFFECT_V2_TARGET_VIEW stagedView;
    if (!CEffectV2Object::Resolve_TargetView(stagedTarget, stagedView)) return false;
    target = std::move(stagedTarget);
    view = std::move(stagedView);
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreviewFacing(
    BUNDLE_PREVIEW_MEMBER& member, const double localMs)
{
    float3_t position;
    float yaw = 0.f;
    return Sample_BundlePreviewPose(member, localMs, position, yaw, true) &&
        member.actor->Apply_NetworkState(position, yaw);
}

bool Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreviewPose(
    BUNDLE_PREVIEW_MEMBER& member, const double localMs, float3_t& position, float& yaw,
    const bool recordTargets)
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    const auto player = level ? level->Get_LocalCharacter() : nullptr;
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    if (level && (!member.targetTracking.empty() ||
        (member.rootMotion && !member.rootMotion->Airborne_Events().empty())))
        level->Collect_KoukuPresentationViews(bosses, players);
    players.erase(std::remove_if(players.begin(), players.end(), [](const auto& view) {
        const auto& s = view.Snapshot;
        using LostArk::Shared::PLAYER_ACTION_STATE;
        return !s.iCurrentHp || s.eAction == PLAYER_ACTION_STATE::DEAD ||
            s.eAction == PLAYER_ACTION_STATE::FALLING || s.eAction == PLAYER_ACTION_STATE::GRABBED ||
            !std::isfinite(s.fPositionX) || !std::isfinite(s.fPositionY) || !std::isfinite(s.fPositionZ);
    }), players.end());
    std::sort(players.begin(), players.end(), [](const auto& a, const auto& b) { return a.Snapshot.iNetEntityId < b.Snapshot.iNetEntityId; });
    std::vector<float3_t> selectionPositions;
    if (member.rootMotion && !member.rootMotion->Airborne_Events().empty())
    {
        const auto& events = member.rootMotion->Airborne_Events();
        selectionPositions.resize(events.size(), member.initialPosition);
        const auto pin = [&](const std::string& id) {
            if (!recordTargets || players.empty()) return false;
            uint32_t seed = member.airborneSelectionSeed;
            for (const unsigned char c : id) seed = (seed ^ c) * 16777619u;
            std::mt19937 generator(seed);
            const auto& target = players[std::uniform_int_distribution<size_t>(0u, players.size() - 1u)(generator)].Snapshot;
            member.airborneSelections[id] = {target.iNetEntityId, {target.fPositionX, target.fPositionY, target.fPositionZ}};
            return true;
        };
        size_t selection = SIZE_MAX;
        for (size_t i = 0u; i < events.size(); ++i)
        {
            const auto& event = events[i];
            if (event.clockMs > localMs) break;
            if (event.phase == "SELECT_PLAYER")
            {
                selection = i;
                if (!member.airborneSelections.contains(event.occurrenceId) && !pin(event.occurrenceId)) return false;
            }
            else if (event.phase == "APPEAR_PLAYER" && !member.airborneAppearancePositions.contains(event.occurrenceId))
            {
                if (!recordTargets || selection == SIZE_MAX) return false;
                const auto& id = events[selection].occurrenceId;
                auto chosen = member.airborneSelections.at(id).first;
                if (std::none_of(players.begin(), players.end(), [&](const auto& view) { return view.Snapshot.iNetEntityId == chosen; }))
                { if (!pin(id)) return false; chosen = member.airborneSelections.at(id).first; }
                const auto target = std::find_if(players.begin(), players.end(), [&](const auto& view) { return view.Snapshot.iNetEntityId == chosen; });
                if (target == players.end()) return false;
                const auto& s = target->Snapshot;
                member.airborneAppearancePositions[event.occurrenceId] = {s.fPositionX, s.fPositionY, s.fPositionZ};
            }
        }
        for (size_t i = 0u; i < events.size(); ++i)
        {
            if (const auto pin = member.airborneSelections.find(events[i].occurrenceId); pin != member.airborneSelections.end())
                selectionPositions[i] = pin->second.second;
            if (const auto pin = member.airborneAppearancePositions.find(events[i].occurrenceId); pin != member.airborneAppearancePositions.end())
                selectionPositions[i] = pin->second;
        }
    }
    yaw = member.initialYawDegrees;
    std::vector<float> rowYaws(member.animations.size(), yaw);
    const auto positionAt = [&](double clock, float3_t& position) {
        position = member.initialPosition;
        if (!member.rootMotion) return true;
        if (!selectionPositions.empty())
            return member.rootMotion->Sample_AirbornePosition(clock, rowYaws,
                member.initialPosition, selectionPositions, position);
        float3_t displacement;
        if (!member.rootMotion->Sample_Displacement(clock, rowYaws, displacement)) return false;
        position = {member.initialPosition.x + displacement.x,
            member.initialPosition.y + displacement.y, member.initialPosition.z + displacement.z};
        return true;
    };
    const auto targetYawAt = [&](const float3_t& origin, const float3_t& target, float& targetYaw) {
        const float dx = target.x - origin.x, dz = target.z - origin.z;
        if (!std::isfinite(dx) || !std::isfinite(dz) || dx * dx + dz * dz <= .000001f) return false;
        targetYaw = XMConvertToDegrees(std::atan2(dx, dz));
        if (CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
            member.pattern.strTargetBossPlacementId) == "BOSS_KAKULSAYDON_G2_BIG_SAYDON") targetYaw -= 90.f;
        return true;
    };
    struct FACING_EVENT final
    {
        double clock = 0.0;
        const KOUKU_SAYDON_COMPOSITION_STAGE* stage = nullptr;
        BUNDLE_PREVIEW_MEMBER::TARGET_TRACKING_WINDOW* tracking = nullptr;
        uint32_t tick = 0u, remainingTicks = 0u;
    };
    std::vector<FACING_EVENT> facingEvents;
    uint32_t stageStartMs = 0u;
    for (const auto& stage : member.facingStages)
    {
        if (stageStartMs <= localMs) facingEvents.push_back({double(stageStartMs), &stage});
        stageStartMs += stage.iDurationMs;
    }
    for (auto& window : member.targetTracking)
    {
        const auto begin = (uint64_t(window.startMs) * 30u + 999u) / 1000u;
        const auto end = ((uint64_t(window.startMs) + window.durationMs) * 30u + 999u) / 1000u;
        for (auto tick = begin; tick < end && double(tick) * 1000.0 / 30.0 <= localMs; ++tick)
            facingEvents.push_back({double(tick) * 1000.0 / 30.0, nullptr, &window,
                uint32_t(tick), uint32_t(end - tick)});
    }
    std::stable_sort(facingEvents.begin(), facingEvents.end(), [](const auto& a, const auto& b) {
        if (a.clock != b.clock) return a.clock < b.clock;
        return a.stage != nullptr && b.stage == nullptr;
    });
    for (const auto& event : facingEvents)
    {
        float3_t origin;
        if (!positionAt(event.clock, origin)) return false;
        if (event.stage)
        {
            const auto& stage = *event.stage;
            if (stage.bRetargetOnEnter)
            {
                auto sample = member.stageFacingYawDegrees.find(stage.strStageId);
                const bool inserted = sample == member.stageFacingYawDegrees.end();
                if (inserted)
                {
                    if (!recordTargets) return false;
                    sample = member.stageFacingYawDegrees.emplace(stage.strStageId, yaw).first;
                }
                if (inserted && player && player->Get_Transform())
                {
                    const auto& target = *player->Get_Transform()->Get_WorldMatrixPtr();
                    (void)targetYawAt(origin, {target._41, target._42, target._43}, sample->second);
                }
                yaw = sample->second;
            }
            for (size_t i = 0u; i < member.animations.size(); ++i)
                if (member.animations[i].iPoseStartMs >= event.clock &&
                    member.animations[i].iPoseStartMs < event.clock + stage.iDurationMs)
                    rowYaws[i] = yaw;
            continue;
        }
        auto& window = *event.tracking;
        auto sample = window.targetSamples.find(event.tick);
        const bool inserted = sample == window.targetSamples.end();
        if (inserted)
        {
            if (!recordTargets) return false;
            sample = window.targetSamples.emplace(event.tick, std::nullopt).first;
        }
        if (inserted && !players.empty())
        {
            auto target = std::find_if(players.begin(), players.end(), [&](const auto& view) {
                return view.Snapshot.iNetEntityId == window.targetEntityId;
            });
            if (target == players.end())
            {
                uint32_t seed = member.airborneSelectionSeed;
                for (const unsigned char c : window.occurrenceId) seed = (seed ^ c) * 16777619u;
                target = players.begin() + seed % players.size();
                window.targetEntityId = target->Snapshot.iNetEntityId;
            }
            const auto& s = target->Snapshot;
            sample->second = float3_t{s.fPositionX, s.fPositionY, s.fPositionZ};
        }
        float targetYaw = yaw;
        if (!sample->second || !targetYawAt(origin, *sample->second, targetYaw)) continue;
        if (window.immediate) yaw = targetYaw;
        else yaw = float(std::remainder(double(yaw) +
            std::remainder(double(targetYaw) - yaw, 360.0) / event.remainingTicks, 360.0));
    }
    if (!positionAt((std::clamp)(localMs, 0.0, double(member.durationMs)), position)) return false;
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Prepare_PreviewEffects()
{
    if (!m_bPreviewPreparationQueued)
    {
        std::set<std::string> targets;
        const auto collect = [&](const auto& pattern) {
            for (const auto& box : pattern.PresentationOccurrences)
                for (const auto& resource : m_PreviewDocument.PresentationResources)
                    if (resource.strResourceId == box.strResourceId && resource.eKind == KIND::EFFECT &&
                        (resource.strResourceKind == "V1_EFFECT" || resource.strResourceKind == "V1_ELEMENT"))
                        targets.insert(resource.strAssetId);
            return Collect_LogicPreviewEffects(m_PreviewDocument, pattern, targets);
        };
        if (!collect(m_PreviewPattern)) { const auto status = m_strStatus; Fail_Preview(status); return false; }
        for (const auto& member : m_BundlePreviewMembers)
            if (!collect(member.pattern)) { const auto status = m_strStatus; Fail_Preview(status); return false; }
        m_PreviewPreparationTargets.assign(targets.begin(), targets.end());
        if (!m_PreviewPreparationTargets.empty())
        {
            std::vector<std::string> admitted;
            std::string status;
            if (!CEffectPresentationService::Queue_ProductTargets_Priority(m_PreviewPreparationTargets, admitted, status))
            { Fail_Preview("Preview Effect preparation could not queue: " + status); return false; }
        }
        m_bPreviewPreparationQueued = true;
    }
    if (m_PreviewPreparationTargets.empty()) return true;
    const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(m_PreviewPreparationTargets);
    // Failed targets are reported by their own occurrence. Preparation time
    // never consumes a short Effect window on the authoring clock.
    if (probe.bCatalogRevisionCurrent && probe.bSettled)
    {
        // Do not leave a held-clock message visible after this gate has opened,
        // or overwrite a later occurrence-specific failure on every sample.
        if (m_strStatus.starts_with("Preparing preview Effects;"))
            m_strStatus = "Preview Effects prepared: " + std::to_string(probe.iPreparedCount) + " ready, " +
                std::to_string(probe.iFailedCount) + " failed, " + std::to_string(probe.iUnavailableCount) + " unavailable.";
        return true;
    }
    if (!probe.strBlockingFailure.empty())
    {
        Fail_Preview("Preview Effect preparation stopped: " + probe.strBlockingFailure);
        return false;
    }
    m_strStatus = "Preparing preview Effects; " + std::to_string(probe.iTargetCount - probe.iPendingCount) +
        "/" + std::to_string(probe.iTargetCount) + " settled (" + std::to_string(probe.iFailedCount) +
        " failed, " + std::to_string(probe.iUnavailableCount) + " unavailable); timeline is held at " +
        std::to_string(Preview_ClockMs()) + " ms.";
    return false;
}

const Client::KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE*
Client::CKoukuSaydonPresentationPlayer::Resolve_PreviewAnimation(
    const BUNDLE_PREVIEW_MEMBER& member, const double localMs,
    const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE*& previous)
{
    const auto eligible = [&](const auto& row) {
        const auto window = member.cloneAnimationWindows.find(row.strOccurrenceId);
        return window == member.cloneAnimationWindows.end() ||
            (localMs >= window->second.first && localMs < window->second.second);
    };
    const auto sampleMs = (std::clamp)(localMs, 0.0, double(member.durationMs));
    const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* selected = nullptr;
    previous = nullptr;
    for (const auto& row : member.animations)
        if (eligible(row) && sampleMs >= row.iPoseStartMs && localMs >= 0.0)
        { previous = selected; selected = &row; }
    if (!selected)
        for (const auto& row : member.animations)
            if (eligible(row)) { selected = &row; break; }
    return selected;
}

void Client::CKoukuSaydonPresentationPlayer::Sync_PreviewSourceVisibility()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    if (level) level->Collect_KoukuPresentationViews(bosses, players);
    for (auto& member : m_BundlePreviewMembers)
    {
        std::shared_ptr<CNpc> replacement;
        // Synthetic split/summon clones do not replace an authoritative boss.
        if (m_bPreviewPlaying && !member.finiteActorLifetime && !member.sourceArchetypeId.empty())
            for (const auto& boss : bosses)
                if (!boss.iOwnerBossNetEntityId && boss.strArchetypeId == member.sourceArchetypeId)
                { replacement = boss.pNpc.lock(); break; }
        const auto previous = member.suppressedSourceActor.lock();
        if (previous == replacement) continue;
        if (previous) previous->Release_CompositionPreviewSuppression();
        member.suppressedSourceActor = replacement;
        if (replacement) replacement->Acquire_CompositionPreviewSuppression();
    }
}

void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreview()
{
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    Sync_PreviewSourceVisibility();
    if (!level || !m_bPreviewPlaying || !Prepare_PreviewEffects()) return;
    std::uint32_t effectiveMs = Preview_ClockMs();
    if (!Resolve_PreviewCaptureClock(effectiveMs, effectiveMs))
    { const auto error = m_strStatus; Fail_Preview(error); return; }
    m_fPreviewClockMs = effectiveMs;
    std::string worldPreviewStatus;
    const auto targets = level->Get_CompositionWorldTargets();
    for (auto& member : m_BundlePreviewMembers)
    {
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        if (member.finiteActorLifetime)
            member.actor->Set_PresentationVisible(localMs >= 0.0 && localMs < member.durationMs);
        const auto model = member.actor->Get_Model();
        (void)model->Set_RootMotionVerticalScale(member.rootMotion ? 0.f : float(member.pattern.fAnimationRootVerticalScale));
        const auto sampleMs = static_cast<float>((std::clamp)(localMs, 0.0, double(member.durationMs)));
        if (!Sample_BundlePreviewFacing(member, localMs))
        { Fail_Preview("Bundle root-motion sample or actor target is unavailable."); return; }
        std::array<double, 3u> bossPosition{};
        double bossYaw = 0.0;
        if (Sample_KoukuSaydonBossMotion(member.pattern, sampleMs, bossPosition, bossYaw))
            (void)member.actor->Apply_NetworkState(
                {float(bossPosition[0]), float(bossPosition[1]), float(bossPosition[2])}, float(bossYaw));
        const KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE* previousAnimation = nullptr;
        const auto* animation = Resolve_PreviewAnimation(member, localMs, previousAnimation);
        std::uint32_t animationIndex = member.initialAnimation;
        float ticks = member.initialTicks;
        if (animation)
        {
            for (std::uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                if (const auto* name = model->Get_AnimationName(i); name && animation->strRuntimeClip == name)
                { animationIndex = i; break; }
            float oldTicks = 0.f, clipTicks = 0.f;
            model->Get_AnimationProgress(animationIndex, oldTicks, clipTicks);
            const float tps = model->Get_AnimationTickPerSecond(animationIndex);
            const float elapsed = (std::max)(0.f, sampleMs - animation->iStartOffsetMs);
            const float age = (std::min)(elapsed, float(animation->iPlayMs));
            double sourceMs = 0.0;
            if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(animation->iSourceStartMs,
                animation->iSourceEndMs, age, animation->fPlayRate, clipTicks * 1000.0 / tps,
                animation->strEndPolicy == "LOOP_TO_WINDOW", sourceMs))
            { Fail_Preview("Animation source range is outside the native clip."); return; }
            ticks = float(sourceMs * tps / 1000.0);
        }
        model->Set_Animation(animationIndex, false, 0.f);
        model->Set_AnimPaused(true);
        model->Set_AnimTrackPosition(animationIndex, ticks);
        model->Update_Animation(0.f);
        CModel::ANIMATION_TRANSITION_POSE logicPose;
        bool logicActive = false;
        std::string blendStatus;
        if (!CKoukuSaydonAnimationBlend::Sample_Pose(*model, member.pattern.AnimationBlendWindows,
            sampleMs, logicPose, logicActive, blendStatus) ||
            (logicActive && !model->Set_AnimationTransitionPose(logicPose)))
        { Fail_Preview("Logic animation blend failed: " + blendStatus); return; }
        if (!logicActive && animation && animation->iBlendInMs && previousAnimation)
        {
            CModel::ANIMATION_TRANSITION_POSE pose;
            pose.targetIndex = animationIndex; pose.targetTicks = ticks;
            pose.durationSeconds = animation->iBlendInMs * .001f;
            pose.elapsedSeconds = (sampleMs - animation->iStartOffsetMs) * .001f;
            pose.playRate = animation->fPlayRate;
            for (uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
                if (previousAnimation->strRuntimeClip == model->Get_AnimationName(i)) { pose.sourceIndex = i; break; }
            float cursor = 0.f, sourceEnd = 0.f;
            if (pose.sourceIndex == UINT32_MAX || !model->Get_AnimationProgress(pose.sourceIndex, cursor, sourceEnd))
                m_strStatus = "Animation blend source clip is unavailable.";
            else
            {
                const float previousTps = model->Get_AnimationTickPerSecond(pose.sourceIndex);
                double previousMs = 0.0;
                if (!CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(previousAnimation->iSourceStartMs,
                    previousAnimation->iSourceEndMs, previousAnimation->iPlayMs, previousAnimation->fPlayRate,
                    sourceEnd * 1000.0 / previousTps, previousAnimation->strEndPolicy == "LOOP_TO_WINDOW", previousMs))
                { Fail_Preview("Animation blend source range is outside the native clip."); return; }
                pose.sourceTicks = float(previousMs * previousTps / 1000.0);
                if (!model->Set_AnimationTransitionPose(pose)) m_strStatus = "Animation blend pose admission failed.";
            }
        }
        for (const auto& box : member.pattern.WorldOccurrences)
        {
            if (m_bBundleWorldExternal && !member.finiteActorLifetime) continue;
            const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                [&](const auto& value) { return value.strWorldId == box.strWorldId; });
            auto& player = member.session.previewWorlds.at(box.strOccurrenceId);
            auto worldTargets = targets;
            worldTargets.bossAnchor = [&member](const std::string& archetype, const std::string& bone,
                CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
            {
                if (archetype != CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(member.pattern.strTargetBossPlacementId) ||
                    !member.actor || !member.actor->Get_Transform())
                { status = "World Object Boss anchor does not match this preview actor: " + archetype; return false; }
                return CWorldSequencePlayer::Resolve_BossBoneAnchor(member.actor->Get_Model(),
                    *member.actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
            };
            worldTargets.objectEmissionAnchor = Make_WorldEmissionAnchor(member.pattern, *world, box);
            const auto span = player->Get_InstanceElapsedSpanMs(world->strSequenceInstanceId,
                box.fPlaybackSpeed, box.iDurationMs);
            if (localMs < box.iStartMs || localMs >= double(box.iStartMs) + span)
            { player->Stop_All(worldTargets, true); continue; }
            if (!player->Is_Playing(world->strSequenceInstanceId) &&
                !player->Play(world->strSequenceInstanceId, worldTargets, box.fPlaybackSpeed,
                    member.worldOffsets.at(box.strOccurrenceId), box.iDurationMs, WorldPlacementFromOccurrence(box)))
            {
                Fail_Preview("Bundle WORLD failed: " + box.strOccurrenceId + ": " + player->Get_Status());
                return;
            }
            if (!player->Seek_InstanceToMs(world->strSequenceInstanceId, float(localMs - box.iStartMs), worldTargets))
            {
                Fail_Preview("Bundle WORLD failed: " + box.strOccurrenceId + ": " + player->Get_Status());
                return;
            }
        }
        member.actor->Synchronize_WeaponPose();
#ifdef _DEBUG
        if (m_bBundleWorldExternal && !member.finiteActorLifetime)
        {
            // External WORLD is admitted only for one zero-offset member. Its
            // freshly sampled BODY, WORLD props and Effects share this clock.
            const auto actor = member.actor;
            const std::string actorArchetype(CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(
                member.pattern.strTargetBossPlacementId));
            const auto bossAnchor = [actor, actorArchetype](const std::string& archetype,
                const std::string& bone, CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
            {
                if (archetype != actorArchetype || !actor || !actor->Get_Transform())
                { status = "World Object Boss anchor does not match this preview actor: " + archetype; return false; }
                return CWorldSequencePlayer::Resolve_BossBoneAnchor(actor->Get_Model(),
                    *actor->Get_Transform()->Get_WorldMatrixPtr(), bone, out, status);
            };
            if (!level->Debug_SampleCompositionWorldPreview(m_PreviewBundleId, true,
                effectiveMs, worldPreviewStatus, bossAnchor))
            { Fail_Preview(worldPreviewStatus); return; }
        }
#endif
        ANIMATION_MODEL_TARGET_VIEW weaponView;
        const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
        if (localMs < 0.0 || localMs >= member.durationMs) Stop_Session(member.session);
        else Sample(member.session, m_PreviewDocument, member.pattern, sampleMs, m_bPreviewPaused,
            *member.actor->Get_Transform()->Get_WorldMatrixPtr(), model, hasWeapon ? &weaponView : nullptr);
    }
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern,
        float(m_fPreviewClockMs), m_bPreviewPaused, m_PreviewPivot, nullptr);
    // A queued/successful Effect must not hide a recoverable WORLD anchor wait.
    if (!worldPreviewStatus.empty()) m_strStatus = std::move(worldPreviewStatus);
    if (m_bPreviewCaptureClockHeld && Preview_ClockMs() == m_iPreviewCaptureBoundaryMs)
        m_bPreviewCaptureBoundarySampled = true;
}

void Client::CKoukuSaydonPresentationPlayer::Set_PreviewPivot(
    const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    m_PreviewPivot = pivot;
    m_PreviewModel = model;
    m_bPreviewPivotReady = true;
}

bool Client::CKoukuSaydonPresentationPlayer::Resolve_PreviewCaptureClock(
    std::uint32_t requestedMs, std::uint32_t& effectiveMs)
{
    effectiveMs = requestedMs;
    if (!m_bPreviewPlaying) return true;
    if (!Prepare_PreviewEffects()) { effectiveMs = Preview_ClockMs(); return m_bPreviewPlaying; }
    const bool wasHeld = m_bPreviewCaptureClockHeld;
    if (wasHeld) requestedMs = m_iPreviewCaptureResumeMs;
    if (!CPresentation_Manager::Get().Are_ScreenPostsEnabled())
    {
        m_bPreviewCaptureClockHeld = false;
        m_bPreviewCaptureBoundarySampled = false;
        m_bPreviewCaptureAllowed = true;
        m_iPreviewCaptureWaitFrames = 0u;
        effectiveMs = requestedMs;
        if (wasHeld) m_PreviewSession.lastClockMs = -1.f;
        return true;
    }
    std::optional<std::uint32_t> firstCaptureMs;
    struct CAPTURE_OWNER { const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern; SESSION* session; std::uint32_t offsetMs; };
    std::vector<CAPTURE_OWNER> owners{{&m_PreviewPattern, &m_PreviewSession, 0u}};
    for (auto& member : m_BundlePreviewMembers)
        owners.push_back({&member.pattern, &member.session,
            static_cast<std::uint32_t>((std::uint64_t(member.offsetTicks) * 1000u + 29u) / 30u)});
    for (const auto& owner : owners)
    for (const auto& box : owner.pattern->PresentationOccurrences)
    {
        const auto boxStartMs = owner.offsetMs + box.iStartMs;
        if (requestedMs < boxStartMs ||
            double(requestedMs) >= double(boxStartMs) + box.iDurationMs) continue;
        const auto resource = std::find_if(m_PreviewDocument.PresentationResources.begin(),
            m_PreviewDocument.PresentationResources.end(), [&](const auto& value) {
                return value.strResourceId == box.strResourceId; });
        if (resource == m_PreviewDocument.PresentationResources.end() || resource->eKind != KIND::EFFECT ||
            (resource->strResourceKind != "V1_EFFECT" && resource->strResourceKind != "V1_ELEMENT")) continue;
        const auto document = CEffectCatalog::Find(resource->strAssetId);
        if (!document) continue; // Existing admission owns missing-document failures.
        for (const auto& element : document->Elements)
        {
            if (!element.bVisible || element.eKind != EFFECT_ELEMENT_KIND::SCREEN_POST ||
                !element.Detail.ScreenPost.bEnabled || element.Detail.ScreenPost.eStatus !=
                    EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE || element.Detail.ScreenPost.eProfile !=
                    EFFECT_SCREEN_POST_PROFILE::SCENE_COLLAPSE_CAPTURE_V1 ||
                (!resource->strElementId.empty() && resource->strElementId != element.strElementId)) continue;
            const double effectRate = Effect_SourceClockRate(*resource, box);
            const double delayMs = double(element.Detail.Timing.fStartDelaySeconds) * 1000.0 / effectRate;
            if (!std::isfinite(delayMs) || delayMs < 0.0 || delayMs >= box.iDurationMs) continue;
            // Float seconds can put an authored integer millisecond a fraction
            // of a microsecond above itself; retain that cursor before rounding up.
            const auto captureMs = static_cast<std::uint32_t>(boxStartMs + std::ceil(delayMs - 0.001));
            const double captureEndMs = double(boxStartMs) + delayMs +
                double(element.Detail.Timing.fLifeTimeSeconds) * 1000.0 / effectRate;
            if (captureMs > requestedMs || double(requestedMs) >= captureEndMs) continue;
            const auto row = owner.session->rows.find(box.strOccurrenceId);
            if (row != owner.session->rows.end())
            {
                if (row->second.failed)
                {
                    m_strStatus = row->second.failureStatus;
                    return false;
                }
                const HRESULT captureResult = CEffectPresentationService::Get_ScreenPostCaptureResult(
                    {row->second.v1EffectHandle}, element.strElementId);
                if (FAILED(captureResult))
                {
                    m_strStatus = "Scene capture failed: " + box.strOccurrenceId + " / " +
                        element.strElementId + "; HRESULT=" + std::to_string(captureResult);
                    return false;
                }
                if (CEffectPresentationService::Has_CapturedScreenPost(
                    {row->second.v1EffectHandle}, element.strElementId)) continue;
            }
            if (!firstCaptureMs || captureMs < *firstCaptureMs) firstCaptureMs = captureMs;
        }
    }
    m_bPreviewCaptureClockHeld = firstCaptureMs.has_value();
    if (firstCaptureMs)
    {
        if (!wasHeld) m_iPreviewCaptureResumeMs = requestedMs;
        const bool sameBoundary = wasHeld && m_iPreviewCaptureBoundaryMs == *firstCaptureMs;
        m_bPreviewCaptureAllowed = sameBoundary && m_bPreviewCaptureBoundarySampled;
        if (!sameBoundary)
        {
            m_bPreviewCaptureBoundarySampled = false;
            m_iPreviewCaptureWaitFrames = 0u;
        }
        m_iPreviewCaptureBoundaryMs = *firstCaptureMs;
        effectiveMs = *firstCaptureMs;
        if (++m_iPreviewCaptureWaitFrames > 120u)
        {
            m_strStatus = "Scene capture did not receive a renderable frame at " +
                std::to_string(effectiveMs) + " ms within 120 preview updates. "
                "Check the active WORLD and Screen Presentation Post, then restart Preview.";
            return false;
        }
    }
    else
    {
        effectiveMs = requestedMs;
        m_bPreviewCaptureBoundarySampled = false;
        m_bPreviewCaptureAllowed = true;
        m_iPreviewCaptureWaitFrames = 0u;
    }
    if (wasHeld || m_bPreviewCaptureClockHeld)
    {
        // A render boundary is one continuing occurrence, not a new playback.
        // Keep its capture and observed anchors across the deferred cursor jump.
        m_PreviewSession.lastClockMs = -1.f;
        for (auto& member : m_BundlePreviewMembers) member.session.lastClockMs = -1.f;
        m_strCompletedPreviewPatternId.clear();
    }
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Sample_Preview(std::uint32_t clockMs,
    bool playing, bool paused, const float4x4_t& pivot, const std::shared_ptr<Engine::CModel>& model)
{
    if (!playing) { Stop_Preview(); return; }
    if (!m_bPreviewPlaying) return;
    Set_PreviewPivot(pivot, model);
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    m_bPreviewPaused = paused;
    ANIMATION_MODEL_TARGET_VIEW weaponView;
    const bool hasWeapon = CAnimationTargetService::Resolve_Model() == model &&
        CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
    Sample(m_PreviewSession, m_PreviewDocument, m_PreviewPattern, float(m_fPreviewClockMs),
        paused, pivot, model, hasWeapon ? &weaponView : nullptr);
    // Resolve_PreviewCaptureClock checks the actual active capture occurrence.
    // Unrelated failed rows remain isolated, just as they do during ordinary
    // playback; a later scene capture must not turn them into a sequence stop.
    Refresh_SharedPresentation();
    // The next Engine Late_Update can now cull and submit this WORLD/camera.
    // The first boundary render is pass-through; only the next may latch it.
    if (m_bPreviewCaptureClockHeld && clockMs == m_iPreviewCaptureBoundaryMs)
        m_bPreviewCaptureBoundarySampled = true;
}

void Client::CKoukuSaydonPresentationPlayer::Pause_Preview(bool paused)
{
    m_strCompletedPreviewPatternId.clear();
    m_bPreviewPaused = paused;
    if (!paused && m_bPreviewPlaying && m_iPreviewDurationMs && m_fPreviewClockMs >= m_iPreviewDurationMs)
        Seek_Preview(0u);
    for (const auto& member : m_BundlePreviewMembers)
        for (const auto& [id, row] : member.session.rows)
        {
            if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
            if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
        }
    for (const auto& [id, row] : m_PreviewSession.rows)
    {
        if (row.effectHandle) CEffectV2Runtime::Set_GroupPaused(row.effectHandle, paused);
        if (row.soundHandle) CGameInstance::Get().Pause_SoundCue(row.soundHandle, paused);
    }
}

void Client::CKoukuSaydonPresentationPlayer::Seek_Preview(std::uint32_t clockMs)
{
    m_bPreviewCaptureClockHeld = false;
    m_bPreviewCaptureBoundarySampled = false;
    m_bPreviewCaptureAllowed = true;
    m_iPreviewCaptureWaitFrames = 0u;
    for (const auto& [id, row] : m_PreviewSession.rows)
        CEffectPresentationService::Set_ScreenPostCaptureAllowed({row.v1EffectHandle}, true);
    m_strCompletedPreviewPatternId.clear();
    m_fPreviewClockMs = (std::min)(clockMs, m_iPreviewDurationMs);
    if (m_fPreviewClockMs >= m_iPreviewDurationMs) Pause_Preview(true);
    // Stop/paused scrubbing must retain the observed birth history and a
    // frozen Effect's original anchor. The V2 external clock handles rewind.
    const auto prepare = [&](SESSION& session) {
        session.lastClockMs = -1.f;
        for (auto row = session.rows.begin(); row != session.rows.end();)
        {
            if (row->second.kind == KIND::EFFECT && !row->second.failed)
            { ++row; continue; }
            if (row->second.effectHandle) CEffectV2Runtime::Stop_Group(row->second.effectHandle);
        if (row->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({row->second.v1EffectHandle});
            if (row->second.soundHandle) CGameInstance::Get().Stop_SoundCue(row->second.soundHandle);
            row = session.rows.erase(row);
        }
    };
    prepare(m_PreviewSession);
    for (auto& member : m_BundlePreviewMembers) prepare(member.session);
    if (Preview_IsBundle()) Sample_BundlePreview();
    // MainApp samples single-pattern WORLD at the new clock before recreating these cue handles.
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Fail_Preview(std::string status)
{
    const std::string patternId = Preview_IsBundle() ? m_PreviewBundleId : m_PreviewPattern.strPatternId;
    Stop_Preview();
    m_strFailedPreviewPatternId = patternId;
    m_strFailedPreviewStatus = std::move(status);
    m_strStatus = m_strFailedPreviewStatus;
}

bool Client::CKoukuSaydonPresentationPlayer::Consume_FailedPreview(std::string& patternId, std::string& status)
{
    if (m_strFailedPreviewPatternId.empty()) return false;
    patternId = std::move(m_strFailedPreviewPatternId);
    status = std::move(m_strFailedPreviewStatus);
    m_strFailedPreviewPatternId.clear();
    m_strFailedPreviewStatus.clear();
    return true;
}

bool Client::CKoukuSaydonPresentationPlayer::Consume_CompletedPreview(std::string& patternId)
{
    if (m_strCompletedPreviewPatternId.empty()) return false;
    patternId = std::move(m_strCompletedPreviewPatternId);
    Stop_Preview();
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Stop_Preview()
{
    Clear_LogicPreview();
#ifdef _DEBUG
    if (m_bBundleWorldExternal)
        if (auto* level = CLevel_KakulSaydonArena::Get_Active()) level->Debug_StopCompositionWorldPreview();
#endif
    m_bBundleWorldExternal = false;
    m_strCompletedPreviewPatternId.clear();
    m_strFailedPreviewPatternId.clear();
    m_strFailedPreviewStatus.clear();
    ++m_iPreviewGeneration;
    m_bModelReferencePreview = false;
    Release_BundlePreviewMembers(m_BundlePreviewMembers);
    m_PreviewBundleId.clear();
    Stop_Session(m_PreviewSession);
    m_bPreviewPlaying = false;
    m_bOwnPreviewClock = false;
    m_bPreviewClockAwaitingFirstUpdate = false;
    m_bPreviewPreparationQueued = false;
    m_PreviewPreparationTargets.clear();
    m_bPreviewCaptureClockHeld = false;
    m_bPreviewCaptureBoundarySampled = false;
    m_bPreviewCaptureAllowed = true;
    m_iPreviewCaptureResumeMs = m_iPreviewCaptureBoundaryMs = m_iPreviewCaptureWaitFrames = 0u;
    m_bPreviewPaused = false;
    m_bPreviewPivotReady = false;
    m_bColliderResourcePreview = false;
    m_PreviewModel.reset();
    Refresh_SharedPresentation();
}

void Client::CKoukuSaydonPresentationPlayer::Sync_MazeMark(
    CARD& mark, const std::string& asset, const float4x4_t& pivot)
{
    if (!IsCardMazeMarkGroup(asset))
    {
        // Bingo and other floor marks keep the original single-attempt path.
        if (mark.assetId != asset)
        {
            if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
            mark = {}; mark.assetId = asset;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
            if (Ensure_EffectResource("GROUP", asset, effects))
                if (const auto* group = effects->Find_Group(asset))
                {
                    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                    playback.PivotWorld = pivot;
                    playback.fDurationSeconds = 0.f;
                    playback.bProductOwned = true;
                    mark.handle = CEffectV2Runtime::Play_Group(*group, effects, playback, m_Device, m_Context);
                }
            if (!mark.handle) m_strStatus = "Maze floor mark unavailable: " + asset;
        }
        if (mark.handle)
        {
            CEffectV2Runtime::Set_GroupPivot(mark.handle, pivot);
            std::string failure;
            if (CEffectV2Runtime::Consume_GroupFailure(mark.handle, failure))
            {
                CEffectV2Runtime::Stop_Group(mark.handle); mark.handle = 0u;
                m_strStatus = "Maze floor mark failed: " + asset + ": " + failure;
            }
        }
        return;
    }
    if (mark.assetId != asset)
    {
        if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
        mark = {};
        mark.assetId = asset;
    }
    const auto generation = CEffectV2Runtime::Cache_Generation();
    const auto nowMs = static_cast<std::uint64_t>(std::chrono::duration_cast<
        std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    mark.mazeRetry.ObserveGeneration(generation);
    if (mark.handle)
    {
        CEffectV2Runtime::Set_GroupPivot(mark.handle, pivot);
        std::string failure;
        if (!CEffectV2Runtime::Consume_GroupFailure(mark.handle, failure)) return;
        CEffectV2Runtime::Stop_Group(mark.handle);
        mark.handle = 0u;
        mark.mazeRetry.Defer(nowMs);
        m_strStatus = "Maze floor mark failed: " + asset + ": " + failure;
        return;
    }
    if (!mark.mazeRetry.TryBegin(nowMs)) return;
    const std::string resourceKey = "GROUP:" + asset;
    if (mark.mazeRetry.attempts > 1u)
    {
        // Only this resource is read again, at most twice per mark occurrence/generation.
        m_EffectResourceFailures.erase(resourceKey);
        m_EffectResources.erase(resourceKey);
    }
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
    if (!Ensure_EffectResource("GROUP", asset, effects))
    {
        const std::string reason = m_strStatus;
        m_strStatus = "Maze floor mark unavailable: " + asset + ": " + reason;
        return;
    }
    const auto* group = effects ? effects->Find_Group(asset) : nullptr;
    if (!group)
    {
        m_strStatus = "Maze floor mark group missing from snapshot: " + asset;
        return;
    }
    EFFECT_V2_GROUP_PLAYBACK_DESC playback;
    playback.PivotWorld = pivot;
    playback.fDurationSeconds = 0.f;
    playback.bProductOwned = true;
    mark.handle = CEffectV2Runtime::Play_Group(*group, effects, playback, m_Device, m_Context);
    // A handle may fail on the next frame; do not reset the attempt budget here.
    if (!mark.handle)
        m_strStatus = "Maze floor mark spawn failed: " + asset + ": " + CEffectV2Runtime::Last_Error();
}

void Client::CKoukuSaydonPresentationPlayer::Update_MazeMarks(
    const std::vector<KOUKU_CARD_PRESENTATION_VIEW>& players)
{
    using namespace LostArk::Shared;
    const auto suitName = [](MECHANIC_CARD_SYMBOL suit) -> const char*
    {
        switch (suit)
        {
        case MECHANIC_CARD_SYMBOL::HEART: return "heart";
        case MECHANIC_CARD_SYMBOL::SPADE: return "spade";
        case MECHANIC_CARD_SYMBOL::CLUB: return "club";
        case MECHANIC_CARD_SYMBOL::DIAMOND: return "diamond";
        default: return nullptr;
        }
    };
    const auto groundPivot = [](const float4x4_t& world)
    {
        // Translation only: character scale/yaw must not resize or rotate the symbol.
        float4x4_t pivot;
        XMStoreFloat4x4(&pivot, XMMatrixTranslation(world._41, world._42 + .02f, world._43));
        return pivot;
    };
    std::set<std::uint32_t> livePlayers, liveTargets, liveExits;
    bool mazeActive = false;
    for (const auto& view : players)
    {
        const auto& s = view.Snapshot;
        if (!s.iCurrentHp || s.eCardMazeRole == CARD_MAZE_ROLE::NONE || (s.CardMaze.flags & 8u)) continue;
        mazeActive = true;
        // The telescope owner never carries a suit marker, even with an old Debug snapshot.
        if (s.eCardMazeRole != CARD_MAZE_ROLE::HUNTER) continue;
        const char* suit = suitName(s.eCardMazeSuit);
        if (!suit) continue;
        if (!(s.CardMaze.flags & 2u) && !s.CardMaze.transferStartTick)
            if (auto character = view.pCharacter.lock(); character && character->Get_Transform())
            {
                livePlayers.insert(s.iNetEntityId);
                Sync_MazeMark(m_MazePlayerMarks[s.iNetEntityId], std::string("cardmaze.mark.") + suit,
                    groundPivot(*character->Get_Transform()->Get_WorldMatrixPtr()));
            }
        if ((s.CardMaze.flags & 4u) && !s.CardMaze.transferStartTick)
        {
            liveExits.insert(s.iNetEntityId);
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslation(s.CardMaze.exitX, s.CardMaze.exitY + .02f, s.CardMaze.exitZ));
            Sync_MazeMark(m_MazeExits[s.iNetEntityId], std::string("cardmaze.exit.") + suit, pivot);
        }
    }
    if (const auto* arena = CLevel_KakulSaydonArena::Get_Active(); mazeActive && arena)
    {
        std::vector<KOUKU_MAZE_TARGET_VIEW> targets;
        arena->Collect_KoukuMazeTargets(targets);
        for (const auto& target : targets)
        {
            const char* suit = nullptr;
            if (target.archetypeId == "MONSTER_KOUKU_CARD_HEART") suit = "heart";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_SPADE") suit = "spade";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_CLUB") suit = "club";
            else if (target.archetypeId == "MONSTER_KOUKU_CARD_DIAMOND") suit = "diamond";
            const auto npc = target.npc.lock();
            if (!suit || !npc || !npc->Get_Transform()) continue;
            liveTargets.insert(target.entityId);
            Sync_MazeMark(m_MazeTargetMarks[target.entityId], std::string("cardmaze.mark.") + suit,
                groundPivot(*npc->Get_Transform()->Get_WorldMatrixPtr()));
        }
    }
    const auto removeStale = [](auto& marks, const auto& live)
    {
        for (auto i = marks.begin(); i != marks.end();)
        {
            if (live.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = marks.erase(i);
        }
    };
    /* The bingo board. Both masks are room state the Server owns, so this
       only chooses which of the two authored decals sits on each painted
       cell and drops the ones the Server has cleared. */
    {
        const auto& board = CCombatHUDViewModel::Get().Get_BingoBoard();
        std::set<std::int32_t> liveBingo;
        for (std::int32_t cell = 0; cell < LostArk::Shared::KOUKU_BINGO_CELL_COUNT; ++cell)
        {
            const std::uint32_t bit = 1u << cell;
            if (0u == (board.iWhiteMask & bit)) continue;
            liveBingo.insert(cell);
            float4x4_t pivot;
            XMStoreFloat4x4(&pivot, XMMatrixTranslation(
                LostArk::Shared::Kouku_BingoCellCenterX(cell), .02f,
                LostArk::Shared::Kouku_BingoCellCenterZ(cell)));
            Sync_MazeMark(m_BingoMarks[cell],
                (0u != (board.iRedMask & bit)) ? "bingo.skull.red" : "bingo.skull.white",
                pivot);
        }
        for (auto i = m_BingoMarks.begin(); i != m_BingoMarks.end();)
        {
            if (liveBingo.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = m_BingoMarks.erase(i);
        }
    }
    /* The bingo bomb's mark. It rides one named carrier and the Server can
       cancel it mid-flight, so it stays a followed state rather than a
       timeline. Height and size live in the authored document, so the pivot
       here is only the ground point under its carrier. The planted half is a
       World Sequence the Server names the moment it plants. */
    {
        const auto& bombBoard = CCombatHUDViewModel::Get().Get_BingoBoard();
        std::set<std::int32_t> liveBombs;
        for (std::uint8_t index = 0u; index < bombBoard.iBombCount; ++index)
        {
            const auto& bomb = bombBoard.Bombs[index];
            const char* bombAsset = nullptr;
            float4x4_t bombPivot;
            if (LostArk::Shared::BINGO_BOMB_PHASE::MARKED == bomb.ePhase)
            {
                std::shared_ptr<CCharacter> carrier;
                for (const auto& view : players)
                    if (view.Snapshot.iNetEntityId == bomb.iCarrierNetEntityId)
                    { carrier = view.pCharacter.lock(); break; }
                if (!carrier || !carrier->Get_Transform()) continue;
                const float4x4_t& carrierWorld =
                    *carrier->Get_Transform()->Get_WorldMatrixPtr();
                XMStoreFloat4x4(&bombPivot, XMMatrixTranslation(
                    carrierWorld._41, carrierWorld._42, carrierWorld._43));
                bombAsset = "bingo.bomb.mark";
            }
            /* PLANTED falls through: dropping the slot out of liveBombs is
               what takes the mark off the carrier, and the sequence has already
               started where it stood. */
            else continue;
            const std::int32_t slot = static_cast<std::int32_t>(index);
            liveBombs.insert(slot);
            Sync_MazeMark(m_BingoBombs[slot], bombAsset, bombPivot);
        }
        for (auto i = m_BingoBombs.begin(); i != m_BingoBombs.end();)
        {
            if (liveBombs.contains(i->first)) { ++i; continue; }
            if (i->second.handle) CEffectV2Runtime::Stop_Group(i->second.handle);
            i = m_BingoBombs.erase(i);
        }
    }
    /* The bingo hammer is authored now: four World Sequence templates, one
       per sweep direction, and one instance per anchor. The Server names the
       instance when it rolls the anchor, so there is nothing to pose here. */
    removeStale(m_MazePlayerMarks, livePlayers);
    removeStale(m_MazeTargetMarks, liveTargets);
    removeStale(m_MazeExits, liveExits);
}

void Client::CKoukuSaydonPresentationPlayer::Reset()
{
    m_LightPlayerPivots.clear();
    m_LightBossFollowers.clear();
    m_iProductReloadRunEpoch = 0u;
    for (auto& [id, session] : m_TargetedCombatSessions)
    { Stop_Session(session.playback); Stop_Session(session.sourceBossPlayback); }
    m_TargetedCombatSessions.clear();
    m_TargetedCombatVisuals.clear();
    Stop_Session(m_FearSession);
    m_FearSession.key.clear();
    m_strCompletedFearKey.clear();
    m_QueuedV1Effects.clear();
    for (auto& [id, session] : m_BossSessions) Stop_Session(session);
    for (auto& [id, session] : m_ChildBossSessions) Stop_Session(session);
    m_BossSessions.clear();
    m_ChildBossSessions.clear();
    for (auto& [id, session] : m_MarioEntrySessions) Stop_Session(session);
    m_MarioEntrySessions.clear();
    Stop_Session(m_ProductBundleSession);
    for (const auto& [id, card] : m_Cards)
        if (card.handle) CEffectV2Runtime::Stop_Group(card.handle);
    m_Cards.clear();
    for (const auto& [cell, mark] : m_BingoMarks)
        if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
    m_BingoMarks.clear();
    for (const auto& [slot, bomb] : m_BingoBombs)
        if (bomb.handle) CEffectV2Runtime::Stop_Group(bomb.handle);
    m_BingoBombs.clear();
    for (const auto& [id, exit] : m_MazeExits)
        if (exit.handle) CEffectV2Runtime::Stop_Group(exit.handle);
    m_MazeExits.clear();
    for (auto* marks : { &m_MazePlayerMarks, &m_MazeTargetMarks })
    {
        for (const auto& [id, mark] : *marks)
            if (mark.handle) CEffectV2Runtime::Stop_Group(mark.handle);
        marks->clear();
    }
    m_ColliderDebugOverrides.clear();
    Stop_Preview();
    Restore_Scene();
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_HasActiveWorldBox(const std::string_view occurrenceId) const
{
#ifdef _DEBUG
    if (occurrenceId.empty() || !m_bPreviewPlaying || m_bModelReferencePreview) return false;
    const auto contains = [&](const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, const SESSION& session,
        const double clockMs, const bool bundle)
    {
        const auto box = std::find_if(pattern.WorldOccurrences.begin(), pattern.WorldOccurrences.end(),
            [&](const auto& value) {
                return value.strOccurrenceId == occurrenceId && clockMs >= value.iStartMs &&
                    clockMs < double(value.iStartMs) + value.iDurationMs;
            });
        if (box == pattern.WorldOccurrences.end()) return false;
        if (!bundle)
        {
            const auto* level = CLevel_KakulSaydonArena::Get_Active();
            return level && level->Debug_HasVisibleCompositionWorldBox(occurrenceId);
        }
        const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
            [&](const auto& value) { return value.strWorldId == box->strWorldId; });
        const auto player = session.previewWorlds.find(box->strOccurrenceId);
        float4x4_t pivot;
        return world != m_PreviewDocument.Worlds.end() && player != session.previewWorlds.end() &&
            player->second->Try_GetObjectPivot(world->strSequenceInstanceId, pivot);
    };
    if (contains(m_PreviewPattern, m_PreviewSession, m_fPreviewClockMs, false)) return true;
    for (const auto& member : m_BundlePreviewMembers)
        if (contains(member.pattern, member.session,
            m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0, true)) return true;
#endif
    return false;
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_WorldPlacementAuthoring(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document)
{
#ifdef _DEBUG
    auto* level = CLevel_KakulSaydonArena::Get_Active();
    if (!level || !m_bPreviewPlaying || m_bModelReferencePreview) return;
    const auto targets = level->Get_CompositionWorldTargets();
    const auto refresh = [&](KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, SESSION& session, const bool bundle)
    {
        const auto source = std::find_if(document.Patterns.begin(), document.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == pattern.strPatternId; });
        const bool placementPreview = pattern.strPatternId == "preview.kouku.resource";
        if (!placementPreview && (source == document.Patterns.end() || !source->strLoadError.empty())) return false;
        bool changed = false;
        for (auto& box : pattern.WorldOccurrences)
        {
            // Synthetic placement previews still own the authored stable occurrence IDs.
            const auto owner = placementPreview ? std::find_if(document.Patterns.begin(), document.Patterns.end(),
                [&](const auto& value) {
                    return value.strLoadError.empty() && std::any_of(value.WorldOccurrences.begin(), value.WorldOccurrences.end(),
                        [&](const auto& row) { return row.strOccurrenceId == box.strOccurrenceId; });
                }) : source;
            if (owner == document.Patterns.end()) continue;
            const auto edited = std::find_if(owner->WorldOccurrences.begin(), owner->WorldOccurrences.end(),
                [&](const auto& value) { return value.strOccurrenceId == box.strOccurrenceId && value.strWorldId == box.strWorldId; });
            if (edited == owner->WorldOccurrences.end() || box.Placement == edited->Placement) continue;
            const auto placement = WorldPlacementFromOccurrence(*edited);
            std::string status;
            bool applied = false;
            if (bundle)
            {
                const auto world = std::find_if(m_PreviewDocument.Worlds.begin(), m_PreviewDocument.Worlds.end(),
                    [&](const auto& value) { return value.strWorldId == box.strWorldId; });
                const auto player = session.previewWorlds.find(box.strOccurrenceId);
                if (world != m_PreviewDocument.Worlds.end() && player != session.previewWorlds.end())
                {
                    applied = player->second->Validate_ObjectPlacement(world->strSequenceInstanceId, placement, status);
                    if (applied && player->second->Is_Playing(world->strSequenceInstanceId))
                    {
                        applied = player->second->Set_ObjectPlacement(world->strSequenceInstanceId, placement, targets);
                        if (!applied) status = player->second->Get_Status();
                    }
                }
            }
            else applied = level->Debug_SetCompositionWorldPlacement(box.strOccurrenceId, placement, status);
            if (!applied)
            {
                m_strStatus = "WORLD placement preview kept its previous pose: " + box.strOccurrenceId + "; " + status;
                continue;
            }
            box.Placement = edited->Placement;
            changed = true;
        }
        return changed;
    };
    (void)refresh(m_PreviewPattern, m_PreviewSession, false);
    bool bundleChanged = false;
    for (auto& member : m_BundlePreviewMembers)
        bundleChanged |= refresh(member.pattern, member.session, true);
    if (bundleChanged) Sample_BundlePreview();
#endif
}

void Client::CKoukuSaydonPresentationPlayer::Refresh_ColliderAuthoring(
    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& document, std::uint64_t generation)
{
    if (m_iColliderAuthoringGeneration == generation) return;
    m_iColliderAuthoringGeneration = generation;
    m_ColliderDebugOverrides.clear();
    for (const auto& pattern : document.Patterns)
        for (const auto& box : pattern.PresentationOccurrences)
            m_ColliderDebugOverrides.emplace(box.strOccurrenceId, box.bDebugRender);
    if (!m_bPreviewPlaying) return;
    Refresh_WorldPlacementAuthoring(document);
    bool changed = false;
    for (auto& box : m_PreviewPattern.PresentationOccurrences)
    {
        const auto resource = std::find_if(document.PresentationResources.begin(), document.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId && value.eKind == KIND::COLLIDER; });
        if (resource == document.PresentationResources.end()) continue;
        const auto old = std::find_if(m_PreviewDocument.PresentationResources.begin(), m_PreviewDocument.PresentationResources.end(),
            [&box](const auto& value) { return value.strResourceId == box.strResourceId; });
        if (old != m_PreviewDocument.PresentationResources.end() && *old != *resource)
        { *old = *resource; changed = true; }
        for (const auto& pattern : document.Patterns)
        {
            const auto source = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
                [&box](const auto& value) { return value.strOccurrenceId == box.strOccurrenceId; });
            if (source == pattern.PresentationOccurrences.end()) continue;
            auto staged = *source;
            if (m_bColliderResourcePreview) staged.iStartMs = 0u;
            if (box != staged) { box = std::move(staged); changed = true; }
            break;
        }
    }
    if (changed)
    {
        m_PreviewDocument.Worlds = document.Worlds;
        if (m_bColliderResourcePreview && !m_PreviewPattern.Stages.empty())
            m_PreviewPattern.Stages.front().iDurationMs = m_PreviewPattern.PresentationOccurrences.front().iDurationMs;
        m_iPreviewDurationMs = Pattern_Duration(m_PreviewPattern);
        m_fPreviewClockMs = (std::min)(m_fPreviewClockMs, double(m_iPreviewDurationMs - 1u));
        Stop_Session(m_PreviewSession);
    }
}

bool Client::CKoukuSaydonPresentationPlayer::Preview_PresentationGeometry(
    const std::string& patternId, const OCCURRENCE& occurrence)
{
    if (!m_bPreviewPlaying || m_bModelReferencePreview) return false;
    if (m_bColliderResourcePreview)
    {
        const auto owner = std::find_if(m_PreviewDocument.Patterns.begin(), m_PreviewDocument.Patterns.end(),
            [&](const auto& value) { return value.strPatternId == patternId; });
        if (owner == m_PreviewDocument.Patterns.end() ||
            std::none_of(owner->PresentationOccurrences.begin(), owner->PresentationOccurrences.end(),
                [&](const auto& value) { return value.strOccurrenceId == occurrence.strOccurrenceId &&
                    value.strResourceId == occurrence.strResourceId; })) return false;
    }
    const auto finite = [](const auto& values, const double minimum, const double maximum) {
        return std::all_of(values.begin(), values.end(), [=](const double value) {
            return std::isfinite(value) && value >= minimum && value <= maximum;
        });
    };
    if (!finite(occurrence.PositionOffset, -100000.0, 100000.0) ||
        !finite(occurrence.RotationDegrees, -36000.0, 36000.0) ||
        !finite(occurrence.Scale, 0.001, 10000.0)) return false;
    const auto update = [&](KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, SESSION& session) {
        if (pattern.strPatternId != patternId && !m_bColliderResourcePreview) return false;
        const auto box = std::find_if(pattern.PresentationOccurrences.begin(), pattern.PresentationOccurrences.end(),
            [&](const auto& value) { return value.strOccurrenceId == occurrence.strOccurrenceId &&
                value.strResourceId == occurrence.strResourceId; });
        if (box == pattern.PresentationOccurrences.end()) return false;
        const auto resource = std::find_if(m_PreviewDocument.PresentationResources.begin(), m_PreviewDocument.PresentationResources.end(),
            [&](const auto& value) { return value.strResourceId == box->strResourceId &&
                (value.eKind == KIND::COLLIDER || value.eKind == KIND::EFFECT); });
        if (resource == m_PreviewDocument.PresentationResources.end()) return false;
        auto edited = *box;
        edited.PositionOffset = occurrence.PositionOffset;
        edited.RotationDegrees = occurrence.RotationDegrees;
        edited.Scale = occurrence.Scale;
        bool anchorChanged = Is_CenteredWorldCircle(*resource, edited) != Is_CenteredWorldCircle(*resource, *box);
        if (resource->eKind == KIND::EFFECT)
        {
            edited.strAnchorKind = occurrence.strAnchorKind;
            edited.bFollowBoss = occurrence.bFollowBoss;
            edited.strBone = occurrence.strBone;
            edited.strBoneTarget = occurrence.strBoneTarget;
            edited.strWorldId = occurrence.strWorldId;
            edited.strWorldOccurrenceId = occurrence.strWorldOccurrenceId;
            edited.iWorldEmissionIndex = occurrence.iWorldEmissionIndex;
            if (!Validate_EffectAnchor(m_PreviewDocument, pattern, edited, m_strStatus)) return false;
            anchorChanged = edited.strAnchorKind != box->strAnchorKind || edited.bFollowBoss != box->bFollowBoss ||
                edited.strBone != box->strBone || edited.strBoneTarget != box->strBoneTarget ||
                edited.strWorldId != box->strWorldId || edited.strWorldOccurrenceId != box->strWorldOccurrenceId ||
                edited.iWorldEmissionIndex != box->iWorldEmissionIndex;
        }
        const auto active = session.rows.find(box->strOccurrenceId);
        if (anchorChanged)
        {
            // A new anchor owns a new occurrence playback. Other rows and the
            // session's observed boss history retain their current clock/state.
            if (active != session.rows.end())
            {
                if (active->second.effectHandle) CEffectV2Runtime::Stop_Group(active->second.effectHandle);
                if (active->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({active->second.v1EffectHandle});
                session.rows.erase(active);
            }
            session.effectAnchorHistories.erase(box->strOccurrenceId);
            *box = std::move(edited);
            return true;
        }
        if (active != session.rows.end() && active->second.hasPlacementAnchor &&
            !active->second.failed && !active->second.waitingForAnchor)
        {
            // Frozen rows retain their first anchor; following rows retain the
            // current sampled anchor and all prior particle birth transforms.
            auto placed = edited;
            placed.strBone.clear();
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                placed.PositionOffset[axis] *= active->second.placementAnchorScale[axis];
                placed.Scale[axis] *= active->second.placementAnchorScale[axis];
            }
            float4x4_t pivot{};
            if (!Make_Pivot(placed, active->second.placementAnchor, nullptr, pivot)) return false;
            if (resource->eKind == KIND::EFFECT && active->second.effectHandle &&
                !CEffectV2Runtime::Rebuild_GroupPlacement(active->second.effectHandle, pivot,
                    Effect_PivotSampler(edited, session.rootHistory, active->second.effectPivotHistory),
                    m_Device, m_Context))
            {
                m_strStatus = "Effect geometry preview failed: " + CEffectV2Runtime::Last_Error();
                return false;
            }
            if (resource->eKind == KIND::EFFECT && active->second.v1EffectHandle &&
                (!CEffectPresentationService::Update_WorldRoot({active->second.v1EffectHandle}, pivot) ||
                 !CEffectPresentationService::Seek_WorldRoot({active->second.v1EffectHandle},
                    (std::max)(0.f, active->second.lastAge), Effect_V1TransformProvider(edited, pivot,
                        session.rootHistory, active->second.effectPivotHistory, active->second.sourceAnchorSampler), true,
                    static_cast<float>(edited.iDurationMs) / 1000.f)))
            {
                m_strStatus = "V1 Effect geometry preview lost its active handle: " +
                    CEffectPresentationService::Get_Status();
                return false;
            }
            active->second.pivot = pivot;
            if (resource->eKind == KIND::COLLIDER)
                active->second.wire = Collider_Wire(*resource, placed);
        }
        else if (active != session.rows.end())
        {
            if (active->second.effectHandle) CEffectV2Runtime::Stop_Group(active->second.effectHandle);
            if (active->second.v1EffectHandle) CEffectPresentationService::Stop_WorldRoot({active->second.v1EffectHandle});
            session.rows.erase(active);
        }
        box->PositionOffset = occurrence.PositionOffset;
        box->RotationDegrees = occurrence.RotationDegrees;
        box->Scale = occurrence.Scale;
        return true;
    };
    for (auto& member : m_BundlePreviewMembers)
    {
        if (!update(member.pattern, member.session)) continue;
        const double localMs = m_fPreviewClockMs - double(member.offsetTicks) * 1000.0 / 30.0;
        if (member.actor && localMs >= 0.0 && localMs < member.durationMs)
        {
            ANIMATION_MODEL_TARGET_VIEW weaponView;
            const bool hasWeapon = member.actor->Try_GetAnimationModelTarget(ANIMATION_BONE_TARGET::WEAPON, weaponView);
            Sample(member.session, m_PreviewDocument, member.pattern, float(localMs), m_bPreviewPaused,
                *member.actor->Get_Transform()->Get_WorldMatrixPtr(), member.actor->Get_Model(),
                hasWeapon ? &weaponView : nullptr);
        }
        return true;
    }
    if (!update(m_PreviewPattern, m_PreviewSession)) return false;
    // MainApp samples the single-pattern owner later in this same frame with its
    // actual animation target and weapon view. Its displayed clock is unchanged.
    return true;
}

void Client::CKoukuSaydonPresentationPlayer::Render_Debug() const
{
#ifdef _DEBUG
    if (m_LightProvider)
        for (const auto& light : m_LightProvider->lights)
            if (light.debugRender) Draw_LightWire(light.desc);
    const auto draw = [this](const SESSION& session, bool preview)
    {
        for (const auto& [id, row] : session.rows)
            if (!row.failed && !row.waitingForAnchor && row.kind == KIND::COLLIDER)
            {
                const auto override = m_ColliderDebugOverrides.find(id);
                const bool visible = !preview && override != m_ColliderDebugOverrides.end() ? override->second : row.debugRender;
                if (visible) CHitAreaWire::Draw(row.pivot, row.wire, 0xff40dfff);
            }
    };
    for (const auto& [id, session] : m_BossSessions) draw(session, false);
    for (const auto& [id, session] : m_ChildBossSessions) draw(session, false);
    for (const auto& [id, session] : m_MarioEntrySessions) draw(session, false);
    if (m_bPreviewPlaying) draw(m_PreviewSession, true);
    for (const auto& member : m_BundlePreviewMembers) draw(member.session, true);
#endif
}
```

### Tools/KoukuSaydonPipeline/retarget_showtime_camera_scale.py

```python
"""Rescale the four saved Showtime shots around the source actor, not world zero.

Only camera eye/lookAt and the PATTERN_ONLY selection box center change.
The source camera remains in an exclusive backup. No runtime file is written;
Publish-MapAuthoring -Scope CameraShots performs validation and publication.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import tempfile

SHOTS = tuple(f"kouku.gate3.showtime.camera.{i}" for i in range(1, 5))
REL = Path("Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json")
# SCENE02B matinee_15, group 68, move track 119; UE centimetres -> client metres.
SOURCE_ACTOR = (4.459576606750488 / 100, 130.58836364746094 / 100, 94191.4921875 / 100)
SOURCE_DRAW_SCALE = 1.2000000476837158
SHOT2_FLOOR_LIFT = 0.4


def transform(vector, ratio):
    if len(vector) != 3 or not all(math.isfinite(v) for v in vector):
        raise ValueError("Camera vector must contain three finite numbers")
    return [s + ratio * (v - s) for s, v in zip(SOURCE_ACTOR, vector)]


def rewrite(text, ratio, *, floor_clearance=False):
    before = json.loads(text)
    decoder = json.JSONDecoder()
    edits = []
    counts = {}
    for shot_id in SHOTS:
        marker = '"shotId": ' + json.dumps(shot_id)
        if text.count(marker) != 1:
            raise ValueError(f"Shot must exist exactly once: {shot_id}")
        begin = text.rfind("{", 0, text.index(marker))
        shot, size = decoder.raw_decode(text[begin:])
        if shot.get("activation") != "PATTERN_ONLY" or shot.get("sequenceInstanceId"):
            raise ValueError("Do not alter an automatic or World Sequence-bound camera")
        block = text[begin:begin + size]
        count = 0

        def replace(match):
            nonlocal count
            raw = match.group(2)
            values = json.loads(raw)
            vector = transform(values, ratio)
            if floor_clearance and shot_id == SHOTS[1]:
                vector[1] += SHOT2_FLOOR_LIFT
            changed = iter(vector)
            count += 1
            # Preserve array whitespace, indentation and line endings.
            numbers = r"-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?"
            return match.group(1) + re.sub(numbers, lambda _: format(next(changed), ".17g"), raw)

        block = re.sub(r'("(?:eye|lookAt|center)"\s*:\s*)(\[[^\]]+\])', replace, block)
        expected = 3 + 2 * len(shot["cameraTrack"]["keyframes"])
        if count != expected:
            raise ValueError(f"Unexpected vector count in {shot_id}: {count}/{expected}")
        counts[shot_id] = len(shot["cameraTrack"]["keyframes"])
        edits.append((begin, begin + size, block))
    for begin, end, block in sorted(edits, reverse=True):
        text = text[:begin] + block + text[end:]
    after = json.loads(text)
    b = {s["shotId"]: s for s in before["shots"]}
    for shot in after["shots"]:
        original = b[shot["shotId"]]
        if shot["shotId"] not in SHOTS:
            assert shot == original, "Unrelated shot changed"
            continue
        # Undo the permitted fields in the comparison document; all other
        # fields, including FOV, roll, interpolation and cut timing must match.
        shot["eye"], shot["lookAt"] = original["eye"], original["lookAt"]
        shot["box"]["center"] = original["box"]["center"]
        for key, old in zip(shot["cameraTrack"]["keyframes"], original["cameraTrack"]["keyframes"]):
            key["eye"], key["lookAt"] = old["eye"], old["lookAt"]
    assert before == after, "Non-camera-vector data changed"
    header = re.compile(r'("revision"\s*:\s*)' + str(before["revision"]) + r'(?=\s*,)')
    text, changed = header.subn(lambda m: m.group(1) + str(before["revision"] + (2 if floor_clearance else 1)), text, count=1)
    assert changed == 1
    return text, counts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--apply", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    path = root / REL
    original = path.read_bytes()
    if original.startswith(b"\xef\xbb\xbf"):
        raise ValueError("Unexpected BOM; inspect source encoding before editing")
    catalog = json.loads((root / "Data/Actors/BossCatalog.json").read_text(encoding="utf-8-sig"))
    boss, = [b for b in catalog["bosses"] if b["archetypeId"] == "BOSS_KAKULSAYDON_G3_SAYDON"]
    if boss.get("presentationScale") != 1 or boss.get("bodyModelPreScale") != .017:
        raise ValueError("Boss scale changed; recalculate against the original actor")
    ratio = boss["bodyModelPreScale"] / (.01 * SOURCE_DRAW_SCALE)
    backup = root / "out/ShowtimeBernContinuation20260918/camerashots.before-scale.json"
    # Re-running must never apply a second multiplication.
    if backup.exists():
        source = backup.read_bytes().decode("utf-8")
        candidate, counts = rewrite(source, ratio, floor_clearance=True)
        if original == candidate.encode("utf-8"):
            print("Already applied; no changes")
            return
        previous, _ = rewrite(source, ratio)
        if original not in (backup.read_bytes(), previous.encode("utf-8")):
            raise ValueError("Camera changed after backup; manual rebase required")
    else:
        candidate, counts = rewrite(original.decode("utf-8"), ratio, floor_clearance=True)
    result = candidate.encode("utf-8")
    assert original.count(b"\r\n") == result.count(b"\r\n")
    print(json.dumps(dict(ratio=ratio, keys=counts, beforeSha256=hashlib.sha256(original).hexdigest(),
                         afterSha256=hashlib.sha256(result).hexdigest(), apply=args.apply)))
    if args.apply:
        backup.parent.mkdir(parents=True, exist_ok=True)
        if not backup.exists():
            with backup.open("xb") as stream:
                stream.write(original)
        if path.read_bytes() != original:
            raise ValueError("Camera source changed during preparation")
        staged = None
        try:
            with tempfile.NamedTemporaryFile(dir=path.parent, prefix=".showtime-camera-", delete=False) as stream:
                staged = Path(stream.name)
                stream.write(result)
                stream.flush()
                os.fsync(stream.fileno())
            if path.read_bytes() != original:
                raise ValueError("Camera source changed before commit")
            os.replace(staged, path)
        finally:
            if staged is not None and staged.exists():
                staged.unlink()


if __name__ == "__main__":
    main()
```

### Tools/KoukuSaydonPipeline/test_showtime_camera_clearance.py

```python
"""Numeric regression for the saved Showtime camera, not a visual acceptance test."""
import json
import math
from pathlib import Path
import unittest

import retarget_showtime_camera_scale as fix

ROOT = Path(__file__).resolve().parents[2]
FLOOR_Y = 1.317626  # Installed SL05:export:342, floor08a upper triangles.
OFFSET = (-.1146, .0141, .4151)


def normalized(v):
    size = math.sqrt(sum(x*x for x in v))
    return [x / size for x in v]


def cross(a, b):
    return [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]]


class CameraClearance(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (ROOT/'out/ShowtimeBernContinuation20260918/camerashots.before-scale.json').read_bytes().decode('utf8')
        ratio = .017/(.01*fix.SOURCE_DRAW_SCALE)
        cls.old = json.loads(fix.rewrite(cls.source, ratio)[0])
        cls.result, _ = fix.rewrite(cls.source, ratio, floor_clearance=True)
        cls.new = json.loads(cls.result)

    def test_only_second_shot_height_and_revision_change(self):
        old = {s['shotId']: s for s in self.old['shots']}
        for shot in self.new['shots']:
            before = old[shot['shotId']]
            if shot['shotId'] != fix.SHOTS[1]:
                self.assertEqual(shot, before)
                continue
            pairs = [(shot['eye'], before['eye']), (shot['lookAt'], before['lookAt']),
                     (shot['box']['center'], before['box']['center'])]
            pairs += [(k[f], p[f]) for k,p in zip(shot['cameraTrack']['keyframes'],before['cameraTrack']['keyframes']) for f in ('eye','lookAt')]
            for actual, previous in pairs:
                self.assertEqual(actual[0], previous[0])
                self.assertEqual(actual[2], previous[2])
                self.assertAlmostEqual(actual[1]-previous[1], .4)
            for k,p in zip(shot['cameraTrack']['keyframes'],before['cameraTrack']['keyframes']):
                self.assertEqual({x:v for x,v in k.items() if x not in ('eye','lookAt')},
                                 {x:v for x,v in p.items() if x not in ('eye','lookAt')})
        self.assertEqual(self.new['revision'], self.old['revision']+1)

    def test_eye_and_near_plane_clear_floor_every_millisecond(self):
        minimum = float('inf')
        for shot in self.new['shots']:
            if shot['shotId'] not in fix.SHOTS: continue
            keys=shot['cameraTrack']['keyframes']
            for a,b in zip(keys,keys[1:]):
                for ms in range(a['timeMs'], b['timeMs']+1):
                    t=(ms-a['timeMs'])/(b['timeMs']-a['timeMs'])
                    eye=[x+(y-x)*t for x,y in zip(a['eye'],b['eye'])]
                    look=[x+(y-x)*t for x,y in zip(a['lookAt'],b['lookAt'])]
                    up=[x+(y-x)*t for x,y in zip(a['up'],b['up'])]
                    forward=normalized([y-x for x,y in zip(eye,look)])
                    right=normalized(cross(up,forward)); up=cross(forward,right)
                    fov=a['fovYDegrees']+(b['fovYDegrees']-a['fovYDegrees'])*t
                    half=.1*math.tan(math.radians(fov)/2)
                    for aspect in (4/3,16/9,21/9):
                        bottom=eye[1]+OFFSET[1]+.1*forward[1]-half*(abs(up[1])+aspect*abs(right[1]))
                        minimum=min(minimum,bottom-FLOOR_Y)
                        self.assertGreater(bottom-FLOOR_Y, .1, (shot['shotId'],ms,aspect))
        print('Minimum near-plane floor clearance (metres):',minimum)

    def test_installed_authoring_is_expected_correction(self):
        self.assertEqual((ROOT/fix.REL).read_bytes(),self.result.encode('utf8'))


if __name__ == '__main__':
    unittest.main()
```

### Tools/KoukuSaydonPipeline/test_preview_source_visibility.py

```python
"""Compile the production NPC visibility methods; check preview lifecycle wiring separately."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]


class PreviewSourceVisibility(unittest.TestCase):
    def test_native_visibility_counter(self):
        text=(ROOT/'Client/Public/Npc.h').read_text(encoding='utf8')
        names=('Set_PresentationVisible','Acquire_CompositionPreviewSuppression',
               'Release_CompositionPreviewSuppression','Is_PresentationVisible')
        methods=[]
        for name in names:
            matches=re.findall(r'^\s*(?:void|bool) '+name+r'\([^\n]*$',text,re.M)
            self.assertEqual(len(matches),1,name)
            methods.append(matches[0].strip())
        fixture='struct Visibility { bool m_bPresentationVisible=true; unsigned m_iCompositionPreviewSuppressions=0;\n'+'\n'.join(methods)+'\n};\n'
        fixture+='''int main() {
          Visibility v;
          if (!v.Is_PresentationVisible()) return 1;
          v.Acquire_CompositionPreviewSuppression();
          if (v.Is_PresentationVisible()) return 2;
          v.Acquire_CompositionPreviewSuppression();
          v.Release_CompositionPreviewSuppression();
          if (v.Is_PresentationVisible()) return 3;
          v.Set_PresentationVisible(false);
          v.Release_CompositionPreviewSuppression();
          if (v.Is_PresentationVisible()) return 4;
          v.Set_PresentationVisible(true);
          if (!v.Is_PresentationVisible()) return 5;
          v.Release_CompositionPreviewSuppression();
          if (!v.Is_PresentationVisible()) return 6;
          for (unsigned i=0;i<10000;++i) {
            v.Acquire_CompositionPreviewSuppression();
            v.Release_CompositionPreviewSuppression();
          }
          return v.Is_PresentationVisible() ? 0 : 7;
        }'''
        vs=Path(os.environ.get('VSINSTALLDIR','C:/Program Files/Microsoft Visual Studio/2022/Community'))
        with tempfile.TemporaryDirectory(prefix='showtime-visibility-') as temp:
            source=Path(temp)/'visibility.cpp'
            source.write_text(fixture,encoding='ascii')
            setup=vs/'Common7/Tools/VsDevCmd.bat'
            command=f'call "{setup}" -arch=x64 -host_arch=x64 >nul && cl /nologo /EHsc visibility.cpp /Fe:visibility.exe && visibility.exe'
            result=subprocess.run(command,shell=True,cwd=temp,capture_output=True)
            self.assertEqual(result.returncode,0,result.stdout.decode(errors='replace')+result.stderr.decode(errors='replace'))

    def test_production_lifecycle_wiring(self):
        text=(ROOT/'Client/Private/KoukuSaydonPresentationPlayer.cpp').read_text(encoding='utf8')
        release=text[text.index('void Client::CKoukuSaydonPresentationPlayer::Release_BundlePreviewMembers('):text.index('bool Client::CKoukuSaydonPresentationPlayer::Prepare_CloneSplitPreview(')]
        self.assertIn('source->Release_CompositionPreviewSuppression();',release)
        begin=text[text.index('bool Client::CKoukuSaydonPresentationPlayer::Begin_BundlePreview('):text.index('bool Client::CKoukuSaydonPresentationPlayer::Begin_ModelReferencePreview(')]
        self.assertLess(begin.index('Stop_Preview();'),begin.index('Sync_PreviewSourceVisibility();'))
        self.assertLess(begin.index('m_bPreviewPlaying ='),begin.index('Sync_PreviewSourceVisibility();'))
        self.assertLess(begin.index('Sync_PreviewSourceVisibility();'),begin.index('if (!externalWorldPreview) Sample_BundlePreview();'))
        sync=text[text.index('void Client::CKoukuSaydonPresentationPlayer::Sync_PreviewSourceVisibility()'):text.index('void Client::CKoukuSaydonPresentationPlayer::Sample_BundlePreview()')]
        for guard in ('!member.finiteActorLifetime','!boss.iOwnerBossNetEntityId','boss.strArchetypeId == member.sourceArchetypeId','previous == replacement'):
            self.assertIn(guard,sync)
        npc=(ROOT/'Client/Private/Npc.cpp').read_text(encoding='utf8')
        self.assertIn('if (!Is_PresentationVisible()) return;',npc)
        self.assertIn('if (!Is_PresentationVisible()) return S_OK;',npc)


if __name__=='__main__': unittest.main()
```
