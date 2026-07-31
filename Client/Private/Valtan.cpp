#include "Valtan.h"

#include "Body_Valtan.h"
#include "GameInstance.h"
#include "Model.h"
#include "Navigation.h"

#include "Part_Equipment.h"
#include "Transform.h"

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
	m_pTargetTransform = desc.pTargetTransform;

	if (nullptr != desc.pNavigationPrototypeTag)
		m_strNavigationPrototypeTag = desc.pNavigationPrototypeTag;

	if (FAILED(__super::Initialize(&desc)))
		return E_FAIL;

	m_pTransformCom->Scale(desc.fScale, desc.fScale, desc.fScale);

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(desc.vPosition.x, desc.vPosition.y, desc.vPosition.z, 1.f));

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
#endif
}

HRESULT CValtan::Render()
{
	return S_OK;
}

HRESULT CValtan::Ready_PartObjects()
{
	CBody_Valtan::BODY_VALTAN_DESC bodyDesc{};

	bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	bodyDesc.pParentState = &m_iState;

	if (FAILED(__super::Add_PartObject(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Body_Valtan"),
		TEXT("Part_Body"),
		&bodyDesc)))
		return E_FAIL;

	m_pBodyModelCom =
		dynamic_pointer_cast<CModel>(
			__super::Get_Component(
				TEXT("Part_Body"),
				TEXT("Com_Model")));

	const shared_ptr<CTransform> pBodyVisualRoot =
		dynamic_pointer_cast<CTransform>(
			__super::Get_Component(
				TEXT("Part_Body"),
				g_strTransformComTag));

	if (nullptr == m_pBodyModelCom ||
		nullptr == pBodyVisualRoot)
		return E_FAIL;

	CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};

	weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();

	weaponDesc.iPrototypeLevelIndex = ETOUI(LEVEL::ASSET_TEST);

	weaponDesc.strModelTag = TEXT("Prototype_Component_Model_ValtanWeapon");

	weaponDesc.strShaderTag = TEXT("Prototype_Component_Shader_VtxMeshBinary");

	weaponDesc.pSkeletonModel =
		m_pBodyModelCom;

	weaponDesc.pSocketBoneName =
		"b_wp_r_01";

	weaponDesc.pSocketRootMatrix =
		pBodyVisualRoot->Get_WorldMatrixPtr();

	return __super::Add_PartObject(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_GameObject_Part_Equipment"),
		TEXT("Part_Weapon_R"),
		&weaponDesc);
}

HRESULT CValtan::Ready_Components()
{
	if (m_strNavigationPrototypeTag.empty())
		return S_OK;

	return __super::Add_Component(
		ETOUI(LEVEL::ASSET_TEST),
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

unique_ptr<CValtan> CValtan::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CValtan>(new CValtan(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CValtan");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CValtan::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CValtan>(new CValtan(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CValtan");
		return nullptr;
	}
	return pInstance;
}
