#pragma once

#include "ActionPresentationTimeline.h"
#include "AnimationEffectCueDocument.h"
#include "Client_Defines.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameObject.h"

#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
class CTransform;
NS_END

NS_BEGIN(Client)

/* A town NPC: one skinned model that stands where it is put and plays a clip.

Deliberately not a CCharacter. That type assembles equipment parts, weapon
sockets and a class logic from a CHARACTER_SPEC, none of which an NPC has -- the
cook already merges an NPC's body and head into a single mesh, so there is one
model and nothing to assemble.

Everything an instance differs by is in NPC_DESC, so the placement tool can spawn
the same prototype many times with different positions and clips. */
class CNpc final : public CGameObject
{
public:
	typedef struct tagNpcDesc : public CGameObject::GAMEOBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		wstring_t strModelTag;
		wstring_t strShaderTag;
		/* Stable catalog owner plus the optional shared animation event document.
		The Esther roster uses owner-specific actionClips with one common
		"Esther" cue document, filtered against this NPC's actual model clips. */
		std::string strArchetypeId;
		std::string strAnimationEffectCueAssetId;

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
	} NPC_DESC;

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
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees);
	/* Applies one authoritative NPC action snapshot. The spawn packet has no
	start tick, so ClientReplication primes the first clip separately; the first
	world snapshot seeks that clip to exact Server age and catches up due cues. */
	bool_t Apply_NetworkAction(
		std::string_view actionId,
		uint32_t iServerTick,
		uint32_t iActionStartTick);
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
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	DEFERRED_EMISSIVE_OVERRIDE m_HitFlash;
	f32_t m_fHitFlashRemainingSeconds = { 0.f };
	std::string m_strArchetypeId;
	std::string m_strAnimationEffectCueAssetId;
	std::string m_strServerActionId;
	uint32_t m_iLastServerTick = 0u;
	uint32_t m_iServerActionStartTick = 0u;
	f32_t m_fServerActionAgeSeconds = 0.f;
	ANIMATION_EFFECT_CUE_DOCUMENT m_EffectCueDocument;
	std::unordered_set<std::string> m_SpawnedEffectCueOccurrences;
#ifdef _DEBUG
	bool_t m_isCombatColliderDebugVisible = { false };
#endif

private:
	HRESULT Ready_Components(const NPC_DESC* pDesc);
	HRESULT Bind_ShaderResources();
	bool_t Load_EffectCues();
	bool_t Build_ActionTimeline(
		std::string_view actionId,
		std::vector<std::string>& OutClips,
		std::vector<uint32_t>& OutAnimationIndices,
		std::vector<ACTION_PRESENTATION_CLIP_TIMING>& OutTimings) const;
	void Spawn_DueEffectCues(
		std::string_view actionId,
		const std::vector<std::string>& Clips,
		const std::vector<ACTION_PRESENTATION_CLIP_TIMING>& Timings,
		f32_t fActionAgeSeconds);

public:
	static unique_ptr<CNpc> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
