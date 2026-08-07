#include "Character.h"

#include "AnimationSkillBindingDocument.h"
#include "Collider.h"
#include "Effect_Catalog.h"
#include "Effect_PresentationService.h"
#include "GameInstance.h"
#include "Navigation.h"
#include "Part_Body.h"
#include "Part_Equipment.h"
#include "PlayerSkillCatalog.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t CLIP_BLEND_SECONDS = 0.12f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
}

CCharacter::CCharacter(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CCharacter::CCharacter(const CCharacter& Prototype)
	: CContainerObject { Prototype }
{
}

CCharacter::~CCharacter()
{
}

HRESULT CCharacter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCharacter::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<CHARACTER_DESC*>(pArg);

	m_pSpec = pDesc->pSpec;
	m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
	m_fMoveSpeed = pDesc->fSpeedPerSec > 0.f ?
		pDesc->fSpeedPerSec : 5.f;
	//nickname과 local control 여부 추가
	m_strNickName = pDesc->strNickName;
	m_isLocallyControlled = pDesc->isLocallyControlled;

	if (nullptr != pDesc->pNavigationPrototypeTag)
	{
		m_strNavigationPrototypeTag =
			pDesc->pNavigationPrototypeTag;
	}

	if (nullptr == m_pSpec)
		return E_FAIL;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(
			pDesc->vPosition.x,
			pDesc->vPosition.y,
			pDesc->vPosition.z,
			1.f));

	if (FAILED(Ready_Components()) ||
		FAILED(Ready_PartObjects()))
	{
		return E_FAIL;
	}

	/* A missing/corrupt presentation document must not prevent the Character,
	IDLE/RUN, or the Animation Tool repair target from existing. Approved skill
	actions remain valid network state even when their optional clip mapping is
	unavailable. */
	Load_ClipChains();
	Load_EffectCues();

	// Remote Character는 local keyboard logic를 만들지 않는다.
	if (m_isLocallyControlled &&
		nullptr != m_pSpec->pCreateLogic)
	{
		m_pLogic = m_pSpec->pCreateLogic();
	}

	return S_OK;
}

/* Extraction references may seed authoring, but runtime consumes only the
validated authored document. Animation Tool Save is therefore the single
presentation path used by local and remote Characters. */
bool_t CCharacter::Load_ClipChains()
{
	if (nullptr == m_pSpec->pAssetName)
		return false;
	std::vector<std::string> availableClips;
	availableClips.reserve(m_pBodyModel->Get_NumAnimations());
	for (uint32_t index = 0; index < m_pBodyModel->Get_NumAnimations(); ++index)
	{
		const char_t* clipName = m_pBodyModel->Get_AnimationName(index);
		if (nullptr != clipName)
			availableClips.emplace_back(clipName);
	}

	ANIMATION_SKILL_BINDING_DOCUMENT document;
	std::string status;
	if (!CAnimationSkillBindingDocument::Load(
		m_pSpec->pAssetName,
		m_pSpec->eCharacterClass,
		CPlayerSkillCatalog::Get_Skills(),
		availableClips,
		document,
		status))
	{
		OutputDebugStringA(("Character skill animation load failed: " +
			status + "\n").c_str());
		return false;
	}

	std::vector<CLIP_CHAIN> stagedChains;
	stagedChains.reserve(document.Bindings.size());
	for (const ANIMATION_SKILL_BINDING& binding : document.Bindings)
	{
		const PLAYER_SKILL_DEFINITION* definition =
			CPlayerSkillCatalog::Find_ById(binding.iSkillId);
		if (nullptr == definition)
			return false;
		CLIP_CHAIN chain{};
		chain.iSkillId = static_cast<int32_t>(binding.iSkillId);
		const bool_t isHold =
			LostArk::Shared::PLAYER_SKILL_KIND::HOLD == definition->eSkillKind;
		chain.isServerStaged = isHold ||
			LostArk::Shared::PLAYER_SKILL_KIND::COMBO == definition->eSkillKind;
		chain.clips.reserve(binding.Clips.size());
		for (const ANIMATION_SKILL_CLIP& clip : binding.Clips)
		{
			const bool_t isHoldLoop =
				isHold && 1u == chain.clips.size() && 3u == binding.Clips.size();
			chain.clips.push_back(
				{ clip.strClipName, clip.iPlayMs, clip.fPlayRate, isHoldLoop });
		}
		stagedChains.push_back(std::move(chain));
	}
	if (stagedChains.empty())
		return false;
	if (nullptr != m_pChain)
		m_PendingChains = std::move(stagedChains);
	else
		m_Chains = std::move(stagedChains);
	return true;
}

bool_t CCharacter::Reload_SkillAnimationBindings()
{
	return Load_ClipChains();
}

bool_t CCharacter::Load_EffectCues()
{
	if (nullptr == m_pSpec || nullptr == m_pSpec->pAssetName ||
		nullptr == m_pBodyModel)
		return false;
	std::vector<std::string> clips;
	clips.reserve(m_pBodyModel->Get_NumAnimations());
	for (uint32_t index = 0u; index < m_pBodyModel->Get_NumAnimations(); ++index)
	{
		const char_t* pName = m_pBodyModel->Get_AnimationName(index);
		if (nullptr != pName)
			clips.emplace_back(pName);
	}
	ANIMATION_EFFECT_CUE_DOCUMENT staged;
	std::string status;
	if (!CAnimationEffectCueDocument::Load(
		m_pSpec->pAssetName, clips, staged, status))
	{
		OutputDebugStringA(("Character Effect cue load isolated: " +
			status + "\n").c_str());
		return false;
	}
	for (const ANIMATION_EFFECT_CUE& cue : staged.Cues)
	{
		if ("root" != cue.strAnchorSlotId &&
			!m_pBodyModel->Has_Bone(cue.strAnchorSlotId.c_str()))
		{
			OutputDebugStringA(("Character Effect cue anchor rejected: " +
				cue.strAnchorSlotId + "\n").c_str());
			return false;
		}
	}
	m_EffectCueDocument = std::move(staged);
	return true;
}

void CCharacter::Reset_EffectCueCursor(
	const std::uint32_t iActionStartTick)
{
	m_strEffectCueClip.clear();
	m_iPreviousEffectCueTimeMs = UINT32_MAX;
	m_iEffectActionStartTick = iActionStartTick;
}

f32_t CCharacter::Get_EffectPlaybackRate() const
{
	if (nullptr == m_pChain || m_iChainStep < 0 ||
		m_iChainStep >= static_cast<int32_t>(m_pChain->clips.size()))
	{
		return 1.f;
	}
	const f32_t fRate = m_pChain->clips[m_iChainStep].playRate;
	return std::isfinite(fRate) && fRate > 0.f ? fRate : 1.f;
}

void CCharacter::Spawn_FallbackEffect(
	const LostArk::Shared::SKILL_ID iSkillId)
{
	const PLAYER_SKILL_DEFINITION* pDefinition =
		CPlayerSkillCatalog::Find_ById(iSkillId);
	if (nullptr == pDefinition || pDefinition->strEffectId.empty() ||
		!CEffectCatalog::Contains(pDefinition->strEffectId) ||
		nullptr == m_pBodyModel)
		return;
	const char_t* pClipName = m_pBodyModel->Get_AnimationName(
		m_pBodyModel->Get_CurrentAnimIndex());
	if (nullptr != pClipName && std::any_of(
		m_EffectCueDocument.Cues.begin(), m_EffectCueDocument.Cues.end(),
		[pClipName](const ANIMATION_EFFECT_CUE& Cue)
		{
			return Cue.strClipName == pClipName;
		}))
	{
		return;
	}
	EFFECT_SPAWN_DESC Desc;
	Desc.strEffectAssetId = pDefinition->strEffectId;
	Desc.pOwner = static_pointer_cast<CCharacter>(shared_from_this());
	Desc.strAnchorSlotId = "root";
	Desc.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
	Desc.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
	Desc.iActionStartTick = m_iEffectActionStartTick;
	Desc.iCueStartMs = 0u;
	Desc.fPlaybackRate = Get_EffectPlaybackRate();
	std::string status;
	CEffectPresentationService::Spawn(Desc, status);
}

void CCharacter::Update_EffectCues()
{
	if (nullptr == m_pBodyModel || 0u == m_iEffectActionStartTick)
		return;
	const uint32_t iAnimation = m_pBodyModel->Get_CurrentAnimIndex();
	const char_t* pClipName = m_pBodyModel->Get_AnimationName(iAnimation);
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	if (nullptr == pClipName || !m_pBodyModel->Get_AnimationProgress(
		iAnimation, fPosition, fDuration))
		return;
	const f32_t fTicksPerSecond =
		m_pBodyModel->Get_AnimationTickPerSecond(iAnimation);
	if (!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f)
		return;
	const uint32_t iCurrentMs = static_cast<uint32_t>((std::max)(
		0.f, fPosition / fTicksPerSecond * 1000.f));
	if (m_strEffectCueClip != pClipName ||
		(UINT32_MAX != m_iPreviousEffectCueTimeMs &&
			iCurrentMs < m_iPreviousEffectCueTimeMs))
	{
		m_strEffectCueClip = pClipName;
		m_iPreviousEffectCueTimeMs = UINT32_MAX;
	}
	for (const ANIMATION_EFFECT_CUE& Cue : m_EffectCueDocument.Cues)
	{
		if (Cue.strClipName != m_strEffectCueClip)
			continue;
		const bool_t bCrossed = UINT32_MAX == m_iPreviousEffectCueTimeMs ?
			Cue.iStartMs <= iCurrentMs :
			Cue.iStartMs > m_iPreviousEffectCueTimeMs &&
				Cue.iStartMs <= iCurrentMs;
		if (!bCrossed)
			continue;
		EFFECT_SPAWN_DESC Desc;
		Desc.strEffectAssetId = Cue.strEffectAssetId;
		Desc.pOwner = static_pointer_cast<CCharacter>(shared_from_this());
		Desc.strAnchorSlotId = Cue.strAnchorSlotId;
		Desc.LocalTransform = Cue.LocalTransform;
		Desc.eFollowPolicy = Cue.eFollowPolicy;
		Desc.eStopPolicy = Cue.eStopPolicy;
		Desc.iCueDurationMs = Cue.iEndMs - Cue.iStartMs;
		Desc.iActionStartTick = m_iEffectActionStartTick;
		Desc.iCueStartMs = Cue.iStartMs;
		Desc.fPlaybackRate = Get_EffectPlaybackRate();
		std::string status;
		CEffectPresentationService::Spawn(Desc, status);
	}
	m_iPreviousEffectCueTimeMs = iCurrentMs;
}

void CCharacter::Commit_PendingClipChains()
{
	if (nullptr != m_pChain || m_PendingChains.empty())
		return;
	m_Chains = std::move(m_PendingChains);
}

bool_t CCharacter::Start_Clip(const CLIP_STEP& Step)
{
	if (!Set_Animation(Step.clip.c_str(), Step.loop))
		return false;

	m_pBodyModel->Set_AnimTrackPosition(m_pBodyModel->Get_CurrentAnimIndex(), 0.f);
	m_pBodyModel->Set_AnimationSpeed(Step.playRate);
	return true;
}

bool_t CCharacter::Is_ClipFinished() const
{
	if (nullptr == m_pBodyModel)
		return false;

	const uint32_t iAnimation = m_pBodyModel->Get_CurrentAnimIndex();
	f32_t fPosition = 0.f;
	f32_t fDuration = 0.f;
	if (!m_pBodyModel->Get_AnimationProgress(
		iAnimation, fPosition, fDuration))
		return false;
	if (fDuration <= 0.f)
		return false;

	f32_t fLimit = fDuration;
	if (nullptr != m_pChain && m_iChainStep >= 0 &&
		m_iChainStep < static_cast<int32_t>(m_pChain->clips.size()))
	{
		const uint32_t iPlayMs = m_pChain->clips[m_iChainStep].playMs;
		const f32_t fTicksPerSecond =
			m_pBodyModel->Get_AnimationTickPerSecond(iAnimation);
		if (0u != iPlayMs && std::isfinite(fTicksPerSecond) &&
			fTicksPerSecond > 0.f)
		{
			fLimit = (std::min)(fDuration,
				static_cast<f32_t>(iPlayMs) * 0.001f * fTicksPerSecond);
		}
	}

	/* Same test CAnimation uses to report a non-looping clip as done; the track
	is left past the end rather than clamped. */
	return fPosition >= fLimit;
}

bool_t CCharacter::Play_Skill(
	int32_t iSkillId,
	bool_t isCombo)
{
	if (nullptr != m_pChain)
		return false;

	/* The document has exactly one row per PlayerSkills definition, so a kind
	mismatch is a stale/invalid presentation contract rather than a tripod fallback. */
	const CLIP_CHAIN* pPick = nullptr;
	for (const CLIP_CHAIN& chain : m_Chains)
	{
		if (chain.iSkillId != iSkillId || chain.isServerStaged != isCombo)
			continue;
		pPick = &chain;
		break;
	}

	if (nullptr == pPick || !Start_Clip(pPick->clips[0]))
		return false;

	m_pChain = pPick;
	m_iChainStep = 0;
	return true;
}

void CCharacter::Update_Chain()
{
	if (nullptr == m_pChain || !Is_ClipFinished())
		return;

	/* A combo holds on its last clip until the server confirms the next stage.
	Every other mode keeps running to the end by itself. */
	if (m_pChain->isServerStaged)
		return;

	/* A user-authored presentation clip can be shorter than the Server action.
	Hold its final pose until the replicated action becomes NONE; otherwise the
	client would return to locomotion before the authoritative duration ends. */
	if (m_iChainStep + 1 >= static_cast<int32_t>(m_pChain->clips.size()))
		return;

	++m_iChainStep;
	Start_Clip(m_pChain->clips[m_iChainStep]);
}

void CCharacter::Update_NetworkTransform(f32_t fTimeDelta)
{
	if (!m_hasNetworkState ||
		nullptr == m_pTransformCom)
	{
		return;
	}
	//current pos
	const vector_t current =
		m_pTransformCom->Get_State(STATE::POSITION);
	//network에서 받은 target pos와의 거리
	const vector_t target = XMVectorSet(
		m_vNetworkTargetPosition.x,
		m_vNetworkTargetPosition.y,
		m_vNetworkTargetPosition.z,
		1.f);
	//current와 target blending
	const f32_t blend = (std::min)(1.f, 12.f * fTimeDelta);

	vector_t next = XMVectorLerp(
		current,
		target,
		blend);

	const vector_t remaining = target - next;

	if (XMVectorGetX(XMVector3LengthSq(remaining)) < 0.0001f)
	{
		next = target;
	}

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSetW(next, 1.f));

	//G3에서는 위치만 보간하고, Yaw는 서버 최신값을 즉시 반영
	m_pTransformCom->Rotation(
		0.f,
		m_fNetworkTargetYawDegrees,
		0.f);
}

shared_ptr<CModel> CCharacter::Get_BodyModel() const
{
	return m_pBodyModel;
}

void CCharacter::Set_Position(fvector_t vPosition)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPosition);
}

bool_t CCharacter::Apply_NetworkState(const float3_t& position, f32_t yawDegrees, bool_t isMoving)
{
	//client replication이 서버 상태를 character 표현 상태로 전달하는 public 함수이다.
	//character는 S2C_WORLD_SNAPSHOT을 직접 받지 않는다.
	//transform 존재 검사 -> position / yaw 유한성 검사 -> network target position 저장
	//network target yaw 저장 -> m_hasNetworkstate = true -> set locomotion(ismoving)
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	//network state apply!
	m_hasNetworkState = true;
	m_vNetworkTargetPosition = position;
	m_fNetworkTargetYawDegrees = yawDegrees;

	Set_Locomotion(isMoving);
	return true;
}

bool_t CCharacter::Advance_ComboStage(const std::uint8_t comboStage)
{
	if (nullptr == m_pChain || 0u == comboStage)
		return false;
	const int32_t step = static_cast<int32_t>(comboStage) - 1;
	if (step >= static_cast<int32_t>(m_pChain->clips.size()))
		return false;
	m_iChainStep = step;
	return Start_Clip(m_pChain->clips[step]);
}

bool_t CCharacter::Apply_NetworkAction(
	const LostArk::Shared::PLAYER_ACTION_STATE action,
	const LostArk::Shared::SKILL_ID skillId,
	const std::uint32_t actionStartTick,
	const std::uint8_t comboStage)
{
	using namespace LostArk::Shared;
	if (static_cast<std::uint8_t>(action) >=
		static_cast<std::uint8_t>(PLAYER_ACTION_STATE::END))
	{
		return false;
	}
	if (PLAYER_ACTION_STATE::SKILL == action)
	{
		if (INVALID_SKILL_ID == skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick)
		{
			return true;
		}
		/* A newer authoritative edge replaces an ACTIVE action even when the NONE
		snapshot between them was dropped. Clearing before the pending document
		commit also makes Animation Tool live reload pointer-safe. */
		m_pChain = nullptr;
		m_iChainStep = 0;
		Commit_PendingClipChains();
		m_iCurrentEffectSkillId = skillId;
		Reset_EffectCueCursor(actionStartTick);

		bool_t presented = Play_Skill(
			static_cast<int32_t>(skillId),
			comboStage > 0u);
		if (presented && comboStage > 1u)
			presented = Advance_ComboStage(comboStage);
		if (!presented)
		{
			/* Presentation data is not network authority. Record this edge once and
			keep transform/HUD/other entities consuming the snapshot. */
			OutputDebugStringA((
				"Character skill presentation unavailable for skill " +
				std::to_string(skillId) + " at action tick " +
				std::to_string(actionStartTick) + "\n").c_str());
			m_pChain = nullptr;
			m_iChainStep = 0;
			Set_Animation(
				m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
				true);
		}
		Spawn_FallbackEffect(skillId);
		m_iLastNetworkActionStartTick = actionStartTick;
	}
	else if (PLAYER_ACTION_STATE::TRIGGER_MOVE == action)
	{
		if (INVALID_SKILL_ID != skillId || 0u == actionStartTick)
			return false;
		if (m_eNetworkAction == action &&
			m_iLastNetworkActionStartTick == actionStartTick)
		{
			return true;
		}
		if (PLAYER_ACTION_STATE::SKILL == m_eNetworkAction)
		{
			m_pChain = nullptr;
			m_iChainStep = 0;
			Commit_PendingClipChains();
			Set_Animation(CHARACTER_ANIM::RUN, true);
		}
		m_iLastNetworkActionStartTick = actionStartTick;
	}
	else if (PLAYER_ACTION_STATE::DEAD == action)
	{
		m_pChain = nullptr;
		m_iChainStep = 0;
		Commit_PendingClipChains();
		Set_Animation(CHARACTER_ANIM::DEAD, false);
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
	}
	else if (PLAYER_ACTION_STATE::SKILL == m_eNetworkAction)
	{
		m_pChain = nullptr;
		m_iChainStep = 0;
		Commit_PendingClipChains();
		Set_Animation(
			m_isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
			true);
		m_iCurrentEffectSkillId = INVALID_SKILL_ID;
		m_iEffectActionStartTick = 0u;
	}
	m_eNetworkAction = action;
	return true;
}

void CCharacter::Apply_NetworkStance(const LostArk::Shared::PLAYER_STANCE_ID stance)
{
	m_eStance = stance;
	for (uint32_t i = 0; i < m_pSpec->iNumWeapons; ++i)
	{
		const WEAPON_PART_SPEC& weapon = m_pSpec->pWeapons[i];
		Set_PartVisible(
			weapon.pPartTag,
			LostArk::Shared::PLAYER_STANCE_ID::NONE == weapon.eRequiredStance ||
				weapon.eRequiredStance == stance);
	}
}

void CCharacter::Set_PartVisible(const tchar_t* pPartTag, const bool_t isVisible)
{
	const auto pPart = dynamic_cast<CPart_Equipment*>(
		__super::Find_PartObject(pPartTag));
	if (nullptr != pPart)
		pPart->Set_Visible(isVisible);
}

bool_t CCharacter::Set_Animation(CHARACTER_ANIM eAnim, bool_t isLoop)
{
	if (eAnim >= CHARACTER_ANIM::END)
		return false;
	return Set_Animation(m_pSpec->AnimationClips[ETOUI(eAnim)], isLoop);
}

bool_t CCharacter::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == m_pBodyModel || nullptr == pClipName)
		return false;
	m_pBodyModel->Set_AnimationSpeed(1.f);
	return m_pBodyModel->Set_Animation(
		pClipName,
		isLoop,
		CLIP_BLEND_SECONDS);
}

PATH_RESULT_CODE CCharacter::Request_Move(fvector_t vGoalPosition)
{
	//유효하지 않은 Grid return 예외처리
	if (nullptr == m_pNavigationCom || nullptr == m_pTransformCom)
		return PATH_RESULT_CODE::INVALID_GRID;
	//PathFollower 객체에게 Path 요청
	const PATH_RESULT_CODE eResult = m_PathFollower.Request_Path(
		m_pNavigationCom,
		m_pTransformCom->Get_State(STATE::POSITION),
		vGoalPosition);

	if (PATH_RESULT_CODE::SUCCESS == eResult)
		Set_Locomotion(m_PathFollower.Has_Path());

	return eResult;
}

void CCharacter::Cancel_Move()
{
	m_PathFollower.Cancel();
	Set_Locomotion(false);
}

void CCharacter::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CCharacter::Update(f32_t fTimeDelta)
{
	//network state -> apply snapshot, !networkstate -> pathfinding
	if (m_hasNetworkState)
	{
		Update_NetworkTransform(fTimeDelta);
	}
	else
	{
		m_PathFollower.Update(
			m_pTransformCom,
			m_fMoveSpeed,
			fTimeDelta);

		Set_Locomotion(m_PathFollower.Has_Path());
	}

	//Set_Locomotion(m_PathFollower.Has_Path());

	/* A running chain owns the clip until it ends, so it advances before the logic
	gets a say and Is_PlayingSkill() is already correct when the logic reads it. */
	Update_Chain();

	/* Class code may only update presentation. Input and gameplay commands are
	owned by PlayerController and its command sink. */
	if (m_isLocallyControlled && nullptr != m_pLogic)
		m_pLogic->Update_Presentation(*this, fTimeDelta);

	__super::Update(fTimeDelta);
	if (nullptr != m_pColliderCom)
		m_pColliderCom->Update(XMLoadFloat4x4(
			m_pTransformCom->Get_WorldMatrixPtr()));
	Update_EffectCues();
}

void CCharacter::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

#ifdef _DEBUG
	if (m_isNavigationDebugVisible && nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
	if (m_isNavigationDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CCharacter::Render()
{
	return S_OK;
}

HRESULT CCharacter::Ready_Components()
{
	using namespace LostArk::Shared::WorldCollision;
	CBounding_OBB::BOUNDING_OBB_DESC colliderDesc{};
	colliderDesc.vCenter = float3_t(0.f, PLAYER_CENTER_OFFSET_Y, 0.f);
	colliderDesc.vSize = float3_t(
		PLAYER_HALF_EXTENT_X * 2.f,
		PLAYER_HALF_EXTENT_Y * 2.f,
		PLAYER_HALF_EXTENT_Z * 2.f);
	if (FAILED(__super::Add_Component(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_Component_Collider_Player"),
		TEXT("Com_Collider"),
		m_pColliderCom,
		&colliderDesc)))
	{
		return E_FAIL;
	}
	m_pColliderCom->Update(XMLoadFloat4x4(
		m_pTransformCom->Get_WorldMatrixPtr()));

	if (m_strNavigationPrototypeTag.empty())
		return S_OK;

	return __super::Add_Component(
		m_iPrototypeLevelIndex,
		m_strNavigationPrototypeTag,
		TEXT("Com_Navigation"),
		m_pNavigationCom);
}

HRESULT CCharacter::Ready_PartObjects()
{
	/* Part tags are ordered because CContainerObject keeps parts in a std::map and
	updates them in tag order. The body has to run first: socketed parts read its
	bone matrices during their own Update. */
	CPart_Body::PART_BODY_DESC bodyDesc{};
	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	bodyDesc.strModelTag = m_pSpec->pBodyModelTag;
	bodyDesc.strShaderTag = m_pSpec->pShaderTag;
	bodyDesc.iHiddenMeshMask = m_pSpec->iBodyHiddenMeshMask;
	bodyDesc.pInitialAnimation = m_pSpec->AnimationClips[ETOUI(CHARACTER_ANIM::IDLE)];

	if (FAILED(__super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Part_Body"),
		TEXT("Part_00_Body"),
		&bodyDesc)))
		return E_FAIL;

	m_pBodyModel = dynamic_pointer_cast<CModel>(
		__super::Get_Component(TEXT("Part_00_Body"), TEXT("Com_Model")));
	if (nullptr == m_pBodyModel)
		return E_FAIL;

	m_pBodyModel->Enable_RootMotionSuppression(
		ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);

	/* Skinned equipment: no socket, it borrows the body's bone palette. */
	for (uint32_t i = 0; i < m_pSpec->iNumEquipment; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC equipmentDesc{};
		equipmentDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		equipmentDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		equipmentDesc.strModelTag = m_pSpec->pEquipment[i].pModelTag;
		equipmentDesc.strShaderTag = m_pSpec->pShaderTag;
		equipmentDesc.iHiddenMeshMask = m_pSpec->pEquipment[i].iHiddenMeshMask;
		equipmentDesc.pSkeletonModel = m_pBodyModel;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			m_pSpec->pEquipment[i].pPartTag,
			&equipmentDesc)))
			return E_FAIL;
	}

	/* The weapon is the same part class in socket mode. It gets its own class once
	hit windows and trails need somewhere to live. */
	for (uint32_t i = 0; i < m_pSpec->iNumWeapons; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
		weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		weaponDesc.strModelTag = m_pSpec->pWeapons[i].pModelTag;
		weaponDesc.strShaderTag = m_pSpec->pWeaponShaderTag;
		weaponDesc.pSkeletonModel = m_pBodyModel;
		weaponDesc.pSocketBoneName = m_pSpec->pWeapons[i].pSocketBone;
		weaponDesc.fSocketYawDegrees = m_pSpec->pWeapons[i].fSocketYawDegrees;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			m_pSpec->pWeapons[i].pPartTag,
			&weaponDesc)))
			return E_FAIL;
	}

	return S_OK;
}

void CCharacter::Set_Locomotion(bool_t isMoving)
{
	if (m_isMoving == isMoving)
		return;

	m_isMoving = isMoving;
	if (Is_PlayingSkill())
		return;

	Set_Animation(
		isMoving ? CHARACTER_ANIM::RUN : CHARACTER_ANIM::IDLE,
		true);
}

unique_ptr<CCharacter> CCharacter::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CCharacter>(new CCharacter(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][Character] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CCharacter::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCharacter>(new CCharacter(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][Character] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
