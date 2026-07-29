#include "MapAssetObject.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"

CMapAssetObject::CMapAssetObject(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CMapAssetObject::~CMapAssetObject()
{
}

HRESULT CMapAssetObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapAssetObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const MAP_ASSET_DESC desc = *static_cast<MAP_ASSET_DESC*>(pArg);
	if (0 == desc.placementId || desc.assetId.empty() ||
		desc.modelPrototypeTag.empty())
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components(desc.modelPrototypeTag)))
		return E_FAIL;

	m_iPlacementId = desc.placementId;
	m_AssetId = desc.assetId;
	m_bApplyBottomCenter = desc.applyBottomCenter;
	m_bVisible = desc.visible;
	Set_PlacementTransform(desc.position, desc.rotationDegrees, desc.scale);
	return S_OK;
}

void CMapAssetObject::Late_Update(f32_t fTimeDelta)
{
	if (!m_bVisible)
		return;

	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CMapAssetObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t meshIndex = 0; meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
	{
		const uint32_t hasNormalTexture =
			m_pModelCom->Has_MaterialTexture(meshIndex, aiTextureType_NORMALS) ? 1u : 0u;
		if (FAILED(m_pModelCom->Bind_Material(
			m_pShaderCom, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_HasNormalTexture", &hasNormalTexture, sizeof(hasNormalTexture))) ||
			(0 != hasNormalTexture && FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_NormalTexture", meshIndex, aiTextureType_NORMALS))) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(meshIndex)))
			return E_FAIL;
	}

	return S_OK;
}

void CMapAssetObject::Set_PlacementTransform(const float3_t& position,
	const float3_t& rotationDegrees, const float3_t& scale)
{
	m_vPlacementPosition = position;
	m_vRotationDegrees = rotationDegrees;
	m_vScale = scale;
	const float3_t worldOrigin = Compute_WorldOrigin(position, rotationDegrees, scale);
	m_pTransformCom->Scale(scale.x, scale.y, scale.z);
	m_pTransformCom->Rotation(
		rotationDegrees.x, rotationDegrees.y, rotationDegrees.z);
	m_pTransformCom->Set_State(
		STATE::POSITION, XMVectorSet(worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f));
}

HRESULT CMapAssetObject::Ready_Components(const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		ETOUI(LEVEL::ASSET_TEST),
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::ASSET_TEST), modelPrototypeTag,
			TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapAssetObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;

	return S_OK;
}

float3_t CMapAssetObject::Compute_WorldOrigin(const float3_t& placementPosition,
	const float3_t& rotationDegrees, const float3_t& scale) const
{
	float3_t worldOrigin = placementPosition;
	if (m_bApplyBottomCenter && m_pModelCom->Has_LocalBounds())
	{
		const float3_t& minimum = m_pModelCom->Get_LocalBoundsMin();
		const float3_t& maximum = m_pModelCom->Get_LocalBoundsMax();
		const vector_t localAnchor = XMVectorSet(
			(minimum.x + maximum.x) * 0.5f,
			minimum.y,
			(minimum.z + maximum.z) * 0.5f,
			1.f);
		const matrix_t scaleMatrix = XMMatrixScaling(
			scale.x, scale.y, scale.z);
		const matrix_t rotationMatrix = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(rotationDegrees.x),
			XMConvertToRadians(rotationDegrees.y),
			XMConvertToRadians(rotationDegrees.z));
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(localAnchor, scaleMatrix * rotationMatrix));
		worldOrigin.x -= anchorOffset.x;
		worldOrigin.y -= anchorOffset.y;
		worldOrigin.z -= anchorOffset.z;
	}

	return worldOrigin;
}

unique_ptr<CMapAssetObject> CMapAssetObject::Create(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CMapAssetObject>(
		new CMapAssetObject(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CPrototype> CMapAssetObject::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CMapAssetObject>(new CMapAssetObject(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}
