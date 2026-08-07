#pragma once

#include "Client_Defines.h"
#include "AnimationEffectCueDocument.h"
#include "ContainerObject.h"
#include "CharacterSpec.h"
#include "NavPathFollower.h"
#include "Network/PacketMessages.h"

NS_BEGIN(Engine)
class CModel;
class CNavigation;
class CCollider;
class CTransform;
NS_END

NS_BEGIN(Client)

/* One playable character, whatever the class. Everything class-specific arrives
as a CHARACTER_SPEC plus an ICharacterLogic, so this stays shared by the team.

Input stays outside this object. PlayerController submits semantic commands and
replication applies approved movement or action presentation. */
class CCharacter final : public CContainerObject
{
public:
	/* One approved skill's authored presentation chain. ACTIVE runs every clip in
	order; COMBO advances only when the replicated Server comboStage changes. */
	struct CLIP_STEP
	{
		std::string clip;
		uint32_t playMs = 0u;
		f32_t playRate = 1.f;
		bool_t loop = false;
	};

	struct CLIP_CHAIN
	{
		int32_t iSkillId = {};
		bool_t isServerStaged = false;
		std::vector<CLIP_STEP> clips;
	};

public:
	typedef struct tagCharacterDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		const CHARACTER_SPEC* pSpec = { nullptr };
		const tchar_t* pNavigationPrototypeTag = { nullptr };
		float3_t vPosition = {};

		std::string strNickName;
		bool_t isLocallyControlled = { false };
	} CHARACTER_DESC;

private:
	CCharacter(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
	/* The logic is a unique_ptr, so the implicit copy is deleted. Clones build
	their own from the spec; only the prototype's empty state is copied. */
	CCharacter(const CCharacter& Prototype);
public:
	virtual ~CCharacter();

public:
	const CHARACTER_SPEC* Get_Spec() const {
		return m_pSpec;
	}
	shared_ptr<Engine::CModel> Get_BodyModel() const;
	shared_ptr<Engine::CTransform> Get_Transform() const {
		return m_pTransformCom;
	}

	const std::string& Get_NickName() const
	{
		return m_strNickName;
	}

	bool_t Is_LocallyControlled() const
	{
		return m_isLocallyControlled;
	}

	void Set_Position(fvector_t vPosition);
	//charcter represent function
	bool_t Apply_NetworkState(
		const float3_t& position,
		f32_t yawDegrees,
		bool_t isMoving);
	/* comboStage is the server's 1-based stage, 0 outside a combo. The client
	never counts stages itself. */
	bool_t Apply_NetworkAction(
		LostArk::Shared::PLAYER_ACTION_STATE action,
		LostArk::Shared::SKILL_ID skillId,
		std::uint32_t actionStartTick,
		std::uint8_t comboStage = 0);
	void Apply_NetworkStance(LostArk::Shared::PLAYER_STANCE_ID stance);

	bool_t Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop);
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);
	void Cancel_Move();
	bool_t Is_Moving() const {
		return m_isMoving;
	}

#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) {
		m_isNavigationDebugVisible = isVisible;
	}
#endif

	/* Applies an approved skill action to presentation. Input code must never call
	this directly. isCombo comes from the Server-owned comboStage, so an ACTIVE
	skill never gets trapped in a reference-only COMBO tripod chain. */
	bool_t Play_Skill(
		int32_t iSkillId,
		bool_t isCombo);
	/* Animation Tool Save refreshes the scene Character through the same staged
	runtime loader. An action in flight keeps its current vector alive; the new
	set commits at the next authoritative action edge. Failure preserves the
	previously loaded chains. */
	bool_t Reload_SkillAnimationBindings();
	bool_t Is_PlayingSkill() const {
		return nullptr != m_pChain;
	}
	int32_t Get_PlayingSkillId() const {
		return nullptr != m_pChain ? m_pChain->iSkillId : 0;
	}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(f32_t fTimeDelta) override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual void Late_Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	const CHARACTER_SPEC* m_pSpec = { nullptr };
	unique_ptr<ICharacterLogic> m_pLogic;
	shared_ptr<Engine::CModel> m_pBodyModel = { nullptr };
	shared_ptr<Engine::CNavigation> m_pNavigationCom = { nullptr };
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	f32_t m_fMoveSpeed = { 5.f };
	bool_t m_isMoving = { false };
	wstring_t m_strNavigationPrototypeTag;

#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
#endif

	std::vector<CLIP_CHAIN> m_Chains;
	std::vector<CLIP_CHAIN> m_PendingChains;
	/* The chain being played, and how far into it. Null when idle. */
	const CLIP_CHAIN* m_pChain = { nullptr };
	int32_t m_iChainStep = {};

	std::string m_strNickName;
	bool_t m_isLocallyControlled = { false };

	//network persentation state
	bool_t m_hasNetworkState = { false };
	LostArk::Shared::PLAYER_ACTION_STATE m_eNetworkAction =
		LostArk::Shared::PLAYER_ACTION_STATE::NONE;
	LostArk::Shared::PLAYER_STANCE_ID m_eStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	std::uint32_t m_iLastNetworkActionStartTick = 0;
	ANIMATION_EFFECT_CUE_DOCUMENT m_EffectCueDocument;
	std::string m_strEffectCueClip;
	std::uint32_t m_iPreviousEffectCueTimeMs = UINT32_MAX;
	std::uint32_t m_iEffectActionStartTick = 0u;
	LostArk::Shared::SKILL_ID m_iCurrentEffectSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	//next goal pos that server notice
	float3_t m_vNetworkTargetPosition = {};
	f32_t m_fNetworkTargetYawDegrees = { 0.f };

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	void Set_Locomotion(bool_t isMoving);
	bool_t Load_ClipChains();
	void Commit_PendingClipChains();
	/* Plays a clip from its first frame. Set_Animation alone only switches the
	index, so a clip that already ran would resume at its end -- which chains
	that repeat a clip do hit. */
	bool_t Start_Clip(const CLIP_STEP& Step);
	void Set_PartVisible(const tchar_t* pPartTag, bool_t isVisible);
	/* Jumps the running chain to the server's stage. Fails when no chain runs or
	the stage is past its end, so the caller keeps the pose it had. */
	bool_t Advance_ComboStage(std::uint8_t comboStage);
	bool_t Is_ClipFinished() const;
	/* Moves to the next clip once the current one ends, and drops back to idle
	after the last. */
	void Update_Chain();

	//server snapshot interpolation
	void Update_NetworkTransform(f32_t fTimeDelta);
	bool_t Load_EffectCues();
	void Reset_EffectCueCursor(std::uint32_t iActionStartTick);
	void Update_EffectCues();
	void Spawn_FallbackEffect(LostArk::Shared::SKILL_ID iSkillId);
	f32_t Get_EffectPlaybackRate() const;

public:
	static unique_ptr<CCharacter> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
