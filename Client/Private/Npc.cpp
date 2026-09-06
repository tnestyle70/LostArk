#include "Npc.h"
#include "EffectV2_Runtime.h"

#include "Collider.h"
#include "DeferredMaterialRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr f32_t HIT_FLASH_DURATION_SECONDS = 0.12f;
	constexpr f32_t HIT_FLASH_PEAK_INTENSITY = 4.f;
	constexpr const char_t* ROOT_MOTION_BONE = "b_root";
	constexpr int32_t ROOT_MOTION_VERTICAL_AXIS = 2;
}

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
	if (!std::isfinite(pDesc->fCollisionRadius) ||
		pDesc->fCollisionRadius < 0.f)
	{
		return E_INVALIDARG;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pDesc)))
		return E_FAIL;

	Apply_ImmediateTransform(pDesc->vPosition, pDesc->fYawDegree);

	m_fOutlineWidth = pDesc->fOutlineWidth;
	m_vOutlineColor = pDesc->vOutlineColor;
	m_bSuppressRootMotion = pDesc->bSuppressRootMotion;
	m_bInterpolateNetworkTransform = pDesc->bInterpolateNetworkTransform;

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
	const char_t* pResolvedIdle = m_pModelCom->Get_AnimationName(
		m_pModelCom->Get_CurrentAnimIndex());
	if (nullptr == pResolvedIdle || '\0' == pResolvedIdle[0])
		return E_FAIL;
	m_strDefaultIdleClip = pResolvedIdle;
	m_pModelCom->Set_AnimationSpeed(1.f);
	/* Authored town placements are Server-transform authoritative. Their clips
	may pose translated root keys but must not move presentation away from the
	replicated transform. Esther leaves suppression disabled because its existing
	action chains intentionally use authored root motion. */
	if (m_bSuppressRootMotion)
	{
		m_pModelCom->Enable_RootMotionSuppression(
			ROOT_MOTION_BONE, ROOT_MOTION_VERTICAL_AXIS);
	}

	return S_OK;
}

bool_t CNpc::Set_Animation(const char_t* pClipName, bool_t isLoop)
{
	if (nullptr == pClipName || nullptr == m_pModelCom)
		return false;
	m_pModelCom->Set_AnimationSpeed(1.f);
	if (!m_pModelCom->Set_Animation(pClipName, isLoop))
		return false;
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
	return true;
}

bool_t CNpc::Play_NetworkAction(
	const char_t* pClipName,
	const bool_t isLoop,
	const f32_t fPlaybackRate,
	const f32_t fBlendSeconds)
{
	m_fTransientActionRemainingSeconds = 0.f;
	m_strTransientReturnClip.clear();
	if (nullptr == pClipName || '\0' == pClipName[0] ||
		nullptr == m_pModelCom || !std::isfinite(fPlaybackRate) ||
		fPlaybackRate < 0.1f || fPlaybackRate > 4.f ||
		!std::isfinite(fBlendSeconds) ||
		fBlendSeconds < 0.f || fBlendSeconds > 2.f ||
		!m_pModelCom->Set_Animation(pClipName, isLoop, fBlendSeconds))
	{
		return false;
	}
	m_pModelCom->Set_AnimationSpeed(fPlaybackRate);
	if (!m_pModelCom->Start_Animation(
			m_pModelCom->Get_CurrentAnimIndex(), isLoop))
	{
		return false;
	}
	/* Keep the existing NPC effect/cutin hook on every semantic action edge,
	including a restart that resolves to the same clip name. */
	CEffectV2Runtime::Notify_Clip(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		pClipName);
	return true;
}

bool_t CNpc::Play_TransientNetworkAction(
	const char_t* pClipName,
	const f32_t fPlaybackRate,
	const f32_t fDurationSeconds,
	const char_t* pReturnClip,
	const bool_t isReturnLoop,
	const f32_t fReturnPlaybackRate,
	const f32_t fBlendSeconds)
{
	if (nullptr == pReturnClip || '\0' == pReturnClip[0] ||
		!std::isfinite(fDurationSeconds) || fDurationSeconds <= 0.f ||
		fDurationSeconds > 5.f || !std::isfinite(fReturnPlaybackRate) ||
		fReturnPlaybackRate < 0.1f || fReturnPlaybackRate > 4.f ||
		!Play_NetworkAction(
			pClipName, false, fPlaybackRate, fBlendSeconds))
	{
		return false;
	}
	m_fTransientActionRemainingSeconds = fDurationSeconds;
	m_strTransientReturnClip = pReturnClip;
	m_fTransientReturnPlaybackRate = fReturnPlaybackRate;
	m_isTransientReturnLoop = isReturnLoop;
	return true;
}

bool_t CNpc::Play_DefaultIdle(const f32_t fBlendSeconds)
{
	return !m_strDefaultIdleClip.empty() && Play_NetworkAction(
		m_strDefaultIdleClip.c_str(), true, 1.f, fBlendSeconds);
}

bool_t CNpc::Apply_NetworkState(
	const float3_t& position,
	const f32_t yawDegrees,
	const std::uint32_t iServerTick)
{
	if (nullptr == m_pTransformCom ||
		!std::isfinite(position.x) ||
		!std::isfinite(position.y) ||
		!std::isfinite(position.z) ||
		!std::isfinite(yawDegrees))
	{
		return false;
	}
	if (!m_bInterpolateNetworkTransform)
	{
		Apply_ImmediateTransform(position, yawDegrees);
		return true;
	}
	/* Spawn messages carry no simulation tick. They place the object exactly;
	interpolation begins with the first world snapshot that has a tick. */
	if (0u == iServerTick)
	{
		m_NetworkTransformInterpolator.Reset();
		Apply_ImmediateTransform(position, yawDegrees);
		return true;
	}
	return m_NetworkTransformInterpolator.Push(
		position, yawDegrees, iServerTick);
}

void CNpc::Trigger_HitFlash()
{
	m_fHitFlashRemainingSeconds = HIT_FLASH_DURATION_SECONDS;
	m_HitFlash.isEnabled = true;
	m_HitFlash.vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY;
	m_HitFlash.usesSurfaceDetailMask = true;
}

void CNpc::Priority_Update(f32_t fTimeDelta)
{
}

void CNpc::Update(f32_t fTimeDelta)
{
	if (m_bInterpolateNetworkTransform)
		Update_NetworkTransform(fTimeDelta);
	if (m_fTransientActionRemainingSeconds > 0.f)
	{
		m_fTransientActionRemainingSeconds -= fTimeDelta;
		if (m_fTransientActionRemainingSeconds <= 0.f)
		{
			const std::string returnClip = m_strTransientReturnClip;
			const f32_t returnRate = m_fTransientReturnPlaybackRate;
			const bool_t returnLoop = m_isTransientReturnLoop;
			m_fTransientActionRemainingSeconds = 0.f;
			m_strTransientReturnClip.clear();
			if (!returnClip.empty())
			{
				(void)Play_NetworkAction(
					returnClip.c_str(), returnLoop, returnRate, 0.05f);
			}
		}
	}
	if (m_bSuppressRootMotion)
	{
		m_pModelCom->Update_Animation(fTimeDelta);
	}
	else
	{
		m_pModelCom->Play_Animation(fTimeDelta);
	}
	Update_CombatCollider();
	CEffectV2Runtime::Tick(
		EFFECT_V2_TARGET::From_Npc(static_pointer_cast<CNpc>(shared_from_this())),
		m_pDevice, m_pContext);
	if (m_fHitFlashRemainingSeconds > 0.f)
	{
		m_fHitFlashRemainingSeconds -= fTimeDelta;
		if (m_fHitFlashRemainingSeconds <= 0.f)
		{
			m_fHitFlashRemainingSeconds = 0.f;
			m_HitFlash = {};
		}
		else
		{
			m_HitFlash.fIntensity = HIT_FLASH_PEAK_INTENSITY *
				(m_fHitFlashRemainingSeconds / HIT_FLASH_DURATION_SECONDS);
		}
	}
}

void CNpc::Apply_ImmediateTransform(
	const float3_t& position,
	const f32_t yawDegrees)
{
	float3_t drawn = position;
	f32_t drawnYaw = yawDegrees;
#ifdef _DEBUG
	m_vDebugUnadjustedPosition = position;
	m_fDebugUnadjustedYawDegrees = yawDegrees;
	drawn.x += m_vDebugPresentationOffset.x;
	drawn.y += m_vDebugPresentationOffset.y;
	drawn.z += m_vDebugPresentationOffset.z;
	drawnYaw += m_fDebugPresentationYawOffset;
#endif
	m_pTransformCom->Set_State(STATE::POSITION,
		XMVectorSet(drawn.x, drawn.y, drawn.z, 1.f));
	// Rotation keeps the transform's current scale, so a Debug scale set once
	// through Set_DebugPresentationScale survives every replicated pose.
	m_pTransformCom->Rotation(0.f, drawnYaw, 0.f);
	Update_CombatCollider();
}

void CNpc::Update_NetworkTransform(const f32_t fTimeDelta)
{
	if (nullptr == m_pTransformCom)
		return;
	NPC_NETWORK_TRANSFORM_FRAME frame{};
	if (!m_NetworkTransformInterpolator.Advance(fTimeDelta, frame))
		return;
	float3_t drawn = frame.vPosition;
	f32_t drawnYaw = frame.fYawDegrees;
#ifdef _DEBUG
	m_vDebugUnadjustedPosition = frame.vPosition;
	m_fDebugUnadjustedYawDegrees = frame.fYawDegrees;
	drawn.x += m_vDebugPresentationOffset.x;
	drawn.y += m_vDebugPresentationOffset.y;
	drawn.z += m_vDebugPresentationOffset.z;
	drawnYaw += m_fDebugPresentationYawOffset;
#endif
	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
		drawn.x,
		drawn.y,
		drawn.z,
		1.f));
	m_pTransformCom->Rotation(0.f, drawnYaw, 0.f);
}

void CNpc::Update_CombatCollider()
{
	if (nullptr == m_pColliderCom || nullptr == m_pTransformCom)
		return;
	matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
#ifdef _DEBUG
	// Presentation tuning must not move or resize the Server-radius mirror.
	if (m_fDebugPresentationScale != 1.f ||
		m_vDebugPresentationOffset.x != 0.f || m_vDebugPresentationOffset.y != 0.f ||
		m_vDebugPresentationOffset.z != 0.f)
	{
		for (size_t axis = 0u; axis < 3u; ++axis)
			world.r[axis] = XMVector3Normalize(world.r[axis]);
		world.r[3] = XMVectorSet(m_vDebugUnadjustedPosition.x,
			m_vDebugUnadjustedPosition.y, m_vDebugUnadjustedPosition.z, 1.f);
	}
#endif
	m_pColliderCom->Update(world);
}

#ifdef _DEBUG
void CNpc::Set_DebugWeaponScale(const f32_t fScale)
{
	if (std::isfinite(fScale) && fScale > 0.f && fScale <= 10000.f)
		m_fDebugWeaponScale = fScale;
}

void CNpc::Set_DebugWeaponRotation(
	const float3_t& vCatalogDegrees, const float3_t& vTargetDegrees)
{
	if (!std::isfinite(vCatalogDegrees.x) || !std::isfinite(vCatalogDegrees.y) ||
		!std::isfinite(vCatalogDegrees.z) || !std::isfinite(vTargetDegrees.x) ||
		!std::isfinite(vTargetDegrees.y) || !std::isfinite(vTargetDegrees.z))
	{
		return;
	}
	const matrix_t catalogRotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vCatalogDegrees.x), XMConvertToRadians(vCatalogDegrees.y),
		XMConvertToRadians(vCatalogDegrees.z));
	const matrix_t targetRotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(vTargetDegrees.x), XMConvertToRadians(vTargetDegrees.y),
		XMConvertToRadians(vTargetDegrees.z));
	// A pure rotation's transpose is its inverse. R(catalog) * delta then
	// equals R(saved Euler), including mixed axes and Save/Reload in one run.
	XMStoreFloat4x4(&m_DebugWeaponRotation,
		XMMatrixTranspose(catalogRotation) * targetRotation);
}

void CNpc::Set_DebugPresentationScale(const f32_t fScale)
{
	if (!std::isfinite(fScale) || fScale <= 0.f || nullptr == m_pTransformCom)
		return;
	m_fDebugPresentationScale = fScale;
	m_pTransformCom->Scale(fScale, fScale, fScale);
}

void CNpc::Set_DebugPresentationOffset(const float3_t& vOffset)
{
	if (!std::isfinite(vOffset.x) || !std::isfinite(vOffset.y) ||
		!std::isfinite(vOffset.z))
	{
		return;
	}
	m_vDebugPresentationOffset = vOffset;
	if (nullptr != m_pTransformCom)
		m_pTransformCom->Set_State(STATE::POSITION, XMVectorSet(
			m_vDebugUnadjustedPosition.x + vOffset.x,
			m_vDebugUnadjustedPosition.y + vOffset.y,
			m_vDebugUnadjustedPosition.z + vOffset.z, 1.f));
	Update_CombatCollider();
}

void CNpc::Set_DebugPresentationYawOffset(const f32_t fYawOffsetDegrees)
{
	if (!std::isfinite(fYawOffsetDegrees))
		return;
	m_fDebugPresentationYawOffset = fYawOffsetDegrees;
	// Rotation keeps the current scale; the collider re-reads the Server yaw.
	if (nullptr != m_pTransformCom)
		m_pTransformCom->Rotation(0.f,
			m_fDebugUnadjustedYawDegrees + fYawOffsetDegrees, 0.f);
	Update_CombatCollider();
}
#endif

void CNpc::Late_Update(f32_t fTimeDelta)
{
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONBLEND,
		static_pointer_cast<CGameObject>(shared_from_this()));
#ifdef _DEBUG
	if (m_isCombatColliderDebugVisible && nullptr != m_pColliderCom)
		CGameInstance::Get().Add_DebugComponent(m_pColliderCom);
#endif
}

HRESULT CNpc::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	const uint32_t iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (uint32_t i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(Bind_DeferredMaterialInputs(
				*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
			FAILED(m_pModelCom->Bind_BoneMatrices(
				m_pShaderCom, "g_BoneMatrices", i)) ||
			FAILED(m_pShaderCom->Begin(0)) ||
			FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	if (nullptr != m_pWeaponModelCom)
	{
		/* The socket bone matrix already carries the body's pre-transform, so
		the weapon world is bone x body world with no second scale or yaw. */
		float4x4_t weaponWorld{};
#ifdef _DEBUG
		const f32_t weaponScale = m_fDebugWeaponScale;
		const matrix_t weaponLocal = XMLoadFloat4x4(&m_DebugWeaponRotation) *
			XMMatrixScaling(weaponScale, weaponScale, weaponScale);
#else
		const matrix_t weaponLocal = XMMatrixIdentity();
#endif
		XMStoreFloat4x4(&weaponWorld,
			weaponLocal *
			m_pModelCom->Get_BoneMatrix(m_strWeaponSocketBone.c_str()) *
			XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &weaponWorld)))
			return E_FAIL;
		const uint32_t iNumWeaponMeshes = m_pWeaponModelCom->Get_NumMeshes();
		for (uint32_t i = 0; i < iNumWeaponMeshes; ++i)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					*m_pWeaponModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
				FAILED(m_pWeaponModelCom->Bind_BoneMatrices(
					m_pShaderCom, "g_BoneMatrices", i)) ||
				FAILED(m_pShaderCom->Begin(0)) ||
				FAILED(m_pWeaponModelCom->Render(i)))
				return E_FAIL;
		}
		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
			return E_FAIL;
	}
	if (m_fOutlineWidth > 0.f)
	{
		/* Pass 3 of the esther shader: front-culled hull pushed along the
		skinned normal, stencil-tested against the body drawn just above. */
		if (FAILED(m_pShaderCom->Bind_RawValue(
				"g_OutlineWidth", &m_fOutlineWidth, sizeof(m_fOutlineWidth))) ||
			FAILED(m_pShaderCom->Bind_RawValue(
				"g_OutlineColor", &m_vOutlineColor, sizeof(m_vOutlineColor))))
			return E_FAIL;
		for (uint32_t i = 0; i < iNumMeshes; ++i)
		{
			if (FAILED(Bind_DeferredMaterialInputs(
					*m_pModelCom, m_pShaderCom, i, {}, &m_HitFlash)) ||
				FAILED(m_pModelCom->Bind_BoneMatrices(
					m_pShaderCom, "g_BoneMatrices", i)) ||
				FAILED(m_pShaderCom->Begin(3)) ||
				FAILED(m_pModelCom->Render(i)))
				return E_FAIL;
		}
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
	m_strModelTag = pDesc->strModelTag;

	if (!pDesc->strWeaponModelTag.empty() || nullptr != pDesc->pWeaponSocketBone)
	{
		if (pDesc->strWeaponModelTag.empty() ||
			nullptr == pDesc->pWeaponSocketBone ||
			'\0' == pDesc->pWeaponSocketBone[0] ||
			!m_pModelCom->Has_Bone(pDesc->pWeaponSocketBone))
		{
			return E_INVALIDARG;
		}
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			pDesc->strWeaponModelTag,
			TEXT("Com_WeaponModel"),
			m_pWeaponModelCom)))
			return E_FAIL;
		m_strWeaponSocketBone = pDesc->pWeaponSocketBone;
		/* Freeze the prototype's initial pose until weapon clip synchronization
		is authored. Cloning does not restore the skeleton's bind pose. */
		m_pWeaponModelCom->Set_AnimPaused(true);
		m_pWeaponModelCom->Refresh_BoneCombinedMatrices();
	}

	if (pDesc->fCollisionRadius > 0.f)
	{
		Engine::CBounding_Sphere::BOUNDING_SPHERE_DESC colliderDesc{};
		colliderDesc.vCenter = float3_t(
			0.f, pDesc->fCollisionRadius, 0.f);
		colliderDesc.fRadius = pDesc->fCollisionRadius;
		if (FAILED(__super::Add_Component(
			pDesc->iPrototypeLevelIndex,
			TEXT("Prototype_Component_Collider_WorldEntity"),
			TEXT("Com_CombatCollider"),
			m_pColliderCom,
			&colliderDesc)))
		{
			return E_FAIL;
		}
	}

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
