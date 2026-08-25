#include "MapAssetObject.h"

#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include "Profiler.h"

#include "MapAssetRenderUtils.h"

#include <algorithm>
#include <cmath>

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

	//매개 변수로 받은 Arg를 MAP_ASSET_DESC*로 캐스팅해서 초기화 데이터 채워넣기
	const MAP_ASSET_DESC desc = *static_cast<MAP_ASSET_DESC*>(pArg);

	const vector_t quaternion = XMLoadFloat4(&desc.rotationQuaternion);
	const float quaternionLength = XMVectorGetX(XMVector4Length(quaternion));

	if (0 == desc.placementId || desc.assetId.empty() ||
		desc.modelPrototypeTag.empty() ||
		!std::isfinite(quaternionLength) || quaternionLength < 0.000001f ||
		std::abs(desc.signedScale.x) < 0.000001f ||
		std::abs(desc.signedScale.y) < 0.000001f ||
		std::abs(desc.signedScale.z) < 0.000001f)
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(Ready_Components(
			desc.prototypeLevelIndex, desc.modelPrototypeTag)))
		return E_FAIL;

	m_iPlacementId = desc.placementId;
	m_AssetId = desc.assetId;
	m_bApplyBottomCenter = desc.applyBottomCenter;
	m_bVisible = desc.visible;
	m_RenderProfile = desc.renderProfile;
	m_bHasWaterProfile = desc.hasWaterProfile;
	m_WaterProfile = desc.waterProfile;
	m_fPresentationOpacityMultiplier = 1.f;
	Set_PresentationVortexProfile(
		PRESENTATION_VORTEX_PROFILE::NONE, 0.f);

	/*CModel이 로드하며 만든 local AABB를 한 번만 bounding sphere로 변환한다.*/
	Ready_CullBounds();

	Set_PlacementTransform(
		desc.position, desc.rotationQuaternion, desc.signedScale);
	return S_OK;
}

void CMapAssetObject::Update(f32_t fTimeDelta)
{
	m_fElapsedTime += fTimeDelta;
	if (m_RenderProfile.renderMode == MAP_ASSET_RENDER_MODE::BACKGROUND)
	{
		const float4_t* cameraPosition = CGameInstance::Get().Get_CamPosition();
		if (nullptr != cameraPosition)
			m_pTransformCom->Set_State(
				STATE::POSITION, XMLoadFloat4(cameraPosition));
	}
}

void CMapAssetObject::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	Engine::CProfiler* pProfiler =
		CGameInstance::Get().Get_Profiler();

	//Visible 여부와 Frustum 통과 여부와 무관한 전체 placement 개수
	//MapAssetObject가 Update를 돌면서 Profiler의 Counter 증가 시키기 O(N)
	if (nullptr != pProfiler)
	{
		pProfiler->Add_Counter(
			Engine::EProfilerCounter::MapPlacements);

		pProfiler->Add_Counter(
			Engine::EProfilerCounter::MapFallbackObjects);
	}

	if (!m_bVisible)
		return;
	const MAP_ASSET_RENDER_PROFILE effectiveProfile =
		Get_EffectiveRenderProfile();
	if (MAP_ASSET_RENDER_MODE::DEFERRED ==
		effectiveProfile.renderMode &&
		CGameInstance::Get().Is_ShadowLightEnabled())
	{
		/* Shadow visibility follows authored placement visibility, not the
		camera frustum, so an off-screen caster cannot pop its shadow. */
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::SHADOW,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
	//해당 Asset이 Background인지 여부 체크, sky background는 카메라를 따라 움직이기 때문에,
	//Frustum Culling에서 제외
	const bool_t bBackground =
		effectiveProfile.renderMode ==
		MAP_ASSET_RENDER_MODE::BACKGROUND;

	//Sky/BackGround는 카메라를 따라 움직이므로 일반적인 world-space Frustum 대상에서 제외
	//bounds가 없는 모델은 fail-open으로 렌더
	if (!bBackground && m_bHasWorldCullBounds &&
		!CGameInstance::Get().isIn_Frustum_InWorldSpace(
			XMLoadFloat3(&m_vWorldCullCenter), m_fWorldCullRadius))
		return;

	//Profiler에 Visible Instance 추가
	if (nullptr != pProfiler)
	{
		pProfiler->Add_Counter(
			Engine::EProfilerCounter::MapVisibleInstances);
	}

	RENDERGROUP renderGroup = RENDERGROUP::NONBLEND;

	if (effectiveProfile.renderMode == MAP_ASSET_RENDER_MODE::TRANSLUCENT ||
		effectiveProfile.renderMode == MAP_ASSET_RENDER_MODE::ADDITIVE ||
		effectiveProfile.renderMode == MAP_ASSET_RENDER_MODE::WATER)
		renderGroup = RENDERGROUP::BLEND;
	else if (effectiveProfile.renderMode == MAP_ASSET_RENDER_MODE::BACKGROUND)
		renderGroup = RENDERGROUP::PRIORITY;

	CGameInstance::Get().Add_RenderObject(
		renderGroup,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CMapAssetObject::Render()
{
	/* Late_Update may already have queued this object when a presentation cue
	   hides it. Re-check at draw time so the previous frame cannot leak through. */
	if (!m_bVisible || m_fPresentationOpacityMultiplier <= 0.f)
		return S_OK;

	HRESULT renderResult = Bind_ShaderResources();
	if (SUCCEEDED(renderResult))
	{
		MAP_ASSET_RENDER_PROFILE presentationProfile =
			Get_EffectiveRenderProfile();
		const bool_t bWater =
			MAP_ASSET_RENDER_MODE::WATER == presentationProfile.renderMode &&
			m_bHasWaterProfile;
		if (MAP_ASSET_RENDER_MODE::WATER == presentationProfile.renderMode &&
			!m_bHasWaterProfile)
		{
			/* The catalog load refuses a WATER asset without a row, so this can
			   only be a placement built before that check. Draw it as an
			   ordinary translucent surface rather than through a water pass
			   with identity parameters. */
			presentationProfile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
		}
		const uint32_t passIndex = CMapAssetRenderUtils::Select_Pass(
			presentationProfile, m_bMirrored);
		presentationProfile.opacity *= m_fPresentationOpacityMultiplier;
		renderResult = Bind_WaterShaderResources(bWater);

		for (uint32_t meshIndex = 0;
			SUCCEEDED(renderResult) &&
			meshIndex < m_pModelCom->Get_NumMeshes(); ++meshIndex)
		{
			if (FAILED(
				CMapAssetRenderUtils::Bind_Material(
					m_pModelCom, m_pShaderCom, meshIndex,
					presentationProfile, m_fElapsedTime)) ||

				FAILED(m_pShaderCom->Begin(passIndex)) ||

				FAILED(m_pModelCom->Render(meshIndex)))
			{
				renderResult = E_FAIL;
				break;
			}
		}
	}

	/* CShader clones share one FX11 effect. Reset even after a failed bind or
	   draw so a later character, prop or ordinary map asset cannot inherit the
	   presentation-only branch. */
	const HRESULT resetResult = Reset_PresentationVortexShaderResources();
	const HRESULT waterResetResult = Bind_WaterShaderResources(false);
	if (FAILED(renderResult))
		return renderResult;
	return FAILED(resetResult) ? resetResult : waterResetResult;
}

HRESULT CMapAssetObject::Render_Shadow()
{
	constexpr uint32_t STATIC_SHADOW_PASS_BASE = 12u;
	if (!m_bVisible || m_fPresentationOpacityMultiplier <= 0.f)
		return S_OK;
	MAP_ASSET_RENDER_PROFILE presentationProfile =
		Get_EffectiveRenderProfile();
	if (MAP_ASSET_RENDER_MODE::DEFERRED != presentationProfile.renderMode)
		return S_OK;
	if (FAILED(Bind_ShadowShaderResources()))
		return E_FAIL;

	const uint32_t iCullPass = CMapAssetRenderUtils::Select_Pass(
		presentationProfile, m_bMirrored);
	if (iCullPass > 2u)
		return E_UNEXPECTED;
	presentationProfile.opacity *= m_fPresentationOpacityMultiplier;

	for (uint32_t iMesh = 0;
		iMesh < m_pModelCom->Get_NumMeshes(); ++iMesh)
	{
		if (FAILED(CMapAssetRenderUtils::Bind_Material(
				m_pModelCom, m_pShaderCom, iMesh,
				presentationProfile, m_fElapsedTime)) ||
			FAILED(m_pShaderCom->Begin(
				STATIC_SHADOW_PASS_BASE + iCullPass)) ||
			FAILED(m_pModelCom->Render(iMesh)))
		{
			return E_FAIL;
		}
	}
	return S_OK;
}

void CMapAssetObject::Set_PresentationOpacityMultiplier(
	const f32_t multiplier)
{
	/* A corrupt presentation value must fail closed instead of reaching the
	   shader as NaN. Valid callers may only attenuate authored opacity. */
	m_fPresentationOpacityMultiplier = std::isfinite(multiplier) ?
		(std::clamp)(multiplier, 0.f, 1.f) : 0.f;
}

void CMapAssetObject::Set_PresentationVortexProfile(
	const PRESENTATION_VORTEX_PROFILE profile,
	const f32_t strength)
{
	const bool_t validProfile =
		PRESENTATION_VORTEX_PROFILE::NONE == profile ||
		PRESENTATION_VORTEX_PROFILE::DARK_APERTURE == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_RING == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC == profile;
	if (!validProfile || !std::isfinite(strength))
	{
		m_ePresentationVortexProfile =
			PRESENTATION_VORTEX_PROFILE::NONE;
		m_fPresentationVortexStrength = 0.f;
		return;
	}
	const f32_t boundedStrength =
		(std::clamp)(strength, 0.f, 1.f);
	if (PRESENTATION_VORTEX_PROFILE::NONE == profile ||
		boundedStrength <= 0.f)
	{
		m_ePresentationVortexProfile =
			PRESENTATION_VORTEX_PROFILE::NONE;
		m_fPresentationVortexStrength = 0.f;
		return;
	}
	m_ePresentationVortexProfile = profile;
	m_fPresentationVortexStrength = boundedStrength;
}

MAP_ASSET_RENDER_PROFILE CMapAssetObject::Get_EffectiveRenderProfile() const
{
	MAP_ASSET_RENDER_PROFILE profile = m_RenderProfile;
	if (!std::isfinite(m_fPresentationVortexStrength) ||
		m_fPresentationVortexStrength <= 0.f)
	{
		return profile;
	}
	if (PRESENTATION_VORTEX_PROFILE::DARK_APERTURE ==
		m_ePresentationVortexProfile)
	{
		/* Additive blending cannot remove light. The aperture uses the same
		   authored texture/opacity but must alpha-blend a dark procedural color. */
		profile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
	}
	else if (PRESENTATION_VORTEX_PROFILE::RED_RING ==
		m_ePresentationVortexProfile)
	{
		profile.renderMode = MAP_ASSET_RENDER_MODE::ADDITIVE;
	}
	else if (PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC ==
		m_ePresentationVortexProfile)
	{
		/* The cloud discs must retain their dark burgundy body. Additive blending
		   would turn the source texture into the bright blue/red square that this
		   presentation profile is specifically meant to suppress. */
		profile.renderMode = MAP_ASSET_RENDER_MODE::TRANSLUCENT;
	}
	return profile;
}

void CMapAssetObject::Set_PlacementTransform(const float3_t& position,
	const float4_t& rotationQuaternion, const float3_t& signedScale)
{
	vector_t quaternion = XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion));

	if (XMVectorGetW(quaternion) < 0.f)
		quaternion = XMVectorNegate(quaternion);

	XMStoreFloat4(&m_vRotationQuaternion, quaternion);

	m_vPlacementPosition = position;
	m_vSignedScale = signedScale;
	m_bMirrored = signedScale.x * signedScale.y * signedScale.z < 0.f;

	const float3_t worldOrigin = Compute_WorldOrigin(
		position, m_vRotationQuaternion, signedScale);

	const matrix_t world = XMMatrixScaling(
		signedScale.x, signedScale.y, signedScale.z) *
		XMMatrixRotationQuaternion(quaternion);

	m_pTransformCom->Set_State(STATE::RIGHT, world.r[0]);
	m_pTransformCom->Set_State(STATE::UP, world.r[1]);
	m_pTransformCom->Set_State(STATE::LOOK, world.r[2]);
	m_pTransformCom->Set_State(
		STATE::POSITION, XMVectorSet(worldOrigin.x, worldOrigin.y, worldOrigin.z, 1.f));

	//bottom-center 보정을 포함한 최종 world 행렬을 사용해야 실제 렌더 위치와 bounds가 일치한다.
	Update_WorldCullBounds();
}

HRESULT CMapAssetObject::Ready_Components(
	uint32_t prototypeLevelIndex,
	const std::wstring& modelPrototypeTag)
{
	if (FAILED(__super::Add_Component(
		prototypeLevelIndex,
		TEXT("Prototype_Component_Shader_VtxMeshBinary"),
		TEXT("Com_Shader"), m_pShaderCom)) ||
		FAILED(__super::Add_Component(
			prototypeLevelIndex, modelPrototypeTag,
			TEXT("Com_Model"), m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapAssetObject::Bind_ShaderResources()
{
	matrix_t world = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr());
	world.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	const matrix_t inverseTranspose =
		XMMatrixTranspose(XMMatrixInverse(nullptr, world));
	float4x4_t storedInverseTranspose{};
	XMStoreFloat4x4(&storedInverseTranspose, inverseTranspose);
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
		m_pShaderCom, "g_WorldMatrix")) ||
		FAILED(m_pShaderCom->Bind_Matrix(
			"g_WorldInvTransposeMatrix", &storedInverseTranspose)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
		FAILED(CGameInstance::Get().Bind_Transform(
			m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)) ||
		FAILED(Bind_PresentationVortexShaderResources(
			m_ePresentationVortexProfile,
			m_fPresentationVortexStrength)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapAssetObject::Bind_PresentationVortexShaderResources(
	const PRESENTATION_VORTEX_PROFILE profile,
	const f32_t strength)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	const bool_t validProfile =
		PRESENTATION_VORTEX_PROFILE::NONE == profile ||
		PRESENTATION_VORTEX_PROFILE::DARK_APERTURE == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_RING == profile ||
		PRESENTATION_VORTEX_PROFILE::RED_CLOUD_DISC == profile;
	const PRESENTATION_VORTEX_PROFILE boundedProfile =
		validProfile && std::isfinite(strength) && strength > 0.f ?
		profile : PRESENTATION_VORTEX_PROFILE::NONE;
	const f32_t boundedStrength =
		PRESENTATION_VORTEX_PROFILE::NONE == boundedProfile ? 0.f :
		(std::clamp)(strength, 0.f, 1.f);
	const uint32_t rawProfile =
		static_cast<uint32_t>(boundedProfile);

	/* Strength is written first. Even if the profile write fails, resetting
	   strength to zero disables both procedural shader branches. */
	const HRESULT strengthResult = m_pShaderCom->Bind_RawValue(
		"g_PresentationVortexStrength",
		&boundedStrength, sizeof(boundedStrength));
	const HRESULT profileResult = m_pShaderCom->Bind_RawValue(
		"g_PresentationVortexProfile",
		&rawProfile, sizeof(rawProfile));
	return FAILED(strengthResult) || FAILED(profileResult) ? E_FAIL : S_OK;
}

HRESULT CMapAssetObject::Reset_PresentationVortexShaderResources()
{
	return Bind_PresentationVortexShaderResources(
		PRESENTATION_VORTEX_PROFILE::NONE, 0.f);
}

HRESULT CMapAssetObject::Bind_WaterShaderResources(const bool_t bEnabled)
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	/* Disabled is the identity water: no fresnel, no distortion, no reflection
	   and a flat normal, so the water passes cannot tint or bend anything if
	   they are ever reached without a row. */
	const MAP_ASSET_WATER_PROFILE identity{};
	const MAP_ASSET_WATER_PROFILE& profile =
		bEnabled ? m_WaterProfile : identity;

	/* The auxiliary textures are declared by the water document but the
	   runtime does not own an SRV for them yet, so both flags stay zero and
	   the shader falls back to the model's own diffuse and normal. */
	constexpr uint32_t hasDetailNormalTexture = 0u;
	constexpr uint32_t hasReflectionTexture = 0u;
	const f32_t elapsedTime = bEnabled ? m_fElapsedTime : 0.f;

	struct WATER_SCALAR_BINDING final
	{
		const char_t* pName;
		f32_t value;
	};
	const WATER_SCALAR_BINDING scalars[]
	{
		{ "g_WaterOpacity", profile.opacity },
		{ "g_WaterOpacityPower", profile.opacityPower },
		{ "g_WaterFresnelIntensity", profile.fresnelIntensity },
		{ "g_WaterFresnelPower", profile.fresnelPower },
		{ "g_WaterScreenDistortionIntensity", profile.screenDistortionIntensity },
		{ "g_WaterNormalIntensity", profile.normalIntensity },
		{ "g_WaterDetailNormalIntensity", profile.detailNormalIntensity },
		{ "g_WaterReflectionIntensity", profile.reflectionIntensity },
		{ "g_WaterDiffuseTiling", profile.diffuseTiling },
		{ "g_ElapsedTime", elapsedTime },
	};
	struct WATER_VECTOR_BINDING final
	{
		const char_t* pName;
		const float4_t* pValue;
	};
	const WATER_VECTOR_BINDING vectors[]
	{
		{ "g_WaterDiffuseColor", &profile.diffuseColor },
		{ "g_WaterReflectionColor", &profile.reflectionColor },
		{ "g_WaterNormalTilingPanning", &profile.normalTilingPanning },
		{ "g_WaterDetailNormalTilingPanning",
			&profile.detailNormalTilingPanning },
		{ "g_WaterReflectionTilingPanning", &profile.reflectionTilingPanning },
	};

	HRESULT firstFailure = S_OK;
	const auto record = [&firstFailure](const HRESULT result)
	{
		if (FAILED(result) && SUCCEEDED(firstFailure))
			firstFailure = result;
	};

	for (const WATER_SCALAR_BINDING& binding : scalars)
	{
		record(m_pShaderCom->Bind_RawValue(
			binding.pName, &binding.value, sizeof(binding.value)));
	}
	for (const WATER_VECTOR_BINDING& binding : vectors)
	{
		record(m_pShaderCom->Bind_RawValue(
			binding.pName, binding.pValue, sizeof(*binding.pValue)));
	}
	record(m_pShaderCom->Bind_RawValue(
		"g_HasDetailNormalTexture",
		&hasDetailNormalTexture, sizeof(hasDetailNormalTexture)));
	record(m_pShaderCom->Bind_RawValue(
		"g_HasReflectionTexture",
		&hasReflectionTexture, sizeof(hasReflectionTexture)));
	if (bEnabled)
	{
		record(CGameInstance::Get().Bind_CamPosition(
			m_pShaderCom, "g_vCamPosition"));
	}
	return firstFailure;
}

HRESULT CMapAssetObject::Bind_ShadowShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(
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

void CMapAssetObject::Ready_CullBounds()
{
	m_bHasLocalCullBounds = false;
	m_bHasWorldCullBounds = false;
	m_vLocalCullCenter = {};
	m_fLocalCullRadius = 0.f;
	m_vWorldCullCenter = {};
	m_fWorldCullRadius = 0.f;

	if (nullptr == m_pModelCom ||
		!m_pModelCom->Has_LocalBounds())
		return;

	const float3_t& minimum =
		m_pModelCom->Get_LocalBoundsMin();
	const float3_t& maximum =
		m_pModelCom->Get_LocalBoundsMax();
	//예외처리 <- 근데 무슨 경우?
	if (!std::isfinite(minimum.x) ||
		!std::isfinite(minimum.y) ||
		!std::isfinite(minimum.z) ||
		!std::isfinite(maximum.x) ||
		!std::isfinite(maximum.y) ||
		!std::isfinite(maximum.z) ||
		maximum.x < minimum.x ||
		maximum.y < minimum.y ||
		maximum.z < minimum.z)
	{
		return;
	}

	m_vLocalCullCenter = float3_t(
		(minimum.x + maximum.x) * 0.5f,
		(minimum.y + maximum.y) * 0.5f,
		(minimum.z + maximum.z) * 0.5f);

	const f32_t extentX =
		(maximum.x - minimum.x) * 0.5f;
	const f32_t extentY =
		(maximum.y - minimum.y) * 0.5f;
	const f32_t extentZ =
		(maximum.z - minimum.z) * 0.5f;

	const f32_t radius = std::sqrt(
		extentX * extentX +
		extentY * extentY +
		extentZ * extentZ);

	if (!std::isfinite(radius))
		return;

	//점이나 얇은 mesh가 쉽게 잘리지 않도록 최소 반지름
	m_fLocalCullRadius =
		radius > 0.05f ? radius : 0.05f;

	m_bHasLocalCullBounds = true;
}

void CMapAssetObject::Update_WorldCullBounds()
{
	m_bHasWorldCullBounds = false;

	if (!m_bHasLocalCullBounds ||
		nullptr == m_pTransformCom)
		return;

	const matrix_t world =
		XMLoadFloat4x4(
			m_pTransformCom->Get_WorldMatrixPtr());

	const vector_t worldCenter =
		XMVector3TransformCoord(
			XMLoadFloat3(&m_vLocalCullCenter),
			world);

	float3_t storedWorldCenter{};
	XMStoreFloat3(
		&storedWorldCenter,
		worldCenter);
	//vector의 길이 가지고 오기. row 0 1 2의 x y z의 길이 구하기
	const f32_t scaleX =
		XMVectorGetX(
			XMVector3Length(world.r[0]));
	const f32_t scaleY =
		XMVectorGetX(
			XMVector3Length(world.r[1]));
	const f32_t scaleZ =
		XMVectorGetX(
			XMVector3Length(world.r[2]));
	//가장 큰 스케일 기준으로 컬링
	f32_t maximumScale = scaleX;
	if (scaleY > maximumScale)
		maximumScale = scaleY;
	if (scaleZ > maximumScale)
		maximumScale = scaleZ;
	//worldRadius scale 보정
	const f32_t worldRadius =
		m_fLocalCullRadius *
		maximumScale *
		1.02f + 0.05f;
	
	if (!std::isfinite(storedWorldCenter.x) ||
		!std::isfinite(storedWorldCenter.y) ||
		!std::isfinite(storedWorldCenter.z) ||
		!std::isfinite(worldRadius) ||
		maximumScale < 0.000001f)
	{
		return;
	}

	m_vWorldCullCenter = storedWorldCenter;
	m_fWorldCullRadius = worldRadius;
	m_bHasWorldCullBounds = true;
}

float3_t CMapAssetObject::Compute_WorldOrigin(const float3_t& placementPosition,
	const float4_t& rotationQuaternion, const float3_t& signedScale) const
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
		const matrix_t transform = XMMatrixScaling(
			signedScale.x, signedScale.y, signedScale.z) *
			XMMatrixRotationQuaternion(
				XMQuaternionNormalize(XMLoadFloat4(&rotationQuaternion)));
		float3_t anchorOffset{};
		XMStoreFloat3(&anchorOffset,
			XMVector3TransformCoord(localAnchor, transform));
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
