#include "Npc.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"

CNpc::CNpc(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject{ pDevice, pContext }
{
}

CNpc::~CNpc()
{
}

HRESULT CNpc::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNpc::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const NPC_DESC* pDesc = static_cast<const NPC_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->vPosition.x, pDesc->vPosition.y, pDesc->vPosition.z, 1.f));
	if (0.f != pDesc->fYawDegree)
		m_pTransformCom->Rotation(0.f, pDesc->fYawDegree, 0.f);

	/* With no animation set the bone palette is never filled, so every vertex
	collapses onto the origin and the NPC simply vanishes -- a wrong clip name
	looks exactly like a failed load. Fall back to the model's first clip so a
	bad name is visible as a wrong pose instead of nothing at all. */
	if (nullptr == pDesc->pIdleClip ||
		!m_pModelCom->Set_Animation(pDesc->pIdleClip, pDesc->isLoop))
	{
		if (0 == m_pModelCom->Get_NumAnimations())
			return E_FAIL;
		m_pModelCom->Set_Animation(0u, pDesc->isLoop);
	}

	return S_OK;
}

bool_t CNpc::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName || nullptr == m_pModelCom)
		return false;
	return m_pModelCom->Set_Animation(pClipName, isLoop);
}

void CNpc::Priority_Update(f32_t fTimeDelta)
{
}

void CNpc::Update(f32_t fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
}

void CNpc::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CNpc::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const uint32_t iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (uint32_t i = 0; i < iNumMeshes; ++i)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(i, aiTextureType_NORMALS) ? 1u : 0u;

		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormalTexture, sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture && FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0))) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CNpc::Ready_Components(const NPC_DESC* pDesc)
{
	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strShaderTag,
		TEXT("Com_Shader"),
		m_pShaderCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(
		pDesc->iPrototypeLevelIndex,
		pDesc->strModelTag,
		TEXT("Com_Model"),
		m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CNpc::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CNpc> CNpc::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CNpc>(new CNpc(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype()))
		MSG_BOX("Failed to Created : CNpc");

	return move(pInstance);
}

shared_ptr<CPrototype> CNpc::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CNpc>(new CNpc(*this));

	if (FAILED(pInstance->Initialize(pArg)))
		MSG_BOX("Failed to Cloned : CNpc");

	return pInstance;
}
