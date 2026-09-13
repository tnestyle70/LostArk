#include "DeferredMaterialRenderUtils.h"

#include "BinaryAsset/ModelAssetData.h"
#include "Model.h"
#include "Shader.h"

#include <cmath>

namespace
{
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
	const uint32_t iHasFullSurfaceEmissiveOverride =
		hasValidOverride ? 1u : 0u;
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
		return Model.Bind_SourceCharacter(pShader, iMeshIndex);
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
	return Model.Bind_SourceCharacter(pShader, iMeshIndex);
}
