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

DEFERRED_MATERIAL_PROFILE Resolve_DeferredMaterialProfile(
	std::string_view strProfileId,
	std::string_view strMaterialName);

HRESULT Bind_DeferredMaterialInputs(
	Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader,
	uint32_t iMeshIndex,
	const DEFERRED_MATERIAL_PROFILE& Profile = {});

NS_END
