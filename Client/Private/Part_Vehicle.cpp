#include "Part_Vehicle.h"
#include "BinaryAsset/ModelAssetData.h"

#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "MapAssetRenderUtils.h"

#include <cmath>

namespace
{
	constexpr uint32_t SOURCE_TRANSLUCENT_TWO_SIDED_PASS = 9u;
	constexpr uint32_t SOURCE_TRANSLUCENT_ONE_SIDED_PASS = 10u;

	uint32_t Resolve_TranslucentSourcePass(const Engine::MODEL_SURFACE_PARAMETERS* surface)
	{
		if (nullptr == surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
			return 0u;
		switch (surface->sourceCharacter.program)
		{
		case 18u: return SOURCE_TRANSLUCENT_TWO_SIDED_PASS;
		case 88u: return SOURCE_TRANSLUCENT_ONE_SIDED_PASS;
		default: return 0u;
		}
	}
}

CPart_Vehicle::CPart_Vehicle(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CPartObject { pDevice, pContext }
{
}

CPart_Vehicle::~CPart_Vehicle()
{
}

HRESULT CPart_Vehicle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart_Vehicle::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const auto pDesc = static_cast<PART_VEHICLE_DESC*>(pArg);
	m_strIdleClip = pDesc->strIdleClip;
	m_strRunClip = pDesc->strRunClip;
	m_strSeatBone = pDesc->strSeatBone;

	if (FAILED(__super::Initialize(pArg)) || FAILED(Ready_Components(pDesc)) ||
		!m_pModelCom->Has_Bone(m_strSeatBone.c_str()) ||
		!m_pModelCom->Set_Animation(m_strIdleClip.c_str(), true))
	{
		return E_FAIL;
	}
	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		m_hasTranslucentMeshes |= 0u != Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
	return S_OK;
}

bool_t CPart_Vehicle::Set_Moving(const bool_t isMoving)
{
	if (nullptr == m_pModelCom)
		return false;
	if (m_isMoving == isMoving)
		return true;
	m_isMoving = isMoving;
	return m_pModelCom->Set_Animation(
		(isMoving ? m_strRunClip : m_strIdleClip).c_str(), true);
}

bool_t CPart_Vehicle::Try_Get_SeatWorldPosition(float3_t& outPosition) const
{
	if (nullptr == m_pModelCom || nullptr == m_pTransformCom ||
		nullptr == m_pParentMatrix || !m_pModelCom->Has_Bone(m_strSeatBone.c_str()))
	{
		return false;
	}
	const matrix_t seat = m_pModelCom->Get_BoneMatrix(m_strSeatBone.c_str()) *
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()) *
		XMLoadFloat4x4(m_pParentMatrix);
	float3_t staged{};
	XMStoreFloat3(&staged, seat.r[3]);
	if (!std::isfinite(staged.x) || !std::isfinite(staged.y) || !std::isfinite(staged.z))
		return false;
	outPosition = staged;
	return true;
}

void CPart_Vehicle::Priority_Update(f32_t fTimeDelta)
{
}

void CPart_Vehicle::Update(f32_t fTimeDelta)
{
	m_pModelCom->Update_Animation(fTimeDelta);

	__super::Update_CombinedWorldMatrix(
		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
}

void CPart_Vehicle::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
	if (m_hasTranslucentMeshes)
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::BLEND,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
	if (CGameInstance::Get().Is_ShadowLightEnabled())
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
}

HRESULT CPart_Vehicle::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		uint32_t materialPass = 0u;
		const auto* surface = m_pModelCom->Get_MaterialSurface(i);
		if (0u != Resolve_TranslucentSourcePass(surface))
			continue;
		if (surface &&
			surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
			(surface->sourceCharacter.program == 6u || surface->sourceCharacter.program == 7u ||
			 surface->sourceCharacter.program == 19u ||
			 surface->sourceCharacter.program == 20u))
			materialPass = 6u;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(materialPass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Group(RENDERGROUP group)
{
	return RENDERGROUP::BLEND == group ? Render_Translucent() : Render();
}

HRESULT CPart_Vehicle::Render_Translucent()
{
	if (FAILED(Bind_ShaderResources()) ||
		FAILED(CMapAssetRenderUtils::Bind_SourceCharacterForwardLights(m_pShaderCom)))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		const uint32_t pass = Resolve_TranslucentSourcePass(m_pModelCom->Get_MaterialSurface(i));
		if (0u == pass)
			continue;
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, nullptr)) ||
			FAILED(m_pModelCom->Bind_SourceCharacterForwardLight(m_pShaderCom, i)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(pass)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Render_Shadow()
{
	constexpr uint32_t ANIMATED_SHADOW_PASS = 1u;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	for (uint32_t i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
	{
		if (FAILED(m_pModelCom->Bind_Material(
				m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(ANIMATED_SHADOW_PASS)) ||
			FAILED(m_pModelCom->Render(i)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT CPart_Vehicle::Ready_Components(const PART_VEHICLE_DESC* pDesc)
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

HRESULT CPart_Vehicle::Bind_ShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
		return E_FAIL;
	return S_OK;
}

HRESULT CPart_Vehicle::Bind_ShadowShaderResources()
{
	if (FAILED(__super::Bind_WorldMatrix(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_ShadowLight_ShaderResource(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
	{
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<CPart_Vehicle> CPart_Vehicle::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CPart_Vehicle>(new CPart_Vehicle(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		OutputDebugStringA("[Client][PartVehicle] Create failed.\n");
		return nullptr;
	}
	return pInstance;
}

shared_ptr<CPrototype> CPart_Vehicle::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CPart_Vehicle>(new CPart_Vehicle(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Client][PartVehicle] Clone failed.\n");
		return nullptr;
	}
	return pInstance;
}
