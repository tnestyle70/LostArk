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
	void Stop_All();

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
	std::vector<ACTIVE_INSTANCE> m_Active;
	std::unordered_map<std::string, shared_ptr<CModel>> m_ModelCache;
	std::string m_Status;
};

NS_END
