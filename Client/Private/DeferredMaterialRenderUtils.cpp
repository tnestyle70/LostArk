#include "DeferredMaterialRenderUtils.h"

#include "Model.h"
#include "Shader.h"

namespace
{
	constexpr std::string_view VALTAN_MATERIAL_PROFILE =
		"material.valtan.monster-base.v1";

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
		return 1.f;
	}
}

Client::DEFERRED_MATERIAL_PROFILE Client::Resolve_DeferredMaterialProfile(
	std::string_view strProfileId,
	std::string_view strMaterialName)
{
	DEFERRED_MATERIAL_PROFILE Profile{};
	if (strProfileId == VALTAN_MATERIAL_PROFILE)
	{
		/* UE3 parent MI evidence: the E map is a grayscale carrier.  Tint and
		   intensity are material parameters, not colors baked into that map. */
		Profile.vEmissiveColor = float4_t(0.15f, 1.5f, 0.9f, 1.f);
		Profile.fEmissiveIntensity =
			Resolve_ValtanEmissiveIntensity(strMaterialName);
	}
	return Profile;
}

HRESULT Client::Bind_DeferredMaterialInputs(
	Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader,
	uint32_t iMeshIndex,
	const DEFERRED_MATERIAL_PROFILE& Profile)
{
	if (nullptr == pShader || iMeshIndex >= Model.Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

	const uint32_t iHasNormal = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_NORMALS) ? 1u : 0u;
	const uint32_t iHasSpecular = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_SPECULAR) ? 1u : 0u;
	const uint32_t iHasEmissive = Model.Has_MaterialTexture(
		iMeshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;

	if (FAILED(Model.Bind_Material(
		pShader, "g_DiffuseTexture", iMeshIndex, aiTextureType_DIFFUSE, 0)) ||
		FAILED(pShader->Bind_RawValue(
			"g_HasNormalTexture", &iHasNormal, sizeof(iHasNormal))) ||
		(0u != iHasNormal && FAILED(Model.Bind_Material(
			pShader, "g_NormalTexture", iMeshIndex, aiTextureType_NORMALS, 0))) ||
		FAILED(pShader->Bind_RawValue(
			"g_HasSpecularTexture", &iHasSpecular, sizeof(iHasSpecular))) ||
		FAILED(pShader->Bind_RawValue("g_SpecularIntensity",
			&Profile.fSpecularIntensity, sizeof(Profile.fSpecularIntensity))) ||
		FAILED(pShader->Bind_RawValue("g_SpecularPower",
			&Profile.fSpecularPower, sizeof(Profile.fSpecularPower))) ||
		(0u != iHasSpecular && FAILED(Model.Bind_Material(
			pShader, "g_SpecularTexture", iMeshIndex, aiTextureType_SPECULAR, 0))) ||
		FAILED(pShader->Bind_RawValue(
			"g_HasEmissiveTexture", &iHasEmissive, sizeof(iHasEmissive))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveColor",
			&Profile.vEmissiveColor, sizeof(Profile.vEmissiveColor))) ||
		FAILED(pShader->Bind_RawValue("g_EmissiveIntensity",
			&Profile.fEmissiveIntensity, sizeof(Profile.fEmissiveIntensity))) ||
		(0u != iHasEmissive && FAILED(Model.Bind_Material(
			pShader, "g_EmissiveTexture", iMeshIndex, aiTextureType_EMISSIVE, 0))))
	{
		return E_FAIL;
	}
	return S_OK;
}
