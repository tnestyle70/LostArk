#include "BinaryAssetObject.h"

#include "BinaryAsset/CookedModel.h"
#include "GameInstance.h"
#include "Shader.h"

CBinaryAssetObject::CBinaryAssetObject(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CBinaryAssetObject::~CBinaryAssetObject()
{
}

HRESULT CBinaryAssetObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBinaryAssetObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	auto pDesc = static_cast<BINARY_ASSET_DESC*>(pArg);
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(Ready_Components(pDesc->asset)))
		return E_FAIL;

	m_pTransformCom->Scaling(pDesc->scale, pDesc->scale, pDesc->scale);
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(pDesc->position.x, pDesc->position.y, pDesc->position.z, 1.f));
	return S_OK;
}

void CBinaryAssetObject::Update(f32_t fTimeDelta)
{
	if (nullptr != m_pModel)
		m_pModel->Update_Animation(fTimeDelta);
}

void CBinaryAssetObject::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CBinaryAssetObject::Render()
{
	if (nullptr == m_pModel || FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModel->Get_NumMeshes(); ++i)
	{
		if (FAILED(m_pModel->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", i, COOKED_TEXTURE_SLOT::DIFFUSE)))
			return E_FAIL;
		if (m_pModel->Is_Skinned())
		{
			if (FAILED(m_pModel->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices")))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pModel->Bind_Material(
				m_pShaderCom, "g_NormalTexture", i, COOKED_TEXTURE_SLOT::NORMAL)))
				return E_FAIL;
		}
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
		if (FAILED(m_pModel->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CBinaryAssetObject::Ready_Components(const MODEL_ASSET_LOAD_DESC& assetDesc)
{
	m_pModel = Engine::CCookedModel::Create(m_pDevice, m_pContext, assetDesc);
	if (nullptr == m_pModel)
		return E_FAIL;

	if (m_pModel->Is_Skinned())
	{
		auto pShader = CShader::Create(
			m_pDevice,
			m_pContext,
			TEXT("../Bin/ShaderFiles/Shader_VtxAnimMesh.hlsl"),
			VTXANIMMESH::Elements,
			VTXANIMMESH::iNumElements);
		if (nullptr == pShader)
			return E_FAIL;
		m_pShaderCom = shared_ptr<CShader>(move(pShader));
	}
	else if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"),
		m_pShaderCom)))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CBinaryAssetObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_Transform(m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

unique_ptr<CBinaryAssetObject> CBinaryAssetObject::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CBinaryAssetObject>(new CBinaryAssetObject(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CPrototype> CBinaryAssetObject::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CBinaryAssetObject>(new CBinaryAssetObject(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}
