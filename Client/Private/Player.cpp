#include "Player.h"

#include "Body_Player.h"
#include "Weapon.h"
#include "Explosion.h"
#include "GameInstance.h"

CPlayer::CPlayer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CContainerObject { pDevice, pContext }
{
}

CPlayer::~CPlayer()
{
}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	CContainerObject::CONTAINEROBJECT_DESC		Desc{};
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = 90.f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;



	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	return S_OK;
}

void CPlayer::Priority_Update(f32_t fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}

void CPlayer::Update(f32_t fTimeDelta)
{
	if (GetKeyState(VK_DOWN) & 0x8000)
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (GetKeyState(VK_LEFT) & 0x8000)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);
	}

	if (GetKeyState(VK_RIGHT) & 0x8000)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
	}


	if (GetKeyState(VK_UP) & 0x8000)
	{
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);

		if (m_iState & PLAYER_STATE::IDLE)
			m_iState ^= PLAYER_STATE::IDLE;

		m_iState |= PLAYER_STATE::WALK;
	}
	else
	{
		if(m_iState & PLAYER_STATE::WALK)
			m_iState ^= PLAYER_STATE::WALK;

		m_iState |= PLAYER_STATE::IDLE;
	}

	float4_t		vPickPos = {};
	if (GetKeyState(VK_LBUTTON) & 0x8000 && 
		true == CGameInstance::Get().Picking(vPickPos))
	{
		m_pTransformCom->Set_State(STATE::POSITION, XMLoadFloat4(&vPickPos));
	}

	// m_pNavigationCom->SetUp_OnNavigation(m_pTransformCom);
	m_pColliderCom->Update(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

	__super::Update(fTimeDelta);
}

void CPlayer::Late_Update(f32_t fTimeDelta)
{


	__super::Late_Update(fTimeDelta);
	CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND, static_pointer_cast<CGameObject>(shared_from_this()));

#ifdef _DEBUG
	CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
	CGameInstance::Get().Add_DebugComponent(m_pNavigationCom);
#endif
}

HRESULT CPlayer::Render()
{


	return S_OK;
}

HRESULT CPlayer::Ready_PartObjects()
{
	CBody_Player::BODY_DESC			Body_Desc{};
	Body_Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	Body_Desc.pParentState = &m_iState;

	if (FAILED(__super::Add_PartObject(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Body_Player"),
		TEXT("Part_Body"), &Body_Desc)))
		return E_FAIL;

	CWeapon::WEAPON_DESC		Weapon_Desc{};
	Weapon_Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	Weapon_Desc.pParentState = &m_iState;
	Weapon_Desc.pSocketModel = static_pointer_cast<CModel>(m_PartObjects[TEXT("Part_Body")]->Get_Component(TEXT("Com_Model")));

	if (FAILED(__super::Add_PartObject(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Weapon"),
		TEXT("Part_Weapon"), &Weapon_Desc)))
		return E_FAIL;

	CExplosion::EFFECT_DESC		Effect_Desc{};
	Effect_Desc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
	Effect_Desc.pParentState = &m_iState;
	Effect_Desc.pSocketModel = static_pointer_cast<CModel>(m_PartObjects[TEXT("Part_Body")]->Get_Component(TEXT("Com_Model")));

	if (FAILED(__super::Add_PartObject(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_GameObject_Explosion"),
		TEXT("Part_Effect"), &Effect_Desc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CPlayer::Ready_Components()
{
	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC			NaviDesc{};
	NaviDesc.iStartCellIndex = 0;
	NaviDesc.pTransformCom = m_pTransformCom;

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_TerrainNavigation"),
		TEXT("Com_Navigation"), m_pNavigationCom, &NaviDesc)))
		return E_FAIL;

	/* For.Com_Collider_AABB */
	CBounding_AABB::BOUNDING_AABB_DESC			AABBDesc{};	
	AABBDesc.vSize = float3_t(0.8f, 1.2f, 0.8f);
	AABBDesc.vCenter = float3_t(0.f, AABBDesc.vSize.y * 0.5f, 0.f);	

	if (FAILED(__super::Add_Component(ETOUI(LEVEL::GAMEPLAY), TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider_AABB"), m_pColliderCom, &AABBDesc)))
		return E_FAIL;

	


	return S_OK;
}

unique_ptr<CPlayer> CPlayer::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto	pInstance = unique_ptr<CPlayer>(new CPlayer(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CMonster");

	return move(pInstance);
}

shared_ptr<CPrototype> CPlayer::Clone(void* pArg)
{
	auto	pInstance = shared_ptr<CPlayer>(new CPlayer(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CPlayer");

	return pInstance;
}