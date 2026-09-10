#pragma once

#include "Client_Defines.h"
#include "DeployPropRuntime.h"
#include "MapAssetCatalog.h"
#include "MapPlacementRuntime.h"
#include "WorldSequenceDocument.h"

#include <string>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CWorldSequenceObject;
class EFFECT_V2_CATALOG_SNAPSHOT;

/* One playback path for authored world sequences. The Map Tool preview and the
   product level both evaluate a sequence here so a sequence can never look one
   way in the editor and another way in the game. The player only reads the
   document; starting and stopping stay with the caller that owns the gameplay
   reason for playing. */
class CWorldSequencePlayer final
{
public:

	struct OBJECT_PLACEMENT final
	{
		float3_t position{};
		float3_t rotationDegrees{};
		float3_t scale{1.f, 1.f, 1.f};
		bool operator==(const OBJECT_PLACEMENT& other) const
		{
			return position.x == other.position.x && position.y == other.position.y && position.z == other.position.z &&
				rotationDegrees.x == other.rotationDegrees.x && rotationDegrees.y == other.rotationDegrees.y && rotationDegrees.z == other.rotationDegrees.z &&
				scale.x == other.scale.x && scale.y == other.scale.y && scale.z == other.scale.z;
		}
	};

	struct PLAYER_ANCHOR
	{
		uint64_t entityId = 0;
		float4x4_t world{};
		bool_t emissionOverride = false;
		bool_t liveBossAnchor = false;
	};
	struct TARGET_SET final
	{
		uint32_t levelIndex = {};
		const CMapAssetCatalog* pCatalog = nullptr;
		std::vector<MAP_RUNTIME_PLACED_ENTRY>* pPlacements = nullptr;
		CDeployPropRuntime* pDeployRuntime = nullptr;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		std::function<std::vector<PLAYER_ANCHOR>()> playerAnchors;
		// Live BODY bone pose; separate from a frozen projectile emission origin.
		std::function<bool_t(const std::string&, const std::string&, PLAYER_ANCHOR&, std::string&)> bossAnchor;
		// Occurrence-local real milliseconds at birth -> frozen world origin.
		std::function<bool_t(f32_t, float4x4_t&)> objectEmissionAnchor;

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
		bool_t restoreRuntimeVisible = false;
	};

	CWorldSequencePlayer() = default;
	~CWorldSequencePlayer();
	CWorldSequencePlayer(const CWorldSequencePlayer&) = delete;
	CWorldSequencePlayer& operator=(const CWorldSequencePlayer&) = delete;

	/* Reads the published runtime document beside the executable. The live
	   targets are required because the document is admitted against the
	   placements and Deploy props this level actually created. */
	bool_t Load_Area(const std::string& areaId, const TARGET_SET& targets);
	bool_t Set_Document(const CWorldSequenceDocument& document, const TARGET_SET& targets, std::string& status);
	bool_t Prepare_InstanceResources(const std::string& instanceId, const TARGET_SET& targets);
	static bool_t Resolve_BossBoneAnchor(const std::shared_ptr<Engine::CModel>& model,
		const float4x4_t& root, const std::string& bone, PLAYER_ANCHOR& out, std::string& status);
	static void Collect_ValidationTargets(const TARGET_SET& targets,
		WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy);
	bool_t Has_ActiveInstances() const { return !m_Active.empty(); }
	bool_t Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out) const;
	bool_t Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out) const;
	std::string Get_ObjectSampleStatus(const std::string& instanceId) const;
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
	bool_t Play(const std::string& instanceId, const TARGET_SET& targets, f32_t playbackSpeed = 1.f, const float3_t& positionOffset = {}, uint32_t durationMs = 0u,
		const std::optional<OBJECT_PLACEMENT>& placement = {});
	bool_t Validate_ObjectPlacement(const std::string& instanceId,
		const std::optional<OBJECT_PLACEMENT>& placement, std::string& status) const;
	// Update the existing object at its current clock; invalid edits leave its pose and motion unchanged.
	bool_t Set_ObjectPlacement(const std::string& instanceId,
		const std::optional<OBJECT_PLACEMENT>& placement, const TARGET_SET& targets);
	// A Server result changes the motion of an existing object, retaining its placement and CModel.
	bool_t Apply_ObjectMotion(const std::string& targetInstanceId,
		const std::string& motionInstanceId, const TARGET_SET& targets);
	bool_t Is_Playing(const std::string& instanceId) const;
	/* The camera cue runs on the cutscene's own clock. Only the player owns
	   that clock, so it hands out a read-only sample instead of letting a
	   second owner count the same time. false means the instance is not
	   playing and the caller must not pose a camera from a stale value. */
	bool_t Try_GetElapsedMs(
		const std::string& instanceId,
		f32_t& outElapsedMs) const;
	/* Returns only the pose successfully applied to the live placement. The
	   authored record remains the replay baseline, never the current pose. */
	bool_t Try_GetSampledPlacementRecord(const std::string& instanceId,
		uint64_t placementId, MAP_PLACEMENT_RECORD& outRecord) const;
	/* Authoring needs to hold a cutscene on one frame and step to any point
	   of it. Paused instances stop advancing but keep their baselines, so a
	   scrub never restarts the sequence or loses the placed pose. */
	void Set_Paused(bool_t paused) { m_bPaused = paused; }
	bool_t Is_Paused() const noexcept { return m_bPaused; }
	/* Moves every playing instance to the same wall-clock point and applies
	   that frame at once. false means nothing is playing to scrub. */
	bool_t Seek_AllToMs(f32_t elapsedMs, const TARGET_SET& targets);
	bool_t Seek_InstanceToMs(const std::string& instanceId, f32_t elapsedMs, const TARGET_SET& targets);
	void Stop_Instance(const std::string& instanceId, const TARGET_SET& targets, bool_t restorePlacements);
	/* The longest authored span across the playing instances, so the tool can
	   size a scrub bar without guessing. */
	f32_t Get_LongestElapsedSpanMs() const;
	// Explicit duration limits births; admitted Object Effect tails finish afterwards.
	f32_t Get_InstanceElapsedSpanMs(const std::string& instanceId,
		f32_t playbackSpeed = 1.f, uint32_t durationMs = 0u) const;

	/* Stopping hands every animated Deploy target back: an authoring preview
	   left running blocks the prop's state from being set, so a second play
	   could never restore it. */
	void Stop_All(const TARGET_SET& targets, bool_t restorePlacements = false);

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
	struct OBJECT_INSTANCE
	{
		std::string slotId;
		uint64_t entityId = 0;
		uint32_t emissionIndex = 0;
		uint32_t levelIndex = ETOUI(LEVEL::END);
		shared_ptr<CWorldSequenceObject> object;
	};
	struct OBJECT_MODEL
	{
		shared_ptr<CModel> model;
		ComPtr<ID3D11ShaderResourceView> diffuse;
	};
	struct ACTIVE_INSTANCE final
	{
		std::string instanceId;
		std::string motionInstanceId;
		f32_t motionStartMs = 0.f;
		f32_t elapsedMs = 0.f;
		f32_t playbackSpeed = 1.f;
		float3_t positionOffset{};
		std::optional<OBJECT_PLACEMENT> placement;
		std::vector<PLACEMENT_BASELINE> placementBaselines;
		std::unordered_map<uint64_t, MAP_PLACEMENT_RECORD> sampledPlacements;
		std::vector<uint64_t> deployTargets;
		uint32_t durationMs = 0;
		std::string objectSampleStatus;
		std::vector<OBJECT_INSTANCE> objects;
		struct EFFECT_INSTANCE
		{
			std::string key;
			uint32_t handle = 0;
		};
		std::vector<EFFECT_INSTANCE> effects;
		std::unordered_map<std::string, PLAYER_ANCHOR> emissionAnchors;
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
	bool_t Prepare_ObjectResources(const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets);
	bool_t Prepare_ObjectMotionChain(const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets);
	bool_t Apply_Objects(ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const WORLD_SEQUENCE_TEMPLATE& sequence, const TARGET_SET& targets, f32_t localMs, bool_t visible, bool_t holdFinalPose = false,
		f32_t emissionStartMs = 0.f, f32_t emissionRate = 1.f, const std::string& emissionMotionId = {});
	bool_t Get_EmissionAnchor(ACTIVE_INSTANCE& active, const TARGET_SET& targets, const std::string& key,
		f32_t birthMs, const PLAYER_ANCHOR& baseline, PLAYER_ANCHOR& out);
	bool_t Sample_ObjectWorld(const ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const WORLD_SEQUENCE_TEMPLATE& sequence, const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
		const std::string& slotId, const PLAYER_ANCHOR& anchor, uint32_t emitter, f32_t ageMs, float4x4_t& out);
	bool_t Apply_ObjectEffects(ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const TARGET_SET& targets);
	void Release_Objects(ACTIVE_INSTANCE& active);
	void Release_DeployPreviews(
		const ACTIVE_INSTANCE& active,
		const TARGET_SET& targets);

private:
	CWorldSequenceDocument m_Document;
	bool_t m_bPaused = false;
	std::vector<ACTIVE_INSTANCE> m_Active;
	// Finished clocks no longer tick, but own their held pose until explicit stop/replay.
	std::vector<ACTIVE_INSTANCE> m_Held;
	std::unordered_map<std::string, shared_ptr<CModel>> m_ModelCache;
	std::unordered_map<std::string, OBJECT_MODEL> m_ObjectModels;
	std::unordered_map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> m_EffectSnapshots;
	std::string m_Status;
};

NS_END
