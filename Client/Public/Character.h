#pragma once

#include "Client_Defines.h"
#include "ActionPresentationTimeline.h"
#include "AnimationEffectCueDocument.h"
#include "ContainerObject.h"
#include "CharacterSpec.h"
#include "DeferredMaterialRenderUtils.h"
#include "EstherActionSoundCueDocument.h"
#include "FaceCustomizeApplier.h"
#include "NavPathFollower.h"
#include "PlayerHandGripTransform.h"
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
		uint32_t sourceStartMs = 0u;
	};

	/* A stage runs its own clips back to back. ACTIVE owns one stage, so its whole
	chain still plays itself out; a staged skill owns one per Server combo stage
	and waits on the last clip of the running stage. */
	struct CLIP_STAGE
	{
		std::vector<CLIP_STEP> clips;
	};

	struct CLIP_CHAIN
	{
		int32_t iSkillId = {};
		bool_t isServerStaged = false;
		std::vector<CLIP_STAGE> stages;
	};

	/* Runtime-only input prepared by the Equipment presentation service. Stable
		catalog IDs remain outside CContainerObject; this descriptor contains only
		the already-admitted prototype and attachment needed for one atomic clone. */
	struct EQUIPMENT_PREVIEW_PART_DESC
	{
		std::string runtimePartId;
		wstring_t modelPrototypeTag;
		bool_t isSocketed = false;
		std::string socketBoneId;
		f32_t socketYawDegrees = 0.f;
		LostArk::Shared::PLAYER_STANCE_ID requiredStance =
			LostArk::Shared::PLAYER_STANCE_ID::NONE;
		uint32_t hiddenMeshMask = 0u;
	};

public:
	typedef struct tagCharacterDesc : public CContainerObject::CONTAINEROBJECT_DESC
	{
		uint32_t iPrototypeLevelIndex = {};
		const CHARACTER_SPEC* pSpec = { nullptr };
		/* The replicated class this Character stands for. END keeps the spec's
		own class; a class-less avatar spec (the KoukuSaydon clown) needs the
		wearer's class here so quick slots and skill documents keep resolving. */
		LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
			LostArk::Shared::CHARACTER_CLASS_ID::END;
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
	/* The replicated class: the spec's unless the desc overrode it. */
	LostArk::Shared::CHARACTER_CLASS_ID Get_CharacterClass() const {
		return m_eCharacterClass;
	}
	/* Avatar pieces (EQUIPMENT_SLOT_KIND::AVATAR_HEAD / AVATAR_ARMOR) the class spec declares.
	Hiding one is a real unequip on the world character: the avatar part stops rendering and
	the default piece it covered shows again (the portrait draws the same parts). The character
	info window's eye toggles do NOT use this -- they only pass a preview mask to
	Render_PreviewParts. A kind the spec has no part for reports false and ignores the call. */
	bool_t Has_AvatarPart(EQUIPMENT_SLOT_KIND eKind) const;
	bool_t Is_AvatarPartVisible(EQUIPMENT_SLOT_KIND eKind) const;
	void Set_AvatarPartVisible(EQUIPMENT_SLOT_KIND eKind, bool_t isVisible);
	shared_ptr<Engine::CModel> Get_BodyModel() const;
	uint32_t Get_PrototypeLevelIndex() const
	{
		return m_iPrototypeLevelIndex;
	}
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
		bool_t isMoving,
		std::uint32_t iServerTick);
	/* comboStage is the server's 1-based stage, 0 outside a combo. The client
	never counts stages itself. */
	bool_t Apply_NetworkAction(
		LostArk::Shared::PLAYER_ACTION_STATE action,
		LostArk::Shared::SKILL_ID skillId,
		std::uint32_t serverTick,
		std::uint32_t actionStartTick,
		f32_t actionFacingYawDegrees,
		std::uint8_t comboStage = 0,
		bool_t hasSkillTarget = false,
		const float3_t& skillTarget = {});
	bool_t Try_Get_NetworkActionState(
		LostArk::Shared::PLAYER_ACTION_STATE& outAction) const
	{
		if (!m_hasNetworkState)
			return false;
		outAction = m_eNetworkAction;
		return true;
	}
	bool_t Try_Get_SkillTargetRoot(float4x4_t& outWorld) const;
	void Apply_NetworkStance(LostArk::Shared::PLAYER_STANCE_ID stance);
	/* Replication hands over the replicated owner presentation while the Server
	   reports GRABBED. The character keeps only a weak reference and the admitted
	   grip; every Update re-resolves the socket so a vanished owner falls back to
	   the Server transform instead of a stale matrix. */
	bool_t Apply_NetworkAttachment(
		const std::shared_ptr<const IPlayerHandGripSocketSource>& pSource,
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot);
	void Clear_NetworkAttachment();
	/* A Model View clone may mirror the live scene stance, but only after that
	   scene Character has consumed an authoritative snapshot. Before then its
	   NONE member is initialization state, not a valid Lance stance. */
	bool_t Try_Get_NetworkStance(
		LostArk::Shared::PLAYER_STANCE_ID& outStance) const
	{
		if (!m_hasNetworkState ||
			m_eStance >= LostArk::Shared::PLAYER_STANCE_ID::END)
		{
			return false;
		}
		outStance = m_eStance;
		return true;
	}

	bool_t Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop);
	bool_t Set_Animation(const char_t* pClipName, bool_t isLoop);
	PATH_RESULT_CODE Request_Move(fvector_t vGoalPosition);
	bool_t Try_SampleTargetGround(
		f32_t x,
		f32_t z,
		float3_t& outPosition) const;
	void Cancel_Move();
	bool_t Is_Moving() const {
		return m_isMoving;
	}

#ifdef _DEBUG
	void Set_NavigationDebugVisible(bool_t isVisible) {
		m_isNavigationDebugVisible = isVisible;
	}
	void Set_CombatColliderDebugVisible(bool_t isVisible) {
		m_isCombatColliderDebugVisible = isVisible;
	}
	void Set_SkillHitAreaDebugVisible(bool_t isVisible) {
		m_isSkillHitAreaDebugVisible = isVisible;
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
	bool_t Apply_EquipmentPreview(
		const std::vector<EQUIPMENT_PREVIEW_PART_DESC>& parts,
		uint32_t occupiedSlotsMask,
		std::string& outError);
	bool_t Reset_EquipmentPreview(std::string& outError);
	/* Face customizing sliders (retail add_*_ui additive poses), presentation only.
	Empty for classes whose spec names no face slider race. */
	bool_t Has_FaceSliders() const {
		return m_FaceCustomize.Get_SliderCount() > 0;
	}
	const CFaceCustomizeApplier& Get_FaceCustomize() const {
		return m_FaceCustomize;
	}
	bool_t Set_FaceSliderWeight(size_t iSlider, f32_t fWeight) {
		return m_FaceCustomize.Set_Weight(iSlider, fWeight);
	}
	void Reset_FaceSliders() {
		m_FaceCustomize.Reset_Weights();
	}
	/* Hides every HEAD-slot equipment part (helmet, avatar head) so the face can
	be inspected; true restores the normal default/avatar visibility rule. */
	void Set_HeadPartsVisible(bool_t isVisible);
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
	/* Draws every part again with explicit forward passes (the character info window portrait
	renders into its own target with the preview camera bound in CPipeLine beforehand). The
	normal frame render is untouched: parts still queue themselves in Late_Update. */
	/* Avatar preview override for this draw only: for every kind whose bit
	(1 << ETOUI(EQUIPMENT_SLOT_KIND)) is in iAvatarOverrideKinds, the piece is hidden when the
	same bit is in iAvatarHiddenKinds and shown otherwise, regardless of the real equip state
	(character info eye toggles, avatar book try-on). Kinds outside the override keep their real
	state. Part visibility is re-derived for this draw and restored right after; the world
	character is untouched -- real equip/unequip goes through Set_AvatarPartVisible. */
	HRESULT Render_PreviewParts(uint32_t iSkinnedPassIndex, uint32_t iSocketedPassIndex,
		uint32_t iAvatarOverrideKinds = 0u, uint32_t iAvatarHiddenKinds = 0u);

private:
	const CHARACTER_SPEC* m_pSpec = { nullptr };
	LostArk::Shared::CHARACTER_CLASS_ID m_eCharacterClass =
		LostArk::Shared::CHARACTER_CLASS_ID::END;
	unique_ptr<ICharacterLogic> m_pLogic;
	shared_ptr<Engine::CModel> m_pBodyModel = { nullptr };
	shared_ptr<Engine::CNavigation> m_pNavigationCom = { nullptr };
	shared_ptr<Engine::CCollider> m_pColliderCom = { nullptr };
	CNavPathFollower m_PathFollower;
	uint32_t m_iPrototypeLevelIndex = {};
	f32_t m_fMoveSpeed = { 5.f };
	bool_t m_isMoving = { false };
	/* Negative when no delayed idle commit is pending. */
	f32_t m_fPendingIdleSeconds = { -1.f };
	wstring_t m_strNavigationPrototypeTag;

#ifdef _DEBUG
	bool_t m_isNavigationDebugVisible = { false };
	bool_t m_isCombatColliderDebugVisible = { false };
	bool_t m_isSkillHitAreaDebugVisible = { true };
	/* A client-side prediction of an object the current clip spawned, drawn
	as a wire so the Server's projectile judgement can be seen; it moves along
	the facing at the authored speed and dies at the authored distance/life. */
	struct DEBUG_PROJECTILE_WIRE
	{
		size_t iCue = 0;
		f32_t fX = 0.f;
		f32_t fY = 0.f;
		f32_t fZ = 0.f;
		f32_t fDirectionX = 0.f;
		f32_t fDirectionZ = 1.f;
		f32_t fRemainingDistance = -1.f;
		f32_t fRemainingSeconds = 0.f;
	};
	std::vector<DEBUG_PROJECTILE_WIRE> m_DebugProjectiles;
	std::string m_strDebugProjectileClip;
	f32_t m_fDebugProjectileClipMs = { 0.f };
#endif

	std::vector<CLIP_CHAIN> m_Chains;
	std::vector<CLIP_CHAIN> m_PendingChains;
	/* The chain being played, and how far into it. Null when idle. */
	const CLIP_CHAIN* m_pChain = { nullptr };
	int32_t m_iChainStage = {};
	int32_t m_iChainStep = {};

	std::string m_strNickName;
	bool_t m_isLocallyControlled = { false };

	//network persentation state
	bool_t m_hasNetworkState = { false };
	LostArk::Shared::PLAYER_ACTION_STATE m_eNetworkAction =
		LostArk::Shared::PLAYER_ACTION_STATE::NONE;
	/* The clip step of a replicated KNOCKDOWN: fall -> land -> lying loop, and
	one standup after the server releases the action. Presentation only. */
	enum class KNOCKDOWN_STEP : std::uint8_t
	{ NONE, FALLING, LANDING, DOWN, STANDUP };
	KNOCKDOWN_STEP m_eKnockdownStep = KNOCKDOWN_STEP::NONE;
	LostArk::Shared::PLAYER_STANCE_ID m_eStance =
		LostArk::Shared::PLAYER_STANCE_ID::NONE;
	std::uint32_t m_iLastNetworkActionStartTick = 0;
	ESTHER_ACTION_SOUND_PLAYBACK_STATE m_EstherActionSoundState;
	f32_t m_fActionPresentationSeconds = 0.f;
	ANIMATION_EFFECT_CUE_DOCUMENT m_EffectCueDocument;
	f32_t m_fPreviousEffectCueStageWallSeconds = -1.f;
	f32_t m_fPreviousSoundCueStageWallSeconds = -1.f;
	f32_t m_fPreviousShakeCueStageWallSeconds = -1.f;
	std::uint32_t m_iEffectActionStartTick = 0u;
	f32_t m_fEffectActionFacingYawDegrees = 0.f;
	bool_t m_bHasEffectActionFacingYaw = false;
	LostArk::Shared::SKILL_ID m_iCurrentEffectSkillId =
		LostArk::Shared::INVALID_SKILL_ID;
	bool_t m_hasNetworkSkillTarget = false;
	float3_t m_NetworkSkillTarget{};
	DEFERRED_EMISSIVE_OVERRIDE m_ActionEmissiveOverride;
	struct NETWORK_TRANSFORM_SAMPLE
	{
		std::uint32_t iServerTick = 0;
		float3_t vPosition = {};
		f32_t fYawDegrees = 0.f;
	};
	static constexpr size_t NETWORK_SAMPLE_CAPACITY = 8;
	NETWORK_TRANSFORM_SAMPLE m_NetworkSamples[NETWORK_SAMPLE_CAPACITY] = {};
	size_t m_iNetworkSampleCount = 0;
	f32_t m_fPlaybackServerTick = 0.f;
	/* Follows the network yaw at TURN_DEGREES_PER_SECOND instead of jumping to
	it. Presentation only: the server's value stays the one gameplay reads. */
	f32_t m_fPresentationYawDegrees = { 0.f };
	/* Presentation attachment while the Server reports GRABBED. The Server
	   position stays in m_NetworkSamples; the socket only replaces the rendered
	   feet position before the parts compose their world matrices, so body,
	   equipment, collider wire and nameplate all read the same value. */
	std::weak_ptr<const IPlayerHandGripSocketSource> m_pAttachmentSocketSource;
	LostArk::Shared::PLAYER_ATTACHMENT_SLOT m_eAttachmentSlot =
		LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
	PLAYER_HAND_GRIP_LOCAL_OFFSET m_AttachmentGripLocalOffset{};
	bool_t m_bAttachmentPresented = { false };
	float3_t m_LastAttachedPosition{};
	/* Negative when no release blend is running. */
	f32_t m_fAttachmentReleaseBlendSeconds = { -1.f };
	CBoneChainSimulation m_BoneChains;
	FACE_SLIDER_DOCUMENT m_FaceSliderDocument;
	CFaceCustomizeApplier m_FaceCustomize;
	bool_t m_isEquipmentPreviewActive = false;
	uint32_t m_iEquipmentPreviewOccupiedSlotsMask = 0u;
	/* Set_AvatarPartVisible state; Apply_DefaultEquipmentVisibility derives the parts from it. */
	bool_t m_isAvatarHeadHidden = false;
	bool_t m_isAvatarArmorHidden = false;
	std::vector<std::pair<wstring_t,
		LostArk::Shared::PLAYER_STANCE_ID>> m_EquipmentPreviewPartStances;

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	/* Loads the spec's face slider document and resolves it against the body
	skeleton. Failure is isolated: the character keeps working without sliders. */
	void Load_FaceSliders();
	void Set_Locomotion(bool_t isMoving);
	/* Applies the state Set_Locomotion decided on. Idle arrives here only after
	LOCOMOTION_IDLE_DELAY_SECONDS without a run in between. */
	void Commit_Locomotion(bool_t isMoving);
	/* IDLE and RUN can belong to the current stance instead of the class. Every
	other state resolves straight off the spec. */
	const char_t* Resolve_LocomotionClip(CHARACTER_ANIM eAnim) const;
	bool_t Load_ClipChains();
	void Commit_PendingClipChains();
	/* Plays a clip from its first frame. Set_Animation alone only switches the
	index, so a clip that already ran would resume at its end -- which chains
	that repeat a clip do hit. */
	bool_t Start_Clip(const CLIP_STEP& Step);
	/* Resolves the authoritative stage age onto its authored sequential clip
	chain. The input is wall time; playRate converts it to source clip time. */
	bool_t Seek_ActiveStageForward(f32_t fActionAgeSeconds);
	bool_t Resolve_ClipTiming(
		const CLIP_STEP& Step,
		std::uint32_t& iOutAnimation,
		f32_t& fOutSourceDurationSeconds) const;
	bool_t Build_ActiveStageTimeline(
		std::vector<ACTION_PRESENTATION_CLIP_TIMING>& OutTimings,
		std::vector<std::uint32_t>* pOutAnimations = nullptr) const;
	void Set_PartVisible(const tchar_t* pPartTag, bool_t isVisible);
	HRESULT Render_PreviewPartsInternal(uint32_t iSkinnedPassIndex, uint32_t iSocketedPassIndex);
	void Apply_DefaultEquipmentVisibility(uint32_t occupiedSlotsMask);
	void Restore_DefaultEquipmentVisibility();
	void Sync_EquipmentPreviewStanceVisibility();
	/* Jumps the running chain to the server's stage. Fails when no chain runs or
	the stage is past its end, so the caller keeps the pose it had. */
	bool_t Advance_ComboStage(std::uint8_t comboStage);
	bool_t Is_ClipFinished() const;
	/* Moves to the next clip once the current one ends, and drops back to idle
	after the last. */
	void Update_Chain();
	/* Advances the knockdown clip step when its current clip ends: fall to
	land, land to the lying loop, and standup back to locomotion. */
	void Update_KnockdownPresentation();

	//server snapshot interpolation
	void Update_NetworkTransform(f32_t fTimeDelta);
	/* Runs after Update_NetworkTransform and before the parts compose: while
	   GRABBED it replaces the interpolated position with the owner socket, and
	   for ATTACHMENT_RELEASE_BLEND_SECONDS after release it eases from the last
	   socket position onto the Server knockback path. */
	void Update_NetworkAttachmentTransform(f32_t fTimeDelta);
#ifdef _DEBUG
	void Draw_SkillHitAreaDebug() const;
	void Update_SkillProjectileDebug(f32_t fTimeDelta);
	/* True only when the Server would use the legacy maximumRange fallback for
	   this exact authoritative skill stage. This is presentation diagnostics;
	   it never creates a Client hit or changes Server targeting. */
	bool_t Try_Get_CurrentFallbackHitRange(f32_t& fOutRangeMeters) const;
#endif
	bool_t Load_EffectCues();
	void Reset_EffectCueCursor(
		std::uint32_t iActionStartTick,
		f32_t fActionFacingYawDegrees);
	void Update_EffectCues();
	void Update_SoundCues();
	void Update_CameraShakeCues();
	void Spawn_FallbackEffect(LostArk::Shared::SKILL_ID iSkillId);
	f32_t Get_EffectPlaybackRate() const;
	void Update_ActionEmissiveOverride(
		LostArk::Shared::PLAYER_ACTION_STATE action,
		LostArk::Shared::SKILL_ID skillId);

public:
	static unique_ptr<CCharacter> Create(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;
};

NS_END
