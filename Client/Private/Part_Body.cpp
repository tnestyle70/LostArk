#include "Part_Body.h"

#include "GameInstance.h"

CPart_Body::CPart_Body(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Body::~CPart_Body()
{
}

HRESULT CPart_Body::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Body::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_BODY_DESC*>(pArg);
	m_iHiddenMeshMask = pDesc->iHiddenMeshMask;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	/* Fall back to the first clip so a class with a mistyped name still animates
	instead of standing in its bind pose. */
	if (nullptr == pDesc->pInitialAnimation ||
		!m_pModelCom->Set_Animation(pDesc->pInitialAnimation, true))
		m_pModelCom->Set_Animation(0u, true);

	return S_OK;
}

bool_t CPart_Body::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName)
		return false;
	return m_pModelCom->Set_Animation(pClipName, isLoop);
}

void CPart_Body::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Body::Update(f32_t fTimeDelta)
{
	/* The body drives the clock every frame; the logic only picks the clip. Parts
	that borrow this palette read it at render time, so they need no ordering. */
	m_pModelCom->Play_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CPart_Body::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CPart_Body::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (0 != (m_iHiddenMeshMask & (1u << i)))
			continue;

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

HRESULT CPart_Body::Ready_Components(const PART_BODY_DESC* pDesc)
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

HRESULT CPart_Body::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CPart_Body> CPart_Body::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Body>(new CPart_Body(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartBody] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Body::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Body>(new CPart_Body(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartBody] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
