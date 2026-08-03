#include "Valtan.h"

#include "Body_Valtan.h"
#include "GameInstance.h"
#include "Model.h"
#include "Navigation.h"

#include "Part_Equipment.h"
#include "Transform.h"

#include <array>

namespace
{
	/* The only pattern whose source order is confirmed. The other att_battle
	groups are not chained here: the cooked .wanim files carry no notify events,
	so their step order and strike frames are still unverified. */
	constexpr array<const char_t*, 3> AXE_COMBO_CLIPS
	{
		"att_battle_2_01",
		"att_battle_2_02",
		"att_battle_2_03",
	};

	constexpr f32_t REPATH_INTERVAL = 0.35f;
	constexpr f32_t REPATH_GOAL_DELTA_SQ = 0.25f;

	constexpr const char_t* CLIP_IDLE = "idle_battle_1";
	constexpr const char_t* CLIP_RUN = "run_battle_1";
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
	m_pTargetTransform = desc.pTargetTransform;

	if (nullptr != desc.pNavigationPrototypeTag)
		m_strNavigationPrototypeTag =
			desc.pNavigationPrototypeTag;

	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;

	m_pTransformCom->Scale(desc.fScale, desc.fScale, desc.fScale);

	m_pTransformCom->Set_State(
		STATE::POSITION,
		XMVectorSet(
			desc.vPosition.x,
			desc.vPosition.y,
			desc.vPosition.z,
			1.f));

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return Ready_PartObjects();
}

void CValtan::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CValtan::Update(f32_t fTimeDelta)
{
	if (m_fPatternCooldown > 0.f)
	{
		m_fPatternCooldown -= fTimeDelta;
		if (m_fPatternCooldown < 0.f)
			m_fPatternCooldown = 0.f;
	}

	/* A running pattern owns the behaviour until its animation chain ends, so
	the target may walk away without cancelling the combo. */
	if (VALTAN_STATE::PATTERN == m_eState)
	{
		Update_Pattern();
		__super::Update(fTimeDelta);
		return;
	}

	if (nullptr == m_pNavigationCom)
	{
		Stop_Chase();
		Set_LocomotionState(false);
		__super::Update(fTimeDelta);
		return;
	}

	shared_ptr<CTransform> pTargetTransform =
		m_pTargetTransform.lock();
	if (nullptr == pTargetTransform)
	{
		Stop_Chase();
		Set_LocomotionState(false);
		__super::Update(fTimeDelta);
		return;
	}

	const vector_t vPosition =
		m_pTransformCom->Get_State(STATE::POSITION);
	const vector_t vTargetPosition =
		pTargetTransform->Get_State(STATE::POSITION);
	const vector_t vHorizontalOffset =
		XMVectorSetY(vTargetPosition - vPosition, 0.f);
	const f32_t fTargetDistance = XMVectorGetX(
		XMVector3Length(vHorizontalOffset));

	if (m_fPatternCooldown <= 0.f &&
		fTargetDistance <= m_fAttackDistance &&
		Begin_AxeCombo(vTargetPosition))
	{
		__super::Update(fTimeDelta);
		return;
	}

	if (fTargetDistance <= m_fStopDistance)
	{
		Stop_Chase();
		Set_LocomotionState(false);
		__super::Update(fTimeDelta);
		return;
	}

	Update_Chase(fTimeDelta, vTargetPosition);
	__super::Update(fTimeDelta);
}

void CValtan::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

#ifdef _DEBUG
	if (m_isNavigationDebugVisible &&
		nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(
			m_pNavigationCom);
#endif
}

HRESULT CValtan::Render()
{
	return S_OK;
}

HRESULT CValtan::Ready_PartObjects()
{
	CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};
	bodyDesc.pParentMatrix =
		m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;

	if (FAILED(__super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Body_Valtan"),
		TEXT("Part_Body"),
		&bodyDesc)))
		return E_FAIL;

	const auto iter = m_PartObjects.find(
		TEXT("Part_Body"));
	if (iter == m_PartObjects.end())
		return E_FAIL;

	m_pBodyPart =
		dynamic_pointer_cast<CBody_Valtan>(
			iter->second);
	if (nullptr == m_pBodyPart)
		return E_FAIL;

	/* The axe rides a bone of the body rig, so it needs that rig's model and
	visual root rather than the container's own transform. */
	const shared_ptr<CModel> pBodyModel =
		dynamic_pointer_cast<CModel>(
			__super::Get_Component(
				TEXT("Part_Body"),
				TEXT("Com_Model")));

	const shared_ptr<CTransform> pBodyVisualRoot =
		dynamic_pointer_cast<CTransform>(
			__super::Get_Component(
				TEXT("Part_Body"),
				g_strTransformComTag));

	if (nullptr == pBodyModel ||
		nullptr == pBodyVisualRoot)
		return E_FAIL;

	CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};

	weaponDesc.pParentMatrix =
		m_pTransformCom->Get_WorldMatrixPtr();
	weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
	weaponDesc.strModelTag =
		TEXT("Prototype_Component_Model_ValtanWeapon");
	weaponDesc.strShaderTag =
		TEXT("Prototype_Component_Shader_VtxMeshBinary");
	weaponDesc.pSkeletonModel = pBodyModel;
	weaponDesc.pSocketBoneName = "b_wp_r_01";
	weaponDesc.pSocketRootMatrix =
		pBodyVisualRoot->Get_WorldMatrixPtr();

	return __super::Add_PartObject(
		m_iPrototypeLevelIndex,
		TEXT("Prototype_GameObject_Part_Equipment"),
		TEXT("Part_Weapon_R"),
		&weaponDesc);
}

HRESULT CValtan::Ready_Components()
{
	if (m_strNavigationPrototypeTag.empty())
		return S_OK;

	return __super::Add_Component(
		m_iPrototypeLevelIndex,
		m_strNavigationPrototypeTag,
		TEXT("Com_Navigation"),
		m_pNavigationCom);
}

void CValtan::Update_Chase(
	f32_t fTimeDelta,
	fvector_t vTargetPosition)
{
	m_fRepathTime -= fTimeDelta;
	if (m_fRepathTime <= 0.f)
	{
		float3_t vCurrentGoal{};
		XMStoreFloat3(
			&vCurrentGoal,
			vTargetPosition);

		const f32_t fGoalDeltaX =
			vCurrentGoal.x - m_vLastPathGoal.x;
		const f32_t fGoalDeltaZ =
			vCurrentGoal.z - m_vLastPathGoal.z;
		const bool_t isNewGoal =
			false == m_hasLastPathGoal ||
			fGoalDeltaX * fGoalDeltaX +
				fGoalDeltaZ * fGoalDeltaZ >=
				REPATH_GOAL_DELTA_SQ ||
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
				Stop_Chase();
			}
		}

		m_fRepathTime = REPATH_INTERVAL;
	}

	m_PathFollower.Update(
		m_pTransformCom,
		m_fMoveSpeed,
		fTimeDelta);
	Set_LocomotionState(
		m_PathFollower.Has_Path());
}

PATH_RESULT_CODE CValtan::Request_PathToTarget(
	fvector_t vGoalPosition)
{
	if (nullptr == m_pNavigationCom ||
		nullptr == m_pTransformCom)
		return PATH_RESULT_CODE::INVALID_GRID;

	return m_PathFollower.Request_Path(
		m_pNavigationCom,
		m_pTransformCom->Get_State(
			STATE::POSITION),
		vGoalPosition);
}

void CValtan::Stop_Chase()
{
	m_PathFollower.Cancel();
	m_hasLastPathGoal = false;
	m_fRepathTime = 0.f;
}

void CValtan::Set_LocomotionState(bool_t isChasing)
{
	if (VALTAN_STATE::PATTERN == m_eState)
		return;

	const VALTAN_STATE eNextState =
		isChasing ?
		VALTAN_STATE::CHASE :
		VALTAN_STATE::IDLE;
	if (m_eState == eNextState)
		return;

	const char_t* pAnimationName =
		isChasing ? CLIP_RUN : CLIP_IDLE;

	if (nullptr == m_pBodyPart ||
		false == m_pBodyPart->Set_Animation(
			pAnimationName,
			true))
		return;

	m_eState = eNextState;
}

bool_t CValtan::Begin_AxeCombo(
	fvector_t vTargetPosition)
{
	if (nullptr == m_pBodyPart ||
		false == m_pBodyPart->Set_Animation(
			AXE_COMBO_CLIPS[0],
			false))
		return false;

	Stop_Chase();

	/* Aim once at the start. The combo does not re-aim, so the player can read
	it and step out. */
	const vector_t vPosition =
		m_pTransformCom->Get_State(STATE::POSITION);
	const vector_t vHorizontalTarget =
		XMVectorSetY(
			vTargetPosition,
			XMVectorGetY(vPosition));

	if (XMVectorGetX(XMVector3LengthSq(
		vHorizontalTarget - vPosition)) > 0.000001f)
		m_pTransformCom->LookAt(vHorizontalTarget);

	m_eState = VALTAN_STATE::PATTERN;
	m_ePattern = VALTAN_PATTERN::AXE_COMBO;
	m_iPatternStep = 0;
	return true;
}

void CValtan::Update_Pattern()
{
	/* A missing clip ends the pattern instead of holding PATTERN forever. */
	if (VALTAN_PATTERN::AXE_COMBO != m_ePattern ||
		nullptr == m_pBodyPart)
	{
		Finish_Pattern();
		return;
	}

	if (false ==
		m_pBodyPart->Is_AnimationFinished())
		return;

	++m_iPatternStep;
	if (m_iPatternStep >= AXE_COMBO_CLIPS.size())
	{
		Finish_Pattern();
		return;
	}

	if (false == Play_AxeComboStep())
		Finish_Pattern();
}

bool_t CValtan::Play_AxeComboStep()
{
	if (nullptr == m_pBodyPart ||
		m_iPatternStep >= AXE_COMBO_CLIPS.size())
		return false;

	return m_pBodyPart->Set_Animation(
		AXE_COMBO_CLIPS[m_iPatternStep],
		false);
}

void CValtan::Finish_Pattern()
{
	m_ePattern = VALTAN_PATTERN::NONE;
	m_iPatternStep = 0;
	m_fPatternCooldown = m_fPatternInterval;
	m_eState = VALTAN_STATE::IDLE;

	if (nullptr != m_pBodyPart)
		m_pBodyPart->Set_Animation(
			CLIP_IDLE,
			true);
}

unique_ptr<CValtan> CValtan::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CValtan>(
		new CValtan(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CValtan");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CPrototype> CValtan::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CValtan>(
		new CValtan(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CValtan");
		return nullptr;
	}

	return pInstance;
}
