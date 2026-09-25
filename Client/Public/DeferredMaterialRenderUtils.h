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
	true: camera-facing rim for a hit response, preserving the surface. */
	bool_t usesSurfaceDetailMask = false;
	// Per-object presentation state; shared materials remain immutable.
	bool_t isCombatHovered = false;
};

DEFERRED_MATERIAL_PROFILE Resolve_DeferredMaterialProfile(
	std::string_view strProfileId,
	std::string_view strMaterialName);

HRESULT Bind_DeferredMaterialInputs(
	Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader,
	uint32_t iMeshIndex,
	const DEFERRED_MATERIAL_PROFILE& Profile = {},
	const DEFERRED_EMISSIVE_OVERRIDE* pEmissiveOverride = nullptr,
	const ComPtr<ID3D11ShaderResourceView>& diffuseOverride = nullptr,
	// Only callers that selected a binary character base pass may omit legacy inputs.
	bool_t nativeBinaryBasePass = false);

HRESULT Bind_CombatPresentationInputs(Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader, uint32_t meshIndex,
	const DEFERRED_EMISSIVE_OVERRIDE& presentation);

// An actor's complete visible body/parts are masked before outlining. The final
// CLEAR pass returns only the temporary high stencil bit to its previous zero.
enum class COMBAT_HOVER_PHASE { MASK, OUTLINE, CLEAR };
HRESULT Render_CombatHoverSilhouetteMesh(Engine::CModel& Model,
    const shared_ptr<Engine::CShader>& pShader, uint32_t meshIndex, COMBAT_HOVER_PHASE phase);

// Called after the visible mesh, with the same world, bone and material inputs.
HRESULT Render_CombatHoverMesh(Engine::CModel& Model,
	const shared_ptr<Engine::CShader>& pShader, uint32_t iMeshIndex,
	const DEFERRED_EMISSIVE_OVERRIDE* pOverride, bool_t isSkinned,
	bool_t forward = false, bool_t reflected = false);

NS_END
