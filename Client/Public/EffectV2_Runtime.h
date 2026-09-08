#pragma once

#include "Client_Defines.h"
#include "EffectV2_Document.h"
#include "EffectV2_Target.h"
#include "Engine_Defines.h"

#include <memory>
#include <deque>
#include <span>
#include <string>

NS_BEGIN(Client)

class EFFECT_V2_CATALOG_SNAPSHOT;

/* Recorded owner-root history. A discontinuity marks the jump into a sample;
   intervals crossing it and requests outside recorded time fail explicitly. */
class EFFECT_V2_PIVOT_HISTORY final
{
public:
	void Reset();
	bool_t Record(f32_t fGroupAgeSeconds, const float4x4_t& Pivot,
		bool_t bDiscontinuity, std::string& strOutError);
	bool_t Sample(f32_t fGroupAgeSeconds, float4x4_t& OutPivot,
		std::string& strOutError) const;
private:
	struct SAMPLE final { f32_t fSeconds; float4x4_t Pivot; bool_t bDiscontinuity; };
	std::deque<SAMPLE> m_Samples;
};

struct EFFECT_V2_GROUP_PLAYBACK_DESC final
{
	float4x4_t PivotWorld{};
	/* Real elapsed age of the owning occurrence. The runtime converts this to
	   the authored group clock with fPlaybackRate, including late snapshots. */
	f32_t fInitialAgeSeconds = 0.f;
	f32_t fPlaybackRate = 1.f;
	/* -1 preserves authored lifetime. Zero repeats until Stop_Group; positive
	   values stretch each child through this occurrence's real duration. */
	f32_t fDurationSeconds = -1.f;
	/* Negative values retain the leaf envelope. Explicit zero disables fade.
	   Dissolve-out start/end are normalized positions in the child lifetime. */
	f32_t fFadeInSeconds = -1.f;
	f32_t fFadeOutSeconds = -1.f;
	f32_t fDissolveOutStart = -1.f;
	f32_t fDissolveOutEnd = -1.f;
	bool_t bProductOwned = false;
	/* Group authored seconds -> recorded/authored root. Child placement is
	   composed internally before an emitter samples its individual births. */
	CEffectV2Object::PIVOT_SAMPLER PivotSampler;
	/* Caller owns absolute group time; render/layer updates cannot advance it. */
	bool_t bExternalClock = false;
};

/* One authored animation occurrence expressed in the owning Stage wall clock.
   Effect binding startMs stays in the model source clock; the runtime uses
   this map to apply sourceStart/playRate and to distinguish repeated uses of
   the same clip name. */
struct EFFECT_V2_CLIP_OCCURRENCE_CLOCK final
{
	std::string strClipOccurrenceId;
	f32_t fStageWallStartSeconds = 0.f;
	f32_t fSourceStartSeconds = 0.f;
	f32_t fSourceDurationSeconds = 0.f;
	f32_t fLoopWallDurationSeconds = 0.f;
	f32_t fPlaybackRate = 1.f;
	bool_t bLoop = false;

	bool operator==(const EFFECT_V2_CLIP_OCCURRENCE_CLOCK&) const = default;
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
	/* Local Composition's legacy clip lane uses the sampled body clock. The
	   caller resets on seek/occurrence changes; product and Effect Tool clocks
	   retain the ordinary Tick contract. */
	static void Sample_LocalClipPreview(
		const EFFECT_V2_TARGET& Target, bool_t bPaused, f32_t fPlaybackRate,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Server pattern stage clock: pActionId is the stage actionId ("" = no
	   stage), fAgeSeconds the stage-local age. Bindings keyed by stage spawn
	   when the age crosses their startMs. */
	static void Sync_Stage(
		const EFFECT_V2_TARGET& Target,
		const char_t* pActionId,
		f32_t fAgeSeconds,
		std::span<const EFFECT_V2_CLIP_OCCURRENCE_CLOCK> ClipOccurrences,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Compatibility overload for a Stage-clock-only caller. Clip-occurrence
	   rows fail closed until their typed occurrence map is supplied. */
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
		std::span<const EFFECT_V2_CLIP_OCCURRENCE_CLOCK> ClipOccurrences,
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
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
	static uint64_t Cache_Generation();
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
	/* A leaf uses the same instance/clock/stop handle through a one-child group. */
	static uint32_t Play_Leaf(
		const std::string& strEffectId,
		std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot,
		const EFFECT_V2_GROUP_PLAYBACK_DESC& Playback,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Update_Group(uint32_t iHandle, const EFFECT_V2_GROUP& Group);
	static bool_t Sample_Group(uint32_t iHandle, f32_t fGroupAgeSeconds,
		bool_t bPaused, const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static bool_t Seek_Group(uint32_t iHandle, f32_t fGroupAgeSeconds,
		const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	/* Moves the lane pivot and every live child immediately. */
	static void Set_GroupPivot(uint32_t iHandle, const float4x4_t& PivotWorld);
	/* Rebuild only this externally clocked occurrence at its current age. The
	   handle, pause state, snapshot and other lanes survive the placement edit. */
	static bool_t Rebuild_GroupPlacement(uint32_t iHandle, const float4x4_t& PivotWorld,
		CEffectV2Object::PIVOT_SAMPLER Sampler, const ComPtr<ID3D11Device>& pDevice,
		const ComPtr<ID3D11DeviceContext>& pContext);
	static void Set_GroupPaused(uint32_t iHandle, bool_t bPaused);
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
