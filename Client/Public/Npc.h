#pragma once

#include "Client_Defines.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CCollider;
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
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees);
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
	f32_t m_fOutlineWidth = { 0.f };
	float4_t m_vOutlineColor = { 1.f, 1.f, 1.f, 1.f };
#ifdef _DEBUG
	bool_t m_isCombatColliderDebugVisible = { false };
#endif

private:
	HRESULT Ready_Components(const NPC_DESC* pDesc);
	HRESULT Bind_ShaderResources();

public:
	static unique_ptr<CNpc> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
