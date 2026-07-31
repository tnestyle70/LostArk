#include "Character.h"

#include "GameInstance.h"
#include "Navigation.h"
#include "Part_Body.h"
#include "Part_Equipment.h"

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

	if (nullptr != pDesc->pNavigationPrototypeTag)
		m_strNavigationPrototypeTag =
			pDesc->pNavigationPrototypeTag;

	if (nullptr == m_pSpec)
		return E_FAIL;

	if (FAILED(__super::Initialize(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z, 1.f));

	if (FAILED(Ready_Components()) ||
		FAILED(Ready_PartObjects()))
		return E_FAIL;

	if (nullptr != m_pSpec->pCreateLogic)
		m_pLogic = m_pSpec->pCreateLogic();

	return S_OK;
}

shared_ptr<CModel> CCharacter::Get_BodyModel() const
{
	return m_pBodyModel;
}

void CCharacter::Set_Position(fvector_t vPosition)
{
	m_pTransformCom->Set_State(STATE::POSITION, vPosition);
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
	return m_pBodyModel->Set_Animation(pClipName, isLoop);
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
	m_PathFollower.Update(
		m_pTransformCom,
		m_fMoveSpeed,
		fTimeDelta);
	Set_Locomotion(m_PathFollower.Has_Path());

	/* Class-specific logic stays separate from common navigation locomotion. */
	if (nullptr != m_pLogic)
		m_pLogic->Update(*this, fTimeDelta);

	__super::Update(fTimeDelta);
}

void CCharacter::Late_Update(f32_t fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

#ifdef _DEBUG
	if (m_isNavigationDebugVisible && nullptr != m_pNavigationCom)
		CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
#endif
}

HRESULT CCharacter::Render()
{
	return S_OK;
}

HRESULT CCharacter::Ready_Components()
{
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

	/* Skinned equipment: no socket, it borrows the body's bone palette. */
	for (uint32_t i = 0; i < m_pSpec->iNumEquipment; ++i)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC equipmentDesc{};
		equipmentDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		equipmentDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		equipmentDesc.strModelTag = m_pSpec->pEquipment[i].pModelTag;
		equipmentDesc.strShaderTag = m_pSpec->pShaderTag;
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
	if (nullptr != m_pSpec->pWeaponModelTag)
	{
		CPart_Equipment::PART_EQUIPMENT_DESC weaponDesc{};
		weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
		weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
		weaponDesc.strModelTag = m_pSpec->pWeaponModelTag;
		weaponDesc.strShaderTag = m_pSpec->pWeaponShaderTag;
		weaponDesc.pSkeletonModel = m_pBodyModel;
		weaponDesc.pSocketBoneName = m_pSpec->pWeaponSocketBone;

		if (FAILED(__super::Add_PartObject(
			m_iPrototypeLevelIndex,
			TEXT("Prototype_GameObject_Part_Equipment"),
			TEXT("Part_90_Weapon_R"),
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
		MSG_BOX("Failed to Created : CCharacter");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CCharacter::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CCharacter>(new CCharacter(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCharacter");
		return nullptr;
	}
	return pInstance;
}
