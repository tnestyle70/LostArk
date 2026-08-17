#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <string_view>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

struct DEFERRED_MATERIAL_PROFILE
{
	float4_t vEmissiveColor = float4_t(1.f, 1.f, 1.f, 1.f);
	f32_t fEmissiveIntensity = 1.f;
	f32_t fSpecularIntensity = 1.f;
	f32_t fSpecularPower = 50.f;
};

/* Transient presentation energy layered over an authored character material.
The diffuse texture remains the detail mask; no shared CMaterial is mutated. */
struct DEFERRED_EMISSIVE_OVERRIDE
{
	bool_t isEnabled = false;
	float4_t vColor = float4_t(1.f, 1.f, 1.f, 1.f);
	f32_t fIntensity = 0.f;
	/* false: diffuse luminance weights the whole surface (skill glow).
	true: normal-map bump strength times specular lights only creases and
	metal, so a hit flash keeps the silhouette readable. */
	bool_t usesSurfaceDetailMask = false;
};

DEFERRED_MATERIAL_PROFILE Resolve_DeferredMaterialProfile(
	std::string_view strProfileId,
	std::string_view strMaterialName);

HRESULT Bind_DeferredMaterialInputs(
	Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader,
	uint32_t iMeshIndex,
	const DEFERRED_MATERIAL_PROFILE& Profile = {},
	const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = nullptr);

NS_END
