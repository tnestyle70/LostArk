#include "Body_Valtan.h"

#include "GameInstance.h"
#include "Model.h"

CBody_Valtan::CBody_Valtan(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CBody_Valtan::~CBody_Valtan()
{
}

bool_t CBody_Valtan::Set_Animation(
	const char_t* pAnimationName,
	bool_t isLoop)
{
	if (nullptr == m_pModelCom ||
		nullptr == pAnimationName ||
		false == m_pModelCom->Set_Animation(
			pAnimationName,
			isLoop))
		return false;

	const uint32_t iAnimationIndex =
		m_pModelCom->Get_CurrentAnimIndex();
	if (false == m_pModelCom->Set_AnimTrackPosition(
		iAnimationIndex,
		0.f))
		return false;

	m_isAnimationFinished = false;
	return true;
}

HRESULT CBody_Valtan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBody_Valtan::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const uint32_t iPrototypeLevelIndex =
		static_cast<BODY_VALTAN_DESC*>(pArg)->iPrototypeLevelIndex;

	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components(iPrototypeLevelIndex)))
		return E_FAIL;

	/* The source model faces a different forward axis than Engine LOOK(+Z). */
	m_pTransformCom->Rotation(0.f, -90.f, 0.f);

	if (false == Set_Animation("idle_battle_1", true))
		return E_FAIL;

	return S_OK;
}

void CBody_Valtan::Priority_Update(f32_t fTimeDelta)
{
}

void CBody_Valtan::Update(f32_t fTimeDelta)
{
	/* Played every frame regardless of what the boss is doing. The behaviour
	state lives in CValtan and is not read here. */
	m_isAnimationFinished =
		m_pModelCom->Play_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(
			m_pTransformCom->Get_WorldMatrixPtr()));
}

void CBody_Valtan::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(
			shared_from_this()));
}

HRESULT CBody_Valtan::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0;
		i < m_pModelCom->Get_NumMeshes();
		++i)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(
				i,
				aiTextureType_NORMALS) ?
			1u : 0u;

		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom,
			"g_DiffuseTexture",
			i,
			aiTextureType_DIFFUSE,
			0)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture",
				&hasNormalTexture,
				sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture &&
				FAILED(m_pModelCom->Bind_Material(
					m_pShaderCom,
					"g_NormalTexture",
					i,
					aiTextureType_NORMALS,
					0))) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom,
				"g_BoneMatrices",
				i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CBody_Valtan::Ready_Components(uint32_t iPrototypeLevelIndex)
{
	if (FAILED(__super::Add_Component(
		iPrototypeLevelIndex,
		TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		iPrototypeLevelIndex,
		TEXT("Prototype_Component_Model_Valtan"),
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBody_Valtan::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(
		m_pShaderCom,
		"g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom,
			"g_ViewMatrix",
			D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom,
			"g_ProjMatrix",
			D3DTS::PROJ)))
		return E_FAIL;

	return S_OK;
}

unique_ptr<CBody_Valtan> CBody_Valtan::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CBody_Valtan>(
		new CBody_Valtan(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBody_Valtan");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CPrototype> CBody_Valtan::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CBody_Valtan>(
		new CBody_Valtan(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBody_Valtan");
		return nullptr;
	}

	return pInstance;
}
