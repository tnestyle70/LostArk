#pragma once

#include "Client_Defines.h"
#include "EffectV2_Document.h"
#include "EffectV2_Target.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class EFFECT_V2_CATALOG_SNAPSHOT;

struct EFFECT_V2_GROUP_PLAYBACK_DESC final
{
	float4x4_t PivotWorld{};
	/* Real elapsed age of the owning occurrence. The runtime converts this to
	   the authored group clock with fPlaybackRate, including late snapshots. */
	f32_t fInitialAgeSeconds = 0.f;
	f32_t fPlaybackRate = 1.f;
	bool_t bProductOwned = false;
};

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
	/* Local Action Composition preview only. The caller supplies the current
	   parsed authoring snapshot, so saved groups/bindings can be reviewed on the
	   next seek without a separate build or publish step. */
	static void Sync_StageAuthoring(
		const EFFECT_V2_TARGET& Target,
		const char_t* pActionId,
		f32_t fAgeSeconds,
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
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
	static bool_t Prewarm_Group(
		const EFFECT_V2_GROUP& Group,
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Set_Ignored(const EFFECT_V2_TARGET& Target, bool_t bIgnored);
	/* Drop lazy source caches and re-arm active clip/stage lanes. Existing
	   spawned effects keep their authored stop policy; the next occurrence reads
	   the newly saved Data/Effects/V2 documents. */
	static void Invalidate_Caches();
	static const std::string& Last_Error();

	/* Free-running group lane for the tool: plays an in-memory group against
	   one immutable, typed authoring snapshot.  Leaf resolution never falls
	   through to Product playback caches.  The clock starts at 0 and advances
	   only through Advance_FreeGroups. No target, no bone following. Returns
	   0 when the snapshot/group closure cannot be staged; a finished group
	   drops its handle (Group_Seconds < 0). Update_Group re-applies the edited
	   children to a running preview: offset/yaw/scale move spawned objects at
	   once, timing and stop policy retarget children that have not spawned yet,
	   appended children join the lane. */
	static uint32_t Play_Group(
		const EFFECT_V2_GROUP& Group,
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
		const float4x4_t& PivotWorld,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static uint32_t Play_Group(
		const EFFECT_V2_GROUP& Group,
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
		const EFFECT_V2_GROUP_PLAYBACK_DESC& Playback,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Update_Group(uint32_t iHandle, const EFFECT_V2_GROUP& Group);
	/* Moves the lane pivot; the next Update_Group re-places spawned objects. */
	static void Set_GroupPivot(uint32_t iHandle, const float4x4_t& PivotWorld);
	static void Stop_Group(uint32_t iHandle);
	static f32_t Group_Seconds(uint32_t iHandle);
	/* Returns one deferred document/prototype/object spawn failure exactly once.
	   The tool consumes it and stops the affected preview lane. */
	static bool_t Consume_GroupFailure(
		uint32_t iHandle, std::string& strOutFailure);
	static void Advance_FreeGroups(
		f32_t fTimeDelta,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Product-owned groups have a MainApp clock independent from whether the
	   authoring tool exists or is visible. */
	static void Advance_ProductGroups(
		f32_t fTimeDelta,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
};

NS_END
