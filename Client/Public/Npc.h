#pragma once

#include "Client_Defines.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameObject.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <string>

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
NS_END

NS_BEGIN(Client)

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
class CNpc final : public CGameObject
{
public:
	typedef struct tagNpcDesc : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;

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
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	/* Restarts the selected clip even when the previous action used the same
	clip. The network action edge owns restart timing; the model only owns how
	the authored clip is blended and played. */
	bool_t Play_NetworkAction(
		const char_t* pClipName,
		bool_t isLoop,
		f32_t fPlaybackRate,
		f32_t fBlendSeconds);
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
		std::uint32_t iServerTick = 0u);
	void Trigger_HitFlash();
#ifdef _DEBUG
	void Set_CombatColliderDebugVisible(bool_t isVisible) {
		m_isCombatColliderDebugVisible = isVisible;
	}
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
	shared_ptr<Engine::CModel> m_pModelCom = { nullptr };
	wstring_t m_strModelTag;
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
	std::string m_strDefaultIdleClip;
	CNpcNetworkTransformInterpolator m_NetworkTransformInterpolator;
	bool_t m_bSuppressRootMotion = false;
	bool_t m_bInterpolateNetworkTransform = false;
	f32_t m_fTransientActionRemainingSeconds = 0.f;
	std::string m_strTransientReturnClip;
	f32_t m_fTransientReturnPlaybackRate = 1.f;
	bool_t m_isTransientReturnLoop = true;
	f32_t m_fOutlineWidth = { 0.f };
	float4_t m_vOutlineColor = { 1.f, 1.f, 1.f, 1.f };
#ifdef _DEBUG
	bool_t m_isCombatColliderDebugVisible = { false };
#endif

private:
	HRESULT Ready_Components(const NPC_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	void Apply_ImmediateTransform(
		const float3_t& position,
		f32_t yawDegrees);
	void Update_NetworkTransform(f32_t fTimeDelta);

public:
	static unique_ptr<CNpc> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
