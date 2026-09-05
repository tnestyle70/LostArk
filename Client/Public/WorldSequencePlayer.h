#pragma once

#include "Client_Defines.h"
#include "DeployPropRuntime.h"
#include "MapAssetCatalog.h"
#include "MapPlacementRuntime.h"
#include "WorldSequenceDocument.h"

#include <string>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

/* One playback path for authored world sequences. The Map Tool preview and the
   product level both evaluate a sequence here so a sequence can never look one
   way in the editor and another way in the game. The player only reads the
   document; starting and stopping stay with the caller that owns the gameplay
   reason for playing. */
class CWorldSequencePlayer final
{
public:
	struct TARGET_SET final
	{
		uint32_t levelIndex = {};
		const CMapAssetCatalog* pCatalog = nullptr;
		std::vector<MAP_RUNTIME_PLACED_ENTRY>* pPlacements = nullptr;
		CDeployPropRuntime* pDeployRuntime = nullptr;

		bool_t Is_Complete() const noexcept
		{
			return nullptr != pCatalog && nullptr != pPlacements &&
				nullptr != pDeployRuntime;
		}
	};

	/* Baseline transforms are captured when an instance starts so a sequence
	   composes against the placed pose instead of accumulating drift. */
	struct PLACEMENT_BASELINE final
	{
		uint64_t placementId = {};
		MAP_PLACEMENT_RECORD record;
		bool_t runtimeVisible = false;
	};

	CWorldSequencePlayer() = default;
	CWorldSequencePlayer(const CWorldSequencePlayer&) = delete;
	CWorldSequencePlayer& operator=(const CWorldSequencePlayer&) = delete;

	/* Reads the published runtime document beside the executable. The live
	   targets are required because the document is admitted against the
	   placements and Deploy props this level actually created. */
	bool_t Load_Area(const std::string& areaId, const TARGET_SET& targets);
	void Clear();

	bool_t Is_Ready() const noexcept
	{
		return !m_Document.Get_AreaId().empty();
	}
	const CWorldSequenceDocument& Get_Document() const noexcept
	{
		return m_Document;
	}
	const std::string& Get_Status() const noexcept { return m_Status; }

	/* Starts one authored instance. Restarting an already playing instance
	   rewinds it against the baseline captured by the first start. */
	bool_t Play(const std::string& instanceId, const TARGET_SET& targets);
	bool_t Is_Playing(const std::string& instanceId) const;
	/* The camera cue runs on the cutscene's own clock. Only the player owns
	   that clock, so it hands out a read-only sample instead of letting a
	   second owner count the same time. false means the instance is not
	   playing and the caller must not pose a camera from a stale value. */
	bool_t Try_GetElapsedMs(
		const std::string& instanceId,
		f32_t& outElapsedMs) const;
	/* Authoring needs to hold a cutscene on one frame and step to any point
	   of it. Paused instances stop advancing but keep their baselines, so a
	   scrub never restarts the sequence or loses the placed pose. */
	void Set_Paused(bool_t paused) { m_bPaused = paused; }
	bool_t Is_Paused() const noexcept { return m_bPaused; }
	/* Moves every playing instance to the same wall-clock point and applies
	   that frame at once. false means nothing is playing to scrub. */
	bool_t Seek_AllToMs(f32_t elapsedMs, const TARGET_SET& targets);
	/* The longest authored span across the playing instances, so the tool can
	   size a scrub bar without guessing. */
	f32_t Get_LongestElapsedSpanMs() const;

	/* Stopping hands every animated Deploy target back: an authoring preview
	   left running blocks the prop's state from being set, so a second play
	   could never restore it. */
	void Stop_All(const TARGET_SET& targets);

	/* Advances every playing instance and writes the sampled presentation. A
	   target that disappears stops only its own instance. */
	void Update(f32_t timeDelta, const TARGET_SET& targets);

	/* Shared evaluation used by both the product player and the Map Tool. */
	static WORLD_SEQUENCE_TRANSFORM_KEY Sample_Track(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const WORLD_SEQUENCE_TRACK& track,
		f32_t timeMs);
	static MAP_PLACEMENT_RECORD Compose_SampledRecord(
		const MAP_PLACEMENT_RECORD& baseline,
		bool_t baselineRuntimeVisible,
		const WORLD_SEQUENCE_TRANSFORM_KEY& key);
	static const WORLD_SEQUENCE_TRACK* Find_Track(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId);
	static const WORLD_SEQUENCE_ANIMATION_TRACK* Find_AnimationTrack(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId);
	/* The clip a slot is playing at this point of the sequence, plus the time
	   that clip's window ends. A slot with one track answers with that track
	   and the sequence duration, so a chain and a single clip read the same. */
	static const WORLD_SEQUENCE_ANIMATION_TRACK* Find_AnimationTrackAt(
		const WORLD_SEQUENCE_TEMPLATE& sequence,
		const std::string& slotId,
		f32_t localMs,
		f32_t& outWindowEndMs);
	static MAP_RUNTIME_PLACED_ENTRY* Find_Placement(
		std::vector<MAP_RUNTIME_PLACED_ENTRY>& placements,
		uint64_t placementId);
	static bool_t Try_ParseTargetId(
		const WORLD_SEQUENCE_BINDING& binding,
		uint64_t& outTargetId);
	/* Writes one composed record onto its live presentation. The model cache
	   belongs to the caller so a static batch member is cloned once per asset
	   instead of once per frame. */
	static bool_t Apply_RuntimeRecord(
		const TARGET_SET& targets,
		std::unordered_map<std::string, shared_ptr<CModel>>& modelCache,
		MAP_RUNTIME_PLACED_ENTRY& entry,
		const MAP_PLACEMENT_RECORD& record);

private:
	struct ACTIVE_INSTANCE final
	{
		std::string instanceId;
		f32_t elapsedMs = 0.f;
		std::vector<PLACEMENT_BASELINE> placementBaselines;
		std::vector<uint64_t> deployTargets;
	};

	/* A finished sequence keeps its last authored frame; only a broken one
	   gives its targets back. Releasing on completion would snap an unfolded
	   bridge back to the folded pose the clip starts from. */
	enum class APPLY_RESULT
	{
		PLAYING,
		FINISHED,
		FAILED,
	};
	APPLY_RESULT Apply_Instance(ACTIVE_INSTANCE& active, const TARGET_SET& targets);
	void Release_DeployPreviews(
		const ACTIVE_INSTANCE& active,
		const TARGET_SET& targets);

private:
	CWorldSequenceDocument m_Document;
	bool_t m_bPaused = false;
	std::vector<ACTIVE_INSTANCE> m_Active;
	std::unordered_map<std::string, shared_ptr<CModel>> m_ModelCache;
	std::string m_Status;
};

NS_END
