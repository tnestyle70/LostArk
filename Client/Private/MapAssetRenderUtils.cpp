#include "MapAssetRenderUtils.h"

#include "Model.h"
#include "Shader.h"

uint32_t CMapAssetRenderUtils::Select_Pass(const MAP_ASSET_RENDER_PROFILE& profile, 
	bool_t mirrored)
{
	//map asset의 profile의 cullmode를 사용해서 culloffset 변수와 modeoffset 변수를 설정해준다
	MAP_ASSET_CULL_MODE cullMode = profile.cullMode;

	if (mirrored && MAP_ASSET_CULL_MODE::TWO_SIDED != cullMode)
	{
		cullMode = MAP_ASSET_CULL_MODE::CULL_BACK == cullMode ?
			MAP_ASSET_CULL_MODE::CULL_FRONT :
			MAP_ASSET_CULL_MODE::CULL_BACK;
	}
	//culloffset 설정
	const uint32_t  cullOfset =
		MAP_ASSET_CULL_MODE::CULL_BACK == cullMode ? 0u :
		MAP_ASSET_CULL_MODE::CULL_FRONT == cullMode ? 1u : 2u;
	//deffered translucent background
	const uint32_t modeOffset =
		MAP_ASSET_RENDER_MODE::DEFERRED == profile.renderMode ? 0u :
		MAP_ASSET_RENDER_MODE::TRANSLUCENT == profile.renderMode ? 3u :
		MAP_ASSET_RENDER_MODE::BACKGROUND == profile.renderMode ? 6u : 9u;
	
	return modeOffset + cullOfset;
}

HRESULT Client::CMapAssetRenderUtils::Bind_Material(
	const shared_ptr<Engine::CModel>& model,
	const shared_ptr<Engine::CShader>& shader,
	uint32_t meshIndex,
	const MAP_ASSET_RENDER_PROFILE& profile,
	f32_t elapsedTime)
{
	if (nullptr == model ||
		nullptr == shader ||
		meshIndex >= model->Get_NumMeshes())
	{
		return E_INVALIDARG;
	}

	const uint32_t hasNormalTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_NORMALS) ? 1u : 0u;

	const uint32_t hasEmissiveTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;

	const uint32_t hasSpecularTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_SPECULAR) ? 1u : 0u;

	const uint32_t hasOpacityTexture =
		model->Has_MaterialTexture(
			meshIndex, aiTextureType_OPACITY) ? 1u : 0u;

	const float2_t uvOffset(
		profile.uvSpeed.x * elapsedTime,
		profile.uvSpeed.y * elapsedTime);

	if (FAILED(model->Bind_Material(
		shader,
		"g_DiffuseTexture",
		meshIndex,
		aiTextureType_DIFFUSE)) ||

		FAILED(shader->Bind_RawValue(
			"g_UVScale",
			&profile.uvScale,
			sizeof(profile.uvScale))) ||

		FAILED(shader->Bind_RawValue(
			"g_UVOffset",
			&uvOffset,
			sizeof(uvOffset))) ||

		FAILED(shader->Bind_RawValue(
			"g_Opacity",
			&profile.opacity,
			sizeof(profile.opacity))) ||

		FAILED(shader->Bind_RawValue(
			"g_OpacityPower",
			&profile.opacityPower,
			sizeof(profile.opacityPower))) ||

		FAILED(shader->Bind_RawValue(
			"g_ColorTint",
			&profile.colorTint,
			sizeof(profile.colorTint))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasNormalTexture",
			&hasNormalTexture,
			sizeof(hasNormalTexture))) ||

		(0 != hasNormalTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_NormalTexture",
				meshIndex,
				aiTextureType_NORMALS))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasEmissiveTexture",
			&hasEmissiveTexture,
			sizeof(hasEmissiveTexture))) ||

		FAILED(shader->Bind_RawValue(
			"g_EmissiveIntensity",
			&profile.emissiveIntensity,
			sizeof(profile.emissiveIntensity))) ||

		(0 != hasEmissiveTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_EmissiveTexture",
				meshIndex,
				aiTextureType_EMISSIVE))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasSpecularTexture",
			&hasSpecularTexture,
			sizeof(hasSpecularTexture))) ||

		FAILED(shader->Bind_RawValue(
			"g_SpecularIntensity",
			&profile.specularIntensity,
			sizeof(profile.specularIntensity))) ||

		FAILED(shader->Bind_RawValue(
			"g_SpecularPower",
			&profile.specularPower,
			sizeof(profile.specularPower))) ||

		(0 != hasSpecularTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_SpecularTexture",
				meshIndex,
				aiTextureType_SPECULAR))) ||

		FAILED(shader->Bind_RawValue(
			"g_HasOpacityTexture",
			&hasOpacityTexture,
			sizeof(hasOpacityTexture))) ||

		(0 != hasOpacityTexture &&
			FAILED(model->Bind_Material(
				shader,
				"g_OpacityTexture",
				meshIndex,
				aiTextureType_OPACITY))))
	{
		return E_FAIL;
	}

	return S_OK;
}
