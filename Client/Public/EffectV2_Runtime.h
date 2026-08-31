#pragma once

#include "Client_Defines.h"
#include "EffectV2_Target.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CEffectV2Runtime final
{
public:
	static void Notify_Clip(
		const EFFECT_V2_TARGET& Target,
		const char_t* pClipName);
	static void Tick(
		const EFFECT_V2_TARGET& Target,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Server pattern stage clock: pActionId is the stage actionId ("" = no
	   stage), fAgeSeconds the stage-local age. Bindings keyed by stage spawn
	   when the age crosses their startMs. */
	static void Sync_Stage(
		const EFFECT_V2_TARGET& Target,
		const char_t* pActionId,
		f32_t fAgeSeconds,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Destructive seek/reset for a tool-owned local preview target.  Product
	   gameplay must continue to use Sync_Stage; CValtan admits this call only
	   for its non-authoritative Action Composition preview instance. */
	static void Reset_LocalPreviewTarget(const EFFECT_V2_TARGET& Target);
	static void Prewarm_Archetype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strArchetypeId);
	static void Set_Ignored(const EFFECT_V2_TARGET& Target, bool_t bIgnored);
	static void Invalidate_Caches();
	static const std::string& Last_Error();
};

NS_END
