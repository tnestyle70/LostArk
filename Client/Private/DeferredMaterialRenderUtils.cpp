#include "DeferredMaterialRenderUtils.h"

#include "BinaryAsset/ModelAssetData.h"
#include "Model.h"
#include "GameInstance.h"
#include "Shader.h"

#include <cmath>

namespace
{
    uint32_t SourceHitColorRow(const Engine::MODEL_SURFACE_PARAMETERS* surface)
    {
        if (!surface || surface->family != Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER)
            return UINT32_MAX;
        // Original Hit_Color uniform packing; all these monster permutations
        // consume it in Base only. Keep shared material/light rows immutable.
        switch (surface->sourceCharacter.program)
        {
        case 21u: case 28u: case 238u: return 14u;
        case 22u: case 92u: case 93u: return 3u;
        case 23u: return 17u;
        case 24u: case 25u: return 15u;
        case 26u: case 29u: case 30u: case 32u: return 12u;
        case 27u: return 16u;
        case 31u: case 84u: return 13u;
        default: return UINT32_MAX;
        }
    }

    HRESULT BindSourceHitColor(const shared_ptr<Engine::CShader>& shader,
        const Engine::MODEL_SURFACE_PARAMETERS* surface,
        const Client::DEFERRED_EMISSIVE_OVERRIDE* presentation)
    {
        const uint32_t row = SourceHitColorRow(surface);
        if (row == UINT32_MAX || !presentation || !presentation->isEnabled ||
            !presentation->usesSurfaceDetailMask || !std::isfinite(presentation->fIntensity) ||
            presentation->fIntensity <= 0.f) return S_OK;
        auto constants = surface->sourceCharacter.baseConstants;
        constants[row].x = presentation->vColor.x * presentation->fIntensity;
        constants[row].y = presentation->vColor.y * presentation->fIntensity;
        constants[row].z = presentation->vColor.z * presentation->fIntensity;
        return shader->Bind_RawValue("g_SourceCharacterBaseConstants", constants.data(), sizeof(constants));
    }

	constexpr std::string_view VALTAN_MATERIAL_PROFILE =
		"material.valtan.monster-base.v1";
	constexpr float4_t VALTAN_MASKED_TEAL_COLOR =
		float4_t(0.f, 1.35f, 1.55f, 1.f);

	f32_t Resolve_ValtanEmissiveIntensity(std::string_view strMaterialName)
	{
		if (strMaterialName == "mn_rpbf_01_2_mi")
			return 15.f;
		if (strMaterialName == "mn_rpbf_01_1_mi" ||
			strMaterialName == "wp_mn_rpbf_01_1_mi")
			return 10.f;
		if (strMaterialName == "mn_rpbf_01_mi" ||
			strMaterialName == "wp_mn_rpbf_01_mi")
			return 5.f;
		return 0.f;
	}
}

Client::DEFERRED_MATERIAL_PROFILE Client::Resolve_DeferredMaterialProfile(
	std::string_view strProfileId,
	std::string_view strMaterialName)
{
	DEFERRED_MATERIAL_PROFILE Profile{};
	if (strProfileId == VALTAN_MATERIAL_PROFILE)
	{
		/* The deferred path has no per-material ambient RGB target.  The
		   authored E map is the exact body/axe part mask for unlit ambient
		   energy, so keep the mask and apply the teal weights here. */
		Profile.vEmissiveColor = VALTAN_MASKED_TEAL_COLOR;
		Profile.fEmissiveIntensity =
			Resolve_ValtanEmissiveIntensity(strMaterialName);
	}
	return Profile;
}

HRESULT Client::Bind_DeferredMaterialInputs(
	Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader,
	uint32_t iMeshIndex,
	const DEFERRED_MATERIAL_PROFILE& Profile,
	const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride,
	const ComPtr<ID3D11ShaderResourceView>& diffuseOverride,
	bool_t nativeBinaryBasePass)
{
	if (nullptr == pShader || iMeshIndex >= Model.Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

	// A diffuse override can bypass CMaterial's shared-program reset.
	const uint32_t noSurface = 0u;
	pShader->Bind_RawValue("g_SurfaceProgram", &noSurface, sizeof(noSurface));
    pShader->Bind_RawValue("g_SourceCharacterProgram", &noSurface, sizeof(noSurface));
    pShader->Bind_RawValue("g_SourceCharacterRow", &noSurface, sizeof(noSurface));
	pShader->Bind_RawValue("g_HasSurfaceDefinition", &noSurface, sizeof(noSurface));
	const bool_t hasValidOverride =
		nullptr != pEmissiveOverride && pEmissiveOverride->isEnabled &&
		std::isfinite(pEmissiveOverride->fIntensity) &&
		pEmissiveOverride->fIntensity > 0.f;
	const auto* hitSurface = Model.Get_MaterialSurface(iMeshIndex);
	const bool_t nativeHit = hasValidOverride && pEmissiveOverride->usesSurfaceDetailMask &&
		SourceHitColorRow(hitSurface) != UINT32_MAX;
	const uint32_t iHasFullSurfaceEmissiveOverride =
		hasValidOverride && !nativeHit ? 1u : 0u;
	const float4_t vFullSurfaceEmissiveColor = hasValidOverride ?
		pEmissiveOverride->vColor : float4_t(1.f, 1.f, 1.f, 1.f);
	const f32_t fFullSurfaceEmissiveIntensity = hasValidOverride ?
		pEmissiveOverride->fIntensity : 0.f;
	const uint32_t iFullSurfaceEmissiveMaskMode =
		hasValidOverride && pEmissiveOverride->usesSurfaceDetailMask ? 1u : 0u;
	HRESULT hFirstBindFailure = S_OK;
	const auto BindFailed = [&hFirstBindFailure](const HRESULT hResult)
	{
		if (SUCCEEDED(hResult))
			return false;
		if (SUCCEEDED(hFirstBindFailure))
			hFirstBindFailure = hResult;
		return true;
	};

	const auto* surface = nativeBinaryBasePass ? Model.Get_MaterialSurface(iMeshIndex) : nullptr;
	if (surface && surface->family == Engine::MODEL_SURFACE_FAMILY::SOURCE_CHARACTER &&
		surface->sourceCharacter.program != 0u)
	{
		// The binary native base branch reads its own texture registers and returns
		// before legacy lighting/dye. Preserve diffuse admission/reset and the
		// independent hit/skill glow, then publish the same native material row.
		if (BindFailed(diffuseOverride ? pShader->Bind_Texture("g_DiffuseTexture", diffuseOverride) : Model.Bind_Material(
			pShader, "g_DiffuseTexture", iMeshIndex, aiTextureType_DIFFUSE, 0)) ||
			BindFailed(pShader->Bind_RawValue("g_HasFullSurfaceEmissiveOverride",
				&iHasFullSurfaceEmissiveOverride, sizeof(iHasFullSurfaceEmissiveOverride))) ||
			BindFailed(pShader->Bind_RawValue("g_FullSurfaceEmissiveColor",
				&vFullSurfaceEmissiveColor, sizeof(vFullSurfaceEmissiveColor))) ||
			BindFailed(pShader->Bind_RawValue("g_FullSurfaceEmissiveIntensity",
				&fFullSurfaceEmissiveIntensity, sizeof(fFullSurfaceEmissiveIntensity))) ||
			BindFailed(pShader->Bind_RawValue("g_FullSurfaceEmissiveMaskMode",
				&iFullSurfaceEmissiveMaskMode, sizeof(iFullSurfaceEmissiveMaskMode))))
		{
			return hFirstBindFailure;
		}
		const HRESULT native = Model.Bind_SourceCharacter(pShader, iMeshIndex);
		return FAILED(native) ? native : BindSourceHitColor(pShader, hitSurface, pEmissiveOverride);
	}

	const uint32_t iHasNormal = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_NORMALS) ? 1u : 0u;
	const uint32_t iHasSpecular = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_SPECULAR) ? 1u : 0u;
	const uint32_t iHasEmissive = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;

	if (BindFailed(diffuseOverride ? pShader->Bind_Texture("g_DiffuseTexture", diffuseOverride) : Model.Bind_Material(
		pShader, "g_DiffuseTexture", iMeshIndex, aiTextureType_DIFFUSE, 0)) ||
		BindFailed(pShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal))) ||
		(0u != iHasNormal && BindFailed(Model.Bind_Material(
			pShader, "g_NormalTexture", iMeshIndex, aiTextureType_NORMALS, 0))) ||
		BindFailed(pShader->Bind_RawValue(
			"g_HasSpecularTexture", &iHasSpecular, sizeof(iHasSpecular))) ||
		BindFailed(pShader->Bind_RawValue("g_SpecularIntensity",
			&Profile.fSpecularIntensity, sizeof(Profile.fSpecularIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_SpecularPower",
			&Profile.fSpecularPower, sizeof(Profile.fSpecularPower))) ||
		(0u != iHasSpecular && BindFailed(Model.Bind_Material(
			pShader, "g_SpecularTexture", iMeshIndex, aiTextureType_SPECULAR, 0))) ||
		BindFailed(pShader->Bind_RawValue(
			"g_HasEmissiveTexture", &iHasEmissive, sizeof(iHasEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_EmissiveColor",
			&Profile.vEmissiveColor, sizeof(Profile.vEmissiveColor))) ||
		BindFailed(pShader->Bind_RawValue("g_EmissiveIntensity",
			&Profile.fEmissiveIntensity, sizeof(Profile.fEmissiveIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_HasFullSurfaceEmissiveOverride",
			&iHasFullSurfaceEmissiveOverride,
			sizeof(iHasFullSurfaceEmissiveOverride))) ||
		BindFailed(pShader->Bind_RawValue("g_FullSurfaceEmissiveColor",
			&vFullSurfaceEmissiveColor,
			sizeof(vFullSurfaceEmissiveColor))) ||
		BindFailed(pShader->Bind_RawValue("g_FullSurfaceEmissiveIntensity",
			&fFullSurfaceEmissiveIntensity,
			sizeof(fFullSurfaceEmissiveIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_FullSurfaceEmissiveMaskMode",
			&iFullSurfaceEmissiveMaskMode,
			sizeof(iFullSurfaceEmissiveMaskMode))) ||
		(0u != iHasEmissive && BindFailed(Model.Bind_Material(
			pShader, "g_EmissiveTexture", iMeshIndex, aiTextureType_EMISSIVE, 0))))
	{
		return hFirstBindFailure;
	}

	const Engine::MODEL_COLOR_TINT* pColorTint =
		Model.Get_MaterialColorTint(iMeshIndex);
	const uint32_t iHasDyeMask = nullptr != pColorTint &&
		pColorTint->isEnabled &&
		Model.Has_MaterialTexture(iMeshIndex, aiTextureType_BASE_COLOR) ?
		1u : 0u;
	/* Some shaders behind this utility never declare the dye contract; their
	compiled default of 0 is already right, so a rejected name is not an
	error. It still has to be attempted every mesh, or a dyed mesh would leave
	the flag stuck on for the undyed mesh that follows. */
	pShader->Bind_RawValue("g_HasDyeMask", &iHasDyeMask, sizeof(iHasDyeMask));
	const uint32_t iDyeMode = nullptr != pColorTint && pColorTint->isHairMask ? 1u : 0u;
	pShader->Bind_RawValue("g_DyeIsHair", &iDyeMode, sizeof(iDyeMode));
	/* Identity on every material the creation screen has not repainted, so this is a no-op
	everywhere else. Bound every mesh for the same reason the dye flag is. */
	static constexpr float4_t IDENTITY_TINT{ 1.f, 1.f, 1.f, 1.f };
	const float4_t* pDiffuseTint = Model.Get_MaterialDiffuseTint(iMeshIndex);
	pShader->Bind_RawValue("g_DiffuseTint",
		nullptr != pDiffuseTint ? pDiffuseTint : &IDENTITY_TINT, sizeof(float4_t));
	if (0u != iHasDyeMask &&
		(BindFailed(Model.Bind_Material(pShader, "g_DyeMaskTexture",
			iMeshIndex, aiTextureType_BASE_COLOR, 0)) ||
		BindFailed(pShader->Bind_RawValue("g_DyeDiffuseColor",
			&pColorTint->vDiffuse, sizeof(pColorTint->vDiffuse))) ||
		BindFailed(pShader->Bind_RawValue("g_DyeRegionA",
			&pColorTint->vRegionA, sizeof(pColorTint->vRegionA))) ||
		BindFailed(pShader->Bind_RawValue("g_DyeRegionB",
			&pColorTint->vRegionB, sizeof(pColorTint->vRegionB))) ||
		BindFailed(pShader->Bind_RawValue("g_DyeRegionC",
			&pColorTint->vRegionC, sizeof(pColorTint->vRegionC)))))
	{
		return hFirstBindFailure;
	}
	const HRESULT native = Model.Bind_SourceCharacter(pShader, iMeshIndex);
	return FAILED(native) ? native : BindSourceHitColor(pShader, hitSurface, pEmissiveOverride);
}

HRESULT Client::Render_CombatHoverSilhouetteMesh(Engine::CModel& model,
    const shared_ptr<Engine::CShader>& shader, const uint32_t meshIndex, const COMBAT_HOVER_PHASE phase)
{
    if (!shader || meshIndex >= model.Get_NumMeshes()) return E_INVALIDARG;
    const auto viewport = Engine::CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return S_FALSE;
    const float2_t width = {8.f / viewport.x, 8.f / viewport.y};
    const uint32_t stencil = phase == COMBAT_HOVER_PHASE::CLEAR ? 0u : 0x80u;
    if (FAILED(shader->Bind_RawValue("g_CombatHoverNdcWidth", &width, sizeof(width))) ||
        FAILED(shader->Bind_RawValue("g_CombatHoverStencilReference", &stencil, sizeof(stencil))) ||
        FAILED(shader->Begin(phase == COMBAT_HOVER_PHASE::OUTLINE ? 21u : 20u))) return E_FAIL;
    return model.Render(meshIndex);
}

HRESULT Client::Render_CombatHoverMesh(Engine::CModel& Model,
    const shared_ptr<Engine::CShader>& pShader, const uint32_t iMeshIndex,
    const DEFERRED_EMISSIVE_OVERRIDE* pOverride, const bool_t isSkinned,
    const bool_t forward, const bool_t reflected)
{
    if (!pOverride || !pOverride->isCombatHovered) return S_FALSE;
    if (!pShader || iMeshIndex >= Model.Get_NumMeshes()) return E_INVALIDARG;
    const float2_t viewport = Engine::CGameInstance::Get().Get_ViewportSize();
    if (viewport.x <= 0.f || viewport.y <= 0.f) return S_FALSE;
    // Two physical pixels, independent of boss scale or distance.
    const float2_t ndc = { 4.f / viewport.x, 4.f / viewport.y };
    const uint32_t pass = (isSkinned ? 18u : 28u) + (forward ? 1u : 0u);
    const uint32_t reflectedWinding = reflected ? 1u : 0u;
    if (FAILED(pShader->Bind_RawValue("g_CombatHoverReflected", &reflectedWinding, sizeof(reflectedWinding))) ||
        FAILED(pShader->Bind_RawValue("g_CombatHoverNdcWidth", &ndc, sizeof(ndc))) ||
        FAILED(pShader->Begin(pass))) return E_FAIL;
    return Model.Render(iMeshIndex);
}

HRESULT Client::Bind_CombatPresentationInputs(Engine::CModel& Model,
    const shared_ptr<Engine::CShader>& pShader, const uint32_t meshIndex,
    const DEFERRED_EMISSIVE_OVERRIDE& presentation)
{
    if (!pShader || meshIndex >= Model.Get_NumMeshes()) return E_INVALIDARG;
    const auto* surface = Model.Get_MaterialSurface(meshIndex);
    const bool nativeHit = presentation.usesSurfaceDetailMask && SourceHitColorRow(surface) != UINT32_MAX;
    const uint32_t enabled = !nativeHit && presentation.isEnabled && std::isfinite(presentation.fIntensity) &&
        presentation.fIntensity > 0.f ? 1u : 0u;
    const uint32_t maskMode = presentation.usesSurfaceDetailMask ? 1u : 0u;
    const float intensity = enabled ? presentation.fIntensity : 0.f;
    if (FAILED(pShader->Bind_RawValue("g_HasFullSurfaceEmissiveOverride", &enabled, sizeof(enabled))) ||
        FAILED(pShader->Bind_RawValue("g_FullSurfaceEmissiveColor", &presentation.vColor, sizeof(presentation.vColor))) ||
        FAILED(pShader->Bind_RawValue("g_FullSurfaceEmissiveIntensity", &intensity, sizeof(intensity))) ||
        FAILED(pShader->Bind_RawValue("g_FullSurfaceEmissiveMaskMode", &maskMode, sizeof(maskMode)))) return E_FAIL;
    return BindSourceHitColor(pShader, surface, &presentation);
}
