#include "Valtan.h"

#include "ActionPresentationTimeline.h"
#include "ActorCatalog.h"
#include "AnimationSkillBindingDocument.h"
#include "Body_Valtan.h"
#include "Collider.h"
#include "GameInstance.h"
#include "Model.h"
#include "Navigation.h"

#include "Part_Equipment.h"
#include "Transform.h"

#include <cmath>

namespace
{
	constexpr f32_t VALTAN_SERVER_TICK_HZ = 30.f;
	constexpr f32_t VALTAN_PRESENTATION_SEEK_EPSILON_SECONDS = 1.f / 120.f;
}

CValtan::CValtan(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CValtan::~CValtan()
{
}

HRESULT CValtan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CValtan::Initialize(void* pArg)
{
	VALTAN_DESC desc{};
	desc.fSpeedPerSec = 3.f;
	desc.fRotationPerSec = 180.f;
	desc.fScale = 1.5f;
	if (nullptr != pArg)
	{
		desc = *static_cast<VALTAN_DESC*>(pArg);
		if (desc.fSpeedPerSec <= 0.f)
			desc.fSpeedPerSec = 3.f;
		if (desc.fRotationPerSec <= 0.f)
			desc.fRotationPerSec = 180.f;
		if (desc.fScale <= 0.f)
			desc.fScale = 1.5f;
	}

	m_fMoveSpeed = desc.fSpeedPerSec;
	m_iPrototypeLevelIndex = desc.iPrototypeLevelIndex;
	m_isServerAuthoritative = desc.isServerAuthoritative;
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
	return S_OK;
}

void CValtan::Load_PatternBindings()
{
	m_PatternClipByActionId.clear();
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
	std::unordered_map<std::string, std::string> staged;
	for (const Client::BOSS_PATTERN_ANIMATION_BINDING& binding :
		document.Bindings)
	{
		staged.emplace(binding.strActionId, binding.strClipName);
	}
	m_PatternClipByActionId = std::move(staged);
}

void CValtan::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CValtan::Update(f32_t fTimeDelta)
{
	if (m_isServerAuthoritative)
	{
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
#ifdef _DEBUG
	if (m_isNavigationDebugVisible && nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
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

	return __super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Part_Equipment"),
		WEAPON_PART_TAG,
		&weaponDesc);
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
			isChasing ? "run_battle_1" : "idle_battle_1",
			true);
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
	if (0u != iServerTick && 0u != m_iLastServerTick &&
		iServerTick != m_iLastServerTick &&
		!Client::CActionPresentationTimeline::Is_ForwardTick(
			iServerTick, m_iLastServerTick))
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
		const std::string* pClip = nullptr;
		switch (action)
		{
		case LostArk::Shared::WORLD_ENTITY_ACTION::IDLE:
			pClip = &pActor->presentationClips.idle;
			break;
		case LostArk::Shared::WORLD_ENTITY_ACTION::CHASE:
			pClip = &pActor->presentationClips.chase;
			break;
		case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_WINDUP:
			pClip = &pActor->presentationClips.patternWindup;
			break;
		case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_ACTIVE:
			pClip = &pActor->presentationClips.patternActive;
			break;
		case LostArk::Shared::WORLD_ENTITY_ACTION::PATTERN_RECOVERY:
			pClip = &pActor->presentationClips.patternRecovery;
			break;
		case LostArk::Shared::WORLD_ENTITY_ACTION::DEAD:
			pClip = &pActor->presentationClips.dead;
			break;
		default:
			return false;
		}
		if (isPatternState && !actionId.empty())
		{
			const auto bound =
				m_PatternClipByActionId.find(std::string(actionId));
			if (bound != m_PatternClipByActionId.end())
				pClip = &bound->second;
		}
		if (bAnimationEdgeChanged &&
			!m_pBodyModelCom->Start_Animation(pClip->c_str(), true))
		{
			return false;
		}
		if (isPatternState)
		{
			const uint32_t iAnimation =
				m_pBodyModelCom->Get_CurrentAnimIndex();
			f32_t fTrackPosition = 0.f;
			f32_t fTrackDuration = 0.f;
			const f32_t fTicksPerSecond =
				m_pBodyModelCom->Get_AnimationTickPerSecond(iAnimation);
			if (!m_pBodyModelCom->Get_AnimationProgress(
					iAnimation, fTrackPosition, fTrackDuration) ||
				!std::isfinite(fTicksPerSecond) || fTicksPerSecond <= 0.f ||
				!std::isfinite(fTrackDuration) || fTrackDuration <= 0.f)
			{
				return false;
			}
			const Client::ACTION_PRESENTATION_CLIP_TIMING Timing{
				fTrackDuration / fTicksPerSecond, 0u, 1.f, true };
			Client::ACTION_PRESENTATION_SAMPLE Sample;
			if (!Client::CActionPresentationTimeline::Resolve_Sample(
					std::span<const Client::ACTION_PRESENTATION_CLIP_TIMING>(
						&Timing, 1u), fActionAgeSeconds, Sample) ||
				!m_pBodyModelCom->Set_AnimTrackPosition(
					iAnimation,
					Sample.fClipSourceTimeSeconds * fTicksPerSecond))
			{
				return false;
			}
			m_pBodyModelCom->Play_Animation(0.f);
		}
	}

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(position.x, position.y, position.z, 1.f));
	m_pTransformCom->Rotation(0.f, yawDegrees, 0.f);
	if (nullptr != m_pColliderCom)
	{
		m_pColliderCom->Update(XMMatrixTranslationFromVector(
			m_pTransformCom->Get_State(STATE::POSITION)));
	}
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
	}
	else
	{
		m_iServerActionStartTick = 0u;
		m_iServerPatternStageIndex = 0u;
		m_fServerActionAgeSeconds = 0.f;
	}
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
