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
	static void Prewarm_Archetype(
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext,
		const std::string& strArchetypeId);
	static void Set_Ignored(const EFFECT_V2_TARGET& Target, bool_t bIgnored);
	static void Invalidate_Caches();
	static const std::string& Last_Error();

	/* Free-running group lane for the tool: the group clock starts at 0 on
	   Play_Group and advances only through Advance_FreeGroups. No target,
	   no bone following. Returns 0 when the group cannot be expanded; a
	   finished group drops its handle (Group_Seconds < 0). */
	static uint32_t Play_Group(
		const std::string& strGroupId,
		const float4x4_t& PivotWorld,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Stop_Group(uint32_t iHandle);
	static f32_t Group_Seconds(uint32_t iHandle);
	static void Advance_FreeGroups(
		f32_t fTimeDelta,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
};

NS_END
