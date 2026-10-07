# Mario WORLD 사전 준비와 첫 생성 지연 개선 계획

## G00. 이번 구현 경계

Debug의 Mario1~4 및 관문 패턴 녹화에서 인형·공·칼날·갈고리의 첫 발생 순간에 생성 비용이 몰리는 경로를 기존 WORLD 모델 캐시·clone pool·Complete Play 준비 장벽 안에서 개선한다. 선택된 패턴의 수량 산정, 공통 `CWorldSequencePlayer`의 증분 pool 준비, 검증된 WORLD subset 재사용, Level의 실제 발생 소비자를 하나의 준비 흐름으로 연결한다.

시간 측정 없이 특정 hitch 시간이나 개선률을 단정하지 않는다. `CModel` cold load와 clone 하나의 작업은 여전히 동기 구간일 수 있다. 이번 step 계약은 호출당 새 clone 수를 제한하며 millisecond 상한을 보장하지 않는다. 제품 UI·Client를 자동 실행하지 않는다.

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Public/WorldSequencePlayer.h | 기존 일괄 API를 보존하고 증분 준비의 pending/ready 계약 선언 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp | 두 API의 공통 예산 검증·생성·rollback·pool commit 구현 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonPresentationAssetService.h | 선택 요청의 spawn 예약 수량과 실제 root 출력 계약 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationAssetService.cpp | typed occurrence closure와 동시 준비 목표 계산 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Public/Level_KakulSaydonArena.h | 준비 단계의 clone 작업·완료 상태 소유 |
| 수정 | C:/Users/tnest/Desktop/LostArk/Client/Private/Level_KakulSaydonArena.cpp | 준비 장벽의 step 소비·subset 준비·birth 재사용 |

## G01. WorldSequencePlayer.h의 증분 준비 계약

`Prewarm_ObjectInstances` 선언 바로 아래에 `Prewarm_ObjectInstancesStep`을 추가한다. `true`는 그 호출의 정상 처리이며 `ready`가 false이면 준비 중이다. `ready`가 true일 때만 요구한 전체 capacity를 만족한다. 실패는 false이고 ready는 false다. 기존 API는 성공 시 요청한 전체 capacity를 준비하는 의미를 유지한다.

`Clear_PreparedObjects` 바로 아래의 private `Prewarm_ObjectInstancesInternal`이 두 API의 단일 구현이다. 기존 API는 전체 요청량, step API는 1을 최대 신규 생성량으로 전달한다. 기존 동일 owner·WORLD anchor·단일 object binding·enabled·EmissionCount 조건과 objectId별128/owner전체1024 예산을 그대로 검사한다. 전체 요청 목표를 기준으로 예산을 먼저 검사하므로 한 개씩 만들다가 이미 알 수 있던 전체 예산 초과를 뒤늦게 발견하지 않는다.

### C:/Users/tnest/Desktop/LostArk/Client/Public/WorldSequencePlayer.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeployPropRuntime.h"
#include "MapAssetCatalog.h"
#include "MapPlacementRuntime.h"
#include "WorldSequenceDocument.h"

#include <string>
#include <cmath>
#include <functional>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CWorldSequenceObject;
class CNpc;
class EFFECT_V2_CATALOG_SNAPSHOT;
struct EFFECT_DOCUMENT_DESC;
struct EFFECT_WORLD_PREVIEW_TARGET;
struct SAYDON_WEAPON_REPLACEMENT;
struct SAYDON_HAT_REPLACEMENT;

struct WORLD_SEQUENCE_SUBTITLE_SAMPLE final
{
    std::string instanceId, subtitleTrackId, text, position;
    float3_t worldPosition{};
};

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
		std::shared_ptr<Engine::CModel> bodyModel;
	};
	struct TARGET_SET final
	{
		uint32_t levelIndex = {};
		const CMapAssetCatalog* pCatalog = nullptr;
		std::vector<MAP_RUNTIME_PLACED_ENTRY>* pPlacements = nullptr;
		CDeployPropRuntime* pDeployRuntime = nullptr;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		// Level-owned preparation, borrowed only during the call. Live clones retain
		// a separate return token so level teardown never dereferences this owner.
		CWorldSequencePlayer* objectPreparationOwner = nullptr;
		// MapTool samples after MainApp's normal post-update Effect commit. It may
		// commit only the newly-created world roots before seeking that editor frame.
		bool_t bCommitWorldRootEffectsAfterSpawn = false;
		std::function<std::vector<PLAYER_ANCHOR>()> playerAnchors;
		// Supplied by the owning editor or Server-approved Level presentation.
		// Only the exact NPC's rendering is suppressed; gameplay remains replicated.
		std::function<std::shared_ptr<CNpc>(const std::string&)> previewNpc;
		// Live BODY bone pose; separate from a frozen projectile emission origin.
		std::function<bool_t(const std::string&, const std::string&, PLAYER_ANCHOR&, std::string&)> bossAnchor;
		// Occurrence-local real milliseconds at birth -> frozen world origin.
		std::function<bool_t(f32_t, float4x4_t&)> objectEmissionAnchor;
		// Optional presentation-only post transform, sampled at the Object's source clock.
		// Effect frames use the current post transform without rebasing their birth history.
		std::function<bool_t(const std::string&, f32_t, float4x4_t&, std::string&)> objectWorldPostTransform;
		// Optional effect-only replacement for objectWorldPostTransform: attached World
		// Object effects use it while the model pose stays untouched.
		std::function<bool_t(const std::string&, f32_t, float4x4_t&, std::string&)> objectEffectPostTransform;

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
	// Loader-only: parse/validate against its admitted map/Deploy prototypes.
	// One bounded pending Area is replaced on the next preparation and consumed
	// once by activation. Cancellation publishes no usable stage.
	static bool_t Prepare_AreaLoad(uint32_t levelIndex, const std::string& areaId,
		const MAP_LOAD_SCOPE& loadScope, std::string& status,
		const std::function<bool_t()>& isCancellationRequested = nullptr);
	// Product activation never falls back to synchronous file parsing. Failed or
	// missing preparation preserves this player's document and reports its reason.
	bool_t Load_PreparedArea(const std::string& areaId, const TARGET_SET& targets);
	static bool_t Try_CollectPreparedAreaV1EffectTargets(uint32_t levelIndex,
		const std::string& areaId, std::vector<std::string>& outTargets);
	// CPU snapshot admitted by the Loader; lookup performs no IO or GPU work.
	std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> Find_PreparedLeafSnapshot(const std::string& leafId) const;
	bool_t Set_Document(const CWorldSequenceDocument& document, const TARGET_SET& targets, std::string& status);
    // Prepare immutable object-only WORLD subsets before the Server clock starts.
    // Placed/deploy targets retain their normal admission path without caching.
    bool_t Prepare_PlaybackSubset(const std::string& root, const TARGET_SET& targets, std::string& status);
    // Reuse the source's exact admitted subset when target identities still match;
    // otherwise use the ordinary Build_PlaybackSubset and Set_Document path.
    bool_t Set_PlaybackSubset(const CWorldSequencePlayer& source, const std::string& root,
        const TARGET_SET& targets, std::string& status);
    // Editor-only projections use the same world roots, actor bones and source clock.
    // Prepare every replacement before committing; failed edits preserve the live preview.
    bool Preview_EffectDocument(const EFFECT_DOCUMENT_DESC& document,
        const TARGET_SET& targets, std::string& status);
    bool Preview_EffectSelection(const EFFECT_DOCUMENT_DESC& full,
        const EFFECT_DOCUMENT_DESC& selected, const std::vector<std::string>& drawElementIds,
        const std::string& effectTrackId, const TARGET_SET& targets, std::string& status);
    bool Clear_EffectPreviews(std::string& status);
	// Editor draft preview: same validation and stop as Set_Document, but a
	// prepared model whose resource still asks for the same inputs is kept, so
	// a key or clip edit does not reload a 40 MB body and its animation set.
	bool_t Replace_DocumentKeepingModels(const CWorldSequenceDocument& document,
		const TARGET_SET& targets, std::string& status);
	// A single request may stage several independent clocks against one document.
	// Validate/read targets once; prepare every copy before replacing any player.
	static bool_t Set_DocumentBatch(const CWorldSequenceDocument& document, const TARGET_SET& targets,
		const std::vector<CWorldSequencePlayer*>& players, std::string& status);
	bool_t Prepare_InstanceResources(const std::string& instanceId, const TARGET_SET& targets);
	// Prepare hidden clones through the existing Prototype/Clone/Layer path.
	// Stop/completion returns them for later occurrences; failure preserves the pool.
	bool_t Prewarm_ObjectInstances(const std::string& instanceId, uint32_t copies, const TARGET_SET& targets);
	// True may still be pending; ready becomes true only at the requested capacity.
	// Each step creates at most one hidden clone after validating the full budget.
	bool_t Prewarm_ObjectInstancesStep(const std::string& instanceId, uint32_t copies,
		const TARGET_SET& targets, bool_t& ready);
	bool_t Prewarm_HiddenObjectPose(const std::string& instanceId, f32_t elapsedMs, const TARGET_SET& targets);
	static bool_t Resolve_BossBoneAnchor(const std::shared_ptr<Engine::CModel>& model,
		const float4x4_t& root, const std::string& bone, PLAYER_ANCHOR& out, std::string& status);
	static void Collect_ValidationTargets(const TARGET_SET& targets,
		WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy);
	bool_t Has_ActiveInstances() const { return !m_Active.empty(); }
    // Presentation controllers pack source-native parameters; only the named
    // Object clone receives the constants. Shared prototypes remain immutable.
    bool_t Set_ObjectMaterialConstants(const std::string& instanceId,
        const std::string& slotId, const std::string& materialName,
        const Engine::MODEL_SOURCE_CHARACTER_PARAMETERS& parameters);
    // Append read-only samples from the last successful World clock. Hidden or
    // missing Object targets suppress only their actor-bound balloon subtitle.
    void Collect_Subtitles(std::vector<WORLD_SEQUENCE_SUBTITLE_SAMPLE>& out) const;

	/* emissionIndex selects one row of an authored emission list; a seeded
	   emitter keeps the single-object contract and answers index 0 only. */
	bool_t Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out, uint32_t emissionIndex = 0u,
		const std::string& bone = {}, bool_t boneRotation = false, const std::string& effectTrackId = {}) const;
	// No match leaves status empty; an active but unavailable/ambiguous actor fails closed.
	bool_t Try_GetPresentationBossAnchor(const std::string& archetype, const std::string& bone,
		PLAYER_ANCHOR& out, std::string& status) const;
	bool_t Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out, uint32_t emissionIndex = 0u,
		const std::string& bone = {}, bool_t boneRotation = false, const std::string& effectTrackId = {}) const;
	std::string Get_ObjectSampleStatus(const std::string& instanceId) const;
	// The admitted sequence retains object ownership; callers inspect the current visible pose only.
	void Collect_VisibleObjects(std::vector<std::shared_ptr<CWorldSequenceObject>>& out) const;
    struct OBJECT_INSPECTION_SAMPLE final
    {
        std::string instanceId, slotId, objectId, modelAssetId;
        uint32_t emissionIndex = 0u;
        std::shared_ptr<CWorldSequenceObject> object;
    };
    // Append the existing sampled clones, including authored-hidden and held objects.
    // Reading inspection identity never advances a clock or creates a replacement model.
    void Collect_ObjectInspectionSamples(std::vector<OBJECT_INSPECTION_SAMPLE>& out) const;
	struct OBJECT_COLLIDER_SAMPLE
	{
		std::string instanceId, colliderTrackId, behavior;
		std::string shape = "BOX";
		uint32_t emissionIndex = 0;
		float3_t center{}, halfExtents{}, gripPosition{};
		f32_t yawDegrees = 0.f;
		bool_t hasGrip = false;
	};
	// Read the last successful Apply_Objects result; never advance or reconstruct its clock.
	void Collect_ObjectColliderSamples(std::vector<OBJECT_COLLIDER_SAMPLE>& out) const;
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
	/* Placements the Level has taken out of every instance's hands: a popped
	   Mario ball stays hidden however its layout samples it, until the Level
	   hands the placement back. */
	void Set_PlacementSuppressed(uint64_t placementId, bool_t suppressed);
	bool_t Is_PlacementSuppressed(uint64_t placementId) const;
	/* Placements the instances that are playing or holding a pose right now manipulate. Returns an
	   order-independent signature (0 = none) so a caller can see the set change without copying it;
	   pOut (optional) receives the ids. Nothing is allocated when pOut is null. */
	uint64_t Collect_OwnedPlacements(std::unordered_set<uint64_t>* pOut) const;
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
	void Set_Paused(bool_t paused);
    // For a presentation owner that supplies source milliseconds by external seek.
    // Reconcile media position as well as pitch after frame stalls or deferred advances.
    void Set_ExternalSoundClockRate(f32_t rate)
    { if (std::isfinite(rate) && rate > 0.f && rate <= 16.f) { m_ExternalSoundClockRate = rate; m_HasExternalSoundClock = true; } }
    // Movie audio has its own undilated elapsed time. Only cue births are mapped
    // from the visual source timeline; WAV age/duration never inherit camera slomo.
    void Set_ExternalSoundTime(f32_t elapsedMs, std::function<f32_t(f32_t)> sourceToSoundMs)
    {
        if (std::isfinite(elapsedMs) && elapsedMs >= 0.f && sourceToSoundMs)
        { m_ExternalSoundElapsedMs = elapsedMs; m_SourceToSoundTime = std::move(sourceToSoundMs); }
    }
    void Update_SoundTails(f32_t timeDelta);
    // Complete audio without releasing visual ownership; explicit seek may play it again.
    void Finish_Sounds();
    // Level-owned listener audience; visual clocks continue when its sound is inaudible.
    void Set_SoundAudience(std::function<bool(const std::string&)> audience) { m_SoundAudience = std::move(audience); }
    void Retire_InstanceSoundTails(const std::string& instanceId);
	bool_t Is_Paused() const noexcept { return m_bPaused; }
	/* Samples every playing instance at the same wall-clock point. Pass discontinuous=false
	   for a live external clock to preserve audio across slow frames. Only backwards time
	   or an explicit scrub restarts audio; explicit scrub keeps the default true. */
	bool_t Seek_AllToMs(f32_t elapsedMs, const TARGET_SET& targets, bool_t discontinuous = true);
	bool_t Seek_InstanceToMs(const std::string& instanceId, f32_t elapsedMs, const TARGET_SET& targets, bool_t discontinuous = true);
	void Stop_Instance(const std::string& instanceId, const TARGET_SET& targets, bool_t restorePlacements, bool_t preserveSoundTail = false);
	/* The longest authored span across the playing instances, so the tool can
	   size a scrub bar without guessing. */
	f32_t Get_LongestElapsedSpanMs() const;
	// Explicit duration limits births; admitted Object Effect tails finish afterwards.
	f32_t Get_InstanceElapsedSpanMs(const std::string& instanceId,
		f32_t playbackSpeed = 1.f, uint32_t durationMs = 0u) const;

	/* Stopping hands every animated Deploy target back: an authoring preview
	   left running blocks the prop's state from being set, so a second play
	   could never restore it. */
	void Stop_All(const TARGET_SET& targets, bool_t restorePlacements = false, bool_t preserveSoundTail = false);

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
	struct PREPARED_OBJECT_POOL
	{
		bool acceptsReturns = true;
		uint32_t levelIndex = ETOUI(LEVEL::END);
		uint32_t capacity = 0u;
		std::vector<shared_ptr<CWorldSequenceObject>> idle;
	};
	struct OBJECT_INSTANCE
	{
		std::string slotId;
		uint64_t entityId = 0;
		uint32_t emissionIndex = 0;
		uint32_t levelIndex = ETOUI(LEVEL::END);
		shared_ptr<CWorldSequenceObject> object;
		std::shared_ptr<PREPARED_OBJECT_POOL> preparationPool;
		std::shared_ptr<const SAYDON_WEAPON_REPLACEMENT> weaponReplacement;
		std::shared_ptr<const SAYDON_HAT_REPLACEMENT> hatReplacement;
		std::shared_ptr<const void> npcPreviewSuppression;
	};
	struct OBJECT_MODEL
	{
		shared_ptr<CModel> model;
		ComPtr<ID3D11ShaderResourceView> diffuse;
		ID3D11Device* deviceIdentity = nullptr;
		ID3D11DeviceContext* contextIdentity = nullptr;
		const CMapAssetCatalog* catalogIdentity = nullptr;
		// Product boss presentation drawn with this body (empty = single model).
		std::string presentationBossArchetypeId;
	};
    struct SOUND_INSTANCE
    {
        std::string key;
        uint64_t handle = 0u;
        f32_t endElapsedMs = 0.f;
        uint32_t mediaDurationMs = 0u;
        uint64_t mediaCycle = 0u;
        bool_t loopToDuration = false;
    };
    struct RETIRED_SOUND
    {
        std::string ownerId;
        uint64_t handle = 0u;
        f32_t remainingMs = 0.f;
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
		std::unordered_map<uint64_t, float4x4_t> sampledDeployPivots;
		std::vector<uint64_t> deployTargets;
		uint32_t durationMs = 0;
		std::string objectSampleStatus;
        std::string sampledSubtitleTemplateId;
        f32_t sampledSubtitleLocalMs = 0.f;
        bool_t hasSubtitleSample = false;

		std::vector<OBJECT_INSTANCE> objects;
		std::vector<OBJECT_COLLIDER_SAMPLE> objectColliderSamples;
		struct EFFECT_INSTANCE
		{
			std::string key;
			uint32_t handle = 0;
			uint64_t v1Handle = 0;
			std::shared_ptr<const EFFECT_DOCUMENT_DESC> sourceDocument;
			std::optional<OBJECT_PLACEMENT> sampledPlacement;
			float3_t sampledPositionOffset{};
			std::string effectTrackId;
		};
		std::vector<EFFECT_INSTANCE> effects;
        std::vector<SOUND_INSTANCE> sounds;
        bool_t seekSounds = false;
        bool_t soundPlaybackFinished = false;
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
	static bool_t Sample_ObjectWorld(const ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const WORLD_SEQUENCE_TEMPLATE& sequence, const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
		const std::string& slotId, const PLAYER_ANCHOR& anchor, uint32_t emitter, f32_t ageMs, float4x4_t& out, std::string& status,
		bool_t inheritObjectRotation = true,
		const decltype(TARGET_SET::objectWorldPostTransform)& postTransform = {});
	bool_t Apply_ObjectEffects(ACTIVE_INSTANCE& active, const WORLD_SEQUENCE_INSTANCE& instance,
		const TARGET_SET& targets);
    void Apply_Sounds(ACTIVE_INSTANCE& active);
    void Retire_Sounds(ACTIVE_INSTANCE& active);
    void Stop_RetiredSounds(const std::string& ownerId = {});
	void Release_Objects(ACTIVE_INSTANCE& active);
	void Clear_PreparedObjects();
	bool_t Prewarm_ObjectInstancesInternal(const std::string& instanceId, uint32_t copies,
		const TARGET_SET& targets, uint32_t maximumNewCopies, bool_t& ready);
	static bool_t Same_ObjectModelInputs(const WORLD_SEQUENCE_OBJECT_RESOURCE& left, const WORLD_SEQUENCE_OBJECT_RESOURCE& right);
	bool_t Admit_PresentationBossModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
		const TARGET_SET& targets, OBJECT_MODEL& out);
	const OBJECT_MODEL* Find_PreparedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const;
	const OBJECT_MODEL* Find_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const;
	void Remember_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const OBJECT_MODEL& model, const TARGET_SET& targets) const;
	void Release_DeployPreviews(
		const ACTIVE_INSTANCE& active,
		const TARGET_SET& targets);

private:
    struct EFFECT_PREVIEW final
    {
        std::shared_ptr<const EFFECT_DOCUMENT_DESC> document;
        std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET> target;
        f32_t durationSeconds = 0.f;
    };
    struct EFFECT_SELECTION final
    {
        std::string assetId, effectTrackId;
        std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET> target;
    };
    bool Commit_EffectPreviews(std::unordered_map<std::string, EFFECT_PREVIEW> previews,
        std::optional<EFFECT_SELECTION> selection, std::string& status);

    struct PREPARED_PLAYBACK_SUBSET final
    {
        std::shared_ptr<const CWorldSequenceDocument> document;
        uint32_t levelIndex = {};
        ID3D11Device* deviceIdentity = nullptr;
        ID3D11DeviceContext* contextIdentity = nullptr;
        const CMapAssetCatalog* catalogIdentity = nullptr;
    };
	CWorldSequenceDocument m_Document;
    std::unordered_map<std::string, PREPARED_PLAYBACK_SUBSET> m_PreparedPlaybackSubsets;
    std::unordered_map<std::string, EFFECT_PREVIEW> m_EffectPreviews;
    std::optional<EFFECT_SELECTION> m_EffectSelection;
	bool_t m_bPaused = false;
	std::vector<ACTIVE_INSTANCE> m_Active;
    std::vector<RETIRED_SOUND> m_RetiredSounds;
    f32_t m_ExternalSoundClockRate = 1.f;
    bool_t m_HasExternalSoundClock = false;
    f32_t m_ExternalSoundElapsedMs = 0.f;
    std::function<f32_t(f32_t)> m_SourceToSoundTime;
    std::function<bool(const std::string&)> m_SoundAudience;
	// Finished clocks no longer tick, but own their held pose until explicit stop/replay.
	std::vector<ACTIVE_INSTANCE> m_Held;
	std::unordered_map<std::string, shared_ptr<CModel>> m_ModelCache;
	std::unordered_map<std::string, OBJECT_MODEL> m_ObjectModels;
	std::unordered_map<std::string, std::shared_ptr<PREPARED_OBJECT_POOL>> m_PreparedObjectPools;
	std::unordered_map<std::string, std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT>> m_EffectSnapshots;
	std::unordered_set<uint64_t> m_SuppressedPlacements;
	std::string m_Status;
};

NS_END
```

## G02. WorldSequencePlayer_Objects.cpp의 stage와 commit

`Prewarm_ObjectInstances`는 기존 일괄 호출을 private helper에 전달한다. `Prewarm_ObjectInstancesStep`은 같은 helper에 최대 신규1을 전달한다. helper는 ready=false로 시작하고 문서와 owner를 검증한 뒤 기존 `Prepare_InstanceResources`로 모델을 준비한다. 이미 충분한 pool은 추가 생성 없이 ready=true다.

부족하면 전체 목표의1024 상한을 검사하고 이번 단계의 `nextCapacity`를 정한다. idle와 staging vector 저장 공간을 layer 객체 생성 전에 확보한다. 신규 객체는 기존 Prototype/Clone/Layer 경로로 생성하고 즉시 Hide한 뒤 staging에 보관한다. 생성 HRESULT·타입 실패는 해당 단계에서 만든 객체만 Layer에서 제거하며 기존 pool의 capacity와 객체를 보존한다. owner map의 신규 entry 할당도 commit 전에 끝내고, 그 과정의 예외에서는 stage를 rollback한 뒤 기존 예외를 전달한다.

모두 성공한 경우 미리 확보한 idle 저장소로 clone을 넘기고 level/capacity를 갱신한다. ready는 전체 요청 capacity와 비교한다. partial pool은 유효한 숨김 cache로 남고 다음 update가 이어서 준비한다. 취소·Level 종료·owner revision 변경의 기존 token/반환/정리 계약은 변경하지 않는다. 준비 요청 취소가 완료된 공유 cache를 지우도록 만들지 않는다.

### C:/Users/tnest/Desktop/LostArk/Client/Private/WorldSequencePlayer_Objects.cpp 전체 코드

```cpp
#include "WorldSequencePlayer.h"
#include "ActorCatalog.h"
#include "WorldSequenceObject.h"
#include "DeployPropObject.h"
#include "GameInstance.h"
#include "Model.h"
#include "NpcPresentationAssetService.h"
#include "Npc.h"
#include "Valtan.h"
#include "ValtanPresentationAssetService.h"
#include "BinaryAsset/ModelDecoderRegistry.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "RuntimeAssetRoot.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "Effect_PresentationService.h"
#include "Effect_Playback.h"
#include "Effect_DocumentCodec.h"
#include "Profiler.h"
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace Client;
using namespace Engine;

namespace
{
bool_t Apply_ObjectMaterialConstants(CModel& model, const std::string& materialName,
    const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters, std::string& status)
{
    bool exact = false;
    for (uint32_t mesh = 0u; mesh < model.Get_NumMeshes(); ++mesh)
    {
        const auto& name = model.Get_MaterialName(mesh);
        if (name.find(materialName) == std::string::npos) continue;
        const auto* surface = model.Get_MaterialSurface(mesh);
        if (name != materialName || !surface || surface->family != MODEL_SURFACE_FAMILY::SOURCE_CHARACTER ||
            surface->sourceCharacter.program != parameters.program)
        { status = "World Object material name/program mismatch: " + materialName; return false; }
        exact = true;
    }
    if (!exact || model.Override_SourceCharacterConstants(materialName.c_str(), parameters) == 0u)
    { status = "World Object material override was rejected: " + materialName; return false; }
    return true;
}
}

CWorldSequencePlayer::~CWorldSequencePlayer() { Clear(); }

bool_t CWorldSequencePlayer::Set_ObjectMaterialConstants(const std::string& instanceId,
    const std::string& slotId, const std::string& materialName,
    const MODEL_SOURCE_CHARACTER_PARAMETERS& parameters)
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end() || materialName.empty())
    { m_Status = "World Object material target is inactive: " + instanceId; return false; }
    bool matched = false;
    for (const auto& entry : active->objects)
    {
        if (entry.slotId != slotId || !entry.object) continue;
        const auto& model = entry.object->Get_Model();
        if (!model) return false;
        if (!Apply_ObjectMaterialConstants(*model, materialName, parameters, m_Status)) return false;
        matched = true;
    }
    // A source actor can be hidden before its first emission. It still exists
    // once emitted, and its material is checked as soon as the clone appears.
    if (!matched)
    { m_Status = "World Object material slot has no live clone: " + slotId; return false; }
    return true;
}

namespace
{
bool_t Sample_ObjectPresentationPostTransform(
    const decltype(CWorldSequencePlayer::TARGET_SET::objectWorldPostTransform)& callback,
    const std::string& instanceId, const f32_t sourceMs, float4x4_t& out, std::string& status)
{
    XMStoreFloat4x4(&out, XMMatrixIdentity());
    if (callback && !callback(instanceId, sourceMs, out, status))
    {
        if (status.empty()) status = "World Object presentation transform is unavailable: " + instanceId;
        return false;
    }
    for (const auto& row : out.m)
        for (const float component : row)
            if (!std::isfinite(component))
            { status = "World Object presentation transform is not finite: " + instanceId; return false; }
    return true;
}

/* The product Valtan part group: a static weapon on the body's grip bone and
   skinned armour plates on its palette, as CValtan builds them. */
void Fill_PresentationParts(const std::string& archetypeId, CWorldSequenceObject::DESC& desc)
{
    desc.presentationParts.clear();
    desc.materialProfileId.clear();
    const BOSS_ACTOR_ENTRY* actor = archetypeId.empty() ? nullptr : CActorCatalog::Find_Boss(archetypeId);
    if (!actor) return;
    desc.materialProfileId = "material.valtan.monster-base.v1";
    desc.presentationParts.push_back({ CValtanPresentationAssetService::Get_WeaponModelPrototypeTag(archetypeId),
        L"Prototype_Component_Shader_VtxMeshBinary", CValtan::WEAPON_SOCKET_BONE });
    for (const BOSS_ARMOR_PART_ENTRY& armor : actor->armorParts)
        desc.presentationParts.push_back({ CValtan::Build_ArmorModelPrototypeTag(armor.stateMask, archetypeId),
            L"Prototype_Component_Shader_VtxAnimMeshBinary", std::string() });
}

std::string Narrow_PrototypeTag(const wstring_t& tag)
{
    std::string text;
    text.reserve(tag.size());
    for (const wchar_t character : tag) text.push_back(character < 128 ? static_cast<char>(character) : '?');
    return text;
}

bool Sample_ObjectCollider(const WORLD_SEQUENCE_COLLIDER_TRACK& collider,
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const WORLD_SEQUENCE_TRANSFORM_KEY& key,
    const WORLD_SEQUENCE_OBJECT_MOTION& motion, const uint32_t emitter,
    const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement,
    const CWorldSequenceObject& object, CWorldSequencePlayer::OBJECT_COLLIDER_SAMPLE& out)
{
    const float emissionYaw = motion.emissions.empty() ? 0.f : motion.emissions[emitter].yawDegrees;
    const vector_t localScale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    const vector_t placementScale = placement ? XMLoadFloat3(&placement->scale) : XMVectorReplicate(1.f);
    const float placementYaw = placement ? placement->rotationDegrees.y : 0.f;
    const matrix_t groundBasis = XMMatrixScalingFromVector(localScale) *
        XMMatrixRotationY(XMConvertToRadians(emissionYaw)) * XMMatrixScalingFromVector(placementScale) *
        XMMatrixRotationY(XMConvertToRadians(placementYaw));
    out.behavior = collider.behavior;
    out.shape = collider.shape;
    out.yawDegrees = emissionYaw + placementYaw + collider.yawDegrees;
    XMStoreFloat3(&out.halfExtents, XMVectorAbs(XMLoadFloat3(&collider.halfExtents) * localScale * placementScale));
    if (collider.shape == "CYLINDER")
    {
        float3_t scale;
        XMStoreFloat3(&scale, XMVectorAbs(localScale * placementScale));
        // Server worldTrack cylinders use the authored X radius and the larger ground scale.
        out.halfExtents.x = out.halfExtents.z = collider.halfExtents.x * (std::max)(scale.x, scale.z);
    }
    out.hasGrip = collider.behavior == "HOOK_CAPTURE";
    if (out.hasGrip)
    {
#ifdef _DEBUG
        float4x4_t attachment;
        if (!object.Try_GetAttachmentWorld(collider.attachmentBone, attachment)) return false;
        const matrix_t world = XMLoadFloat4x4(&attachment);
        XMStoreFloat3(&out.center, XMVector3TransformCoord(XMLoadFloat3(&collider.positionOffset), world));
        XMStoreFloat3(&out.gripPosition, XMVector3TransformCoord(XMLoadFloat3(&collider.gripLocalOffset), world));
#else
        return false; // Bone attachment preview is an authoring-only capability.
#endif
    }
    else
    {
        const matrix_t world = XMLoadFloat4x4(&object.Get_SampledWorld());
        XMStoreFloat3(&out.center, world.r[3] +
            XMVector3TransformNormal(XMLoadFloat3(&collider.positionOffset), groundBasis));
    }
    return true;
}

// Use the same clip windows, ticks and end policy as WorldSequenceObject::Sample,
// but sample the immutable CModel skeleton without changing the visible palette.
bool Sample_ObjectEffectBone(const std::shared_ptr<CModel>& model,
    const WORLD_SEQUENCE_TEMPLATE& sequence, const std::string& slotId,
    const std::string& bone, const float sampleMs, float4x4_t& out, std::string& error)
{
    if (!model || !model->Has_Bone(bone.c_str()))
    { error = "World Object Effect bone is unavailable: " + bone; return false; }
    f32_t windowEnd = 0.f;
    const auto* animation = CWorldSequencePlayer::Find_AnimationTrackAt(sequence, slotId, sampleMs, windowEnd);
    if (!animation) { XMStoreFloat4x4(&out, model->Get_BoneMatrix(bone.c_str())); return true; }
    uint32_t index = UINT32_MAX;
    for (uint32_t i = 0; i < model->Get_NumAnimations(); ++i)
        if (animation->clipName == model->Get_AnimationName(i)) { index = i; break; }
    float position = 0.f, duration = 0.f;
    if (index == UINT32_MAX || !model->Get_AnimationProgress(index, position, duration) || duration <= 0.f)
    { error = "World Object Effect animation is unavailable: " + animation->clipName; return false; }
    float ticks = 0.f;
    if (!CWorldSequenceDocument::Try_SampleAnimationTicks(*animation, sampleMs, windowEnd,
        model->Get_AnimationTickPerSecond(index), duration, ticks))
    { error = "World Object Effect animation source range is invalid: " + animation->clipName; return false; }
    const uint32_t boneIndex = static_cast<uint32_t>(model->Find_BoneIndex(bone.c_str()));
    if (!model->Sample_AnimationBoneCombinedMatrices(animation->clipName.c_str(), ticks,
        std::span<const uint32_t>(&boneIndex, 1u), std::span<float4x4_t>(&out, 1u)))
    { error = "World Object Effect bone sample failed: " + bone; return false; }
    return true;
}

bool Sample_ObjectEffectAttachments(const EFFECT_DOCUMENT_DESC& document,
    const std::shared_ptr<CModel>& model, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const std::string& slotId, const float sampleMs, const float4x4_t& root,
    std::unordered_map<std::string, float4x4_t>& anchors, std::string& error)
{
    std::unordered_map<std::string, float4x4_t> bones;
    for (const auto& element : document.Elements)
    {
        const auto& attachment = element.ActionCueAttachment;
        if (!element.bVisible || !attachment.bEnabled || !attachment.bFollow) continue;
        if (attachment.strRuntimeAnchorSlotId.empty())
        { error = "World Object Effect source attachment has no stable slot."; return false; }
        matrix_t anchor;
        if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW)
        {
            const auto* camera = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
            if (!camera) { error = "World Object Effect camera anchor is unavailable."; return false; }
            anchor = XMLoadFloat4x4(camera);
        }
        else
        {
            auto found = bones.find(attachment.strRuntimeBoneName);
            if (found == bones.end())
            {
                float4x4_t bone;
                // A static prop has no source-character skeleton (b_root, FX_* sockets).
                // Mirror the owner-anchored product path, which skips a missing bone,
                // but keep the slot present: transform-history sampling requires every
                // follow slot, so the attachment rides the object pivot instead.
                // The explicit effect-track bone (effect.bone) stays strict in the provider.
                if (model && !model->Has_Bone(attachment.strRuntimeBoneName.c_str()))
                    XMStoreFloat4x4(&bone, XMMatrixIdentity());
                else if (!Sample_ObjectEffectBone(model, sequence, slotId, attachment.strRuntimeBoneName,
                    sampleMs, bone, error)) return false;
                found = bones.emplace(attachment.strRuntimeBoneName, bone).first;
            }
            if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::BONE)
                anchor = XMLoadFloat4x4(&found->second) * XMLoadFloat4x4(&root);
            else if (attachment.eOrientation == EFFECT_ATTACHMENT_ORIENTATION::OWNER_YAW)
            {
                float4x4_t yawAnchor;
                if (!CEffectPlayback::Build_OwnerYawBoneAnchorWorld(found->second, root, root, yawAnchor))
                { error = "World Object Effect owner-yaw anchor is invalid."; return false; }
                anchor = XMLoadFloat4x4(&yawAnchor);
            }
            else { error = "World Object Effect source attachment orientation is unsupported."; return false; }
        }
        const auto& socket = attachment.SocketLocalTransform;
        const matrix_t world = XMMatrixScalingFromVector(XMLoadFloat3(&socket.vScale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(socket.vRotationDegrees.x),
                XMConvertToRadians(socket.vRotationDegrees.y), XMConvertToRadians(socket.vRotationDegrees.z)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&socket.vPosition)) * anchor;
        const float determinant = XMVectorGetX(XMMatrixDeterminant(world));
        if (XMMatrixIsNaN(world) || XMMatrixIsInfinite(world) || !std::isfinite(determinant) || std::abs(determinant) < 1.e-12f)
        { error = "World Object Effect source attachment is singular or non-finite."; return false; }
        float4x4_t value; XMStoreFloat4x4(&value, world);
        const auto [found, inserted] = anchors.emplace(attachment.strRuntimeAnchorSlotId, value);
        if (!inserted)
            for (size_t r = 0; r < 4; ++r) for (size_t c = 0; c < 4; ++c)
                if (std::abs(found->second.m[r][c] - value.m[r][c]) > .0001f)
                { error = "World Object Effect source slot has conflicting bones."; return false; }
    }
    return true;
}
}


bool_t CWorldSequencePlayer::Resolve_BossBoneAnchor(const std::shared_ptr<CModel>& model,
    const float4x4_t& root, const std::string& bone, PLAYER_ANCHOR& out, std::string& status)
{
    if (!model || (!bone.empty() && !model->Has_Bone(bone.c_str())))
    { status = "World Object boss BODY bone is unavailable: " + bone; return false; }
    const matrix_t basis = bone.empty() ? XMLoadFloat4x4(&root) :
        model->Get_BoneMatrix(bone.c_str()) * XMLoadFloat4x4(&root);
    XMStoreFloat4x4(&out.world, basis);
    const auto* values = reinterpret_cast<const f32_t*>(&out.world);
    for (size_t i = 0u; i < 16u; ++i)
        if (!std::isfinite(values[i]))
        { status = "World Object boss BODY bone pose is not finite: " + bone; return false; }
    if (std::abs(XMVectorGetX(XMMatrixDeterminant(basis))) < .000001f)
    { status = "World Object boss BODY bone pose is singular: " + bone; return false; }
    out.bodyModel = model;
    // The object sampler preserves the socket translation and normalizes its
    // axes. Import scale belongs to the prop; boss scale is already in this pose.
    return true;
}

void CWorldSequencePlayer::Collect_ValidationTargets(const TARGET_SET& targets,
    WORLD_SEQUENCE_PLACEMENT_MAP& placements, WORLD_SEQUENCE_DEPLOY_MAP& deploy)
{
    placements.clear(); deploy.clear();
    if (!targets.Is_Complete()) return;
    // Load the complete placement source, not only the current rendering scope.
    std::vector<MAP_PLACEMENT_RECORD> records;
    std::string status;
    if (CMapPlacementRuntime::Read_Placements(*targets.pCatalog, records, status))
        for (const auto& row : records)
        {
            const auto* asset = targets.pCatalog->Find(row.assetId);
            placements.emplace(row.placementId, WORLD_SEQUENCE_PLACEMENT_INFO{row.signedScale,
                asset && asset->renderProfile.renderMode != MAP_ASSET_RENDER_MODE::BACKGROUND});
        }
    for (const auto& entry : *targets.pPlacements)
    {
        const auto* asset = targets.pCatalog->Find(entry.record.assetId);
        placements.insert_or_assign(entry.record.placementId, WORLD_SEQUENCE_PLACEMENT_INFO{
            entry.record.signedScale, asset && asset->renderProfile.renderMode != MAP_ASSET_RENDER_MODE::BACKGROUND});
    }
    for (const auto& entry : targets.pDeployRuntime->Get_Entries())
    {
        WORLD_SEQUENCE_DEPLOY_INFO info;
        if (entry.object && !entry.object->Is_StaticDeployModel())
        {
            for (const auto& clip : entry.object->Get_AnimationClips()) info.animationClips.push_back(clip.name);
            info.animationTargetSupported = !info.animationClips.empty();
        }
        deploy.emplace(entry.placement.runtimePlacementId, std::move(info));
    }
}

bool_t CWorldSequencePlayer::Set_Document(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, std::string& status)
{
    const bool_t admitted = Set_DocumentBatch(document, targets, {this}, status);
    m_Status = status;
    return admitted;
}

bool_t CWorldSequencePlayer::Prepare_PlaybackSubset(const std::string& root,
    const TARGET_SET& targets, std::string& status)
{
    if (!targets.Is_Complete())
    { status = "World Object runtime targets are unavailable."; return false; }
    if (const auto found = m_PreparedPlaybackSubsets.find(root);
        found != m_PreparedPlaybackSubsets.end() && found->second.document &&
        found->second.levelIndex == targets.levelIndex &&
        found->second.deviceIdentity == targets.device.Get() &&
        found->second.contextIdentity == targets.context.Get() &&
        found->second.catalogIdentity == targets.pCatalog &&
        found->second.document->Get_AreaId() == m_Document.Get_AreaId() &&
        found->second.document->Get_Revision() == m_Document.Get_Revision())
    { status.clear(); return true; }
    CWorldSequenceDocument staged;
    if (!m_Document.Build_PlaybackSubset({root}, staged, status)) return false;
    // Placement/deploy and live anchors keep their original admission path.
    // Only independent WORLD Object bindings are valid without target tables.
    const bool objectOnly = std::all_of(staged.Get_Instances().begin(), staged.Get_Instances().end(),
        [](const auto& instance) { return instance.anchorKind == "WORLD" &&
            std::all_of(instance.bindings.begin(), instance.bindings.end(), [](const auto& binding) {
                return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
            }); }) &&
        std::all_of(staged.Get_ObjectResources().begin(), staged.Get_ObjectResources().end(),
            [](const auto& object) { return object.anchorKind == "WORLD"; });
    if (!objectOnly)
    { m_PreparedPlaybackSubsets.erase(root); status.clear(); return true; }
    if (!staged.Validate({}, {}, status)) return false;
    PREPARED_PLAYBACK_SUBSET admitted;
    admitted.document = std::make_shared<const CWorldSequenceDocument>(std::move(staged));
    admitted.levelIndex = targets.levelIndex;
    admitted.deviceIdentity = targets.device.Get();
    admitted.contextIdentity = targets.context.Get();
    admitted.catalogIdentity = targets.pCatalog;
    m_PreparedPlaybackSubsets.insert_or_assign(root, std::move(admitted));
    status.clear();
    return true;
}

bool_t CWorldSequencePlayer::Set_PlaybackSubset(const CWorldSequencePlayer& source,
    const std::string& root, const TARGET_SET& targets, std::string& status)
{
    if (targets.Is_Complete() && targets.objectPreparationOwner == &source)
    {
        const auto found = source.m_PreparedPlaybackSubsets.find(root);
        if (found != source.m_PreparedPlaybackSubsets.end() && found->second.document &&
            found->second.levelIndex == targets.levelIndex &&
            found->second.deviceIdentity == targets.device.Get() &&
            found->second.contextIdentity == targets.context.Get() &&
            found->second.catalogIdentity == targets.pCatalog &&
            found->second.document->Get_AreaId() == source.m_Document.Get_AreaId() &&
            found->second.document->Get_Revision() == source.m_Document.Get_Revision())
        {
            // Stage the only copy before mutation, including source == *this.
            // The private cache holds the exact immutable document validated at
            // preparation; every source replacement clears it, even at equal revision.
            CWorldSequenceDocument staged = *found->second.document;
            Stop_All(targets, true);
            Clear_PreparedObjects();
            m_ObjectModels.clear();
            m_EffectSnapshots.clear();
            m_EffectPreviews.clear();
            m_EffectSelection.reset();
            m_Document = std::move(staged);
            status = m_Status = "World Object prepared playback subset admitted.";
            return true;
        }
    }
    CWorldSequenceDocument staged;
    if (!source.m_Document.Build_PlaybackSubset({root}, staged, status)) return false;
    return Set_Document(staged, targets, status);
}

bool CWorldSequencePlayer::Preview_EffectDocument(const EFFECT_DOCUMENT_DESC& document,
    const TARGET_SET& targets, std::string& status)
{
    bool used = false;
    for (const auto& sequence : m_Document.Get_Templates())
        for (const auto& track : sequence.effectTracks)
            used |= track.resourceKind == "V1_EFFECT" && track.resourceId == document.strEffectAssetId;
    if (!used || !targets.Is_Complete())
    { status = "The Effect is not part of this admitted World Sequence."; return false; }
    EFFECT_PREVIEW staged;
    if (!CEffectDocumentCodec::Validate(document, status)) return false;
    const bool empty = document.OwnerControls.empty() &&
        std::none_of(document.Elements.begin(), document.Elements.end(),
            [](const auto& element) { return element.bVisible; }) &&
        std::none_of(document.ModelCues.begin(), document.ModelCues.end(),
            [](const auto& cue) { return cue.bVisible; });
    // Deleting the final row is a valid authoring draft. It suppresses the
    // source instead of asking the renderer to prepare a drawable empty body.
    if (!empty)
    {
        CEffectPlayback timing;
        if (!timing.Stage_Document(document, status) ||
            !CEffectPresentationService::Prepare_WorldPreviewTarget(targets.device, targets.context,
                document, staged.target, status)) return false;
        staged.durationSeconds = timing.Get_DurationSeconds();
    }
    staged.document = std::make_shared<const EFFECT_DOCUMENT_DESC>(document);
    auto previews = m_EffectPreviews;
    previews.insert_or_assign(document.strEffectAssetId, std::move(staged));
    return Commit_EffectPreviews(std::move(previews), {}, status);
}

bool CWorldSequencePlayer::Preview_EffectSelection(const EFFECT_DOCUMENT_DESC& full,
    const EFFECT_DOCUMENT_DESC& selected, const std::vector<std::string>& drawElementIds,
    const std::string& effectTrackId, const TARGET_SET& targets, std::string& status)
{
    bool used = false;
    for (const auto& sequence : m_Document.Get_Templates())
        for (const auto& track : sequence.effectTracks)
            used |= track.resourceKind == "V1_EFFECT" && track.resourceId == full.strEffectAssetId &&
                (effectTrackId.empty() || effectTrackId == track.effectTrackId);
    if (!used || !targets.Is_Complete() || full.strEffectAssetId != selected.strEffectAssetId ||
        drawElementIds.empty() || selected.Elements.empty() ||
        std::any_of(selected.Elements.begin(), selected.Elements.end(), [&](const auto& element) {
            return std::none_of(full.Elements.begin(), full.Elements.end(), [&](const auto& source) {
                return source.strElementId == element.strElementId;
            });
        }))
    { status = "World Sequence Solo needs elements of its exact Effect and occurrence."; return false; }
    EFFECT_PREVIEW staged;
    EFFECT_SELECTION selection{full.strEffectAssetId, effectTrackId, {}};
    CEffectPlayback timing;
    if (!timing.Stage_Document(full, status) ||
        !CEffectPresentationService::Prepare_WorldPreviewTarget(targets.device, targets.context,
            full, staged.target, status) ||
        !CEffectPresentationService::Prepare_WorldPreviewTarget(targets.device, targets.context,
            selected, selection.target, status, &drawElementIds)) return false;
    staged.document = std::make_shared<const EFFECT_DOCUMENT_DESC>(full);
    staged.durationSeconds = timing.Get_DurationSeconds();
    auto previews = m_EffectPreviews;
    previews.insert_or_assign(full.strEffectAssetId, std::move(staged));
    return Commit_EffectPreviews(std::move(previews), std::move(selection), status);
}

bool CWorldSequencePlayer::Clear_EffectPreviews(std::string& status)
{
    return Commit_EffectPreviews({}, {}, status);
}

bool CWorldSequencePlayer::Commit_EffectPreviews(
    std::unordered_map<std::string, EFFECT_PREVIEW> previews,
    std::optional<EFFECT_SELECTION> selection, std::string& status)
{
    const auto targetFor = [](const auto& source, const auto& filter,
        const ACTIVE_INSTANCE::EFFECT_INSTANCE& effect) -> std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET> {
        if (!effect.sourceDocument) return {};
        const auto& id = effect.sourceDocument->strEffectAssetId;
        if (filter && filter->assetId == id &&
            (filter->effectTrackId.empty() || filter->effectTrackId == effect.effectTrackId)) return filter->target;
        const auto found = source.find(id);
        return found == source.end() ? nullptr : found->second.target;
    };
    std::vector<std::pair<EFFECT_WORLD_ROOT_HANDLE,
        std::shared_ptr<const EFFECT_WORLD_PREVIEW_TARGET>>> replacements;
    std::unordered_set<uint64_t> retired;
    for (const auto* instances : {&m_Active, &m_Held})
        for (const auto& active : *instances)
            for (const auto& effect : active.effects)
            {
                if (!effect.v1Handle || !effect.sourceDocument) continue;
                const auto draft = previews.find(effect.sourceDocument->strEffectAssetId);
                if (draft != previews.end() && !draft->second.target)
                { retired.insert(effect.v1Handle); continue; }
                if (targetFor(m_EffectPreviews, m_EffectSelection, effect) !=
                    targetFor(previews, selection, effect))
                    replacements.emplace_back(EFFECT_WORLD_ROOT_HANDLE{effect.v1Handle},
                        targetFor(previews, selection, effect));
            }
    if (!CEffectPresentationService::Replace_WorldRootPreviews(replacements, status)) return false;
    m_EffectPreviews = std::move(previews);
    m_EffectSelection = std::move(selection);
    for (auto* instances : {&m_Active, &m_Held})
        for (auto& active : *instances)
        {
            // Destructive retirement follows successful replacement staging so
            // a bad edit elsewhere still leaves the complete old scene intact.
            std::erase_if(active.effects, [&](const auto& effect) {
                if (!retired.contains(effect.v1Handle)) return false;
                CEffectPresentationService::Stop_WorldRoot({effect.v1Handle});
                return true;
            });
            for (auto& effect : active.effects)
            {
                if (!effect.v1Handle || !effect.sourceDocument) continue;
                const std::string id = effect.sourceDocument->strEffectAssetId;
                const auto found = m_EffectPreviews.find(id);
                effect.sourceDocument = found == m_EffectPreviews.end() ?
                    CEffectCatalog::Find_Loaded(id) : found->second.document;
                const bool visible = !m_EffectSelection || (m_EffectSelection->assetId == id &&
                    (m_EffectSelection->effectTrackId.empty() || m_EffectSelection->effectTrackId == effect.effectTrackId));
                (void)CEffectPresentationService::Set_WorldRootInspectionVisible({effect.v1Handle}, visible);
            }
        }
    status = m_EffectSelection ? "Selected sequence elements staged; actor animation and source time are preserved." :
        "Sequence Effect drafts staged; actor animation and source time are preserved.";
    return true;
}

bool_t CWorldSequencePlayer::Set_DocumentBatch(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, const std::vector<CWorldSequencePlayer*>& players, std::string& status)
{
    if (!targets.Is_Complete())
    { status = "World Object runtime targets are unavailable."; return false; }
    std::unordered_set<CWorldSequencePlayer*> unique;
    if (players.empty() || std::any_of(players.begin(), players.end(), [&](auto* player) {
        return !player || !unique.insert(player).second; }))
    { status = "World Object document batch has an empty or duplicate player."; return false; }
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    // Object-only documents own their bindings; unrelated map/deploy tables
    // are neither read nor consumed by Validate for these occurrences.
    const bool usesPlacedTargets = std::any_of(document.Get_Instances().begin(), document.Get_Instances().end(),
        [](const auto& instance) { return std::any_of(instance.bindings.begin(), instance.bindings.end(),
            [](const auto& binding) { return binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; }); });
    if (usesPlacedTargets) Collect_ValidationTargets(targets, placements, deploy);
    if (!document.Validate(placements, deploy, status))
        return false;
    std::vector<CWorldSequenceDocument> staged;
    staged.reserve(players.size());
    for (size_t i = 0; i < players.size(); ++i) staged.push_back(document);
    for (size_t i = 0; i < players.size(); ++i)
    {
        auto& player = *players[i];
        player.Stop_All(targets, true);
        player.Clear_PreparedObjects();
        player.m_ObjectModels.clear();
        player.m_EffectSnapshots.clear();
        player.m_EffectPreviews.clear();
        player.m_EffectSelection.reset();
        player.m_Document = std::move(staged[i]);
        player.m_Status = "World Object document admitted.";
    }
    status = "World Object document admitted.";
    return true;
}

bool_t CWorldSequencePlayer::Replace_DocumentKeepingModels(const CWorldSequenceDocument& document,
    const TARGET_SET& targets, std::string& status)
{
    if (!targets.Is_Complete())
    {
        status = "World Object runtime targets are unavailable.";
        m_Status = status;
        return false;
    }
    WORLD_SEQUENCE_PLACEMENT_MAP placements;
    WORLD_SEQUENCE_DEPLOY_MAP deploy;
    Collect_ValidationTargets(targets, placements, deploy);
    if (!document.Validate(placements, deploy, status))
    {
        m_Status = status;
        return false;
    }
    CWorldSequenceDocument staged = document;
    Stop_All(targets, true);
    Clear_PreparedObjects();
    // A kept model must still be what its resource asks for. Anything renamed,
    // removed or changed in its model inputs is rebuilt on the next Play.
    size_t kept = 0u;
    for (auto entry = m_ObjectModels.begin(); entry != m_ObjectModels.end();)
    {
        const auto* before = m_Document.Find_ObjectResource(entry->first);
        const auto* after = staged.Find_ObjectResource(entry->first);
        if (nullptr == before || nullptr == after || !Same_ObjectModelInputs(*before, *after))
            entry = m_ObjectModels.erase(entry);
        else
        {
            ++kept;
            ++entry;
        }
    }
    m_EffectSnapshots.clear();
    m_EffectPreviews.clear();
    m_EffectSelection.reset();
    m_Document = std::move(staged);
    status = "World Object document admitted; kept " + std::to_string(kept) + " prepared model(s).";
    m_Status = status;
    return true;
}

bool_t CWorldSequencePlayer::Same_ObjectModelInputs(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& left, const WORLD_SEQUENCE_OBJECT_RESOURCE& right)
{
    return left.modelAssetId == right.modelAssetId && left.modelPreScale == right.modelPreScale &&
        left.animated == right.animated && left.diffuseTextureAssetId == right.diffuseTextureAssetId &&
        left.materialSourceModelAssetId == right.materialSourceModelAssetId &&
        left.animationSetAssetId == right.animationSetAssetId &&
        left.presentationBossArchetypeId == right.presentationBossArchetypeId &&
        left.materialProfile == right.materialProfile && left.mapMaterialBindings == right.mapMaterialBindings;
}

bool_t CWorldSequencePlayer::Admit_PresentationBossModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const TARGET_SET& targets, OBJECT_MODEL& out)
{
    /* Reuse the product boss admission instead of decoding the body again: the
       Level owns the combined body (AnimSet attached), its armour and weapon. */
    const std::string& archetypeId = resource.presentationBossArchetypeId;
    const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
    if (!actor)
    { m_Status = "World Object presentation boss is not in the boss catalog: " + archetypeId; return false; }
    if (actor->bodyModel != resource.modelAssetId || actor->animationSetId != resource.animationSetAssetId ||
        actor->bodyModelPreScale != resource.modelPreScale)
    { m_Status = "World Object body fields must match presentation boss " + archetypeId + ": " + resource.objectId; return false; }
    const wstring_t bodyTag = CValtanPresentationAssetService::Get_BodyModelPrototypeTag(archetypeId);
    if (bodyTag.empty())
    { m_Status = "World Object presentation boss has no product assembly: " + archetypeId; return false; }
    if (FAILED(CValtanPresentationAssetService::Ensure_Prototypes(targets.device, targets.context,
        targets.levelIndex, archetypeId)))
    { m_Status = "World Object presentation boss admission failed: " + archetypeId; return false; }
    out.model = dynamic_pointer_cast<CModel>(CGameInstance::Get().Clone_Prototype(targets.levelIndex, bodyTag));
    if (!out.model || !out.model->Get_NumMeshes() || !out.model->Has_Animations())
    { m_Status = "World Object presentation boss body is unavailable: " + Narrow_PrototypeTag(bodyTag); return false; }
    CWorldSequenceObject::DESC parts;
    Fill_PresentationParts(archetypeId, parts);
    for (const auto& part : parts.presentationParts)
    {
        if (!CGameInstance::Get().Clone_Prototype(targets.levelIndex, part.modelPrototypeTag))
        { m_Status = "World Object presentation boss part is unavailable: " + Narrow_PrototypeTag(part.modelPrototypeTag); return false; }
        if (!part.socketBone.empty() && !out.model->Has_Bone(part.socketBone.c_str()))
        { m_Status = "World Object presentation boss socket bone is unavailable: " + part.socketBone; return false; }
    }
    out.presentationBossArchetypeId = archetypeId;
    out.deviceIdentity = targets.device.Get();
    out.contextIdentity = targets.context.Get();
    out.catalogIdentity = targets.pCatalog;
    return true;
}

const CWorldSequencePlayer::OBJECT_MODEL* CWorldSequencePlayer::Find_PreparedObjectModel(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const
{
    const auto matches = [&](const auto& entry) {
        const auto* prepared = m_Document.Find_ObjectResource(entry.first);
        return prepared && Same_ObjectModelInputs(*prepared, resource) && entry.second.model &&
            entry.second.deviceIdentity == targets.device.Get() &&
            entry.second.contextIdentity == targets.context.Get() && entry.second.catalogIdentity == targets.pCatalog;
    };
    // Preserve the exact prototype identity used by an existing object clone pool.
    const auto exact = m_ObjectModels.find(resource.objectId);
    if (exact != m_ObjectModels.end() && matches(*exact)) return &exact->second;
    for (const auto& entry : m_ObjectModels)
        if (matches(entry)) return &entry.second;
    return nullptr;
}

const CWorldSequencePlayer::OBJECT_MODEL* CWorldSequencePlayer::Find_SharedObjectModel(
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const TARGET_SET& targets) const
{
    const auto* owner = targets.objectPreparationOwner;
    if (!owner || owner->m_Document.Get_AreaId() != m_Document.Get_AreaId() ||
        owner->m_Document.Get_Revision() != m_Document.Get_Revision()) return nullptr;
    return owner->Find_PreparedObjectModel(resource, targets);
}

void CWorldSequencePlayer::Remember_SharedObjectModel(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource,
    const OBJECT_MODEL& model, const TARGET_SET& targets) const
{
    auto* owner = targets.objectPreparationOwner;
    if (!owner || owner == this || owner->m_Document.Get_AreaId() != m_Document.Get_AreaId() ||
        owner->m_Document.Get_Revision() != m_Document.Get_Revision() || !model.model ||
        model.deviceIdentity != targets.device.Get() || model.contextIdentity != targets.context.Get() ||
        model.catalogIdentity != targets.pCatalog) return;
    const auto* prepared = owner->m_Document.Find_ObjectResource(resource.objectId);
    if (!prepared || !Same_ObjectModelInputs(*prepared, resource))
    {
        const auto& resources = owner->m_Document.Get_ObjectResources();
        const auto found = std::find_if(resources.begin(), resources.end(), [&](const auto& row) {
            return Same_ObjectModelInputs(row, resource);
        });
        if (found == resources.end()) return;
        prepared = &*found;
    }
    // The admitted owner document owns this entry and clears it on reload. Each
    // visible object still clones its own CModel/Bones/Animations from the prototype.
    owner->m_ObjectModels.emplace(prepared->objectId, model);
}

bool_t CWorldSequencePlayer::Prepare_ObjectResources(
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    const auto* sequence = m_Document.Find_Template(instance.templateId);
    if (sequence)
        for (const auto& effect : sequence->effectTracks)
        {
            if (effect.resourceKind == "V1_EFFECT")
            {
                const std::vector<std::string> ids{effect.resourceId};
                std::vector<std::string> queued;
                if (!CEffectPresentationService::Queue_ProductTargets_Priority(ids, queued, m_Status)) return false;
                const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(ids);
                if (probe.iFailedCount || probe.iUnavailableCount)
                { m_Status = "World Object V1 effect preparation failed: " + effect.resourceId; return false; }
                if (!probe.bCatalogRevisionCurrent || !probe.bSettled)
                { m_Status = "World Object V1 effect is preparing: " + effect.resourceId; return false; }
                const auto document = CEffectCatalog::Find_Loaded(effect.resourceId);
                const auto binding = std::find_if(instance.bindings.begin(), instance.bindings.end(),
                    [&](const auto& candidate) { return candidate.slotId == effect.slotId &&
                        candidate.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
                const auto* owner = binding == instance.bindings.end() ? nullptr :
                    m_Document.Find_ObjectResource(binding->targetId);
                if (!document || !owner)
                { m_Status = "World Object V1 effect has no prepared document or owner: " + effect.resourceId; return false; }
                for (const auto& cue : document->ModelCues)
                    if (cue.strModelAssetId != owner->modelAssetId)
                    { m_Status = "World Object V1 model cue names a different model: " + cue.strCueId; return false; }
                continue;
            }
            const auto key = effect.resourceKind + ":" + effect.resourceId;
            if (m_EffectSnapshots.contains(key)) continue;
            std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> snapshot;
            std::string error;
            const auto kind = effect.resourceKind == "GROUP" ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF;
            if (!CEffectV2Catalog::Get().Load_ResourceSnapshot(kind, effect.resourceId, snapshot, error))
            { m_Status = "World Object effect admission failed: " + effect.resourceId + " / " + error; return false; }
            EFFECT_V2_GROUP group;
            if (kind == EFFECT_V2_RESOURCE_KIND::GROUP) group = *snapshot->Find_Group(effect.resourceId);
            else
            {
                group.strGroupId = effect.resourceId;
                EFFECT_V2_GROUP_CHILD child;
                child.strChildId = "object.effect.leaf"; child.strResourceId = child.strEffectId = effect.resourceId;
                group.Children.push_back(child);
            }
            if (!CEffectV2Runtime::Prewarm_Group(group, snapshot, targets.device, targets.context))
            { m_Status = "World Object effect prewarm failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
            m_EffectSnapshots.emplace(key, std::move(snapshot));
        }
    for (const auto& binding : instance.bindings)
    {
        if (sequence && binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
        {
            // Reject trimmed native ranges before releasing an existing preview owner.
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId || (animation.sourceStartMs == 0u && animation.sourceEndMs == 0u)) continue;
                uint64_t targetId = 0u;
                const auto object = Try_ParseTargetId(binding, targetId) && targets.pDeployRuntime ?
                    targets.pDeployRuntime->Find(targetId) : nullptr;
                float duration = 0.f, seconds = 0.f;
                if (object)
                    for (const auto& clip : object->Get_AnimationClips())
                        if (clip.name == animation.clipName) { duration = clip.durationSeconds; break; }
                if (!CWorldSequenceDocument::Try_SampleAnimationTicks(animation, static_cast<float>(animation.startMs),
                    static_cast<float>(sequence->durationMs), 1.f, duration, seconds))
                { m_Status = "World sequence Deploy animation source range is unavailable: " + animation.clipName; return false; }
            }
        }
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        if (!resource || resource->modelAssetId.empty())
        { m_Status = "World Object model binding is unavailable: " + binding.targetId; return false; }
        if (!targets.device || !targets.context)
        { m_Status = "World Object render device is unavailable."; return false; }
        auto model = m_ObjectModels.find(resource->objectId);
        if (model != m_ObjectModels.end() &&
            (model->second.deviceIdentity != targets.device.Get() ||
                model->second.contextIdentity != targets.context.Get() || model->second.catalogIdentity != targets.pCatalog))
        { m_Status = "World Object prepared model belongs to different render targets; reload its owner: " + resource->objectId; return false; }
        if (model == m_ObjectModels.end())
        {
            const auto* prepared = Find_PreparedObjectModel(*resource, targets);
            if (!prepared) prepared = Find_SharedObjectModel(*resource, targets);
            if (prepared) model = m_ObjectModels.emplace(resource->objectId, *prepared).first;
        }
        if (model == m_ObjectModels.end() && !resource->presentationBossArchetypeId.empty())
        {
            OBJECT_MODEL staged;
            if (!Admit_PresentationBossModel(*resource, targets, staged)) return false;
            model = m_ObjectModels.emplace(resource->objectId, std::move(staged)).first;
        }
        if (model == m_ObjectModels.end())
        {
            OBJECT_MODEL staged;
            staged.deviceIdentity = targets.device.Get();
            staged.contextIdentity = targets.context.Get();
            staged.catalogIdentity = targets.pCatalog;
            const auto path = CRuntimeAssetRoot::Resolve(resource->modelAssetId);
            if (path.empty())
            { m_Status = "World Object model path is invalid: " + resource->modelAssetId; return false; }
            MODEL_ASSET_LOAD_DESC load;
            if (!CActorCatalog::Build_DerivedModelLoadDescription(resource->modelAssetId,
                resource->materialSourceModelAssetId, load, m_Status)) return false;
            for (const auto& binding : resource->mapMaterialBindings)
            {
                const auto* asset = targets.pCatalog->Find(binding.sourceAssetId);
                if (!asset) { m_Status = "World Object map material asset is missing: " + binding.sourceAssetId; return false; }
                const auto found = std::find_if(asset->materialOverrides.begin(), asset->materialOverrides.end(),
                    [&](const auto& row) { return row.materialName == binding.sourceMaterialName; });
                if (found == asset->materialOverrides.end() || found->surface.family != MODEL_SURFACE_FAMILY::SOURCE_BG_OPAQUE_MASKED)
                { m_Status = "World Object map surface binding is missing or unsupported: " + binding.sourceMaterialName; return false; }
                auto material = *found;
                material.materialName = binding.materialName;
                material.surface.sourceBgUnlit = binding.unlit;
                material.surface.hasBakedLighting = false;
                material.surface.hasStaticShadow = false;
                material.bakedAveragePath.clear(); material.bakedDirectionalPath.clear(); material.staticShadowPath.clear();
                if (!binding.diffuseTextureAssetId.empty())
                {
                    material.surfaceDiffusePath = CRuntimeAssetRoot::Resolve(binding.diffuseTextureAssetId);
                    if (material.surfaceDiffusePath.empty()) { m_Status = "World Object surface texture is invalid"; return false; }
                }
                std::erase_if(load.materialOverrides, [&](const auto& prior) { return prior.materialName == material.materialName; });
                load.materialOverrides.push_back(std::move(material));
            }
            if (resource->materialProfile)
            {
                MODEL_MATERIAL_OVERRIDE material;
                if (!CWorldSequenceDocument::Build_MaterialOverride(*resource->materialProfile, load.assetRoot, material))
                { m_Status = "World Object material admission failed: " + resource->objectId; return false; }
                std::erase_if(load.materialOverrides, [&](const MODEL_MATERIAL_OVERRIDE& prior) {
                    return prior.materialName == material.materialName;
                });
                load.materialOverrides.push_back(std::move(material));
            }
            staged.model = CModel::Create(targets.device, targets.context,
                resource->animated ? MODEL::ANIM : MODEL::NONANIM, load,
                XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
            if (!staged.model || !staged.model->Get_NumMeshes())
            {
                m_Status = "World Object model admission failed: " + resource->modelAssetId;
                // This Create overload decodes the nonempty load.meshPath on this thread.
                const auto report = CModelDecoderRegistry::Get().Get_LastReport();
                if (report.meshPath == load.meshPath)
                {
                    if (!report.succeeded && !report.error.empty()) m_Status += " / " + report.error;
                    else if (report.succeeded) m_Status += " / binary decoded; model setup failed";
                }
                return false;
            }
            if (!resource->animationSetAssetId.empty())
            {
                /* The Valtan bodies ship their clips in a separate AnimSet WModel.
                   Attach it to the prototype before any clone so every occurrence
                   samples the same clip table the product boss uses. */
                const auto animationSetPath = CRuntimeAssetRoot::Resolve(resource->animationSetAssetId);
                if (animationSetPath.empty())
                { m_Status = "World Object animation set path is invalid: " + resource->animationSetAssetId; return false; }
                const unique_ptr<CModel> animationSet = CModel::Create(targets.device, targets.context,
                    MODEL::ANIM, animationSetPath.string().c_str(),
                    XMMatrixScaling(resource->modelPreScale, resource->modelPreScale, resource->modelPreScale));
                if (!animationSet || !animationSet->Has_Animations() ||
                    FAILED(staged.model->Attach_AnimationSet(*animationSet)))
                { m_Status = "World Object animation set does not match the body: " + resource->animationSetAssetId; return false; }
            }
            if (!resource->diffuseTextureAssetId.empty())
            {
                const auto texture = CRuntimeAssetRoot::Resolve(resource->diffuseTextureAssetId);
                if (texture.empty() || FAILED(DirectX::CreateDDSTextureFromFileEx(
                    targets.device.Get(), texture.c_str(), 0u, D3D11_USAGE_DEFAULT,
                    D3D11_BIND_SHADER_RESOURCE, 0u, 0u, DirectX::DDS_LOADER_FORCE_SRGB,
                    nullptr, &staged.diffuse)) || !staged.diffuse)
                { m_Status = "World Object texture admission failed: " + resource->diffuseTextureAssetId; return false; }
            }
            if (!staged.diffuse)
                for (uint32_t mesh = 0; mesh < staged.model->Get_NumMeshes(); ++mesh)
                    if (!staged.model->Has_MaterialTexture(mesh, aiTextureType_DIFFUSE))
                    { m_Status = "World Object diffuse texture is unavailable: " + resource->modelAssetId +
                        " (mesh " + std::to_string(mesh) + ")"; return false; }
            model = m_ObjectModels.emplace(resource->objectId, std::move(staged)).first;
        }
        if (sequence)
            for (const auto& animation : sequence->animationTracks)
            {
                if (animation.slotId != binding.slotId) continue;
                uint32_t index = UINT32_MAX;
                for (uint32_t i = 0; i < model->second.model->Get_NumAnimations(); ++i)
                    if (animation.clipName == model->second.model->Get_AnimationName(i)) { index = i; break; }
                if (index == UINT32_MAX) { m_Status = "World Object clip is absent: " + animation.clipName; return false; }
                if (animation.sourceStartMs != 0u || animation.sourceEndMs != 0u)
                {
                    float position = 0.f, duration = 0.f, ticks = 0.f;
                    if (!model->second.model->Get_AnimationProgress(index, position, duration) ||
                        !CWorldSequenceDocument::Try_SampleAnimationTicks(animation, static_cast<float>(animation.startMs),
                            static_cast<float>(sequence->durationMs), model->second.model->Get_AnimationTickPerSecond(index), duration, ticks))
                    { m_Status = "World Object animation source range exceeds the native clip range: " + animation.clipName; return false; }
                }
            }
        if (sequence)
            for (const auto& effect : sequence->effectTracks)
            {
                if (effect.slotId != binding.slotId) continue;
                if (!effect.bone.empty() && !model->second.model->Has_Bone(effect.bone.c_str()))
                { m_Status = "World Object Effect bone is unavailable: " + effect.bone; return false; }
                if (effect.resourceKind != "V1_EFFECT") continue;
                const auto document = CEffectCatalog::Find_Loaded(effect.resourceId);
                for (const auto& element : document->Elements)
                {
                    const auto& attachment = element.ActionCueAttachment;
                    if (element.bVisible && attachment.bEnabled && attachment.bFollow &&
                        attachment.eOrientation != EFFECT_ATTACHMENT_ORIENTATION::CAMERA_VIEW &&
                        !model->second.model->Has_Bone(attachment.strRuntimeBoneName.c_str()))
                    {
                        // Not a rejection: Sample_ObjectEffectAttachments anchors this slot at the
                        // object pivot, as the owner-anchored product path does for a missing bone.
                        OutputDebugStringA(("[Client][WorldObject] V1 source bone '" + attachment.strRuntimeBoneName +
                            "' is absent on " + resource->modelAssetId + "; slot '" + attachment.strRuntimeAnchorSlotId +
                            "' follows the object pivot (" + effect.resourceId + ")\n").c_str());
                        break;
                    }
                }
            }
        if (sequence)
            for (const auto& collider : sequence->colliderTracks)
                if (collider.slotId == binding.slotId && !collider.attachmentBone.empty() &&
                    !model->second.model->Has_Bone(collider.attachmentBone.c_str()))
                { m_Status = "World Object collider bone is unavailable: " + collider.attachmentBone; return false; }
        Remember_SharedObjectModel(*resource, model->second, targets);
    }
    return true;
}

bool_t CWorldSequencePlayer::Prepare_InstanceResources(const std::string& instanceId, const TARGET_SET& targets)
{
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance || !instance->enabled)
    { m_Status = "World Object state is absent or disabled: " + instanceId; return false; }
    if (!targets.Is_Complete())
    { m_Status = "World Object runtime targets are unavailable."; return false; }
    for (const auto& binding : instance->bindings)
    {
        if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        uint64_t targetId = 0;
        if (!Try_ParseTargetId(binding, targetId) ||
            (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
                !Find_Placement(*targets.pPlacements, targetId)) ||
            (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT &&
                !targets.pDeployRuntime->Find(targetId)))
        { m_Status = "World Object placement is outside the active map scope: " + binding.targetId; return false; }
    }
    return Prepare_ObjectMotionChain(*instance, targets);
}

bool_t CWorldSequencePlayer::Prewarm_ObjectInstances(const std::string& instanceId,
    const uint32_t copies, const TARGET_SET& targets)
{
    bool_t ready = false;
    return Prewarm_ObjectInstancesInternal(instanceId, copies, targets, copies, ready);
}

bool_t CWorldSequencePlayer::Prewarm_ObjectInstancesStep(const std::string& instanceId,
    const uint32_t copies, const TARGET_SET& targets, bool_t& ready)
{
    return Prewarm_ObjectInstancesInternal(instanceId, copies, targets, 1u, ready);
}

bool_t CWorldSequencePlayer::Prewarm_ObjectInstancesInternal(const std::string& instanceId,
    const uint32_t copies, const TARGET_SET& targets, const uint32_t maximumNewCopies, bool_t& ready)
{
    ready = false;
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled || instance->anchorKind != "WORLD" ||
        instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        sequence->objectMotion.EmissionCount() == 0u || sequence->objectMotion.EmissionCount() > 128u || copies == 0u || copies > 128u ||
        (targets.objectPreparationOwner && targets.objectPreparationOwner != this))
    { m_Status = "World Object prewarm requires its owner, one enabled WORLD object binding, 1..128 emissions and copies: " + instanceId; return false; }
    if (!Prepare_InstanceResources(instanceId, targets)) return false;
    const auto& objectId = instance->bindings.front().targetId;
    const auto model = m_ObjectModels.find(objectId);
    if (model == m_ObjectModels.end())
    { m_Status = "World Object prewarm model is unavailable: " + objectId; return false; }
    const auto found = m_PreparedObjectPools.find(objectId);
    auto pool = found == m_PreparedObjectPools.end() ? std::make_shared<PREPARED_OBJECT_POOL>() : found->second;
    if (found != m_PreparedObjectPools.end() && (!pool->acceptsReturns || pool->levelIndex != targets.levelIndex))
    { m_Status = "World Object prewarm belongs to another level; reload its owner: " + objectId; return false; }
    if (pool->capacity >= copies)
    { ready = true; m_Status = "World Object clones already prepared: " + objectId + " / " + std::to_string(pool->capacity); return true; }
    size_t total = copies - pool->capacity;
    for (const auto& [id, prepared] : m_PreparedObjectPools) total += prepared->capacity;
    if (total > 1024u) { m_Status = "World Object prepared clone budget reached (1024)."; return false; }
    const uint32_t nextCapacity = pool->capacity + (std::min)(copies - pool->capacity, maximumNewCopies);
    pool->idle.reserve(copies);
    std::vector<shared_ptr<CWorldSequenceObject>> staged;
    staged.reserve(nextCapacity - pool->capacity);
    const auto rollback = [&]() {
        for (auto& object : staged)
        {
            object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, object);
        }
    };
    try
    {
        for (uint32_t index = pool->capacity; index < nextCapacity; ++index)
        {
            CWorldSequenceObject::DESC desc;
            desc.levelIndex = targets.levelIndex;
            desc.modelPrototype = model->second.model;
            desc.diffuseTexture = model->second.diffuse;
            Fill_PresentationParts(model->second.presentationBossArchetypeId, desc);
            shared_ptr<CGameObject> created;
            const HRESULT result = CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
                CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex, CWorldSequenceObject::LAYER_TAG, &desc, &created);
            auto object = dynamic_pointer_cast<CWorldSequenceObject>(created);
            if (FAILED(result) || !object)
            {
                if (created) CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, created);
                rollback();
                m_Status = "World Object prewarm clone/shader creation failed: " + objectId +
                    " / copy " + std::to_string(index + 1u) + " / HRESULT " + std::to_string(static_cast<int32_t>(result));
                return false;
            }
            object->Hide();
            staged.push_back(std::move(object));
        }
        // Reserve the owner entry before committing clones. A failed insertion
        // removes this step's staged layer objects and leaves the old pool intact.
        if (found == m_PreparedObjectPools.end()) m_PreparedObjectPools.emplace(objectId, pool);
    }
    catch (...)
    {
        rollback();
        throw;
    }
    pool->idle.insert(pool->idle.end(), staged.begin(), staged.end());
    pool->levelIndex = targets.levelIndex;
    pool->capacity = nextCapacity;
    ready = pool->capacity >= copies;
    m_Status = ready ? "World Object hidden clones prepared: " + objectId + " / " + std::to_string(copies) :
        "World Object hidden clones preparing: " + objectId + " / " + std::to_string(pool->capacity) + "/" + std::to_string(copies);
    return true;
}

bool_t CWorldSequencePlayer::Prewarm_HiddenObjectPose(const std::string& instanceId,
    const f32_t elapsedMs, const TARGET_SET& targets)
{
    CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "WorldSequence.HiddenPose.Prewarm");
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    if (!instance || !sequence || !instance->enabled || instance->anchorKind != "WORLD" ||
        instance->motionEnd != WORLD_SEQUENCE_MOTION_END::HOLD || instance->bindings.size() != 1u ||
        instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
        sequence->objectMotion.EmissionCount() != 1u || !sequence->effectTracks.empty() ||
        !std::isfinite(elapsedMs) || elapsedMs < instance->startDelayMs ||
        !std::isfinite(instance->playbackSpeed) || instance->playbackSpeed <= 0.f ||
        targets.objectPreparationOwner != this)
    { m_Status = "Hidden pose prewarm requires one prepared WORLD HOLD model without Effect tracks: " + instanceId; return false; }
    const auto& binding = instance->bindings.front();
    const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
    const auto pool = m_PreparedObjectPools.find(binding.targetId);
    if (!resource || pool == m_PreparedObjectPools.end() || !pool->second->acceptsReturns ||
        pool->second->levelIndex != targets.levelIndex || pool->second->idle.empty())
    { m_Status = "Hidden pose prewarm has no idle prepared clone: " + instanceId; return false; }
    const f32_t localMs = (std::min)(static_cast<f32_t>(sequence->durationMs),
        (elapsedMs - instance->startDelayMs) * instance->playbackSpeed);
    const f32_t ageMs = localMs - sequence->objectMotion.EmissionDelayMs(0u);
    if (!std::isfinite(ageMs) || ageMs < 0.f)
    { m_Status = "Hidden pose prewarm precedes the object birth: " + instanceId; return false; }
    ACTIVE_INSTANCE active; active.instanceId = instanceId;
    PLAYER_ANCHOR anchor; XMStoreFloat4x4(&anchor.world, XMMatrixIdentity());
    float4x4_t world;
    if (!Sample_ObjectWorld(active, *instance, *sequence, *resource, binding.slotId, anchor, 0u, ageMs, world, m_Status,
        true, targets.objectWorldPostTransform)) return false;
    f32_t windowEnd = 0.f;
    const auto* animation = Find_AnimationTrackAt(*sequence, binding.slotId, ageMs, windowEnd);
    // Evaluate the actual animation on the clones which combat will borrow.
    // They remain idle and hidden; no playback, sound, Effect or scene visibility is committed.
    for (const auto& object : pool->second->idle)
    {
        if (!object || !object->Sample(world, false, animation, ageMs, windowEnd))
        { m_Status = "Hidden pose prewarm sample failed: " + instanceId; return false; }
    }
    m_Status = "Hidden endpoint pose prepared: " + instanceId;
    return true;
}

void CWorldSequencePlayer::Clear_PreparedObjects()
{
    // All document replacements invalidate this receipt, including equal revisions.
    m_PreparedPlaybackSubsets.clear();
    for (auto& [id, pool] : m_PreparedObjectPools)
    {
        // Borrowed clones keep this token alive. Once the owner resets, they
        // remove themselves on release instead of returning to an obsolete pool.
        pool->acceptsReturns = false;
        for (auto& object : pool->idle)
        {
            object->Hide();
            CGameInstance::Get().Remove_GameObject_from_Layer(pool->levelIndex, CWorldSequenceObject::LAYER_TAG, object);
        }
        pool->idle.clear();
    }
    m_PreparedObjectPools.clear();
}

void CWorldSequencePlayer::Release_Objects(ACTIVE_INSTANCE& active)
{
    for (const auto& sound : active.sounds) CGameInstance::Get().Stop_SoundCue(sound.handle);
    active.sounds.clear();
    active.objectColliderSamples.clear();
    for (const auto& effect : active.effects)
    {
        if (effect.handle) CEffectV2Runtime::Stop_Group(effect.handle);
        if (effect.v1Handle) CEffectPresentationService::Stop_WorldRoot({effect.v1Handle});
    }
    active.effects.clear();
    active.emissionAnchors.clear();
    for (auto& entry : active.objects)
    {
        entry.weaponReplacement.reset();
        entry.hatReplacement.reset();
        entry.npcPreviewSuppression.reset();
        if (entry.object)
        {
            entry.object->Hide();
            auto& pool = entry.preparationPool;
            if (pool && pool->acceptsReturns && pool->levelIndex == entry.levelIndex && pool->idle.size() < pool->capacity)
            {
                if (entry.object->Reset_ForReuse())
                {
                    pool->idle.push_back(std::move(entry.object));
                    continue;
                }
                --pool->capacity;
                m_Status = "World Object prepared clone could not return to its pool: " +
                    (entry.object->Get_RenderStatus().empty() ? entry.slotId : entry.object->Get_RenderStatus());
            }
            CGameInstance::Get().Remove_GameObject_from_Layer(entry.levelIndex,
                CWorldSequenceObject::LAYER_TAG, entry.object);
        }
    }
    active.objects.clear();
}

bool_t CWorldSequencePlayer::Try_GetPresentationBossAnchor(const std::string& archetype,
    const std::string& bone, PLAYER_ANCHOR& out, std::string& status) const
{
    status.clear();
    const CWorldSequenceObject* selected = nullptr;
    bool declared = false;
    for (const auto& active : m_Active)
    {
        const auto* instance = m_Document.Find_Instance(active.instanceId);
        if (!instance) continue;
        for (const auto& binding : instance->bindings)
        {
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
            const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
            if (!resource) continue;
            const auto* actor = CActorCatalog::Find_Boss(archetype);
            const bool derivedActor = actor && resource->animated &&
                resource->materialSourceModelAssetId == actor->bodyModel &&
                std::abs(resource->modelPreScale - actor->bodyModelPreScale) < .0000001f;
            if (resource->presentationBossArchetypeId != archetype && !derivedActor) continue;
            declared = true;
            for (const auto& entry : active.objects)
            {
                if (entry.slotId != binding.slotId || !entry.object || !entry.object->Is_Visible()) continue;
                if (selected && selected != entry.object.get())
                { status = "Cinematic World boss anchor is ambiguous: " + archetype; return false; }
                selected = entry.object.get();
            }
        }
    }
    if (!selected)
    {
        if (declared) status = "Cinematic World boss anchor is waiting for its sampled actor: " + archetype;
        return false;
    }
    if (!Resolve_BossBoneAnchor(selected->Get_Model(), selected->Get_SampledWorld(), bone, out, status)) return false;
    out.liveBossAnchor = true;
    return true;
}

bool_t CWorldSequencePlayer::Try_GetObjectPivot(const std::string& instanceId, float4x4_t& out,
    const uint32_t emissionIndex, const std::string& bone, const bool_t boneRotation,
    const std::string& effectTrackId) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end()) return false;
    const auto* instance = m_Document.Find_Instance(instanceId);
    const auto* sequence = nullptr != instance ? m_Document.Find_Template(instance->templateId) : nullptr;
    const WORLD_SEQUENCE_EFFECT_TRACK* effect = nullptr;
    if (!effectTrackId.empty())
    {
        if (!instance || !sequence || bone.empty() || !boneRotation || instance->anchorKind != "WORLD" ||
            instance->bindings.size() != 1u || instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
            return false;
        for (const auto& track : sequence->effectTracks)
            if (track.effectTrackId == effectTrackId)
            { if (effect) return false; effect = &track; }
        if (!effect || effect->slotId != instance->bindings.front().slotId || effect->resourceKind != "V1_EFFECT" ||
            !effect->followObject || !effect->inheritObjectRotation || !effect->bone.empty() ||
            !std::isfinite(effect->rotationDegrees.x) || !std::isfinite(effect->rotationDegrees.y) ||
            !std::isfinite(effect->rotationDegrees.z) || std::abs(effect->rotationDegrees.x) > .00001f ||
            std::abs(effect->rotationDegrees.z) > .00001f || !std::isfinite(effect->scale.x) ||
            !std::isfinite(effect->scale.y) || !std::isfinite(effect->scale.z) || effect->scale.x <= 0.f ||
            std::abs(effect->scale.x - effect->scale.y) > .00001f || std::abs(effect->scale.x - effect->scale.z) > .00001f ||
            !std::isfinite(effect->positionOffset.x) || !std::isfinite(effect->positionOffset.y) ||
            !std::isfinite(effect->positionOffset.z)) return false;
    }
    const auto sample = [&](const CWorldSequenceObject& object) {
        const auto& root = object.Get_SampledWorld();
        if (bone.empty()) { out = root; return true; }
        const auto model = object.Get_Model();
        if (!model || !model->Has_Bone(bone.c_str())) return false;
        matrix_t objectWorld = XMLoadFloat4x4(&root);
        if (effect)
        {
            // Match the WORLD Effect root: Bone * EffectTrackTRS * ObjectWorld.
            // Empty IDs retain the original model-bone pivot and scale exactly.
            objectWorld = XMMatrixScalingFromVector(XMLoadFloat3(&effect->scale)) *
                XMMatrixRotationRollPitchYaw(XMConvertToRadians(effect->rotationDegrees.x),
                    XMConvertToRadians(effect->rotationDegrees.y), XMConvertToRadians(effect->rotationDegrees.z)) *
                XMMatrixTranslationFromVector(XMLoadFloat3(&effect->positionOffset)) * objectWorld;
        }
        const matrix_t socket = model->Get_BoneMatrix(bone.c_str()) * objectWorld;
        const vector_t forward = boneRotation ? socket.r[2] : objectWorld.r[2];
        const float x = XMVectorGetX(forward), z = XMVectorGetZ(forward);
        if (!std::isfinite(x) || !std::isfinite(z) || (!boneRotation && x*x + z*z < 0.00000001f)) return false;
        const float sx = XMVectorGetX(XMVector3Length(objectWorld.r[0]));
        const float sy = XMVectorGetX(XMVector3Length(objectWorld.r[1]));
        const float sz = XMVectorGetX(XMVector3Length(objectWorld.r[2]));
        matrix_t pivot = XMMatrixRotationY(std::atan2(x, z));
        if (boneRotation)
        {
            for (size_t axis = 0u; axis < 3u; ++axis)
            {
                if (XMVectorGetX(XMVector3LengthSq(socket.r[axis])) < 0.00000001f) return false;
                pivot.r[axis] = XMVector3Normalize(socket.r[axis]);
            }
        }
        pivot = XMMatrixScaling(sx, sy, sz) * pivot;
        pivot.r[3] = XMVectorSetW(socket.r[3], 1.f);
        XMStoreFloat4x4(&out, pivot);
        const auto* values = reinterpret_cast<const float*>(&out);
        return std::all_of(values, values + 16u, [](float value) { return std::isfinite(value); });
    };
    if (nullptr == sequence || sequence->objectMotion.emissions.empty())
    {
        if (0u != emissionIndex || active->objects.size() != 1 || !active->objects.front().object ||
            !active->objects.front().object->Is_Visible()) return false;
        return sample(*active->objects.front().object);
    }
    // An authored row is one clone per anchor; a WORLD motion has exactly one anchor.
    const CWorldSequenceObject* found = nullptr;
    for (const auto& entry : active->objects)
    {
        if (entry.emissionIndex != emissionIndex || !entry.object) continue;
        if (found) return false;
        found = entry.object.get();
    }
    if (!found || !found->Is_Visible()) return false;
    return sample(*found);
}

void CWorldSequencePlayer::Collect_ObjectColliderSamples(std::vector<OBJECT_COLLIDER_SAMPLE>& out) const
{
    out.clear();
    for (const auto& active : m_Active)
        out.insert(out.end(), active.objectColliderSamples.begin(), active.objectColliderSamples.end());
}

std::string CWorldSequencePlayer::Get_ObjectSampleStatus(const std::string& instanceId) const
{
    const auto active = std::find_if(m_Active.begin(), m_Active.end(),
        [&](const auto& value) { return value.instanceId == instanceId; });
    if (active == m_Active.end()) return "World Object preview is not active.";
    if (!active->objectSampleStatus.empty()) return active->objectSampleStatus;
    const auto* instance = m_Document.Find_Instance(instanceId);
    if (!instance) return "World Object preview instance is unavailable.";
    const bool hasObjects = std::any_of(instance->bindings.begin(), instance->bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
    if (!hasObjects) return "Placed World Object state sampled at its authored map placement.";
    size_t visible = 0;
    const CWorldSequenceObject* first = nullptr;
    for (const auto& entry : active->objects)
        if (entry.object && entry.object->Is_Visible())
        { ++visible; if (!first) first = entry.object.get(); }
    if (!first) return "World Object: 0 visible (outside lifetime or hidden by the current key).";
    const auto& world = first->Get_SampledWorld();
    std::ostringstream status;
    status << "World Object: " << visible << " visible, first position (" << std::fixed << std::setprecision(2)
        << world._41 << ", " << world._42 << ", " << world._43 << ").";
    return status.str();
}

bool_t CWorldSequencePlayer::Get_EmissionAnchor(ACTIVE_INSTANCE& active, const TARGET_SET& targets,
    const std::string& key, const f32_t birthMs, const PLAYER_ANCHOR& baseline, PLAYER_ANCHOR& out)
{
    out = baseline;
    if (!targets.objectEmissionAnchor || baseline.liveBossAnchor) return true;
    const auto existing = active.emissionAnchors.find(key);
    if (existing != active.emissionAnchors.end()) { out = existing->second; return true; }
    if (!std::isfinite(birthMs) || birthMs < 0.f || !targets.objectEmissionAnchor(birthMs, out.world))
    { m_Status = "World Object emission origin is unavailable at " + std::to_string(birthMs) + " ms."; return false; }
    const auto* values = reinterpret_cast<const f32_t*>(&out.world);
    for (size_t i = 0; i < 16u; ++i)
        if (!std::isfinite(values[i]))
        { m_Status = "World Object emission origin is not finite."; return false; }
    const matrix_t world = XMLoadFloat4x4(&out.world);
    if (std::abs(out.world._14) > .00001f || std::abs(out.world._24) > .00001f ||
        std::abs(out.world._34) > .00001f || std::abs(out.world._44 - 1.f) > .00001f ||
        std::abs(XMVectorGetX(XMMatrixDeterminant(world))) < .000001f)
    { m_Status = "World Object emission origin is not an invertible affine transform."; return false; }
    out.emissionOverride = true;
    active.emissionAnchors.emplace(key, out);
    return true;
}

bool_t CWorldSequencePlayer::Sample_ObjectWorld(const ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& slotId,
    const PLAYER_ANCHOR& anchor, const uint32_t emitter, const f32_t ageMs, float4x4_t& out, std::string& status,
    const bool_t inheritObjectRotation,
    const decltype(TARGET_SET::objectWorldPostTransform)& postTransform)
{
    const auto& motion = sequence.objectMotion;
    const auto* track = Find_Track(sequence, slotId);
    const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
    const float seconds = ageMs * 0.001f;
    uint32_t random = motion.seed ^ ((emitter + 1u) * 0x9e3779b9u);
    const auto randomUnit = [&random]()
    { random ^= random << 13; random ^= random >> 17; random ^= random << 5;
        return static_cast<float>(random & 0xffffffu) / 16777215.f; };
    const float spread = XMConvertToRadians(motion.spreadDegrees);
    const matrix_t direction = sequence.effectTracks.empty() ?
        XMMatrixRotationRollPitchYaw((randomUnit() - .5f) * spread, (randomUnit() - .5f) * spread * 2.f, 0.f) :
        XMMatrixRotationY((randomUnit() - .5f) * spread);
    const vector_t velocity = XMVector3TransformNormal(XMLoadFloat3(&motion.velocity), direction);
    // Sample after the existing velocity draws so zero extents preserve old paths.
    // Seed + emitter remain stable across every age sample and attached Effect.
    const float spawnX = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.x;
    const float spawnY = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.y;
    const float spawnZ = (randomUnit() * 2.f - 1.f) * motion.spawnHalfExtents.z;
    const vector_t spawnOffset = XMVectorSet(spawnX, spawnY, spawnZ, 0.f);
    const matrix_t revolution = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(motion.revolutionDegreesPerSecond.x * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.y * seconds),
        XMConvertToRadians(motion.revolutionDegreesPerSecond.z * seconds));
    const vector_t orbit = XMLoadFloat3(&motion.revolutionOffset);
    const bool hasAuthoredEmissions = !motion.emissions.empty();
    const bool hasInstanceOffset = !((active.placement && instance.anchorKind == "WORLD") || anchor.emissionOverride);
    // Legacy emitters keep their existing addition order. Authored rows rotate
    // their local motion first, then add the instance's untranslated ring centre.
    const vector_t instanceOffset = hasInstanceOffset ? XMLoadFloat3(&instance.position) : XMVectorZero();
    const vector_t position = (hasAuthoredEmissions ? XMVectorZero() : instanceOffset) + XMLoadFloat3(&key.positionOffset) + spawnOffset +
        velocity * seconds + XMLoadFloat3(&motion.acceleration) * (.5f * seconds * seconds) +
        XMVector3TransformNormal(orbit, revolution) - orbit;
    const vector_t scale = XMLoadFloat3(&resource.scale) * XMLoadFloat3(&key.scaleMultiplier);
    matrix_t basis = XMLoadFloat4x4(&anchor.world);
    for (int axis = 0; axis < 3; ++axis)
    {
        const float length = XMVectorGetX(XMVector3LengthSq(basis.r[axis]));
        if (!std::isfinite(length) || length < 1e-6f)
        { status = "World Object anchor transform is invalid: " + resource.objectId; return false; }
        basis.r[axis] = XMVectorSetW(XMVector3Normalize(basis.r[axis]), 0.f);
    }
    // Floor colliders use emission + placement facing, without mesh upright correction or self-spin.
    // Attached effects can share that basis without changing the visible model sample.
    const matrix_t rotation = inheritObjectRotation ? XMMatrixRotationQuaternion(XMLoadFloat4(&key.rotationQuaternion)) *
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(motion.angularVelocityDegrees.x * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.y * seconds),
            XMConvertToRadians(motion.angularVelocityDegrees.z * seconds)) : XMMatrixIdentity();
    matrix_t world = XMMatrixScalingFromVector(scale) * rotation * XMMatrixTranslationFromVector(position);
    /* An authored row turns the whole local motion, orbit and seeded spawn
       offset included, before the existing placement / live-anchor composition. */
    if (hasAuthoredEmissions)
    {
        const auto& emission = motion.emissions[(std::min)(static_cast<size_t>(emitter), motion.emissions.size() - 1u)];
        world *= XMMatrixRotationY(XMConvertToRadians(emission.yawDegrees)) *
            XMMatrixTranslationFromVector(XMLoadFloat3(&emission.positionOffset));
        if (hasInstanceOffset)
            world *= XMMatrixTranslationFromVector(instanceOffset);
    }
    const bool localPlacement = active.placement && (instance.anchorKind == "BOSS" || instance.anchorKind == "PLAYER");
    if (!anchor.emissionOverride && !localPlacement) world *= basis;
    if (active.placement)
    {
        const auto& placement = *active.placement;
        world *= XMMatrixScalingFromVector(XMLoadFloat3(&placement.scale)) *
            XMMatrixRotationRollPitchYaw(XMConvertToRadians(placement.rotationDegrees.x),
                XMConvertToRadians(placement.rotationDegrees.y), XMConvertToRadians(placement.rotationDegrees.z)) *
            (anchor.emissionOverride ? XMMatrixIdentity() : XMMatrixTranslationFromVector(XMLoadFloat3(&placement.position)));
    }
    else if (!anchor.emissionOverride) world.r[3] += XMVectorSet(active.positionOffset.x, active.positionOffset.y, active.positionOffset.z, 0.f);
    if (anchor.emissionOverride || localPlacement) world *= basis;
    if (postTransform)
    {
        float4x4_t post;
        if (!Sample_ObjectPresentationPostTransform(postTransform, instance.instanceId, ageMs, post, status)) return false;
        world *= XMLoadFloat4x4(&post);
    }
    XMStoreFloat4x4(&out, world);
    return true;
}

bool_t CWorldSequencePlayer::Apply_Objects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const WORLD_SEQUENCE_TEMPLATE& sequence,
    const TARGET_SET& targets, const f32_t localMs, const bool_t visible, const bool_t holdFinalPose,
    const f32_t emissionStartMs, const f32_t emissionRate, const std::string& emissionMotionId)
{
    active.objectSampleStatus.clear();
    active.objectColliderSamples.clear();
    std::vector<OBJECT_COLLIDER_SAMPLE> colliderSamples;
    for (auto& entry : active.objects)
    {
        entry.object->Hide();
        entry.npcPreviewSuppression.reset();
    }
    if (!visible || std::none_of(instance.bindings.begin(), instance.bindings.end(),
        [](const auto& binding) { return binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; })) return true;
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "BOSS")
    {
        const auto* resource = m_Document.Find_ObjectResource(instance.bindings.front().targetId);
        PLAYER_ANCHOR anchor;
        anchor.liveBossAnchor = true;
        if (!resource || !targets.bossAnchor || !targets.bossAnchor(resource->anchorBossArchetypeId,
            resource->anchorBone, anchor, active.objectSampleStatus))
        {
            if (active.objectSampleStatus.empty()) active.objectSampleStatus = "World Object Boss anchor is unavailable.";
            m_Status = active.objectSampleStatus;
            return true;
        }
        anchors.push_back(anchor);
    }
    else if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
        if (anchors.empty())
        {
            active.objectSampleStatus = "World Object Character anchor is waiting for a living replicated player.";
            return true;
        }
    }
    else
    {
        PLAYER_ANCHOR world;
        XMStoreFloat4x4(&world.world, XMMatrixIdentity());
        anchors.push_back(world);
    }
    const auto& motion = sequence.objectMotion;
    for (const auto& binding : instance.bindings)
    {
        if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
        const auto* resource = m_Document.Find_ObjectResource(binding.targetId);
        const auto model = m_ObjectModels.find(binding.targetId);
        const auto* track = Find_Track(sequence, binding.slotId);
        if (!resource || model == m_ObjectModels.end())
        { m_Status = "World Object model was not prepared: " + binding.targetId; return false; }
        for (const auto& anchor : anchors)
            for (uint32_t emitter = 0; emitter < motion.EmissionCount(); ++emitter)
            {
                const f32_t delayMs = static_cast<f32_t>(motion.EmissionDelayMs(emitter));
                const f32_t ageMs = localMs - delayMs;
                if (ageMs < 0.f || (!holdFinalPose && !sequence.effectTracks.empty() && ageMs >= sequence.durationMs)) continue;
                const f32_t birthMs = emissionStartMs + delayMs / emissionRate;
                if (!sequence.effectTracks.empty() && active.durationMs && birthMs >= active.durationMs) continue;
                PLAYER_ANCHOR emissionAnchor;
                const auto emissionKey = emissionMotionId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                auto found = std::find_if(active.objects.begin(), active.objects.end(), [&](const auto& value)
                { return value.slotId == binding.slotId && value.entityId == anchor.entityId && value.emissionIndex == emitter; });
                if (found == active.objects.end())
                {
                    size_t total = 0;
                    for (const auto& value : m_Active) total += value.objects.size();
                    if (total >= 1024u) { m_Status = "World Object instance budget reached (1024)."; return false; }
                    shared_ptr<CWorldSequenceObject> object;
                    std::shared_ptr<PREPARED_OBJECT_POOL> pool;
                    const auto* prepared = Find_SharedObjectModel(*resource, targets);
                    if (prepared && prepared->model == model->second.model)
                    {
                        const auto available = targets.objectPreparationOwner->m_PreparedObjectPools.find(resource->objectId);
                        if (available != targets.objectPreparationOwner->m_PreparedObjectPools.end() &&
                            available->second->acceptsReturns && available->second->levelIndex == targets.levelIndex &&
                            !available->second->idle.empty())
                        {
                            pool = available->second;
                            object = std::move(pool->idle.back());
                            pool->idle.pop_back();
                        }
                    }
                    if (!object)
                    {
                        CWorldSequenceObject::DESC desc;
                        desc.levelIndex = targets.levelIndex;
                        desc.modelPrototype = model->second.model;
                        desc.diffuseTexture = model->second.diffuse;
                        Fill_PresentationParts(model->second.presentationBossArchetypeId, desc);
                        shared_ptr<CGameObject> staged;
                        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(targets.levelIndex,
                            CWorldSequenceObject::PROTOTYPE_TAG, targets.levelIndex,
                            CWorldSequenceObject::LAYER_TAG, &desc, &staged)))
                        { m_Status = "World Object clone/shader creation failed: " + resource->objectId; return false; }
                        object = dynamic_pointer_cast<CWorldSequenceObject>(staged);
                        if (!object)
                        {
                            CGameInstance::Get().Remove_GameObject_from_Layer(targets.levelIndex, CWorldSequenceObject::LAYER_TAG, staged);
                            m_Status = "World Object clone type is invalid: " + resource->objectId; return false;
                        }
                    }
                    active.objects.push_back({binding.slotId, anchor.entityId, emitter, targets.levelIndex,
                        std::move(object), std::move(pool)});
                    found = active.objects.end() - 1;
                }
                const auto key = track ? Sample_Track(sequence, *track, ageMs) : WORLD_SEQUENCE_TRANSFORM_KEY{};
                float4x4_t stored;
                if (!Sample_ObjectWorld(active, instance, sequence, *resource, binding.slotId, emissionAnchor, emitter,
                    (std::min)(ageMs, static_cast<f32_t>(sequence.durationMs)), stored, m_Status,
                    true, targets.objectWorldPostTransform)) return false;
                f32_t windowEnd = 0.f;
                const auto* animation = Find_AnimationTrackAt(sequence, binding.slotId, ageMs, windowEnd);
                if (!found->object)
                { m_Status = "World Object clone type is invalid: " + resource->objectId; return false; }
                if (!found->object->Get_RenderStatus().empty())
                { m_Status = found->object->Get_RenderStatus() + " / " + resource->objectId; return false; }
                for (const auto& material : sequence.materialTracks)
                {
                    if (material.slotId != binding.slotId) continue;
                    MODEL_SOURCE_CHARACTER_PARAMETERS parameters;
                    const auto& objectModel = found->object->Get_Model();
                    if (!resource->materialProfile || !objectModel ||
                        !CWorldSequenceDocument::Try_SampleMaterialParameters(*resource->materialProfile, material, ageMs, parameters))
                    { m_Status = "World Object native material sample failed: " + material.materialName; return false; }
                    if (!Apply_ObjectMaterialConstants(*objectModel, material.materialName, parameters, m_Status)) return false;
                }
                if (!found->object->Sample(stored, key.visible, animation, ageMs, windowEnd))
                { m_Status = "World Object transform/animation sample failed: " + resource->objectId; return false; }
                if (found->object->Is_Visible() && !binding.previewNpcPlacementId.empty())
                {
                    const auto npc = targets.previewNpc ? targets.previewNpc(binding.previewNpcPlacementId) : nullptr;
                    if (!npc)
                    {
                        found->object->Hide();
                        m_Status = "NPC preview requires its live placement: " + binding.previewNpcPlacementId;
                        return false;
                    }
                    npc->Acquire_CompositionPreviewSuppression();
                    found->npcPreviewSuppression = std::shared_ptr<const void>(npc.get(),
                        [owner = std::weak_ptr<CNpc>(npc)](const void*) {
                            if (const auto current = owner.lock()) current->Release_CompositionPreviewSuppression();
                        });
                }
                if (found->object->Is_Visible())
                    for (const auto& collider : sequence.colliderTracks)
                    {
#ifndef _DEBUG
                        // Release F1 only inspects the Bingo head; authoring bone previews stay Debug-only.
                        if (collider.colliderTrackId != "collider.bingo.hammer.head") continue;
#endif
                        if (collider.slotId != binding.slotId || ageMs < collider.startMs ||
                            ageMs >= static_cast<double>(collider.startMs) + collider.durationMs) continue;
                        OBJECT_COLLIDER_SAMPLE sample;
                        if (!Sample_ObjectCollider(collider, *resource, key, motion, emitter,
                            active.placement, *found->object, sample))
                        { m_Status = "World Object collider attachment sample failed: " + collider.colliderTrackId; return false; }
                        sample.instanceId = active.instanceId;
                        sample.colliderTrackId = collider.colliderTrackId;
                        sample.emissionIndex = emitter;
                        colliderSamples.push_back(std::move(sample));
                    }
                if (anchor.liveBossAnchor &&
                    (resource->objectId == "world.object.kouku.saydon_showtime_gun_left" ||
                     resource->objectId == "world.object.kouku.saydon_showtime_gun_right"))
                    CNpcPresentationAssetService::Track_SaydonWeaponReplacement(found->weaponReplacement,
                        anchor.bodyModel, found->object);
                else found->weaponReplacement.reset();
                if (anchor.liveBossAnchor && resource->objectId == "world.object.kouku.saydon_hat_right")
                    CNpcPresentationAssetService::Track_SaydonHatReplacement(found->hatReplacement,
                        anchor.bodyModel, found->object);
                else found->hatReplacement.reset();
            }
    }
    active.objectColliderSamples = std::move(colliderSamples);
    return true;
}

bool_t CWorldSequencePlayer::Apply_ObjectEffects(ACTIVE_INSTANCE& active,
    const WORLD_SEQUENCE_INSTANCE& instance, const TARGET_SET& targets)
{
    std::vector<PLAYER_ANCHOR> anchors;
    if (instance.anchorKind == "BOSS")
    {
        const auto* resource = m_Document.Find_ObjectResource(instance.bindings.front().targetId);
        PLAYER_ANCHOR anchor;
        anchor.liveBossAnchor = true;
        if (resource && targets.bossAnchor && targets.bossAnchor(resource->anchorBossArchetypeId,
            resource->anchorBone, anchor, active.objectSampleStatus)) anchors.push_back(anchor);
    }
    else if (!targets.objectEmissionAnchor && instance.anchorKind == "PLAYER")
    {
        if (targets.playerAnchors) anchors = targets.playerAnchors();
    }
    else
    {
        PLAYER_ANCHOR anchor;
        XMStoreFloat4x4(&anchor.world, XMMatrixIdentity());
        anchors.push_back(anchor);
    }
    std::unordered_set<std::string> wanted;
    // Resolve every still-visible event from the owning clock. This preserves an
    // earlier NEXT/LOOP tail and makes a direct seek equivalent to ordinary play.
    const auto sampleChain = [&](const std::string& firstId, const f32_t firstStartMs,
        const f32_t cutoffMs) -> bool_t
    {
        const auto* motion = m_Document.Find_Instance(firstId);
        f32_t start = firstStartMs;
        for (uint32_t depth = 0; motion && depth <= 32u; ++depth)
        {
            const auto* sequence = m_Document.Find_Template(motion->templateId);
            if (!sequence) return false;
            const f32_t rate = motion->playbackSpeed * active.playbackSpeed;
            start += motion->startDelayMs;
            const f32_t localMs = (active.elapsedMs - start) * rate;
            if (localMs < 0.f || start > cutoffMs) break;
            const f32_t period = static_cast<f32_t>(motion->CycleSpanMs(*sequence));
            for (const auto& effect : sequence->effectTracks)
            {
                const auto preview = m_EffectPreviews.find(effect.resourceId);
                const bool selectedTrack = !m_EffectSelection ||
                    (m_EffectSelection->assetId == effect.resourceId &&
                     (m_EffectSelection->effectTrackId.empty() || m_EffectSelection->effectTrackId == effect.effectTrackId));
                // V1 keeps sampled history behind its draw mask. V2 has no
                // document Solo projection and is retired until full replay.
                if (m_EffectSelection && effect.resourceKind != "V1_EFFECT") continue;
                if (preview != m_EffectPreviews.end() && !preview->second.target) continue;
                // Each effect owns its declared slot, including a different
                // skeleton/scale in a multi-actor cinematic or a NEXT motion.
                const auto binding = std::find_if(motion->bindings.begin(), motion->bindings.end(),
                    [&](const auto& candidate) { return candidate.slotId == effect.slotId &&
                        candidate.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE; });
                if (binding == motion->bindings.end())
                { m_Status = "World Object effect slot has no object binding: " + effect.slotId; return false; }
                const auto* resource = m_Document.Find_ObjectResource(binding->targetId);
                if (!resource)
                { m_Status = "World Object effect resource is unavailable: " + binding->targetId; return false; }
                const auto snapshot = m_EffectSnapshots.find(effect.resourceKind + ":" + effect.resourceId);
                if (effect.resourceKind != "V1_EFFECT" && snapshot == m_EffectSnapshots.end())
                { m_Status = "World Object effect was not prepared: " + effect.resourceId; return false; }
                const f32_t trigger = static_cast<f32_t>(sequence->EffectStartMs(effect));
                for (uint32_t emitter = 0; emitter < sequence->objectMotion.EmissionCount(); ++emitter)
                {
                    const f32_t birth = static_cast<f32_t>(sequence->objectMotion.EmissionDelayMs(emitter)) + trigger;
                    if (localMs < birth) continue;
                    const bool loop = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP;
                    const uint64_t last = loop ? static_cast<uint64_t>(std::floor((localMs - birth) / period)) : 0u;
                    const uint64_t first = loop ? static_cast<uint64_t>((std::max)(0.0,
                        std::floor((localMs - birth - effect.durationMs) / period) + 1.0)) : 0u;
                    if (last < first) continue;
                    if (last - first > 1024u)
                    { m_Status = "World Object Effect overlap exceeds 1024 occurrences; increase Motion Lifetime."; return false; }
                    for (uint64_t epoch = first; epoch <= last; ++epoch)
                    {
                        const f32_t eventLocal = birth + static_cast<f32_t>(epoch) * period;
                        const f32_t ageMs = localMs - eventLocal;
                        const f32_t birthMs = start + (eventLocal - trigger) / rate;
                        if (ageMs < 0.f || ageMs >= effect.durationMs || birthMs >= cutoffMs) continue;
                        for (const auto& anchor : anchors)
                        {
                            auto key = firstId + ":" + motion->instanceId + ":" + std::to_string(firstStartMs) + ":" +
                                effect.effectTrackId + ":" + std::to_string(epoch) + ":" + std::to_string(emitter) + ":" + std::to_string(anchor.entityId);
                            PLAYER_ANCHOR emissionAnchor;
                            const f32_t emissionStartMs = start + static_cast<f32_t>(epoch) * period / rate;
                            const auto emissionKey = motion->instanceId + ":" + std::to_string(emissionStartMs) + ":" + std::to_string(emitter);
                            if (!Get_EmissionAnchor(active, targets, emissionKey, birthMs, anchor, emissionAnchor)) return false;
                            const auto prepared = m_ObjectModels.find(resource->objectId);
                            if (prepared == m_ObjectModels.end())
                            { m_Status = "World Object Effect model was not prepared: " + resource->objectId; return false; }
                            const bool v1 = effect.resourceKind == "V1_EFFECT";
                            const auto document = !v1 ? nullptr : preview != m_EffectPreviews.end() ?
                                preview->second.document : CEffectCatalog::Find_Loaded(effect.resourceId);
                            if (v1 && !document)
                            { m_Status = "World Object V1 Effect document was not prepared: " + effect.resourceId; return false; }
                            // Deleting every authored row leaves a valid silent catalog source.
                            // Skip before loop/fit duration checks; omitting its key also retires
                            // any earlier live occurrence after a saved source replacement.
                            if (v1 && !document->bSourceContract && document->Elements.empty() &&
                                document->ModelCues.empty() && document->OwnerControls.empty() &&
                                document->RuntimeExtensions.Is_Empty()) continue;
                            const auto resolveDuration = [&](float& duration) {
                                if (preview == m_EffectPreviews.end())
                                    return CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(effect.resourceId, duration);
                                duration = preview->second.durationSeconds;
                                return std::isfinite(duration) && duration > 0.f;
                            };
                            float effectTimeScale = 1.f;
                            if (effect.fitEffectToDuration)
                            {
                                float sourceDuration = 0.f;
                                if (!resolveDuration(sourceDuration) ||
                                    !CWorldSequenceDocument::Try_EffectTimeScale(effect, sourceDuration, effectTimeScale))
                                { m_Status = "World Object Effect fit needs a finite prepared V1 duration: " + effect.resourceId; return false; }
                            }
                            double sourceCycleMs = 0.0;
                            uint64_t firstSourceCycle = 0u, lastSourceCycle = 0u;
                            bool nativeInfiniteLoop = false;
                            if (effect.loopEffectToDuration)
                            {
                                nativeInfiniteLoop = std::any_of(document->Elements.begin(), document->Elements.end(),
                                    [](const auto& element) { return element.SourceRecipe.bEnabled &&
                                        element.SourceRecipe.iEmitterLoopCount == 0u; });
                                if (!nativeInfiniteLoop)
                                {
                                    float sourceDuration = 0.f;
                                    if (!resolveDuration(sourceDuration) ||
                                        !std::isfinite(sourceDuration) || sourceDuration <= 0.f)
                                    { m_Status = "World Object Effect loop needs a finite prepared V1 duration: " + effect.resourceId; return false; }
                                    // A prepared duration includes particle/after-image tails. Repeat
                                    // emission at its authored end, retaining each still-living cycle.
                                    double emissionSeconds = 0.0;
                                    for (const auto& element : document->Elements)
                                    {
                                        if (!element.bVisible) continue;
                                        const auto& recipe = element.SourceRecipe;
                                        const auto& timing = element.Detail.Timing;
                                        const double duration = recipe.bEnabled && recipe.fEmitterDurationSeconds > 0.f ?
                                            static_cast<double>(recipe.fEmitterDurationSeconds) * recipe.iEmitterLoopCount :
                                            timing.fLifeTimeSeconds;
                                        emissionSeconds = (std::max)(emissionSeconds,
                                            timing.fStartDelaySeconds + (recipe.bEnabled ? recipe.fEmitterDelaySeconds : 0.f) + duration);
                                    }
                                    for (const auto& cue : document->ModelCues)
                                        if (cue.bVisible) emissionSeconds = (std::max)(emissionSeconds,
                                            static_cast<double>(cue.fStartDelaySeconds) + cue.fDurationSeconds);
                                    if (!std::isfinite(emissionSeconds) || emissionSeconds <= 0.0)
                                    { m_Status = "World Object Effect loop has no finite emission window: " + effect.resourceId; return false; }
                                    sourceCycleMs = (std::min)(emissionSeconds, static_cast<double>(sourceDuration)) * 1000.0;
                                    const double lastSourceEpoch = std::floor(ageMs / sourceCycleMs);
                                    const double firstSourceEpoch = (std::max)(0.0,
                                        std::floor((ageMs - sourceDuration * 1000.0) / sourceCycleMs) + 1.0);
                                    if (!std::isfinite(lastSourceEpoch) ||
                                        lastSourceEpoch >= static_cast<double>((std::numeric_limits<uint64_t>::max)()) ||
                                        lastSourceEpoch - firstSourceEpoch >= 1024.0)
                                    { m_Status = "World Object Effect source tails exceed the bounded occurrence range."; return false; }
                                    firstSourceCycle = static_cast<uint64_t>(firstSourceEpoch);
                                    lastSourceCycle = static_cast<uint64_t>(lastSourceEpoch);
                                }
                            }
                            const auto occurrenceKey = key;
                            for (uint64_t sourceCycle = firstSourceCycle; sourceCycle <= lastSourceCycle; ++sourceCycle)
                            {
                                const float sourceCycleStartMs = static_cast<float>(sourceCycle * sourceCycleMs);
                                key = occurrenceKey;
                                if (sourceCycleMs > 0.0) key += ":source-cycle:" + std::to_string(sourceCycle);
                                wanted.insert(key);
                                auto found = std::find_if(active.effects.begin(), active.effects.end(),
                                    [&](const auto& value) { return value.key == key; });
                                const float sourceSeconds = (std::max)(0.f, ageMs - sourceCycleStartMs) * .001f * effectTimeScale;
                                ACTIVE_INSTANCE placementState;
                                placementState.positionOffset = active.positionOffset;
                                placementState.placement = active.placement;
                                // This provider outlives this stack frame in PresentationService. All
                                // inputs are owned values or immutable prepared model/document handles.
                                const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER provider =
                                    [placementState, owner = *motion, sequence = *sequence, resource = *resource,
                                     effect, emissionAnchor, emitter, trigger, effectTimeScale, sourceCycleStartMs,
                                     model = prepared->second.model, document, v1]
                                    (float seconds, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& output, std::string& error) -> bool_t
                                {
                                    output.SourceAnchorWorlds.clear();
                                    if (!std::isfinite(seconds) || seconds < 0.f) return false;
                                    const float sampleMs = (std::min)(trigger + (effect.followObject ?
                                        sourceCycleStartMs + seconds * 1000.f / effectTimeScale : 0.f),
                                        static_cast<float>(sequence.durationMs));
                                    float4x4_t objectWorld;
                                    if (!Sample_ObjectWorld(placementState, owner, sequence, resource, effect.slotId,
                                        emissionAnchor, emitter, sampleMs, objectWorld, error,
                                        effect.inheritObjectRotation)) return false;
                                    matrix_t pivot = XMLoadFloat4x4(&objectWorld);
                                    // Preserve existing V2 metre sizing; V1 shares the Object's
                                    // authored owner scale so both doll variants attach proportionally.
                                    if (!v1) for (int axis = 0; axis < 3; ++axis)
                                        pivot.r[axis] = XMVectorSetW(XMVector3Normalize(pivot.r[axis]), 0.f);
                                    if (!effect.bone.empty())
                                    {
                                        float4x4_t bone;
                                        if (!Sample_ObjectEffectBone(model, sequence, effect.slotId, effect.bone, sampleMs, bone, error)) return false;
                                        pivot = XMLoadFloat4x4(&bone) * pivot;
                                    }
                                    const matrix_t local = XMMatrixScalingFromVector(XMLoadFloat3(&effect.scale)) *
                                        XMMatrixRotationRollPitchYaw(XMConvertToRadians(effect.rotationDegrees.x),
                                            XMConvertToRadians(effect.rotationDegrees.y), XMConvertToRadians(effect.rotationDegrees.z)) *
                                        XMMatrixTranslationFromVector(XMLoadFloat3(&effect.positionOffset));
                                    XMStoreFloat4x4(&output.RootWorld, local * pivot);
                                    return !document || Sample_ObjectEffectAttachments(*document, model, sequence, effect.slotId,
                                        sampleMs, output.RootWorld, output.SourceAnchorWorlds, error);
                                };
                                EFFECT_FIXED_STEP_TRANSFORM_SAMPLE frame;
                                if (!provider(sourceSeconds, frame, m_Status)) return false;
                                // Retain raw birth/history transforms. WORLD particles must receive
                                // the current presentation transform after evaluating the whole frame.
                                std::optional<float4x4_t> presentationPost;
                                const auto& effectPostTransform = targets.objectEffectPostTransform ?
                                    targets.objectEffectPostTransform : targets.objectWorldPostTransform;
                                if (effectPostTransform)
                                {
                                    float4x4_t post;
                                    const float currentSourceMs = (std::min)(trigger + ageMs,
                                        static_cast<float>(sequence->durationMs));
                                    if (!Sample_ObjectPresentationPostTransform(effectPostTransform,
                                        motion->instanceId, currentSourceMs, post, m_Status)) return false;
                                    if (!XMMatrixIsIdentity(XMLoadFloat4x4(&post))) presentationPost = post;
                                }
                                // V2 consumes a current pivot, after its normal metre-size conversion.
                                if (!v1 && presentationPost)
                                    XMStoreFloat4x4(&frame.RootWorld, XMLoadFloat4x4(&frame.RootWorld) * XMLoadFloat4x4(&*presentationPost));
                                if (found == active.effects.end())
                                {
                                    size_t total = active.effects.size();
                                    for (const auto& value : m_Active) if (&value != &active) total += value.effects.size();
                                    if (total >= 1024u)
                                    { m_Status = "World Object Effect occurrence budget reached (1024)."; return false; }
                                    if (v1)
                                    {
                                        EFFECT_LEVEL_PLACEMENT_SPAWN_DESC spawn;
                                        spawn.iLevelIndex = targets.levelIndex;
                                        // Handle identity is process-local; no pointer is serialized.
                                        spawn.strPlacementId = "world-object:" + std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ":" + key;
                                        spawn.strEffectAssetId = effect.resourceId;
                                        spawn.RootWorld = frame.RootWorld;
                                        spawn.fInitialSampleTimeSeconds = sourceSeconds;
                                        // Finite sources keep their prepared document and repeat above. Native
                                        // EmitterLoops=0 sources use the shared bounded emission policy.
                                        spawn.fSourceLoopEndSeconds = nativeInfiniteLoop ? effect.durationMs * .001f : 0.f;
                                        spawn.bExternallySampled = true;
                                        spawn.bExternalModelCueAnchors = !document->ModelCues.empty();
                                        spawn.pAuthoringPreview = m_EffectSelection && selectedTrack ?
                                            m_EffectSelection->target : preview != m_EffectPreviews.end() ?
                                                preview->second.target : nullptr;
                                        EFFECT_WORLD_ROOT_HANDLE handle;
                                        if (!CEffectPresentationService::Spawn_LevelPlacement(spawn, handle, m_Status)) return false;
                                        /* The normal runtime keeps new requests pending until MainApp finishes
                                           Object Manager update. MapTool runs after that seam and seeks this
                                           exact frame, so commit only this editor-owned world root now. */
                                        if (targets.bCommitWorldRootEffectsAfterSpawn)
                                            CEffectPresentationService::Commit_PendingWorldRootSpawns({handle});
                                        active.effects.push_back({key, 0u, handle.iValue, document});
                                        active.effects.back().effectTrackId = effect.effectTrackId;
                                    }
                                    else
                                    {
                                        EFFECT_V2_GROUP_PLAYBACK_DESC playback;
                                        playback.PivotWorld = frame.RootWorld;
                                        playback.bExternalClock = true;
                                        playback.bProductOwned = true;
                                        // Sample_Group receives authored seconds; Motion speed is already applied.
                                        playback.fDurationSeconds = -1.f;
                                        const uint32_t handle = effect.resourceKind == "GROUP" ?
                                            CEffectV2Runtime::Play_Group(*snapshot->second->Find_Group(effect.resourceId), snapshot->second,
                                                playback, targets.device, targets.context) :
                                            CEffectV2Runtime::Play_Leaf(effect.resourceId, snapshot->second, playback, targets.device, targets.context);
                                        if (!handle)
                                        { m_Status = "World Object effect play failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                        active.effects.push_back({key, handle});
                                    }
                                    found = active.effects.end() - 1;
                                }
                                if (v1)
                                {
                                    (void)CEffectPresentationService::Set_WorldRootInspectionVisible(
                                        {found->v1Handle}, selectedTrack);
                                    const bool placementEdited = found->sampledPlacement != active.placement ||
                                        found->sampledPositionOffset.x != active.positionOffset.x ||
                                        found->sampledPositionOffset.y != active.positionOffset.y ||
                                        found->sampledPositionOffset.z != active.positionOffset.z;
                                    if (!CEffectPresentationService::Update_WorldRoot({found->v1Handle}, frame.RootWorld) ||
                                        !CEffectPresentationService::Seek_WorldRoot({found->v1Handle}, sourceSeconds, provider,
                                            placementEdited, 0.f, nullptr, nullptr, presentationPost ? &*presentationPost : nullptr))
                                    { m_Status = "World Object V1 effect sample failed: " + effect.resourceId + " / " + CEffectPresentationService::Get_Status(); return false; }
                                    found->sampledPlacement = active.placement;
                                    found->sampledPositionOffset = active.positionOffset;
                                }
                                else
                                {
                                    CEffectV2Runtime::Set_GroupPivot(found->handle, frame.RootWorld);
                                    if (!CEffectV2Runtime::Sample_Group(found->handle, ageMs * .001f, true, targets.device, targets.context))
                                    { m_Status = "World Object effect sample failed: " + effect.resourceId + " / " + CEffectV2Runtime::Last_Error(); return false; }
                                }
                            }
                        }
                    }
                }
            }
            if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
            start += period / rate;
            motion = m_Document.Find_Instance(motion->nextMotionId);
        }
        return true;
    };
    const f32_t end = active.durationMs ? static_cast<f32_t>(active.durationMs) : (std::numeric_limits<f32_t>::max)();
    const bool changed = !active.motionInstanceId.empty() && active.elapsedMs >= active.motionStartMs;
    if (!sampleChain(active.instanceId, 0.f, changed ? (std::min)(end, active.motionStartMs) : end) ||
        (changed && !sampleChain(active.motionInstanceId, active.motionStartMs, end))) return false;
    for (size_t index = 0; index < active.effects.size();)
    {
        if (wanted.contains(active.effects[index].key)) { ++index; continue; }
        if (active.effects[index].handle) CEffectV2Runtime::Stop_Group(active.effects[index].handle);
        if (active.effects[index].v1Handle) CEffectPresentationService::Stop_WorldRoot({active.effects[index].v1Handle});
        active.effects.erase(active.effects.begin() + static_cast<ptrdiff_t>(index));
    }
    return true;
}

bool_t Client::CWorldSequencePlayer::Try_GetSequencePivot(const std::string& instanceId, float4x4_t& out,
 const uint32_t emissionIndex, const std::string& bone, const bool_t boneRotation,
    const std::string& effectTrackId) const
{
 if (Try_GetObjectPivot(instanceId, out, emissionIndex, bone, boneRotation, effectTrackId)) return true;
 if (!bone.empty() || !effectTrackId.empty()) return false;
 // Placed map/deploy aliases have no emission rows; only row 0 can name them.
 if (0u != emissionIndex) return false;
 const auto* instance = Get_Document().Find_Instance(instanceId);
 if (!instance) return false;
 const WORLD_SEQUENCE_BINDING* binding = nullptr;
 // Preserve the existing map-alias choice; a Deploy-only sequence resolves its animated prop.
 for (const auto kind : {WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT, WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT})
 {
  for (const auto& candidate : instance->bindings)
  {
   if (candidate.targetKind != kind) continue;
   if (candidate.slotId == "object") { binding = &candidate; break; }
   if (binding) return false;
   binding = &candidate;
  }
  if (binding) break;
 }
 if (!binding) return false;
 uint64_t placementId = 0;
 if (!CWorldSequencePlayer::Try_ParseTargetId(*binding, placementId)) return false;
 if (binding->targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
 {
  const auto active = std::find_if(m_Active.begin(), m_Active.end(),
   [&](const auto& value) { return value.instanceId == instanceId; });
  if (active == m_Active.end()) return false;
  const auto sampled = active->sampledDeployPivots.find(placementId);
  if (sampled == active->sampledDeployPivots.end()) return false;
  out = sampled->second;
  return true;
 }
 MAP_PLACEMENT_RECORD record;
 if (!Try_GetSampledPlacementRecord(instanceId, placementId, record)) return false;
 XMStoreFloat4x4(&out, XMMatrixScaling(record.signedScale.x, record.signedScale.y, record.signedScale.z) *
  XMMatrixRotationQuaternion(XMLoadFloat4(&record.rotationQuaternion)) *
  XMMatrixTranslation(record.position.x, record.position.y, record.position.z));
 return true;
}

void CWorldSequencePlayer::Set_Paused(const bool_t paused)
{
    m_bPaused = paused;
    auto& audio = CGameInstance::Get();
    for (const auto& active : m_Active)
        for (const auto& sound : active.sounds) audio.Pause_SoundCue(sound.handle, paused);
    for (const auto& sound : m_RetiredSounds) audio.Pause_SoundCue(sound.handle, paused);
}

void CWorldSequencePlayer::Finish_Sounds()
{
    auto& audio = CGameInstance::Get();
    for (auto& active : m_Active)
    {
        for (const auto& sound : active.sounds) audio.Stop_SoundCue(sound.handle);
        active.sounds.clear();
        active.soundPlaybackFinished = true;
        active.seekSounds = false;
    }
    Stop_RetiredSounds();
}

void CWorldSequencePlayer::Stop_RetiredSounds(const std::string& ownerId)
{
    for (auto at = m_RetiredSounds.begin(); at != m_RetiredSounds.end();)
    {
        if (!ownerId.empty() && at->ownerId != ownerId) { ++at; continue; }
        CGameInstance::Get().Stop_SoundCue(at->handle);
        at = m_RetiredSounds.erase(at);
    }
}

void CWorldSequencePlayer::Retire_Sounds(ACTIVE_INSTANCE& active)
{
    // Only handles survive natural visual completion. Camera, actor and control
    // clocks retain their authored duration and explicit Stop still owns cleanup.
    const f32_t elapsedMs = m_SourceToSoundTime ? m_ExternalSoundElapsedMs : active.elapsedMs;
    for (const auto& sound : active.sounds)
        if (!sound.loopToDuration && sound.handle && sound.endElapsedMs > elapsedMs)
            m_RetiredSounds.push_back({active.instanceId, sound.handle, sound.endElapsedMs - elapsedMs});
        else CGameInstance::Get().Stop_SoundCue(sound.handle);
    active.sounds.clear();
}

void CWorldSequencePlayer::Update_SoundTails(const f32_t timeDelta)
{
    if (m_bPaused || !std::isfinite(timeDelta) || timeDelta < 0.f) return;
    for (auto at = m_RetiredSounds.begin(); at != m_RetiredSounds.end();)
    {
        if (m_SoundAudience && !m_SoundAudience(at->ownerId))
        { CGameInstance::Get().Stop_SoundCue(at->handle); at = m_RetiredSounds.erase(at); continue; }
        at->remainingMs -= timeDelta * 1000.f;
        if (at->remainingMs > 0.f && CGameInstance::Get().Is_SoundCueActive(at->handle)) { ++at; continue; }
        CGameInstance::Get().Stop_SoundCue(at->handle);
        at = m_RetiredSounds.erase(at);
    }
}

void CWorldSequencePlayer::Retire_InstanceSoundTails(const std::string& instanceId)
{
    for (auto& active : m_Active)
        if (active.instanceId == instanceId)
        {
            Retire_Sounds(active);
            active.soundPlaybackFinished = true;
        }
}

void CWorldSequencePlayer::Apply_Sounds(ACTIVE_INSTANCE& active)
{
    if (m_SoundAudience && !m_SoundAudience(active.instanceId))
    {
        for (const auto& sound : active.sounds) CGameInstance::Get().Stop_SoundCue(sound.handle);
        active.sounds.clear();
        active.seekSounds = false;
        return;
    }
    if (active.soundPlaybackFinished && !active.seekSounds) return;
    active.soundPlaybackFinished = false;
    std::unordered_set<std::string> wanted;
    auto& audio = CGameInstance::Get();
    const f32_t soundElapsedMs = m_SourceToSoundTime ? m_ExternalSoundElapsedMs : active.elapsedMs;
    const auto sampleChain = [&](const std::string& firstId, const f32_t firstStartMs, const f32_t cutoffMs)
    {
        const auto* motion = m_Document.Find_Instance(firstId);
        f32_t start = firstStartMs;
        for (uint32_t depth = 0u; motion && depth <= 32u; ++depth)
        {
            const auto* sequence = m_Document.Find_Template(motion->templateId);
            if (!sequence) break;
            const f32_t rate = motion->playbackSpeed * active.playbackSpeed;
            if (!std::isfinite(rate) || rate <= 0.f) break;
            start += motion->startDelayMs;
            const f32_t localMs = (active.elapsedMs - start) * rate;
            if (localMs < 0.f || start >= cutoffMs) break;
            const f32_t period = static_cast<f32_t>(motion->CycleSpanMs(*sequence));
            const bool loop = motion->motionEnd == WORLD_SEQUENCE_MOTION_END::LOOP;
            for (const auto& row : sequence->soundTracks)
            {
                if (row.loopToDuration && (active.elapsedMs >= cutoffMs || (!loop && localMs >= period))) continue;
                if (localMs < row.startMs) continue;
                const uint64_t last = loop ? static_cast<uint64_t>(std::floor((localMs - row.startMs) / period)) : 0u;
                uint64_t first = loop ? static_cast<uint64_t>((std::max)(0.0,
                    std::floor((localMs - row.startMs - row.durationMs) / period) + 1.0)) : 0u;
                const auto soundBirthAt = [&](uint64_t epoch)
                {
                    const f32_t birthMs = start + (row.startMs + static_cast<f32_t>(epoch) * period) / rate;
                    return m_SourceToSoundTime ? m_SourceToSoundTime(birthMs) : birthMs;
                };
                if (loop && m_SourceToSoundTime)
                {
                    // Find the first still-audible occurrence in the sound clock.
                    // Source-age bounds would discard tails during visual speedup
                    // or retain expired voices during visual slow motion.
                    uint64_t low = 0u, high = last + 1u;
                    while (low < high)
                    {
                        const uint64_t middle = low + (high - low) / 2u;
                        if ((soundElapsedMs - soundBirthAt(middle)) * rate >= row.durationMs) low = middle + 1u;
                        else high = middle;
                    }
                    first = low;
                }
                if (last < first || last - first > 1024u) continue;
                for (uint64_t epoch = first; epoch <= last; ++epoch)
                {
                    const f32_t eventMs = row.startMs + static_cast<f32_t>(epoch) * period;
                    const f32_t birthMs = start + eventMs / rate;
                    const f32_t soundBirthMs = soundBirthAt(epoch);
                    const f32_t ageMs = m_SourceToSoundTime ?
                        (soundElapsedMs - soundBirthMs) * rate : localMs - eventMs;
                    if (!std::isfinite(ageMs) || ageMs < 0.f || ageMs >= row.durationMs || birthMs >= cutoffMs) continue;
                    const auto key = firstId + ":" + motion->instanceId + ":" + std::to_string(firstStartMs) +
                        ":" + row.soundTrackId + ":" + std::to_string(epoch);
                    wanted.insert(key);
                    auto found = std::find_if(active.sounds.begin(), active.sounds.end(),
                        [&](const auto& sound) { return sound.key == key; });
                    if (found != active.sounds.end() && active.seekSounds)
                    {
                        audio.Stop_SoundCue(found->handle);
                        active.sounds.erase(found);
                        found = active.sounds.end();
                    }
                    if (found == active.sounds.end())
                    {
                        const auto path = CRuntimeAssetRoot::Resolve(row.assetId);
                        uint32_t mediaDurationMs = 0u;
                        const bool loopReady = !row.loopToDuration ||
                            audio.Get_SoundDurationMs(path.wstring(), mediaDurationMs);
                        const auto sampleMs = row.sourceStartMs + static_cast<uint32_t>(ageMs);
                        const auto cycle = row.loopToDuration && mediaDurationMs ? sampleMs / mediaDurationMs : 0u;
                        const auto offsetMs = row.loopToDuration && mediaDurationMs ? sampleMs % mediaDurationMs : sampleMs;
                        const auto handle = loopReady ? audio.Play_SoundCue(path.wstring(), row.volume,
                            offsetMs, m_bPaused, rate * m_ExternalSoundClockRate,
                            row.loopToDuration ? 0u : row.sourceStartMs + row.durationMs) : 0u;
                        // A missing cue is isolated and remembered, so a broken asset
                        // cannot retrigger file I/O or invalidate an otherwise valid scene.
                        active.sounds.push_back({key, handle, soundBirthMs + row.durationMs / rate,
                            mediaDurationMs, cycle, row.loopToDuration});
                        if (!handle) m_Status = "World sequence sound unavailable: " + row.assetId;
                    }
                    else
                    {
                        const auto sampleMs = row.sourceStartMs + static_cast<uint32_t>(ageMs);
                        if (row.loopToDuration && found->mediaDurationMs &&
                            sampleMs / found->mediaDurationMs != found->mediaCycle)
                        {
                            // Stop the previous cycle before starting the current one.
                            // Clock modulo handles frame skips, seek and playback rate
                            // without accumulating drift or overlapping loop voices.
                            audio.Stop_SoundCue(found->handle);
                            found->mediaCycle = sampleMs / found->mediaDurationMs;
                            const auto path = CRuntimeAssetRoot::Resolve(row.assetId);
                            found->handle = audio.Play_SoundCue(path.wstring(), row.volume,
                                sampleMs % found->mediaDurationMs, m_bPaused, rate * m_ExternalSoundClockRate);
                        }
                        else
                        {
                            audio.Set_SoundCuePlaybackRate(found->handle, rate * m_ExternalSoundClockRate);
                            if (m_HasExternalSoundClock && found->handle)
                            {
                                const auto offsetMs = row.loopToDuration && found->mediaDurationMs ?
                                    sampleMs % found->mediaDurationMs : sampleMs;
                                if (!audio.Synchronize_SoundCue(found->handle, offsetMs))
                                {
                                    // A long render/load stall can finish the mixer channel
                                    // while the Movie clock is still inside this sound box.
                                    audio.Stop_SoundCue(found->handle);
                                    const auto path = CRuntimeAssetRoot::Resolve(row.assetId);
                                    found->handle = audio.Play_SoundCue(path.wstring(), row.volume,
                                        offsetMs, m_bPaused, rate * m_ExternalSoundClockRate,
                                        row.loopToDuration ? 0u : row.sourceStartMs + row.durationMs);
                                }
                            }
                        }
                    }
                }
            }
            if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
            start += period / rate;
            motion = m_Document.Find_Instance(motion->nextMotionId);
        }
    };
    const f32_t end = active.durationMs ? static_cast<f32_t>(active.durationMs) : (std::numeric_limits<f32_t>::max)();
    const bool changed = !active.motionInstanceId.empty() && active.elapsedMs >= active.motionStartMs;
    sampleChain(active.instanceId, 0.f, changed ? (std::min)(end, active.motionStartMs) : end);
    if (changed) sampleChain(active.motionInstanceId, active.motionStartMs, end);
    for (auto at = active.sounds.begin(); at != active.sounds.end();)
    {
        if (wanted.contains(at->key)) { ++at; continue; }
        audio.Stop_SoundCue(at->handle);
        at = active.sounds.erase(at);
    }
    active.seekSounds = false;
}

void Client::CWorldSequencePlayer::Collect_VisibleObjects(
    std::vector<std::shared_ptr<CWorldSequenceObject>>& out) const
{
    const auto collect = [&](const auto& instances) {
        for (const auto& instance : instances)
            for (const auto& entry : instance.objects)
                if (entry.object && entry.object->Is_Visible()) out.push_back(entry.object);
    };
    collect(m_Active);
    collect(m_Held);
}

void Client::CWorldSequencePlayer::Collect_ObjectInspectionSamples(
    std::vector<OBJECT_INSPECTION_SAMPLE>& out) const
{
    const auto collect = [&](const auto& instances) {
        for (const auto& active : instances)
        {
            const auto* instance = m_Document.Find_Instance(active.instanceId);
            if (!instance) continue;
            for (const auto& entry : active.objects)
            {
                if (!entry.object) continue;
                const auto binding = std::find_if(instance->bindings.begin(), instance->bindings.end(),
                    [&entry](const auto& value) {
                        return value.slotId == entry.slotId &&
                            value.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE;
                    });
                if (binding == instance->bindings.end()) continue;
                const auto* resource = m_Document.Find_ObjectResource(binding->targetId);
                if (!resource) continue;
                out.push_back({active.instanceId, entry.slotId, resource->objectId,
                    resource->modelAssetId, entry.emissionIndex, entry.object});
            }
        }
    };
    collect(m_Active);
    collect(m_Held);
}
```

## G03. 등록·적용·검증

기존 제품 H/CPP 수정만 있으므로 `.vcxproj`와 `.vcxproj.filters`의 기존 항목을 유지한다. 파일은 원본 UTF-8과 CRLF를 보존한다. WorldSequencePlayer 및 service 두 쌍은 BOM 없음이고 Level 두 파일은 기존 BOM을 유지한다. 후보는 `out/MarioWorldPrewarm20261004/candidate/Client`에 먼저 저장하고 이 전문과 byte 일치를 확인한 다음 제품에 같은 함수·선언 patch만 적용한다. 다른 담당자의 같은 파일 변경을 파일 전체 복사로 덮어쓰지 않는다.

Level 소비자는 매 update에 step을 한 번 호출하고 true+ready=false에서는 WORLD 준비 index를 유지하며 true+ready=true에서만 다음 항목으로 진행한다. 모델·Effect·WORLD revision과 선택 request identity 검사, 취소, Server READY 이전 재생 차단을 그대로 사용한다. 실제 대상 수량은 선택된 mechanics의 근거 있는 emission/concurrency 예산을 사용하고 NEXT와 기존 객체에 적용하는 motion을 새 spawn으로 중복 합산하지 않는다.

확인할 계약은 호출당신규최대1, 증분 capacity와 ready, 이미 충분한 pool의 무생성 완료,128/1024 전체예산 사전 거부, 생성 실패의 해당단계 rollback과 기존 pool 보존이다. 독립 CPU fixture는 실제 변경 함수 전문을 추출해 호출 흐름을 검증하고 GameObject/Layer 생성을 계수하는 대역을 사용한다. 이 검사는 실제 GPU resource·shader·CModel clone 성공을 대신하지 않는다. 해당 native probe와 필요한 제품 TU 최소 컴파일은 담당자와 조율해 수행하고, 실행한 증거만 RESULT에 기록한다. Client/UI 실행은 하지 않는다.

```powershell
git diff --check
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj' -Raw) | Out-Null
[xml](Get-Content -LiteralPath 'Client/Default/Client.vcxproj.filters' -Raw) | Out-Null
```

## G04. 검증된 WORLD subset의 owner와 무효화

`PREPARED_PLAYBACK_SUBSET`은 root stable ID별 불변 `CWorldSequenceDocument`와 level·device·context·catalog identity를 보관한다. `Prepare_PlaybackSubset`은 실제 발생할 object-only WORLD root를 준비 장벽에서 Build+Validate하고 admission된 snapshot을 owner에 저장한다. placement/deploy 및 live anchor 문서는 기존 admission을 유지하며 이 cache에 넣지 않는다. `Set_PlaybackSubset`은 동일 owner·identity·area·revision 조건을 만족하면 snapshot 한 번을 stage한 뒤 기존 재생 상태를 정리하고 commit한다. miss는 기존 Build_PlaybackSubset+Set_Document 경로다. 모든 document 교체와 clear가 지나는 `Clear_PreparedObjects`에서 cache도 지워 같은 revision으로 다시 읽은 문서의 오래된 검증 결과를 재사용하지 않는다. source와 destination이 같은 경우에도 복사를 먼저 끝낸다.

## G05. 선택 패턴에서 clone 목표 수량 산정과 Level 연결

`KoukuSaydonPresentationAssetService`는 Encounter의 실제 spawn occurrence 반복을 보존하며 typed child closure와 그룹을 해석한다. 개별 pattern은 가능한 분기별 최대를 예약하고 Complete Play bundle은 멤버별 요구를 보수적으로 합산한다. 기존 world resource 집합은 모델 준비 목록으로 유지한다. clone 예약은 인형·공·칼날·갈고리와 기존 card/joker 대상만 계산하고 NEXT와 기존 clone에 적용하는 motion을 새 생성으로 중복 합산하지 않는다. objectId별128·owner전체1024를 준비 전에 검사한다.

현재 게시 데이터의 목표는 Mario1 32개, Mario2~4 각50개, 전체 관문 자원 준비88개이다. 전체 관문88은 갈고리33·칼날40·인형4·공4·카드6·조커1이다. 이는 입력 데이터로부터 산정한 보수적 capacity이며 실제 동시 화면 표시 개수나 측정된 시간 개선을 의미하지 않는다. 전체114개 pattern과9개 bundle의 범위 검사는 out의 pool-reservations.json에 기록한다. 선택 데이터가 바뀌면 동일 계산을 다시 수행한다.

`CLevel_KakulSaydonArena`는 모델 준비 다음에 clone 준비 단계를 두고 update마다 step을 한 번 호출한다. pending일 때 현재 항목을 유지하고 ready일 때만 다음 항목으로 이동한다. 실제 네 종류 발생 root만 subset을 먼저 준비하며 birth에서는 Set_PlaybackSubset을 소비한다. 취소·revision·request identity·Server READY 경계는 기존 흐름을 유지한다. 새로운 Level이나 두 번째 모델 런타임은 만들지 않는다.

### C:/Users/tnest/Desktop/LostArk/Client/Public/KoukuSaydonPresentationAssetService.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"
#include "PlayerHandGripTransform.h"
#include "KoukuSaydonCompositionDocument.h"

#include "GameplayDataRevision.h"
#include <memory>
#include <cstdint>
#include <string>
#include <string_view>

NS_BEGIN(Client)

struct KOUKU_SAYDON_ACTION_PRESENTATION final
{
	std::string strActionId;
	std::string strOccurrenceId;
	std::string strClip;
	std::uint32_t iStartOffsetMs = 0u;
	std::uint32_t iSourceStartMs = 0u;
	std::uint32_t iSourceEndMs = 0u;
	std::uint32_t iPlayMs = 0u;
	f32_t fPlayRate = 1.f;
	f32_t fAnimationRootVerticalScale = 1.f;
	bool_t bUnblendedBoneContact = false;
	bool_t bLoopToWindow = false;
	bool_t bHoldAtWindowEnd = false;
	std::uint32_t iBlendInMs = 0u;
	std::string strBlendFromClip;
	f32_t fBlendFromSourceMs = 0.f;
	std::vector<KOUKU_SAYDON_ANIMATION_BLEND_WINDOW> AnimationBlendWindows;
};

/* Loads the embedded-body presentation of every KoukuSaydon arena boss
(BOSS_KAKULSAYDON_* rows whose client contract is boss.kakulsaydon.*), with an
optional weapon following the body clip or its loaded rest pose. Product animation bindings
are admitted for the Gate 1 Kouku only. It deliberately does not share
Valtan's armour prototype or joined presentation graph. */
/* The MN_RPCZ_00-1 madness doll a player wears while PLAYER_MADNESS_FORM::CLOWN.
Shared with the clown CHARACTER_SPEC so the spec and the admission agree. */
inline constexpr const wchar_t* KOUKU_CLOWN_BODY_PROTOTYPE_TAG =
	L"Prototype_Component_Model_KoukuSaydonClown";
/* The toy hammer that same doll swings: the one the Mario-1 monster carries
(WP_MN_RHKP_07, lifted out of REUP.wmodel as a static cook in the monster's
right-hand frame). Socketed to the same Biped bone on the doll, so the grip
is the monster's. Shared with the clown CHARACTER_SPEC. */
inline constexpr const wchar_t* KOUKU_CLOWN_HAMMER_PROTOTYPE_TAG =
	L"Prototype_Component_Model_KoukuSaydonClownHammer";
inline constexpr const char* KOUKU_CLOWN_HAMMER_SOCKET_BONE = "bip001-r-hand";

inline constexpr const wchar_t* KOUKU_MAZE_HAMMER_PROTOTYPE_TAG =
	L"Prototype_Component_Model_KoukuCardMazeHammer";
inline constexpr const char* KOUKU_MAZE_HAMMER_SOCKET_BONE = "bip001-r-hand";

// Immutable selection closure; every row, including delayed tails and spawned
// child patterns, must be ready before an audition command starts its clock.
struct KOUKU_SAYDON_PLAY_RESOURCES final
{
    std::vector<std::string> PatternIds, V1EffectIds, WorldInstanceIds, BossArchetypeIds;
    std::vector<std::pair<std::string, std::string>> V2Effects;
    // Each group reserves one pattern closure, or the concurrent members of a
    // Bundle. Repeated spawn occurrences are retained; motion changes are absent.
    std::vector<std::vector<std::string>> WorldSpawnGroups;
};

struct KOUKU_SAYDON_DRAFT_PRODUCT final
{
    std::string PresentationJson, EncounterJson;
    std::uint32_t iSourceRevision = 0u;
    LostArk::Shared::GameplayDataRevision RowsRevision{};
};

class CKoukuSaydonPresentationAssetService final
{
public:
    static std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> Prepare_DraftProduct(
        const std::string& presentationJson, const std::string& encounterJson, const std::string& gameplayRows,
        std::uint32_t sourceRevision, std::string& status);
    static bool Stage_DraftProduct(std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft, std::string& status);
    static bool Authorize_DraftProduct(const LostArk::Shared::GameplayDataRevision& rowsRevision,
        std::uint32_t runEpoch, std::string& status);
    // One model per call; the exact canonical/draft cache commits only on Server admission.
    static bool Prepare_ProductBindings(std::uint32_t levelIndex,
        std::uint32_t sourceRevision, bool& ready, std::string& status,
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft = {});
    static bool Admit_RunProduct(std::uint32_t levelIndex, std::uint32_t sourceRevision,
        const LostArk::Shared::GameplayDataRevision& rowsRevision, std::uint32_t runEpoch, std::string& status);
    static bool Matches_AdmittedRun(std::uint32_t sourceRevision,
        const LostArk::Shared::GameplayDataRevision& rowsRevision, std::uint32_t runEpoch);
    static std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> Get_AdmittedDraftProduct();
    static bool Collect_CompletePlayResources(const std::vector<std::string>& patternIds,
        const std::vector<std::string>& bundleIds, std::uint32_t sourceRevision,
        KOUKU_SAYDON_PLAY_RESOURCES& output, std::string& status,
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft = {});
	static void Begin_LevelLoad(std::uint32_t iLevelIndex);
	static HRESULT Ensure_MazeHammerPrototype(ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext, std::uint32_t iLevelIndex);
	/* Admits the Polymorph 4134 avatar body (MN_RPCZ_00-1) once per level
	and its socketed hammer, and verifies its own idle/run clips. S_FALSE when it
	is already ready; the failure reason lands in Get_Status(). */
	static HRESULT Ensure_ClownBodyPrototype(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::uint32_t iLevelIndex);
	/* True for a catalog row this service owns: the archetype prefix and the
	client presentation contract both name the KoukuSaydon family. */
	static bool_t Is_ArenaBossArchetype(std::string_view archetypeId);
	static HRESULT Ensure_Prototypes(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext,
		std::uint32_t iLevelIndex,
		std::string_view archetypeId);
	static std::wstring Get_ModelPrototypeTag(std::string_view archetypeId);
	/* Empty when the catalog row declares no weapon. */
	static std::wstring Get_WeaponModelPrototypeTag(std::string_view archetypeId);
	static const char_t* Get_WeaponSocketBone();
	static const wchar_t* Get_GameObjectPrototypeTag();
	/* Product action bindings admitted for one arena boss body. Every arena
	boss loads the same binding document and keeps only the rows whose clip
	its own body owns, so a Saydon pattern resolves on a Saydon body only. */
	static bool_t Try_Resolve_Action(
		std::string_view archetypeId,
		std::string_view actionId,
		KOUKU_SAYDON_ACTION_PRESENTATION& outPresentation,
		std::uint32_t expectedSourceRevision = 0u);
	static bool_t Try_Resolve_AttachmentGrip(std::string_view archetypeId,
		std::string_view patternId, LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
		PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset, std::uint32_t expectedSourceRevision = 0u);
	static bool_t Reload_ProductBindings(std::uint32_t levelIndex,
		std::uint32_t expectedSourceRevision, std::string& status);
	static const std::string& Get_Status();
};

NS_END
```

### C:/Users/tnest/Desktop/LostArk/Client/Private/KoukuSaydonPresentationAssetService.cpp 전체 코드

```cpp
#include "KoukuSaydonPresentationAssetService.h"
#include "KoukuSaydonAnimationBlend.h"
#include "KoukuSaydonCompositionDocument.h"

#include "ActorCatalog.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "Model.h"
#include "Npc.h"
#include "NetworkManager.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>
#include <functional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
	using namespace Client;

	constexpr std::string_view KOUKU_ARCHETYPE =
		"BOSS_KAKULSAYDON_G1_KOUKU";
	constexpr std::string_view KOUKU_PRESENTATION =
		"boss.kakulsaydon.g1.kouku.client.v1";
	/* Every arena boss archetype shares this prefix and this client contract
	family; the Gate 1 Kouku keeps its exact model prototype tag. */
	constexpr std::string_view KOUKU_FAMILY_ARCHETYPE_PREFIX =
		"BOSS_KAKULSAYDON_";
	constexpr std::string_view KOUKU_FAMILY_PRESENTATION_PREFIX =
		"boss.kakulsaydon.";
	constexpr std::string_view KOUKU_FAMILY_PRESENTATION_SUFFIX =
		".client.v1";
	constexpr const wchar_t* KOUKU_MODEL_PROTOTYPE_PREFIX =
		L"Prototype_Component_Model_KoukuSaydon_";
	constexpr const wchar_t* KOUKU_WEAPON_PROTOTYPE_SUFFIX = L"_Weapon";
	/* Both Saydon bodies (MN_RPCT_05/06) socket their held weapon here. */
	constexpr const char_t* KOUKU_WEAPON_SOCKET_BONE = "b_wp_1";
	constexpr std::string_view BINDING_SCHEMA =
		"lostark.kouku-saydon-pattern-bindings";
	constexpr std::uint32_t BINDING_VERSION = 1u;
	constexpr std::uintmax_t MAX_BINDING_BYTES = 8u * 1024u * 1024u;
	constexpr const wchar_t* KOUKU_OBJECT_PROTOTYPE =
		L"Prototype_GameObject_KoukuSaydonPresentation";

	std::mutex g_KoukuAssetMutex;
	std::unordered_map<std::uint32_t, std::unordered_set<std::string>>
		g_ReadyByLevel;
	/* archetypeId -> actionId -> admitted clip row. Keyed per body because the
	same Product document serves every arena boss and each body owns only the
	rows whose clip exists on its rig. */
	std::unordered_map<std::string,
		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION>>
		g_ActionPresentationsByArchetype;
	using ATTACHMENT_GRIPS = std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>;
	std::unordered_map<std::string, ATTACHMENT_GRIPS> g_AttachmentGripsByArchetype;
	std::unordered_map<std::string, std::uint32_t> g_BindingSourceRevisions;
    // The same immutable source backs every model-filtered canonical cache.
    // Admission compares bytes once, then keeps the prepared clip rows in place.
    std::unordered_map<std::string, std::shared_ptr<const std::string>> g_CanonicalBindingSources;
    std::shared_ptr<const std::string> g_LastCanonicalBindingSource;
    struct BINDING_PREPARATION
    {
        std::uint32_t levelIndex = 0u, sourceRevision = 0u;
        std::shared_ptr<const std::string> source;
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft;
        std::vector<std::string> archetypes;
        std::size_t nextArchetype = 0u;
        bool reusesActive = false;
        bool presentationValid = false;
        std::string presentationStatus;
        decltype(g_ActionPresentationsByArchetype) actions;
        decltype(g_AttachmentGripsByArchetype) grips;
        decltype(g_BindingSourceRevisions) revisions;
        decltype(g_CanonicalBindingSources) sources;
    };
    std::unique_ptr<BINDING_PREPARATION> g_BindingPreparation;
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> g_PreparedDraft, g_AdmittedDraft;
    std::uint32_t g_AdmittedRunEpoch = 0u, g_AdmittedSourceRevision = 0u, g_PreparedAuthorizedEpoch = 0u;
    bool g_RunAdmissionFailed = false;
	std::string g_Status = "KoukuSaydon presentation has not been loaded.";

	const DATA_JSON_VALUE* Required(
		const DATA_JSON_VALUE& object,
		const char* name,
		const DATA_JSON_TYPE type)
	{
		const DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool Has_ExactProperties(
		const DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		return std::all_of(names.begin(), names.end(),
			[&object](const std::string_view name)
			{
				return nullptr != object.Find(name);
			});
	}

	bool Has_BindingDocumentProperties(const DATA_JSON_VALUE& root)
	{
		const std::initializer_list<std::string_view> required =
			{"schema", "formatVersion", "bossArchetypeId", "sourceRevision", "bindings"};
		const std::initializer_list<std::string_view> optional =
			{"patterns", "lightResourceRevision", "folders", "bundles", "fearPresentations", "attachmentGrips", "targetedCombatVisuals"};
		if (!root.Is_Object()) return false;
		for (const auto key : required) if (!root.Find(key)) return false;
		for (const auto& [key, value] : root.Get_Object())
			if (std::find(required.begin(), required.end(), key) == required.end() &&
				std::find(optional.begin(), optional.end(), key) == optional.end()) return false;
		return true;
	}

	bool Is_StableToken(const std::string_view value)
	{
		if (value.empty() || value.size() > 255u || value == "." || value == "..")
			return false;
		return std::all_of(value.begin(), value.end(), [](const unsigned char c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
		});
	}

	bool Try_U32(
		const DATA_JSON_VALUE& value,
		const std::uint32_t maximum,
		std::uint32_t& out)
	{
		if (!value.Is_Number())
			return false;
		const double number = value.Get_Number();
		if (!std::isfinite(number) || number < 0.0 ||
			number > static_cast<double>(maximum) || std::floor(number) != number)
		{
			return false;
		}
		out = static_cast<std::uint32_t>(number);
		return true;
	}

	bool Has_Clip(const Engine::CModel& model, const std::string_view clip)
	{
		for (std::uint32_t index = 0u; index < model.Get_NumAnimations(); ++index)
		{
			const char* name = model.Get_AnimationName(index);
			if (nullptr != name && clip == name)
				return true;
		}
		return false;
	}

    bool Try_GetClipDurationMs(const Engine::CModel& model, const std::string_view clip,
        double& outDurationMs)
    {
        for (uint32_t index = 0; index < model.Get_NumAnimations(); ++index)
        {
            const char* name = model.Get_AnimationName(index);
            if (!name || clip != name) continue;
            float cursor = 0.f, duration = 0.f;
            const float ticksPerSecond = model.Get_AnimationTickPerSecond(index);
            if (!model.Get_AnimationProgress(index, cursor, duration) ||
                !std::isfinite(duration) || duration <= 0.f ||
                !std::isfinite(ticksPerSecond) || ticksPerSecond <= 0.f) return false;
            outDurationMs = double(duration) * 1000.0 / ticksPerSecond;
            return true;
        }
        return false;
    }

    bool Read_CanonicalBindingSource(std::shared_ptr<const std::string>& out, std::string& status)
    {
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path(L"Animation/Authored/KoukuSaydon") / L"KoukuSaydon.patternbindings.json");
        std::error_code error;
        const auto bytes = std::filesystem::file_size(path, error);
        if (path.empty() || error || !bytes || bytes > MAX_BINDING_BYTES)
        { status = "KoukuSaydon Product animation binding is missing or oversized."; return false; }
        const auto before = std::filesystem::last_write_time(path, error);
        if (error) { status = "KoukuSaydon Product animation binding timestamp is unavailable."; return false; }
        std::ifstream input(path, std::ios::binary);
        std::string text(static_cast<std::size_t>(bytes), '\0');
        input.read(text.data(), static_cast<std::streamsize>(bytes));
        if (!input || input.peek() != std::char_traits<char>::eof())
        { status = "KoukuSaydon Product animation binding changed during read."; return false; }
        const auto after = std::filesystem::last_write_time(path, error);
        if (error || before != after)
        { status = "KoukuSaydon Product animation binding changed during read."; return false; }
        // Equality, rather than timestamp/size alone, rejects edited contents even
        // when an authoring save preserves those two metadata fields.
        if (!g_LastCanonicalBindingSource || *g_LastCanonicalBindingSource != text)
            g_LastCanonicalBindingSource = std::make_shared<const std::string>(std::move(text));
        out = g_LastCanonicalBindingSource;
        return true;
    }

    std::vector<std::string> Ready_CanonicalBindingArchetypes(const std::uint32_t levelIndex)
    {
        std::vector<std::string> result;
        const auto ready = g_ReadyByLevel.find(levelIndex);
        if (ready != g_ReadyByLevel.end())
            for (const auto& archetype : ready->second)
                if (archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) result.push_back(archetype);
        std::sort(result.begin(), result.end());
        return result;
    }

    bool Have_PreparedCanonicalBindings(const std::uint32_t levelIndex,
        const std::uint32_t sourceRevision, const std::string& source)
    {
        const auto ready = g_ReadyByLevel.find(levelIndex);
        if (!sourceRevision || ready == g_ReadyByLevel.end()) return false;
        bool any = false;
        for (const auto& archetype : ready->second)
        {
            if (!archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) continue;
            const auto revision = g_BindingSourceRevisions.find(archetype);
            const auto bytes = g_CanonicalBindingSources.find(archetype);
            if (revision == g_BindingSourceRevisions.end() || revision->second != sourceRevision ||
                bytes == g_CanonicalBindingSources.end() || !bytes->second || *bytes->second != source ||
                !g_ActionPresentationsByArchetype.contains(archetype) || !g_AttachmentGripsByArchetype.contains(archetype))
                return false;
            any = true;
        }
        return any;
    }

	bool Load_PresentationBindings(
		const Engine::CModel& model,
		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION>& out,
		std::string& outStatus, std::uint32_t& outRevision, ATTACHMENT_GRIPS& outGrips,
		const std::uint32_t expectedRevision = 0u, const std::string* supplied = nullptr, const bool canonical = false,
        std::shared_ptr<const std::string>* outCanonicalSource = nullptr)
	{
        if (!supplied && !canonical && g_AdmittedDraft) supplied = &g_AdmittedDraft->PresentationJson;
        std::shared_ptr<const std::string> canonicalSource;
        std::string_view text;
        if (supplied)
        {
            if (supplied->empty() || supplied->size() > MAX_BINDING_BYTES)
            { outStatus = "Draft animation binding is empty or oversized."; return false; }
            text = *supplied;
        }
        else
        {
            if (!Read_CanonicalBindingSource(canonicalSource, outStatus)) return false;
            text = *canonicalSource;
        }

		DATA_JSON_VALUE root;
		std::string parseError;
		if (!CDataJson::Parse(text, root, parseError) || !Has_BindingDocumentProperties(root))
		{
			outStatus = "KoukuSaydon Product animation binding is malformed: " +
				parseError;
			return false;
		}

		const DATA_JSON_VALUE* schema = Required(
			root, "schema", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* version = Required(
			root, "formatVersion", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* archetype = Required(
			root, "bossArchetypeId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceRevision = Required(
			root, "sourceRevision", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* bindings = Required(
			root, "bindings", DATA_JSON_TYPE::ARRAY);
		// Animation and presentation readers consume the same Product document.
		// Its optional light pin must not reject all otherwise valid animations.
		const DATA_JSON_VALUE* lightRevision = root.Find("lightResourceRevision");
		std::uint32_t parsedVersion = 0u;
		std::uint32_t parsedRevision = 0u;
		std::uint32_t parsedLightRevision = 0u;
		if (nullptr == schema || schema->Get_String() != BINDING_SCHEMA ||
			nullptr == version ||
			!Try_U32(*version, BINDING_VERSION, parsedVersion) ||
			parsedVersion != BINDING_VERSION || nullptr == archetype ||
			archetype->Get_String() != KOUKU_ARCHETYPE ||
			nullptr == sourceRevision ||
			!Try_U32(*sourceRevision,
				(std::numeric_limits<std::uint32_t>::max)(), parsedRevision) ||
			0u == parsedRevision ||
			(expectedRevision != 0u && parsedRevision != expectedRevision) ||
			(nullptr != lightRevision &&
			 (!Try_U32(*lightRevision,
				(std::numeric_limits<std::uint32_t>::max)(), parsedLightRevision) ||
			  0u == parsedLightRevision)) || nullptr == bindings ||
			bindings->Get_Array().empty() ||
			bindings->Get_Array().size() > 16384u)
		{
			outStatus = "KoukuSaydon Product animation binding header is invalid.";
			return false;
		}

		std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> staged;
		std::size_t skipped = 0u;
		std::unordered_set<std::string> duplicates;
		for (const DATA_JSON_VALUE& value : bindings->Get_Array())
		{
            const auto hasAnimationFields = [&value]()
            {
                const std::initializer_list<std::string_view> required = {"actionId", "occurrenceId", "clip", "startOffsetMs", "sourceStartMs", "playMs", "playRate", "endPolicy"};
                const std::initializer_list<std::string_view> optional = {"unblendedBoneContact", "animationRootVerticalScale", "blendInMs", "blendFromClip", "blendFromSourceMs", "holdAtWindowEnd", "sourceEndMs", "animationBlendWindows"};
                if (!value.Is_Object()) return false;
                for (auto name : required) if (!value.Find(name)) return false;
                for (const auto& [key, field] : value.Get_Object())
                    if (std::find(required.begin(), required.end(), key) == required.end() &&
                        std::find(optional.begin(), optional.end(), key) == optional.end()) return false;
                return true;
            };
            if (!hasAnimationFields())
			{
				outStatus = "KoukuSaydon Product animation row has unexpected fields.";
				++skipped; continue;
			}
			const DATA_JSON_VALUE* action = Required(
				value, "actionId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* occurrence = Required(
				value, "occurrenceId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* clip = Required(
				value, "clip", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* startOffset = Required(
				value, "startOffsetMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* sourceStart = Required(
				value, "sourceStartMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* playMs = Required(
				value, "playMs", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* playRate = Required(
				value, "playRate", DATA_JSON_TYPE::NUMBER);
			const DATA_JSON_VALUE* endPolicy = Required(
				value, "endPolicy", DATA_JSON_TYPE::STRING);
			KOUKU_SAYDON_ACTION_PRESENTATION row;
			const DATA_JSON_VALUE* unblended = value.Find("unblendedBoneContact");
			const auto* verticalScale = value.Find("animationRootVerticalScale");
			const auto* holdAtEnd = value.Find("holdAtWindowEnd");
			const auto* sourceEnd = value.Find("sourceEndMs");
			if (nullptr == action || !Is_StableToken(action->Get_String()) ||
				nullptr == occurrence ||
				!Is_StableToken(occurrence->Get_String()) || nullptr == clip ||
				!Is_StableToken(clip->Get_String()) || nullptr == startOffset ||
				!Try_U32(*startOffset, 600000u, row.iStartOffsetMs) ||
				nullptr == sourceStart ||
				!Try_U32(*sourceStart, 600000u, row.iSourceStartMs) ||
				(sourceEnd && !Try_U32(*sourceEnd, 600000u, row.iSourceEndMs)) || nullptr == playMs ||
				!Try_U32(*playMs, 600000u, row.iPlayMs) || 0u == row.iPlayMs ||
				nullptr == playRate || !playRate->Is_Number() ||
				!std::isfinite(playRate->Get_Number()) ||
				playRate->Get_Number() < 0.1 || playRate->Get_Number() > 4.0 ||
				nullptr == endPolicy || (endPolicy->Get_String() != "EXACT" &&
                    endPolicy->Get_String() != "HOLD_LAST_POSE" && endPolicy->Get_String() != "LOOP_TO_WINDOW") ||
                (holdAtEnd && !holdAtEnd->Is_Boolean()) ||
				!Has_Clip(model, clip->Get_String()) || (unblended && !unblended->Is_Boolean()) ||
				(verticalScale && (!verticalScale->Is_Number() || !std::isfinite(verticalScale->Get_Number()) ||
					verticalScale->Get_Number() < 0.0 || verticalScale->Get_Number() > 1.0)))
			{
				outStatus = "KoukuSaydon Product animation row is invalid or unsupported.";
				++skipped; continue;
			}
			row.strActionId = action->Get_String();
			row.strOccurrenceId = occurrence->Get_String();
			row.strClip = clip->Get_String();
			row.fPlayRate = static_cast<f32_t>(playRate->Get_Number());
			row.fAnimationRootVerticalScale = verticalScale ? static_cast<f32_t>(verticalScale->Get_Number()) : 1.f;
			row.bUnblendedBoneContact = unblended && unblended->Get_Boolean();
            row.bLoopToWindow = endPolicy->Get_String() == "LOOP_TO_WINDOW";
            row.bHoldAtWindowEnd = (holdAtEnd && holdAtEnd->Get_Boolean()) || endPolicy->Get_String() == "HOLD_LAST_POSE";
            double nativeDurationMs = 0.0, sampledSourceMs = 0.0;
            if (!Try_GetClipDurationMs(model, row.strClip, nativeDurationMs) ||
                (endPolicy->Get_String() != "HOLD_LAST_POSE" && row.iSourceStartMs >= nativeDurationMs) ||
                !CKoukuSaydonCompositionDocument::Try_SampleAnimationSourceMs(
                    row.iSourceStartMs, row.iSourceEndMs, 0.0, row.fPlayRate,
                    nativeDurationMs, row.bLoopToWindow, sampledSourceMs))
            {
                outStatus = "KoukuSaydon Product animation source range is invalid: " + row.strActionId;
                ++skipped; continue;
            }
            const auto* blendMs = value.Find("blendInMs");
            const auto* blendClip = value.Find("blendFromClip");
            const auto* blendSource = value.Find("blendFromSourceMs");
            if (blendMs || blendClip || blendSource)
            {
                if (!blendMs || !Try_U32(*blendMs, 1000u, row.iBlendInMs) || !row.iBlendInMs ||
                    row.iBlendInMs > row.iPlayMs || !blendClip || !blendClip->Is_String() ||
                    !Has_Clip(model, blendClip->Get_String()) || !blendSource || !blendSource->Is_Number() ||
                    !std::isfinite(blendSource->Get_Number()) || blendSource->Get_Number() < 0.0 ||
                    blendSource->Get_Number() > 600000.0)
                { outStatus = "KoukuSaydon Product animation transition is invalid."; ++skipped; continue; }
                double previousDurationMs = 0.0;
                if (!Try_GetClipDurationMs(model, blendClip->Get_String(), previousDurationMs))
                { outStatus = "KoukuSaydon Product animation transition clip is invalid."; ++skipped; continue; }
                row.strBlendFromClip = blendClip->Get_String();
                // Publisher samples the previous cropped range at its actual
                // transition boundary. Preserve the legacy native-end clamp when
                // an older Product row stores an uncapped previous source time.
                row.fBlendFromSourceMs = float((std::min)(previousDurationMs, blendSource->Get_Number()));
            }
            if (const auto* windows = value.Find("animationBlendWindows"))
            {
                if (!CKoukuSaydonAnimationBlend::Read_ProductWindows(*windows, row.AnimationBlendWindows, outStatus) ||
                    !CKoukuSaydonAnimationBlend::Validate_ModelWindows(model, row.AnimationBlendWindows, outStatus))
                { ++skipped; continue; }
            }
			const std::string actionId = row.strActionId;
			if (duplicates.contains(actionId) || !staged.emplace(actionId, std::move(row)).second)
			{
				staged.erase(actionId);
				duplicates.insert(actionId);
				outStatus = "KoukuSaydon Product animation actionId is duplicated.";
				++skipped; continue;
			}
		}
        ATTACHMENT_GRIPS stagedGrips;
        if (const auto* grips = root.Find("attachmentGrips"))
        {
            const auto* patterns = Required(root, "patterns", DATA_JSON_TYPE::ARRAY);
            if (!grips->Is_Array() || grips->Get_Array().size() > 4096u || !patterns)
            { outStatus = "KoukuSaydon attachmentGrips requires bounded rows and Product patterns."; return false; }
            for (const auto& grip : grips->Get_Array())
            {
                if (!Has_ExactProperties(grip, {"patternId", "attachmentSlot", "gripLocalOffset"}))
                { outStatus = "KoukuSaydon attachment grip fields are malformed."; return false; }
                const auto* id = Required(grip, "patternId", DATA_JSON_TYPE::STRING);
                const auto* slot = Required(grip, "attachmentSlot", DATA_JSON_TYPE::STRING);
                const auto* offset = Required(grip, "gripLocalOffset", DATA_JSON_TYPE::ARRAY);
                if (!id || !Is_StableToken(id->Get_String()) || !slot || slot->Get_String() != "BOSS_LEFT_HAND" ||
                    !offset || offset->Get_Array().size() != 3u ||
                    std::count_if(patterns->Get_Array().begin(), patterns->Get_Array().end(), [&](const auto& pattern)
                    { const auto* patternId = Required(pattern, "patternId", DATA_JSON_TYPE::STRING);
                      return patternId && patternId->Get_String() == id->Get_String(); }) != 1)
                { outStatus = "KoukuSaydon attachment grip has an invalid pattern, slot or offset."; return false; }
                float components[3]{};
                for (std::size_t i = 0; i < 3u; ++i)
                {
                    const auto& number = offset->Get_Array()[i];
                    if (!number.Is_Number() || !std::isfinite(number.Get_Number()) ||
                        std::abs(number.Get_Number()) > CPlayerHandGripTransform::MAX_GRIP_OFFSET_COMPONENT_M)
                    { outStatus = "KoukuSaydon gripLocalOffset must contain finite metre components within +/-10."; return false; }
                    components[i] = static_cast<float>(number.Get_Number());
                }
                const PLAYER_HAND_GRIP_LOCAL_OFFSET parsed{components[0], components[1], components[2]};
                if (!stagedGrips.emplace(id->Get_String(), parsed).second)
                { outStatus = "KoukuSaydon attachment grip patternId is duplicated."; return false; }
            }
        }
        outGrips = std::move(stagedGrips);
		out = std::move(staged);
		outRevision = parsedRevision;
        if (outCanonicalSource) *outCanonicalSource = std::move(canonicalSource);
		outStatus = "Loaded " + std::to_string(out.size()) +
			" KoukuSaydon Product animation action(s), skipped " + std::to_string(skipped) + " invalid row(s).";
		return true;
	}

	HRESULT Reject(const std::string_view reason)
	{
		g_Status = std::string(reason);
		OutputDebugStringA(("[KoukuSaydonPresentation] " + g_Status + "\n").c_str());
		return E_FAIL;
	}

	bool Is_FamilyPresentationId(const std::string_view presentationId)
	{
		return presentationId.size() >
				KOUKU_FAMILY_PRESENTATION_PREFIX.size() +
				KOUKU_FAMILY_PRESENTATION_SUFFIX.size() &&
			presentationId.starts_with(KOUKU_FAMILY_PRESENTATION_PREFIX) &&
			presentationId.ends_with(KOUKU_FAMILY_PRESENTATION_SUFFIX);
	}

	/* Archetype IDs are stable ASCII tokens, so the widening is a plain copy. */
	std::wstring Widen_Ascii(const std::string_view value)
	{
		std::wstring wide;
		wide.reserve(value.size());
		for (const char c : value)
			wide.push_back(static_cast<wchar_t>(static_cast<unsigned char>(c)));
		return wide;
	}
}

void Client::CKoukuSaydonPresentationAssetService::Begin_LevelLoad(
	const std::uint32_t iLevelIndex)
{
	std::scoped_lock lock{ g_KoukuAssetMutex };
	g_ReadyByLevel.erase(iLevelIndex);
	g_ActionPresentationsByArchetype.clear();
	g_AttachmentGripsByArchetype.clear();
	g_BindingSourceRevisions.clear();
    g_CanonicalBindingSources.clear(); g_LastCanonicalBindingSource.reset(); g_BindingPreparation.reset();
    g_PreparedDraft.reset(); g_AdmittedDraft.reset();
    g_AdmittedRunEpoch = g_AdmittedSourceRevision = g_PreparedAuthorizedEpoch = 0u; g_RunAdmissionFailed = false;
	g_Status = "KoukuSaydon presentation is waiting for Product admission.";
}

bool_t Client::CKoukuSaydonPresentationAssetService::Is_ArenaBossArchetype(
	const std::string_view archetypeId)
{
	if (!archetypeId.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX) ||
		!Is_StableToken(archetypeId))
	{
		return false;
	}
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	return nullptr != actor &&
		Is_FamilyPresentationId(actor->clientPresentationId);
}

std::wstring Client::CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag(
	const std::string_view archetypeId)
{
	if (archetypeId == KOUKU_ARCHETYPE)
		return L"Prototype_Component_Model_KoukuSaydon_MN_RPCZ_00";
	if (!Is_ArenaBossArchetype(archetypeId))
		return {};
	return KOUKU_MODEL_PROTOTYPE_PREFIX + Widen_Ascii(archetypeId);
}

std::wstring
Client::CKoukuSaydonPresentationAssetService::Get_WeaponModelPrototypeTag(
	const std::string_view archetypeId)
{
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	if (nullptr == actor || actor->weaponModel.empty() ||
		!Is_ArenaBossArchetype(archetypeId))
	{
		return {};
	}
	return Get_ModelPrototypeTag(archetypeId) + KOUKU_WEAPON_PROTOTYPE_SUFFIX;
}

const char_t*
Client::CKoukuSaydonPresentationAssetService::Get_WeaponSocketBone()
{
	return KOUKU_WEAPON_SOCKET_BONE;
}

const wchar_t*
Client::CKoukuSaydonPresentationAssetService::Get_GameObjectPrototypeTag()
{
	return KOUKU_OBJECT_PROTOTYPE;
}


HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_MazeHammerPrototype(
	ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex)
{
	if (!pDevice || !pContext || iLevelIndex >= ETOUI(LEVEL::END)) return E_INVALIDARG;
	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	constexpr const char* key = "avatar.kouku-saydon.maze-hammer";
	if (ready.contains(key)) return S_FALSE;
	Engine::MODEL_ASSET_LOAD_DESC load;
	std::string status;
	if (!CActorCatalog::Build_ModelLoadDescription(
		"Effect/KoukuSaydon/WorldObjects/WhirlwindHammer/WhirlwindHammer.wmodel", load, status))
		return Reject("Card maze hammer material input failed: " + status);
	// The same world-object mesh is 56.016 cm long. The hand bone already
	// carries the class cm-to-m conversion; only the world's authored x2 remains.
	auto model = Engine::CModel::Create(pDevice, pContext, MODEL::NONANIM, load,
		XMMatrixScaling(2.f, 2.f, 2.f));
	if (!model || !model->Get_NumMeshes()) return Reject("Card maze hammer model failed.");
	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(KOUKU_MAZE_HAMMER_PROTOTYPE_TAG, std::move(model));
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("Card maze hammer prototype commit failed.");
	ready.insert(key);
	return S_OK;
}

HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_ClownBodyPrototype(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex)
{
	/* Polymorph 4134 is the player's MN_RPCZ_00-1 madness doll. Its
	100-bone body embeds its original and offline-tuned clips with variant material;
	the existing MN_RPCZ_00 boss remains a separate catalog presentation. */
	constexpr std::string_view CLOWN_BODY_ASSET =
		"Character/KoukuSaton/MN_RPCZ_00-1/MN_RPCZ_00-1.wmodel";
	constexpr f32_t CLOWN_BODY_PRE_SCALE = 0.017f * 0.709f;
	constexpr std::string_view CLOWN_READY_KEY = "avatar.kouku-saydon.clown";
	constexpr const char_t* CLOWN_IDLE_CLIP = "rpcz00p_idle_battle_1";
	constexpr const char_t* CLOWN_RUN_CLIP = "rpcz00p_run_battle_1";
	/* The held WP_MN_RHKP_07 and Q's fm_g_rhkp_01 are the same mesh in
	different cook bases. Q's centimetre mesh at StartSize 2 matches this
	metre-authored hand-frame cook, but its source anchor omits the Mario
	body-part shrink. Cancel that shrink in this Mario-only weapon so it
	keeps Q's observed size while following the actual hand. The authored Q
	hides its duplicate mesh emitters and retains its charge/swing/hit FX.
	Keep the measured hand-frame pitch: 220 degrees raises the hammer head
	30 degrees above forward in rpcz00p_idle_battle_1. */
	constexpr std::string_view CLOWN_HAMMER_ASSET =
		"Character/Monster/MarioOriginal/REUP/WP_MN_RHKP_07_Static.wmodel";
	constexpr f32_t CLOWN_HAMMER_PRE_SCALE = 1.313f * (2.3730526f / 1.5f);
	constexpr f32_t CLOWN_HAMMER_PITCH_DEGREES = 220.f;
	constexpr f32_t CLOWN_HAMMER_YAW_DEGREES = 0.f;
	constexpr f32_t CLOWN_HAMMER_ROLL_DEGREES = 0.f;
	if (nullptr == pDevice || nullptr == pContext || iLevelIndex >= ETOUI(LEVEL::END))
		return E_INVALIDARG;

	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	if (ready.contains(std::string(CLOWN_READY_KEY)))
		return S_FALSE;
    Engine::MODEL_ASSET_LOAD_DESC bodyLoad;
    std::string materialStatus;
    if (!CActorCatalog::Build_ModelLoadDescription(CLOWN_BODY_ASSET, bodyLoad, materialStatus))
        return Reject("KoukuSaydon clown material input failed: " + materialStatus);
	/* CCharacter turns every playable body with the same -90 degree admission
	yaw (see CPlayableCharacterAssetService); the avatar follows that so it
	faces where the class body faced. */
	unique_ptr<Engine::CModel> body = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyLoad,
		XMMatrixScaling(CLOWN_BODY_PRE_SCALE, CLOWN_BODY_PRE_SCALE, CLOWN_BODY_PRE_SCALE) *
		XMMatrixRotationY(XMConvertToRadians(-90.f)));
	if (nullptr == body || 0u == body->Get_NumMeshes() ||
		0u == body->Get_SkeletonHash() || !body->Has_Animations())
	{
		return Reject("KoukuSaydon clown body has no usable animated geometry.");
	}
	if (!Has_Clip(*body, CLOWN_IDLE_CLIP) || !Has_Clip(*body, CLOWN_RUN_CLIP))
		return Reject("KoukuSaydon clown body is missing its idle/run clips.");
	/* Without the model the doll mimes its own swing clip. The socket bone is
	checked before the body is moved from. A class weapon is a static part on
	the static shader, so the hammer is admitted exactly that way; the rigged
	original would be refused as NONANIM and take the body down with it. */
	if (!body->Has_Bone(KOUKU_CLOWN_HAMMER_SOCKET_BONE))
		return Reject("KoukuSaydon clown body has no right-hand bone for the hammer.");
	const std::filesystem::path hammerPath =
		CRuntimeAssetRoot::Resolve(CLOWN_HAMMER_ASSET);
	if (hammerPath.empty())
		return Reject("KoukuSaydon clown hammer asset path is invalid.");
	unique_ptr<Engine::CModel> hammer = Engine::CModel::Create(
		pDevice, pContext, MODEL::NONANIM, hammerPath.string().c_str(),
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(CLOWN_HAMMER_PITCH_DEGREES),
			XMConvertToRadians(CLOWN_HAMMER_YAW_DEGREES),
			XMConvertToRadians(CLOWN_HAMMER_ROLL_DEGREES)) *
		XMMatrixScaling(CLOWN_HAMMER_PRE_SCALE,
			CLOWN_HAMMER_PRE_SCALE, CLOWN_HAMMER_PRE_SCALE));
	if (nullptr == hammer || 0u == hammer->Get_NumMeshes())
		return Reject("KoukuSaydon clown hammer static cook did not load.");
	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(KOUKU_CLOWN_BODY_PROTOTYPE_TAG, std::move(body));
	staged.emplace_back(KOUKU_CLOWN_HAMMER_PROTOTYPE_TAG, std::move(hammer));
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("KoukuSaydon clown body prototype commit failed.");
	ready.insert(std::string(CLOWN_READY_KEY));
	g_Status = "KoukuSaydon clown body admitted.";
	return S_OK;
}

HRESULT Client::CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const std::uint32_t iLevelIndex,
	const std::string_view archetypeId)
{
	if (nullptr == pDevice || nullptr == pContext ||
		iLevelIndex >= ETOUI(LEVEL::END) || !Is_ArenaBossArchetype(archetypeId))
	{
		return E_INVALIDARG;
	}

	std::scoped_lock lock{ g_KoukuAssetMutex };
	auto& ready = g_ReadyByLevel[iLevelIndex];
	if (ready.contains(std::string(archetypeId)))
		return S_FALSE;

	const bool_t isGateOneKouku = archetypeId == KOUKU_ARCHETYPE;
	const BOSS_ACTOR_ENTRY* actor = CActorCatalog::Find_Boss(archetypeId);
	if (nullptr == actor ||
		(isGateOneKouku &&
		 (actor->clientPresentationId != KOUKU_PRESENTATION ||
		  !actor->weaponModel.empty() || 0.f != actor->weaponModelPreScale)) ||
		(!actor->weaponModel.empty() && actor->weaponModelPreScale <= 0.f))
	{
		return Reject("No exact embedded-body KoukuSaydon boss catalog row exists.");
	}
    Engine::MODEL_ASSET_LOAD_DESC bodyLoad;
    std::string materialStatus;
    if (!CActorCatalog::Build_ModelLoadDescription(actor->bodyModel, bodyLoad, materialStatus))
        return Reject("KoukuSaydon body material input failed: " + materialStatus);

	const f32_t scale = actor->bodyModelPreScale;
	unique_ptr<Engine::CModel> body = Engine::CModel::Create(
		pDevice, pContext, MODEL::ANIM, bodyLoad,
		XMMatrixScaling(scale, scale, scale));
	if (nullptr == body || 0u == body->Get_NumMeshes() ||
		0u == body->Get_SkeletonHash() || !body->Has_Animations())
	{
		return Reject("KoukuSaydon embedded body has no usable animated geometry.");
	}
	if (!actor->animationSetId.empty() && actor->animationSetId != actor->bodyModel)
	{
		const auto donorPath = CRuntimeAssetRoot::Resolve(actor->animationSetId);
		if (donorPath.empty()) return Reject("KoukuSaydon animation donor path is invalid.");
		const auto donor = Engine::CModel::Create(pDevice, pContext, MODEL::ANIM,
			donorPath.string().c_str(), XMMatrixScaling(scale, scale, scale));
		// Attach validates the full skeleton identity and every duplicate clip before mutation.
		// Body geometry and its embedded gameplay clips remain on this same prototype.
		if (!donor || !donor->Has_Animations() || FAILED(body->Attach_AnimationSet(*donor)))
			return Reject("KoukuSaydon animation donor does not match its body: " + actor->animationSetId);
	}
	if (!Has_Clip(*body, actor->presentationClips.idle))
		return Reject("KoukuSaydon body is missing the catalog idle clip.");

	/* The weapon is a second animated model following the body's source clock
	and socket bone. Its own pre-transform converts the weapon's authored
	units; the socket bone matrix later adds the body's pre-transform. */
	unique_ptr<Engine::CModel> weapon;
	if (!actor->weaponModel.empty())
	{
		if (!body->Has_Bone(KOUKU_WEAPON_SOCKET_BONE))
			return Reject("KoukuSaydon body has no weapon socket bone for its catalog weapon.");
        Engine::MODEL_ASSET_LOAD_DESC weaponLoad;
        if (!CActorCatalog::Build_ModelLoadDescription(actor->weaponModel, weaponLoad, materialStatus))
            return Reject("KoukuSaydon weapon material input failed: " + materialStatus);
		/* The catalog rotation turns the weapon's authored axes onto the
		socket's before the scale; the scale is uniform, so the order only
		documents the intent. */
		const f32_t weaponScale = actor->weaponModelPreScale;
		const float3_t& weaponRotation = actor->weaponModelPreRotationDegrees;
		weapon = Engine::CModel::Create(
			pDevice, pContext, MODEL::ANIM, weaponLoad,
			XMMatrixRotationRollPitchYaw(
				XMConvertToRadians(weaponRotation.x),
				XMConvertToRadians(weaponRotation.y),
				XMConvertToRadians(weaponRotation.z)) *
			XMMatrixScaling(weaponScale, weaponScale, weaponScale));
		if (nullptr == weapon || 0u == weapon->Get_NumMeshes())
			return Reject("KoukuSaydon weapon has no usable geometry.");
	}

	/* One Product binding document serves every arena boss; the loader keeps
	only the rows whose clip exists on this body, so a Saydon pattern never
	resolves on the Kouku rig and vice versa. */
	std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> bindings;
	std::string bindingStatus;
	std::uint32_t bindingRevision = 0u;
	ATTACHMENT_GRIPS grips;
    std::shared_ptr<const std::string> canonicalSource;
	if (!Load_PresentationBindings(*body, bindings, bindingStatus, bindingRevision, grips, 0u, nullptr, false, &canonicalSource))
	{
		// A missing action document must not remove the boss body and other tools.
		bindingStatus = "Animation bindings unavailable; boss body remains usable: " + bindingStatus;
		OutputDebugStringA(("[KoukuSaydonPresentation] " + bindingStatus + "\n").c_str());
	}
	bindingStatus = std::string(archetypeId) + ": " + bindingStatus;

	std::vector<std::pair<std::wstring, unique_ptr<Engine::CPrototype>>> staged;
	staged.emplace_back(Get_ModelPrototypeTag(archetypeId), std::move(body));
	if (nullptr != weapon)
		staged.emplace_back(Get_WeaponModelPrototypeTag(archetypeId), std::move(weapon));
	/* One CNpc game-object prototype serves every arena boss of this level;
	it is committed with the first admitted archetype only. */
	if (ready.empty())
		staged.emplace_back(KOUKU_OBJECT_PROTOTYPE, CNpc::Create(pDevice, pContext));
	for (const auto& [tag, prototype] : staged)
	{
		if (tag.empty() || nullptr == prototype)
			return Reject("KoukuSaydon presentation prototype creation failed.");
	}
	if (FAILED(CGameInstance::Get().Add_Prototypes(iLevelIndex, std::move(staged))))
		return Reject("KoukuSaydon presentation prototype commit failed.");

	g_ActionPresentationsByArchetype[std::string(archetypeId)] = std::move(bindings);
	g_AttachmentGripsByArchetype[std::string(archetypeId)] = std::move(grips);
	g_BindingSourceRevisions[std::string(archetypeId)] = bindingRevision;
    g_CanonicalBindingSources[std::string(archetypeId)] = std::move(canonicalSource);
	ready.insert(std::string(archetypeId));
	g_Status = std::move(bindingStatus);
	return S_OK;
}

bool_t Client::CKoukuSaydonPresentationAssetService::Try_Resolve_Action(
	const std::string_view archetypeId,
	const std::string_view actionId,
	KOUKU_SAYDON_ACTION_PRESENTATION& outPresentation, const std::uint32_t expectedSourceRevision)
{
	std::scoped_lock lock{ g_KoukuAssetMutex };
	if (g_RunAdmissionFailed) return false;
	if (expectedSourceRevision && g_BindingSourceRevisions[std::string(archetypeId)] != expectedSourceRevision) return false;
	const auto owner = g_ActionPresentationsByArchetype.find(std::string(archetypeId));
	if (owner == g_ActionPresentationsByArchetype.end())
		return false;
	const auto found = owner->second.find(std::string(actionId));
	if (found == owner->second.end())
		return false;
	outPresentation = found->second;
	return true;
}

bool_t Client::CKoukuSaydonPresentationAssetService::Try_Resolve_AttachmentGrip(
    const std::string_view archetypeId, const std::string_view patternId,
    const LostArk::Shared::PLAYER_ATTACHMENT_SLOT slot,
    PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset, const std::uint32_t expectedSourceRevision)
{
    if (slot != LostArk::Shared::PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND) return false;
    std::scoped_lock lock{ g_KoukuAssetMutex };
    if (g_RunAdmissionFailed) return false;
    const auto revision = g_BindingSourceRevisions.find(std::string(archetypeId));
    if (revision == g_BindingSourceRevisions.end() || !revision->second ||
        (expectedSourceRevision && revision->second != expectedSourceRevision)) return false;
    const auto owner = g_AttachmentGripsByArchetype.find(std::string(archetypeId));
    if (owner == g_AttachmentGripsByArchetype.end()) return false;
    const auto found = owner->second.find(std::string(patternId));
    if (found == owner->second.end()) return false;
    outOffset = found->second;
    return true;
}

const std::string&
Client::CKoukuSaydonPresentationAssetService::Get_Status()
{
	return g_Status;
}


bool_t Client::CKoukuSaydonPresentationAssetService::Reload_ProductBindings(
    std::uint32_t levelIndex, std::uint32_t expectedSourceRevision, std::string& status)
{
    return Admit_RunProduct(levelIndex, expectedSourceRevision, {}, 0u, status);
}

std::shared_ptr<const Client::KOUKU_SAYDON_DRAFT_PRODUCT>
Client::CKoukuSaydonPresentationAssetService::Prepare_DraftProduct(
    const std::string& presentationJson, const std::string& encounterJson, const std::string& gameplayRows,
    const std::uint32_t sourceRevision, std::string& status)
{
    auto candidate = std::make_shared<KOUKU_SAYDON_DRAFT_PRODUCT>();
    candidate->PresentationJson = presentationJson; candidate->EncounterJson = encounterJson;
    candidate->iSourceRevision = sourceRevision;
    if (!sourceRevision || !CNetworkManager::Compute_KoukuDraftRowsRevision(gameplayRows, candidate->RowsRevision) ||
        !CKoukuSaydonPresentationPlayer::Validate_DraftProductJson(presentationJson, sourceRevision, status))
    { if (status.empty()) status = "Draft gameplay/presentation identity is invalid."; return {}; }
    DATA_JSON_VALUE root; std::string error;
    if (encounterJson.empty() || encounterJson.size() > 16u * 1024u * 1024u || !CDataJson::Parse(encounterJson, root, error))
    { status = "Draft encounter is invalid: " + error; return {}; }
    const auto* patterns = Required(root, "patterns", DATA_JSON_TYPE::ARRAY);
    if (!patterns || patterns->Get_Array().empty() || patterns->Get_Array().size() > 4096u)
    { status = "Draft encounter has no bounded pattern closure."; return {}; }
    std::vector<std::string> identities;
    for (const auto& pattern : patterns->Get_Array())
    {
        const auto* identity = Required(pattern, "patternId", DATA_JSON_TYPE::STRING);
        if (!identity) { status = "Draft encounter pattern identity is missing."; return {}; }
        identities.push_back(identity->Get_String());
    }
    KOUKU_SAYDON_PLAY_RESOURCES resources;
    if (!Collect_CompletePlayResources(identities, {}, sourceRevision, resources, status, candidate)) return {};
    status = "Immutable draft Product parsed; canonical files are unchanged.";
    return candidate;
}

bool Client::CKoukuSaydonPresentationAssetService::Stage_DraftProduct(
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft, std::string& status)
{
    if (!draft || !draft->iSourceRevision || !draft->RowsRevision.Is_Valid())
    { status = "Draft Product is not prepared."; return false; }
    std::scoped_lock lock{g_KoukuAssetMutex};
    g_PreparedDraft = std::move(draft); g_PreparedAuthorizedEpoch = 0u;
    status = "Draft Product staged for exact Server hash admission."; return true;
}

bool Client::CKoukuSaydonPresentationAssetService::Authorize_DraftProduct(
    const LostArk::Shared::GameplayDataRevision& rowsRevision, const std::uint32_t runEpoch, std::string& status)
{
    std::scoped_lock lock{g_KoukuAssetMutex};
    if (g_AdmittedDraft && rowsRevision == g_AdmittedDraft->RowsRevision && runEpoch == g_AdmittedRunEpoch) return true;
    if (!rowsRevision.Is_Valid() || !runEpoch || runEpoch <= g_AdmittedRunEpoch ||
        !g_PreparedDraft || rowsRevision != g_PreparedDraft->RowsRevision)
    { status = "Own accepted draft hash/epoch does not match the staged Product."; return false; }
    g_PreparedAuthorizedEpoch = runEpoch; status.clear(); return true;
}

bool Client::CKoukuSaydonPresentationAssetService::Prepare_ProductBindings(
    const std::uint32_t levelIndex, const std::uint32_t sourceRevision, bool& ready, std::string& status,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft)
{
    ready = false;
    std::scoped_lock lock{g_KoukuAssetMutex};
    const auto archetypes = Ready_CanonicalBindingArchetypes(levelIndex);
    if (!sourceRevision || archetypes.empty())
    { status = "Animation preparation requires an exact revision and prepared boss models."; return false; }
    if (draft && (draft->iSourceRevision != sourceRevision || !draft->RowsRevision.Is_Valid()))
    { status = "Draft animation preparation has no exact source/hash identity."; return false; }
    std::shared_ptr<const std::string> source;
    if (draft) source = std::shared_ptr<const std::string>(draft, &draft->PresentationJson);
    else if (!Read_CanonicalBindingSource(source, status)) return false;
    if (!g_BindingPreparation || g_BindingPreparation->levelIndex != levelIndex ||
        g_BindingPreparation->sourceRevision != sourceRevision || g_BindingPreparation->archetypes != archetypes ||
        g_BindingPreparation->draft != draft)
    {
        auto staged = std::make_unique<BINDING_PREPARATION>();
        staged->levelIndex = levelIndex; staged->sourceRevision = sourceRevision;
        staged->source = source; staged->draft = draft; staged->archetypes = archetypes;
        // Animation/resource readiness must not admit a Product that its actual Effect reader rejects.
        // Cache this result per preparation, including failure, rather than parsing every READY frame.
        staged->presentationValid = CKoukuSaydonPresentationPlayer::Validate_ProductJson(
            *source, sourceRevision, staged->presentationStatus, !draft);
        staged->reusesActive = !draft && Have_PreparedCanonicalBindings(levelIndex, sourceRevision, *source);
        if (staged->reusesActive) staged->nextArchetype = archetypes.size();
        else
        {
            staged->actions = g_ActionPresentationsByArchetype; staged->grips = g_AttachmentGripsByArchetype;
            staged->revisions = g_BindingSourceRevisions; staged->sources = g_CanonicalBindingSources;
        }
        g_BindingPreparation = std::move(staged);
    }
    auto& staged = *g_BindingPreparation;
    if (!draft && *source != *staged.source)
    { status = "Canonical animation source changed during preparation. Prepare the saved Product again."; return false; }
    if (!staged.presentationValid)
    { status = "Product presentation preparation preserved the active cache: " + staged.presentationStatus; return false; }
    if (staged.nextArchetype < staged.archetypes.size())
    {
        // Reusing a GPU model does not make its old revision's clip cache ready.
        // Validate one model per PREPARING frame, without touching the live run.
        const auto& archetype = staged.archetypes[staged.nextArchetype];
        const auto model = std::dynamic_pointer_cast<Engine::CModel>(CGameInstance::Get().Clone_Prototype(levelIndex, Get_ModelPrototypeTag(archetype)));
        std::uint32_t revision = 0u;
        std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> rows;
        ATTACHMENT_GRIPS grips;
        if (!model || !Load_PresentationBindings(*model, rows, status, revision, grips, sourceRevision, staged.source.get(), true))
        { status = "Animation preparation preserved the active cache: " + status; return false; }
        staged.actions[archetype] = std::move(rows); staged.grips[archetype] = std::move(grips);
        staged.revisions[archetype] = revision;
        staged.sources[archetype] = draft ? std::shared_ptr<const std::string>{} : staged.source;
        ++staged.nextArchetype;
        status = "Preparing animation bindings " + std::to_string(staged.nextArchetype) + "/" + std::to_string(staged.archetypes.size());
        return true;
    }
    ready = true;
    status = "Animation bindings are staged for the exact selected Product.";
    return true;
}

bool Client::CKoukuSaydonPresentationAssetService::Admit_RunProduct(
    const std::uint32_t levelIndex, const std::uint32_t sourceRevision,
    const LostArk::Shared::GameplayDataRevision& rowsRevision, const std::uint32_t runEpoch, std::string& status)
{
    std::scoped_lock lock{g_KoukuAssetMutex};
    const auto fail = [&](const std::string& reason) { g_RunAdmissionFailed = true; status = reason; return false; };
    if (!sourceRevision) return fail("Admitted Product source revision is missing.");
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> candidate;
    if (rowsRevision.Is_Valid())
    {
        if (g_AdmittedDraft && runEpoch == g_AdmittedRunEpoch && rowsRevision == g_AdmittedDraft->RowsRevision &&
            sourceRevision == g_AdmittedDraft->iSourceRevision) candidate = g_AdmittedDraft;
        else if (g_PreparedDraft && runEpoch == g_PreparedAuthorizedEpoch && runEpoch > g_AdmittedRunEpoch &&
            rowsRevision == g_PreparedDraft->RowsRevision && sourceRevision == g_PreparedDraft->iSourceRevision) candidate = g_PreparedDraft;
        else return fail("Admitted draft hash/source is unavailable locally; previous caches are preserved.");
        if (!runEpoch) return fail("Draft admission requires the Server run epoch.");
    }
    const auto commitAdmission = [&]() {
        g_AdmittedDraft = candidate;
        g_AdmittedRunEpoch = runEpoch; g_AdmittedSourceRevision = sourceRevision; g_RunAdmissionFailed = false;
        status = g_Status = "Product animation bindings admitted for the exact run.";
        return true;
    };
    if (g_BindingPreparation && g_BindingPreparation->levelIndex == levelIndex &&
        g_BindingPreparation->sourceRevision == sourceRevision && g_BindingPreparation->draft == candidate)
    {
        auto& prepared = *g_BindingPreparation;
        if (!prepared.presentationValid || prepared.nextArchetype != prepared.archetypes.size() ||
            Ready_CanonicalBindingArchetypes(levelIndex) != prepared.archetypes)
            return fail("Animation Product reached admission before its preparation barrier completed.");
        // Draft bytes are owned by the exact immutable pointer whose hash/epoch
        // was authorized above. Canonical files also need a final freshness read.
        auto source = prepared.source;
        if (!candidate)
        {
            if (!Read_CanonicalBindingSource(source, status)) return fail(status);
            if (*source != *prepared.source)
                return fail("Canonical animation source changed after READY; the previous cache is preserved. Prepare the saved Product again.");
        }
        if (prepared.reusesActive)
        {
            if (!Have_PreparedCanonicalBindings(levelIndex, sourceRevision, *source))
                return fail("Prepared canonical animation cache changed before admission; the previous cache is preserved.");
        }
        else
        {
            g_ActionPresentationsByArchetype = std::move(prepared.actions);
            g_AttachmentGripsByArchetype = std::move(prepared.grips);
            g_BindingSourceRevisions = std::move(prepared.revisions);
            g_CanonicalBindingSources = std::move(prepared.sources);
        }
        g_BindingPreparation.reset();
        return commitAdmission();
    }
    const bool retained = !g_RunAdmissionFailed && sourceRevision == g_AdmittedSourceRevision && candidate == g_AdmittedDraft &&
        (!candidate || runEpoch == g_AdmittedRunEpoch);
    // Later occurrences keep the immutable admitted pin; they must not copy every
    // archetype's validated rows just to update the room occurrence epoch.
    if (retained) return commitAdmission();
    if (!candidate && !g_AdmittedDraft)
    {
        std::shared_ptr<const std::string> canonicalSource;
        if (!Read_CanonicalBindingSource(canonicalSource, status)) return fail(status);
        if (Have_PreparedCanonicalBindings(levelIndex, sourceRevision, *canonicalSource))
            return commitAdmission();
    }
    // Standalone/late-observer admission without a Complete Play barrier keeps
    // the existing recovery path. A prepared raid never reparses at first combat.
    auto staged = g_ActionPresentationsByArchetype;
    auto stagedGrips = g_AttachmentGripsByArchetype;
    auto revisions = g_BindingSourceRevisions;
    auto sources = g_CanonicalBindingSources;
    const auto ready = g_ReadyByLevel.find(levelIndex);
    if (ready != g_ReadyByLevel.end())
        for (const auto& archetype : ready->second)
        {
            if (!archetype.starts_with(KOUKU_FAMILY_ARCHETYPE_PREFIX)) continue;
            const auto model = std::dynamic_pointer_cast<Engine::CModel>(CGameInstance::Get().Clone_Prototype(levelIndex, Get_ModelPrototypeTag(archetype)));
            std::uint32_t revision = 0u; std::unordered_map<std::string, KOUKU_SAYDON_ACTION_PRESENTATION> rows; ATTACHMENT_GRIPS grips;
            std::shared_ptr<const std::string> canonicalSource;
            if (!model || !Load_PresentationBindings(*model, rows, status, revision, grips, sourceRevision,
                candidate ? &candidate->PresentationJson : nullptr, !candidate, &canonicalSource)) return fail("Product animation admission preserved the previous cache: " + status);
            staged[archetype] = std::move(rows); stagedGrips[archetype] = std::move(grips); revisions[archetype] = revision;
            sources[archetype] = std::move(canonicalSource);
        }
    g_ActionPresentationsByArchetype = std::move(staged); g_AttachmentGripsByArchetype = std::move(stagedGrips);
    g_BindingSourceRevisions = std::move(revisions); g_CanonicalBindingSources = std::move(sources);
    return commitAdmission();
}

bool Client::CKoukuSaydonPresentationAssetService::Matches_AdmittedRun(
    const std::uint32_t sourceRevision, const LostArk::Shared::GameplayDataRevision& rowsRevision, const std::uint32_t runEpoch)
{
    std::scoped_lock lock{g_KoukuAssetMutex};
    return !g_RunAdmissionFailed && sourceRevision == g_AdmittedSourceRevision && runEpoch == g_AdmittedRunEpoch &&
        rowsRevision == (g_AdmittedDraft ? g_AdmittedDraft->RowsRevision : LostArk::Shared::GameplayDataRevision{});
}

std::shared_ptr<const Client::KOUKU_SAYDON_DRAFT_PRODUCT>
Client::CKoukuSaydonPresentationAssetService::Get_AdmittedDraftProduct()
{
    std::scoped_lock lock{g_KoukuAssetMutex}; return g_AdmittedDraft;
}


bool Client::CKoukuSaydonPresentationAssetService::Collect_CompletePlayResources(
    const std::vector<std::string>& patternIds, const std::vector<std::string>& bundleIds,
    const std::uint32_t sourceRevision, KOUKU_SAYDON_PLAY_RESOURCES& output, std::string& status,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft)
{
    try
    {
        if (!sourceRevision || (patternIds.empty() && bundleIds.empty()))
            throw std::runtime_error("Complete Play requires a pinned, nonempty selection.");
        const auto text = [](const DATA_JSON_VALUE& row, const char* key) -> const std::string& {
            const auto* value = Required(row, key, DATA_JSON_TYPE::STRING);
            if (!value) throw std::runtime_error(std::string("Missing dependency identity: ") + key);
            return value->Get_String();
        };
        const auto array = [](const DATA_JSON_VALUE& row, const char* key) -> const auto& {
            const auto* value = Required(row, key, DATA_JSON_TYPE::ARRAY);
            if (!value || value->Get_Array().size() > 16384u)
                throw std::runtime_error(std::string("Invalid dependency array: ") + key);
            return value->Get_Array();
        };
        const auto read = [&](const wchar_t* relative, const char* schema, uint32_t version) {
            std::string data;
            const auto path = CProjectDataRoot::Resolve(relative);
            if (draft)
            {
                if (draft->iSourceRevision != sourceRevision || !draft->RowsRevision.Is_Valid())
                    throw std::runtime_error("Draft dependency identity does not match its selected run.");
                data = std::string_view(schema) == "lostark.encounter-profile" ? draft->EncounterJson : draft->PresentationJson;
                if (data.empty() || data.size() > 16u * 1024u * 1024u)
                    throw std::runtime_error("Draft dependency document is empty or oversized.");
            }
            else
            {
                std::error_code error;
                const auto bytes = std::filesystem::file_size(path, error);
                if (error || !bytes || bytes > 64u * 1024u * 1024u)
                    throw std::runtime_error("Missing/oversized Complete Play dependency document: " + path.string());
                std::ifstream input(path, std::ios::binary);
                data.resize(size_t(bytes)); input.read(data.data(), std::streamsize(bytes));
                if (!input || input.peek() != std::char_traits<char>::eof())
                    throw std::runtime_error("Complete Play dependency document changed during read: " + path.string());
            }
            DATA_JSON_VALUE root; DATA_JSON_PARSE_LIMITS limits; std::string errorText;
            limits.iMaximumBytes = 64u * 1024u * 1024u; limits.iMaximumValues = 4'000'000u;
            uint32_t revision = 0u, parsedVersion = 0u;
            if (!CDataJson::Parse(data, root, errorText, limits) || text(root, "schema") != schema ||
                !root.Find("sourceRevision") || !Try_U32(*root.Find("sourceRevision"), UINT32_MAX, revision) ||
                revision != sourceRevision || !root.Find("formatVersion") ||
                !Try_U32(*root.Find("formatVersion"), version, parsedVersion) || parsedVersion != version)
                throw std::runtime_error("Complete Play dependency document revision/header mismatch: " + path.string() + "; " + errorText);
            return root;
        };
        const auto encounter = read(L"Encounters/KoukuSaydon/KoukuSaydonEncounter.json", "lostark.encounter-profile", 4u);
        const auto presentation = read(L"Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json", "lostark.kouku-saydon-pattern-bindings", 1u);
        std::map<std::string, const DATA_JSON_VALUE*> patterns, visuals, fears, presentationPatterns, bundles;
        const auto index = [&](const DATA_JSON_VALUE& root, const char* field, const char* key, auto& target) {
            if (!root.Find(field)) return;
            for (const auto& row : array(root, field))
                if (!target.emplace(text(row, key), &row).second)
                    throw std::runtime_error("Duplicate Complete Play dependency: " + text(row, key));
        };
        index(encounter, "patterns", "patternId", patterns);
        index(encounter, "bundles", "bundleId", bundles);
        index(presentation, "patterns", "patternId", presentationPatterns);
        index(presentation, "targetedCombatVisuals", "clientVisualId", visuals);
        index(presentation, "fearPresentations", "presentationId", fears);
        std::set<std::string> pending(patternIds.begin(), patternIds.end()), visited, v1, worlds, actors;
        std::set<std::pair<std::string, std::string>> v2;
        std::set<std::string> selectedVisuals, selectedFears;
        std::map<std::string, std::vector<std::string>> worldSpawns;
        std::map<std::string, std::set<std::string>> patternChildren;
        std::set<std::string>* currentChildren = nullptr;
        const auto addPattern = [&](const std::string& id) {
            pending.insert(id);
            if (currentChildren) currentChildren->insert(id);
        };
        const auto addEffect = [&](const DATA_JSON_VALUE& row) {
            if (text(row, "kind") != "EFFECT") return;
            const auto& id = text(row, "assetId"); const auto& kind = text(row, "resourceKind");
            if (id.empty()) throw std::runtime_error("Empty Complete Play Effect ID.");
            if (kind == "V1_EFFECT" || kind == "V1_ELEMENT") v1.insert(id);
            else if (kind == "GROUP" || kind == "LEAF") v2.emplace(kind, id);
            else throw std::runtime_error("Unsupported Complete Play Effect kind: " + kind);
        };
        for (const auto& id : bundleIds)
        {
            const auto found = bundles.find(id);
            if (found == bundles.end()) throw std::runtime_error("Missing Complete Play Bundle: " + id);
            for (const auto& member : array(*found->second, "members")) pending.insert(text(member, "patternId"));
        }
        // Traverse only typed Product identity fields. This includes all branches
        // and every future row, without filtering against stage time or duration.
        std::function<void(const DATA_JSON_VALUE&)> dependencies;
        dependencies = [&](const DATA_JSON_VALUE& value) {
            if (value.Is_Array()) { for (const auto& child : value.Get_Array()) dependencies(child); return; }
            if (!value.Is_Object()) return;
            for (const auto& [key, child] : value.Get_Object())
            {
                if (key == "patternId" || key == "clonePatternId")
                { if (child.Is_String() && !child.Get_String().empty()) addPattern(child.Get_String()); }
                else if (key == "patternIds" || key == "directionPatternIds")
                {
                    if (!child.Is_Array()) throw std::runtime_error("Invalid child pattern dependency array.");
                    for (const auto& id : child.Get_Array())
                    { if (!id.Is_String() || id.Get_String().empty()) throw std::runtime_error("Invalid child pattern identity."); addPattern(id.Get_String()); }
                }
                else if (key == "sequenceInstanceId" || key == "worldSequenceInstanceId" ||
                    key == "targetWorldInstanceId" || key == "motionInstanceId")
                { if (child.Is_String() && !child.Get_String().empty()) worlds.insert(child.Get_String()); }
                else if (key == "fixedVisualId" || key == "trackingVisualId" || key == "clientVisualId" || key == "selectedEffectVisualId")
                { if (child.Is_String() && !child.Get_String().empty()) selectedVisuals.insert(child.Get_String()); }
                else if (key == "visualIds")
                {
                    if (!child.Is_Array()) throw std::runtime_error("Invalid targeted visual dependencies.");
                    for (const auto& id : child.Get_Array())
                    { if (!id.Is_String()) throw std::runtime_error("Invalid targeted visual identity."); selectedVisuals.insert(id.Get_String()); }
                }
                else if (key == "presentationId")
                { if (child.Is_String() && !child.Get_String().empty()) selectedFears.insert(child.Get_String()); }
                dependencies(child);
            }
        };
        while (!pending.empty())
        {
            const auto id = *pending.begin(); pending.erase(pending.begin());
            if (!visited.insert(id).second) continue;
            if (visited.size() > 4096u) throw std::runtime_error("Complete Play pattern closure exceeds its bounded capacity.");
            const auto source = patterns.find(id), visual = presentationPatterns.find(id);
            if (source == patterns.end() || visual == presentationPatterns.end())
                throw std::runtime_error("Missing published Complete Play child pattern: " + id);
            currentChildren = &patternChildren[id];
            dependencies(*source->second);
            // These published Encounter rows create WORLD owners. Recursive
            // motionInstanceId/APPLY_TARGET dependencies only prepare resources.
            if (source->second->Find("worldSequences"))
                for (const auto& row : array(*source->second, "worldSequences"))
                    worldSpawns[id].push_back(text(row, "sequenceInstanceId"));
            const auto actor = CKoukuSaydonCompositionDocument::Resolve_BossArchetypeId(text(*source->second, "targetBossPlacementId"));
            if (actor.empty()) throw std::runtime_error("Missing Complete Play boss identity: " + id);
            actors.emplace(actor);
            for (const auto& row : array(*visual->second, "presentationOccurrences"))
            { addEffect(row); dependencies(row); }
        }
        currentChildren = nullptr;
        std::vector<std::vector<std::string>> spawnGroups;
        const auto appendSpawnClosure = [&](const std::string& root, std::vector<std::string>& group) {
            std::set<std::string> queued{root}, seen;
            while (!queued.empty())
            {
                const auto id = *queued.begin(); queued.erase(queued.begin());
                if (!seen.insert(id).second) continue;
                const auto spawns = worldSpawns.find(id);
                if (spawns != worldSpawns.end())
                {
                    if (group.size() + spawns->second.size() > 16384u)
                        throw std::runtime_error("Complete Play WORLD spawn reservation exceeds its bounded capacity.");
                    group.insert(group.end(), spawns->second.begin(), spawns->second.end());
                }
                const auto children = patternChildren.find(id);
                if (children != patternChildren.end()) queued.insert(children->second.begin(), children->second.end());
            }
        };
        // Whole-raid roots run sequentially: reserve their maximum in the Level,
        // not their sum. A root's reachable branches are conservatively combined.
        for (const auto& id : visited)
        {
            std::vector<std::string> group; appendSpawnClosure(id, group);
            if (!group.empty()) spawnGroups.push_back(std::move(group));
        }
        for (const auto& [id, bundle] : bundles)
        {
            const auto& members = array(*bundle, "members");
            if (!std::all_of(members.begin(), members.end(), [&](const auto& member) {
                return visited.contains(text(member, "patternId")); })) continue;
            std::vector<std::string> group;
            // Two members may name the same pattern and still own distinct props.
            for (const auto& member : members) appendSpawnClosure(text(member, "patternId"), group);
            if (!group.empty()) spawnGroups.push_back(std::move(group));
        }
        for (const auto& id : selectedVisuals)
        {
            const auto found = visuals.find(id);
            if (found == visuals.end()) throw std::runtime_error("Missing Complete Play combat visual: " + id);
            for (const auto& row : array(*found->second, "resources")) addEffect(row);
            if (const auto* contact = found->second->Find("contactEffectAssetId"); contact && contact->Is_String() && !contact->Get_String().empty())
                v1.insert(contact->Get_String());
        }
        for (const auto& id : selectedFears)
        {
            const auto found = fears.find(id);
            if (found == fears.end()) throw std::runtime_error("Missing Complete Play Fear presentation: " + id);
            if (const auto* effect = found->second->Find("effectResource"); effect && !effect->Is_Null()) addEffect(*effect);
        }
        // The same shared state presentations used by Release prewarm are selected
        // by authoritative card/ball state rather than a named occurrence.
        for (const char* symbol : {"heart", "spade", "clober", "dia"})
            for (const char* color : {"red", "black"}) v2.emplace("GROUP", std::string("boss.kouku.card.") + symbol + "." + color);
        for (const char* color : {"red", "blue", "yellow"}) v2.emplace("LEAF", std::string("boss.kouku.ball.smoke.") + color + "_1");
        KOUKU_SAYDON_PLAY_RESOURCES staged;
        staged.PatternIds.assign(visited.begin(), visited.end()); staged.V1EffectIds.assign(v1.begin(), v1.end());
        staged.V2Effects.assign(v2.begin(), v2.end()); staged.WorldInstanceIds.assign(worlds.begin(), worlds.end());
        staged.BossArchetypeIds.assign(actors.begin(), actors.end());
        staged.WorldSpawnGroups = std::move(spawnGroups);
        output = std::move(staged);
        status = "Complete Play dependency closure collected.";
        return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}
```

### C:/Users/tnest/Desktop/LostArk/Client/Public/Level_KakulSaydonArena.h 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"
#include "ArenaCameraProfile.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Effect_PresentationService.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "Level.h"
#include "MapAuthoringHost.h"
#include "MapPlacementRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PlayerController.h"
#include "PartyInteractionView.h"
#include "StatusEffectTextView.h"
#include "RaidGateProgressView.h"
#include "InteractKeyPromptView.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanCinematicCameraController.h"
#include "WorldPlayerChatBubbleView.h"
#include "WorldPlayerNameplateView.h"
#include "WorldSequencePlayer.h"

#include <array>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CNpc;
class CTrigger_Box;
class IPlayerCommandSink;
class IWorldEntityCommandSink;

class CUILayoutRuntime;
class CKoukuMadnessGaugeView;
class CMvpResultView;

class CLevel_KakulSaydonArena final : public CLevel
#ifdef _DEBUG
	, public IMapAuthoringHost
#endif
{
public:
	void Set_MapLightAuthoringOverride(std::shared_ptr<CMapLightPresentationRuntime> lights);
	// Called once after Composition Seek/Stop, immediately before world rendering.
	void Submit_MapLightFrame();
	// Session-only comparison; authoring documents and gate light transforms stay intact.
	enum class MAP_LIGHT_COMPARISON { CURRENT, SOURCE_IMPORT, DISABLED };
	bool_t Set_MapLightComparison(MAP_LIGHT_COMPARISON mode, std::string& outStatus);
	MAP_LIGHT_COMPARISON Get_MapLightComparison() const { return m_eMapLightComparison; }
	void Reset_MapLightComparison();
	uint64_t Get_MapLightComparisonFingerprint() const;
	bool_t Reload_MapLights();
	struct KAKUL_STAGE_MARKER final
	{
		std::string strStageId;
		std::string strPlacementId;
		std::string strDisplayNameKo;
		std::string strSourceLevelId;
	};

	/* One authored camera shot. While the local Character stands inside the
	   box the camera holds this exact pose - the reference footage keeps the
	   background pinned while the party walks - and leaving the box hands the
	   camera back to the ordinary follow view. */
	struct KAKUL_CAMERA_SHOT final
	{
		std::string strShotId;
		std::string strDisplayName;
		uint32_t iDefaultHoldMs = 3000u;
		bool_t bPatternOnly = false;
		VALTAN_CINEMATIC_CAMERA_EASING eTransitionEasing = VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		/* Empty means the box decides. When it names a sequence the
		   shot holds for exactly as long as that sequence plays, so a
		   trigger that starts the sequence also starts the shot. */
		std::string strSequenceInstanceId;
		float3_t vCenter = {};
		float3_t vHalfExtents = {};
		f32_t fYawDegrees = 0.f;
		float3_t vEye = {};
		float3_t vLookAt = {};
		f32_t fFovYDegrees = 60.f;
		uint32_t iBlendInMs = 0u;
		uint32_t iBlendOutMs = 0u;
		uint32_t iPriority = 0u;
		/* A side scrolling stage keeps one framing and slides it with the
		   local Character instead of pinning it in place. Both offsets are
		   added to that Character's position, so the authored eye and lookAt
		   stay as the pose used while no Character exists. */
		bool_t followsPlayer = false;
		float3_t vFollowEyeOffset = {};
		float3_t vFollowLookAtOffset = {};
		/* A shot without a track keeps the single authored pose. With one
		   it is sampled on the bound sequence's own clock by the one
		   cinematic sampler this project owns, so a second easing or
		   spline implementation can never drift from it. */
		bool_t hasCameraTrack = false;
		VALTAN_CINEMATIC_CAMERA_CUE CameraTrack;
	};

private:
	CLevel_KakulSaydonArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	using WORLD_EMISSION_ANCHOR = std::function<bool_t(f32_t, float4x4_t&)>;
	using WORLD_EMISSION_RESOLVER = std::function<bool_t(std::uint32_t, std::string_view, std::string_view, WORLD_EMISSION_ANCHOR&)>;
	void Set_CompositionWorldEmissionResolver(WORLD_EMISSION_RESOLVER resolver)
	{ m_WorldEmissionResolver = std::move(resolver); }
	virtual ~CLevel_KakulSaydonArena();

	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;
	bool_t Get_MadnessGaugePosition(float2_t& screenOffset, f32_t& feetOffsetMeters) const;
	bool_t Set_MadnessGaugePosition(const float2_t& screenOffset, f32_t feetOffsetMeters);
	bool_t Save_MadnessGaugePosition(std::string& status);
	bool_t Reload_MadnessGaugePosition(std::string& status);
	const ARENA_CAMERA_PROFILE& Get_FollowCameraProfile() const
	{ return m_FollowCameraProfile; }
	const ARENA_CAMERA_PROFILE& Get_EffectiveFollowCameraProfile() const
	{ return m_EffectiveFollowCameraProfile; }
	const std::string& Get_FollowCameraProfileStatus() const
	{ return m_strFollowCameraProfileStatus; }
	bool_t Set_FollowCameraProfile(const ARENA_CAMERA_PROFILE& profile,
		std::string& outStatus);


	static CLevel_KakulSaydonArena* Get_Active()
	{
		return s_pActiveInstance;
	}
	void Collect_MinimapMarkers(
		CClientReplication::MINIMAP_MARKER_SNAPSHOT& outSnapshot) const
	{
		m_Replication.Collect_MinimapMarkers(outSnapshot);
	}

	/* Read-only replicated presentation, including the current madness avatar. */
	shared_ptr<CCharacter> Get_LocalCharacter() const
	{
		return m_Replication.Get_LocalCharacter();
	}
	const LostArk::Shared::PLAYER_SNAPSHOT* Get_CameraPlayerSnapshot() const
	{ return m_Replication.Get_CameraPlayerSnapshot(); }
	bool_t Should_ShowPlayerWorldUI(LostArk::Shared::NET_ENTITY_ID entityId) const
	{ return m_Replication.Should_ShowKoukuPlayerWorldUI(entityId); }
	bool_t Should_ShowDamageWorldUI(LostArk::Shared::NET_ENTITY_ID targetId,
		LostArk::Shared::PLAYER_ID sourcePlayerId) const
	{ return m_Replication.Should_ShowKoukuDamageWorldUI(targetId, sourcePlayerId); }
	/* Party roster window (CMainApp): the Server roster and the per-player HP / madness join. */
	void Drain_ChatLines(std::vector<CClientReplication::CHAT_LINE>& outLines)
	{
		m_Replication.Drain_ChatLines(outLines);
	}
	void Render_TransferFailureNotice() { m_PartyTransferNotice.Render_TransferNoticeText(); }
	const LostArk::Shared::S2C_GUIDE_STATE* Get_GuideState() const { return m_Replication.Get_GuideState(); }
	const LostArk::Shared::S2C_PARTY_ROSTER& Get_PartyRoster() const
	{
		return m_Replication.Get_PartyRoster();
	}
	const CReplicatedPlayerHealth& Get_PlayerHealth() const
	{
		return m_Replication.Get_PlayerHealth();
	}

	/* One F1 "KoukuSaydon Arena" gate button. The Server raises the named
	   disabled boss placements, moves only this player to the fixed position
	   through the Debug teleport contract, and the HUD follows one archetype.
	   Positions are Debug authoring values captured from Move Player; the
	   Server still validates navigation, height and collision. A gate with a
	   deferred reason has no navigation yet and only reports that reason. */
	struct KAKUL_DEBUG_GATE final
	{
		const char_t* pLabel = nullptr;
		std::array<const char_t*, 2> BossPlacementIds = { nullptr, nullptr };
		float3_t vPlayerPosition = {};
		const char_t* pHudFocusArchetypeId = nullptr;
		/* Placement of pHudFocusArchetypeId: the boss the Kouku Boss Tool and
		   Complete Play target after this gate is raised. Null keeps the
		   Gate 1 Kouku target. */
		const char_t* pAuditionPlacementId = nullptr;
		const char_t* pDeferredReason = nullptr;
	};
	static constexpr size_t NO_ACTIVE_DEBUG_GATE = static_cast<size_t>(-1);
	static const std::array<KAKUL_DEBUG_GATE, 9>& Get_DebugGates();
	// Shared Server-raid presentation; the editor uses this same owner.
	CPlayerController& Get_DebugPlayerController() { return m_PlayerController; }
	void Debug_ReturnToPlayerCamera();
	void Debug_SetSequenceCombatPending(bool_t pending);
	void Debug_HoldSequenceCombatFade();
	// Includes only this client's Server-admitted Mario presentation override.
	const string& Get_GatePresentationProfileId() const;
	bool_t Prepare_ServerRaidGatePresentation(const std::string& gateId, std::string& status);
	bool_t Apply_ServerRaidGatePresentation(const std::string& gateId, std::uint32_t epoch, std::string& status);
	// An admitted local Sequence shares the existing Level-owned Music channel.
	void Notify_SequencePlaybackStarted();
	void Notify_SequencePlaybackEnded();
	bool_t Is_AtGate3EntryTerrace() const;
	bool_t Begin_ServerRaidCinematicPresentation(std::string& status);
	bool_t End_ServerRaidCinematicPresentation(bool_t restorePrevious, std::string& status);

#ifdef _DEBUG
	/* IMapAuthoringHost: the Debug Map Tool edits this arena's live map in
	   place through these; the arena keeps owning every runtime container. */
	uint32_t Get_MapAuthoringLevelIndex() const override
	{ return ETOUI(LEVEL::KAKULSAYDON_ARENA); }
	const char_t* Get_MapAuthoringLabel() const override { return "Kouku"; }
	CMapPlacementRuntime& Get_MapAuthoringRuntime() override { return m_MapRuntime; }
	const CMapAssetCatalog& Get_MapAuthoringCatalog() const override
	{ return m_MapRuntime.Get_Catalog(); }
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& Get_MapAuthoringPlacements() override
	{ return m_MapRuntime.Get_MutablePlacements(); }
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& Get_MapAuthoringBatches() override
	{ return m_MapRuntime.Get_AuthoringBatches(); }
	CDeployPropRuntime* Get_MapAuthoringDeployRuntime() override { return &m_DeployRuntime; }
	void Set_MapAuthoringActive(bool_t active) override { m_bMapAuthoringActive = active; }
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records) override
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
	CWorldSequencePlayer::TARGET_SET Make_MapAuthoringTargets() override
	{ return Make_WorldSequenceTargets(); }
	bool_t Can_ChangeMapAuthoringStructure(std::string& outReason) const override
	{
		if (Can_ReplaceMapAuthoringTargets())
			return true;
		outReason = "Stop active arena/Object/Composition playback before adding, deleting or reloading map objects.";
		return false;
	}
	bool_t Can_ReplaceMapAuthoringTargets() const
	{
		return !m_SequencePlayer.Has_ActiveInstances() &&
			m_CompositionWorldPreviewCues.empty() && m_OwnedWorldCues.empty() &&
			(!m_pWorldObjectPreview || !m_pWorldObjectPreview->Has_ActiveInstances()) &&
			(!m_pMarioBombPlayer || !m_pMarioBombPlayer->Has_ActiveInstances());
	}
	void Set_DebugGazeView(bool visible, float halfAngleDegrees, float distanceM)
	{ m_bDebugGazeView = visible; m_fDebugGazeHalfAngle = halfAngleDegrees; m_fDebugGazeDistance = distanceM; }
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	struct COMPOSITION_WORLD_PREVIEW_CUE final
	{
		std::string occurrenceId;
		std::string instanceId;
		uint32_t startMs = 0u;
		uint32_t durationMs = 0u;
		f32_t playbackSpeed = 1.f;
		float3_t positionOffset{};
		std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> placement;
		WORLD_EMISSION_ANCHOR emissionAnchor;
		std::string bossArchetypeId;
		std::string actorProfileId;
	};
	bool_t Debug_BeginCompositionWorldPreview(const std::string& patternId,
		std::vector<COMPOSITION_WORLD_PREVIEW_CUE> cues, std::string& status,
		const CWorldSequenceDocument* sourceDocument = nullptr);
	bool_t Debug_HasVisibleCompositionWorldBox(std::string_view occurrenceId) const;
	bool_t Debug_SetCompositionWorldPlacement(const std::string& occurrenceId,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement, std::string& status);
	bool_t Debug_SampleCompositionWorldPreview(const std::string& patternId,
		bool_t playing, uint32_t clockMs, std::string& status,
		const decltype(CWorldSequencePlayer::TARGET_SET::bossAnchor)& bossAnchorOverride = {});
	void Debug_StopCompositionWorldPreview();
	// Applies immediately and remembers this arena's value until process exit.
	bool_t Set_DebugCameraSpeed(f32_t metersPerSecond);

#endif
	/* Despawns the previous gate bosses, requests this gate's placements,
	   submits the player teleport, points the HUD and the pattern audition at
	   the gate boss. Every step is a typed Server command; nothing local is
	   spawned or moved. */
	bool_t Debug_ActivateGate(size_t gateIndex, std::string& outStatus, bool_t preservePlayerPosition = false);
	bool_t Debug_DespawnArenaBosses(std::string& outStatus);
	bool_t Debug_DespawnFireObjects(std::string& outStatus);
	bool_t Debug_ReturnToStart(std::string& outStatus);
	bool_t Consume_DebugReturnToStartSucceeded() { return std::exchange(m_bDebugStartSucceeded, false); }
	void Debug_RetireGateActivation(const std::string& reason);
	size_t Get_ActiveDebugGate() const { return m_iActiveDebugGate; }
	bool_t Is_DebugGateApprovedForServerPlay(size_t gateIndex) const;
	// Changes whenever a new gate activation is submitted, including the same gate.
	std::uint32_t Get_DebugGateGeneration() const { return m_iNextDebugGateRequestSequence; }
    bool Debug_PrepareCompletePlayResources(const std::vector<std::string>& patternIds,
        const std::vector<std::string>& bundleIds, uint32_t sourceRevision,
        bool& ready, std::string& status, bool wholeRaid = false,
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft = {});
    void Debug_ResetCompletePlayPreparation() { m_CompletePlayPreparation.reset(); }

	bool_t Is_DebugGatePending() const { return m_bDebugStartPending || NO_ACTIVE_DEBUG_GATE != m_iPendingDebugGate; }
	const std::string& Get_DebugGateStatus() const { return m_strDebugGateStatus; }
	/* Read-only replicated boss presence used by F1 play preparation. */
	std::shared_ptr<CNpc> Debug_FindArenaBossNpc(std::string_view archetypeId) const
	{
		return m_Replication.Find_ArenaBossNpc(archetypeId);
	}

	// The level owns the replicated player anchor used by local authoring previews.
	bool_t Try_Get_AuthoringPreviewPlacement(
		float3_t& outPosition, std::string& outStatus) const;
	bool_t Try_Get_AuthoringForwardPlacement(
		float3_t& outPosition, std::string& outStatus) const;

	/* The F1 stage selector submits only stable authored placement IDs through
	   the typed Server command sink. Until an authored StageMarkers contract is
	   loaded, the empty allow-list rejects every request instead of inventing a
	   waypoint or teleporting the local Character. */
	bool_t Request_StageTeleport(
		std::uint32_t requestSequence,
		std::string_view placementId,
		std::string& outStatus);
	const std::vector<KAKUL_STAGE_MARKER>& Get_StageMarkers() const
	{
		return m_StageMarkers;
	}
	// MainApp calls once after the final camera, before Render.World.
	void Submit_EntranceTriggerMarkers();
    void Set_TargetedCombatPresentationPlayer(CKoukuSaydonPresentationPlayer* player)
    { m_pTargetedCombatPresentationPlayer = player; m_Replication.Set_TargetedCombatPresentationPlayer(player); }
	void Collect_KoukuPresentationViews(std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
		std::vector<KOUKU_CARD_PRESENTATION_VIEW>& cards) const
	{ m_Replication.Collect_KoukuPresentationViews(bosses, cards); }
	void Collect_KoukuMazeTargets(std::vector<KOUKU_MAZE_TARGET_VIEW>& targets) const
	{ m_Replication.Collect_KoukuMazeTargets(targets); }
	bool_t Sample_CompositionCamera(std::string_view shotId, float seconds, const float3_t& offset, std::string_view ownerKey, uint32_t durationMs, bool_t preview);
	bool_t Is_CompositionCameraEnabled() const;
	bool_t Is_CinematicPresentationActive() const;
	bool_t Is_CinematicInputBlocked() const;
	bool_t Should_HideCinematicPlayers() const;
	void Sync_CinematicPlayerVisibility();
	bool_t Is_LocalMarioStageActive() const;
	void Trace_CinematicPresentation(std::string_view renderingProfile);
	void Stop_CompositionCamera(bool_t force = false);
	bool_t Try_GetCompositionWorldPivot(std::string_view instanceId, float4x4_t& out,
		std::string_view occurrenceId = {}, std::uint32_t emissionIndex = 0u,
        const std::string& bone = {}, bool_t boneRotation = false, const std::string& effectTrackId = {}) const;
    bool_t Create_CompositionPreviewActor(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        std::shared_ptr<CNpc>& outActor, std::string& status);
    void Release_CompositionPreviewActor(const std::shared_ptr<CNpc>& actor);
    CWorldSequencePlayer::TARGET_SET Get_CompositionWorldTargets() { return Make_WorldSequenceTargets(); }
	// Authoring inventory reads the placed centre even before any sequence plays.
	bool_t Try_GetWorldSequencePlacementBaseline(const WORLD_SEQUENCE_INSTANCE& instance,
		float3_t& outPosition, const CWorldSequenceDocument* document = nullptr) const;
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const { return m_pPlayerCommandSink; }
	const CWorldSequenceDocument& Get_WorldSequenceDocument() const { return m_SequencePlayer.Get_Document(); }
	const LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& Get_KoukuBundleState() const { return m_Replication.Get_KoukuBundleState(); }
	std::uint32_t Get_PresentationServerTick() const { return m_Replication.Get_LastServerTick(); }
	const LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& Get_KoukuRaidState() const { return m_Replication.Get_KoukuRaidState(); }
	const LostArk::Shared::S2C_KOUKUSAYDON_RAID_STATE& Get_KoukuRaidReply() const { return m_Replication.Get_KoukuRaidReply(); }
	void Expect_KoukuRaidReply(std::uint32_t requestSequence) { m_Replication.Expect_KoukuRaidReply(requestSequence); }
    bool_t Can_StartCompositionWorld(const std::string& instanceId, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr) const;
	bool_t Try_GetOwnedCompositionWorldPivot(std::uint32_t runEpoch, const std::string& memberId,
		const std::string& sequenceId, const std::string& cueId, float4x4_t& out, std::uint32_t emissionIndex = 0u,
        std::uint32_t patternSequence = 0u, const std::string& bone = {}, bool_t boneRotation = false, const std::string& effectTrackId = {}) const;
	void Get_WorldObjectValidationTargets(WORLD_SEQUENCE_PLACEMENT_MAP&, WORLD_SEQUENCE_DEPLOY_MAP&) const;
	bool_t Reload_WorldObjectRuntime(std::string& status);
#ifdef _DEBUG
	bool_t Debug_BeginWorldObjectPreview(const CWorldSequenceDocument&, const std::string& instanceId,
		std::string& status, bool_t previewAtCharacter = true,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement = {});
	bool_t Debug_SampleWorldObjectPreview(f32_t clockMs, std::string& status);
	void Debug_StopWorldObjectPreview();
	void Debug_DrawWorldObjectColliderPreview() const;
#endif
	void Debug_DrawBingoHammerColliders() const;
	const std::vector<KAKUL_CAMERA_SHOT>& Get_PublishedCameraShots() const { return m_CameraShots; }
	bool_t Reload_PublishedCameraShots(std::string& outStatus) { return Load_CameraShots(outStatus); }
	bool_t Ensure_CameraShotAuthoring(std::string& outStatus);
	bool_t Reload_CameraShotAuthoring(std::string& outStatus);
	bool_t Create_CameraShot(std::string_view name, std::string& outShotId, std::string& outStatus);
	bool_t Update_CameraShot(const KAKUL_CAMERA_SHOT& shot, std::string& outStatus);
	bool_t Capture_CameraShot(std::string_view shotId, std::string& outStatus);
	bool_t Duplicate_CameraShot(std::string_view sourceShotId, std::string_view name, std::string& outShotId, std::string& outStatus);
	bool_t Discard_UnsavedCameraShot(std::string_view shotId, std::string& outStatus);
	bool_t Save_CameraShots(std::string& outStatus);
	static bool_t Parse_CameraShots(std::string_view text, std::vector<KAKUL_CAMERA_SHOT>& outShots, std::string& outStatus);
	static VALTAN_CINEMATIC_CAMERA_CUE CameraShot_ToCue(const KAKUL_CAMERA_SHOT& shot);
	static bool_t Stage_PatternCameraTracks(std::string_view baseline,
		const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& cues, const std::map<std::string, std::string>& names,
		std::string& outText, std::string& outStatus);
	bool_t Save_CameraShotSource(std::string_view expectedSource, const std::string& text, std::string& outStatus);

	const std::vector<KAKUL_CAMERA_SHOT>& Get_CameraShots() const
	{
		return m_bCameraAuthoringLoaded ? m_AuthoringCameraShots : m_CameraShots;
	}

	/* Raises one paper stage bridge: the Deploy prop leaves DESPAWNED and its
	   authored unfold sequence starts on the same frame, so the bridge is
	   never visible in its finished pose before it has unfolded. Playing an
	   already raised bridge is a no-op rather than a rewind. */
	bool_t Request_PaperBridgeUnfold(
		uint64_t leverPlacementId,
		std::string& outStatus);

private:
    bool Prepare_EntryRaidResources(std::string& status);
    bool Prepare_CompletePlayResources(const std::vector<std::string>& patternIds,
        const std::vector<std::string>& bundleIds, uint32_t sourceRevision,
        bool& ready, std::string& status, bool wholeRaid,
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft, bool reloadWorld);
	bool_t Try_GetCinematicWorldBossAnchor(const std::string& archetype, const std::string& bone,
		CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status) const;
	CWorldSequencePlayer::TARGET_SET Make_WorldSequenceTargets();
	void Apply_CutsceneSetVisible(bool_t cutsceneVisible);
	/* The cutscene boss is presentation only, so it is taken off the arena
	   as soon as its sequence stops playing. */
	void Update_CutsceneBossRetire(
		const CWorldSequencePlayer::TARGET_SET& targets);
	bool_t Start_ServerRequestedSequence(
		const std::string& instanceId, f32_t playbackSpeed, const float3_t& positionOffset,
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus, uint32_t durationMs = 0u,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement = {});
	bool_t Load_StageMarkers(std::string& outStatus);
	bool_t Load_CameraShots(std::string& outStatus);
	void Update_CameraShots(f32_t fTimeDelta);
	void Update_CompositionCamera(f32_t fTimeDelta);
	bool_t Resolve_CompositionFollowPose(VALTAN_CINEMATIC_CAMERA_POSE& outPose) const;
	/* The arena and the Mario gimmick are more than a kilometre apart, so a
	   trigger move between them is hidden behind a black screen instead of
	   letting the camera travel that distance on screen. Server owns the
	   move; this only reads the action state it already replicates. */
	void Update_TriggerMoveFade(f32_t fTimeDelta);
	bool_t Load_EntranceTriggerMarkers();
	void Clear_EntranceTriggerMarkers();
	void Update_EntranceTriggerMarkerClocks(f32_t deltaSeconds);
	void Retire_EntranceTriggerMarker(const std::string& sequenceInstanceId);
	/* Turns replicated player state into floating status words. Reads the
	   snapshots only; it never decides that a status is on. */
	void Update_StatusEffectText(f32_t fTimeDelta);
	void Update_CardMazePresentation(f32_t fTimeDelta);
    CKoukuSaydonPresentationPlayer* m_pTargetedCombatPresentationPlayer = nullptr;
	void Submit_JokerTargetMarker();
	void Clear_JokerTargetMarker();
	struct JOKER_TARGET_MARKER final
	{
		LostArk::Shared::NET_ENTITY_ID bossId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::NET_ENTITY_ID targetId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		std::uint32_t patternSequence = 0u;
		bool_t failed = false;
		EFFECT_WORLD_ROOT_HANDLE handle;
	} m_JokerTargetMarker;
	void Update_MarioBallBouncePresentation(f32_t fTimeDelta);
	void Update_MarioLayoutPresentation();
	std::string m_strMarioLayoutInstance;
	bool_t m_bMarioLayoutFailed = false;
	bool_t m_bMarioLayoutStarted = false;
	std::uint32_t m_iMarioBallBounceSnapshotTick = 0u;
	f32_t m_fMarioBallBounceSnapshotSeconds = 0.f;
	bool_t m_bMarioBallBounceRunning = false;
	bool_t m_bMarioBallBounceFailed = false;
	/* Server-popped source balls of the current layout: a newly set slot bit
	   hides that binding through the sequence player and plays the ball's
	   original pop once; a newly set curse bit queues the centred notice. */
	void Update_MarioBallPresentation(f32_t timeDelta);
	void Update_MarioCombatPresentation();
	std::uint32_t m_iMarioDamageTick = 0u;
	bool_t m_bMarioCombatWasActive = false;
	bool_t m_bMarioWasKnockedDown = false;
	std::string m_strMarioBallLayoutInstance;
	std::uint16_t m_iMarioPoppedBallsSeen = 0u;
	std::uint8_t m_iMarioCurseSeen = 0u;
	std::uint8_t m_iMarioCurseNoticeQueue = 0u;
	std::int32_t m_iMarioCurseNoticeColor = -1;
	f32_t m_fMarioCurseNoticeSeconds = 0.f;
	static constexpr f32_t MARIO_PROGRESS_HOLD_SECONDS = 1.f;
	static constexpr f32_t MARIO_PROGRESS_FADE_SECONDS = .4f;
	std::uint8_t m_iMarioProgressColor = 0u;
	std::uint8_t m_iMarioProgressCount = 0u;
	f32_t m_fMarioProgressNoticeSeconds = 0.f;
	// Presentation-only launch markers use published world positions and the existing object player.
	struct MARIO_BOMB_EMITTER
	{
		std::uint8_t stage = 0u;
		std::uint32_t seed = 1u, phaseMs = 0u, durationMs = 0u;
		std::vector<std::string> slots;
		std::vector<std::int64_t> births;
        std::vector<std::int64_t> stoppedBirths;
        float3_t start{}, finish{};
		bool_t failed = false;
	};
	bool_t Ready_MarioBombPresentation(std::string& status);
	void Update_MarioBombPresentation(f32_t timeDelta);
    bool_t Queue_MarioBombContactStop(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& stopped);
    void Consume_MarioBombContactStops(double clockMs);
    std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_PendingMarioBombContactStops;
    std::uint32_t m_iMarioBombStageStartTick = 0u;
	std::unique_ptr<CWorldSequencePlayer> m_pMarioBombPlayer;
	std::vector<MARIO_BOMB_EMITTER> m_MarioBombEmitters;
	std::uint8_t m_iMarioBombStage = 0u;
	double m_fMarioBombStageStartMs = 0.;
	std::uint32_t m_iMarioBombSnapshotTick = 0u;
	f32_t m_fMarioBombSnapshotSeconds = 0.f;
	bool_t m_bMarioBombLoadAttempted = false;
	std::uint32_t m_iCardMazeLastSnapshotTick = 0u;
	f32_t m_fCardMazeSnapshotSeconds = 0.f;
	bool m_bCardMazeMarchPlaying = false;
	/* The telescope deploy stays hidden until the Server's clown box is seen
	   gone or a maze role is dealt; the alive flag also picks the HUD prompt. */
	std::vector<KOUKU_MAZE_TARGET_VIEW> m_CardMazeTargetScratch;
	bool m_bCardMazeClownBoxAlive = false;
	bool m_bCardMazeClownBoxDefeated = false;
	bool m_bCardMazeTelescopeShown = false;
	void Update_DeadScene(f32_t fTimeDelta);
	const KAKUL_CAMERA_SHOT* Find_ActiveCameraShot(
		const float3_t& vPosition) const;
	void Release_CameraShot();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();
	void Update_SourceFollowCamera(f32_t timeDelta, bool_t immediate = false);
	/* Cutscene stage isolation. The whole map is loaded, so a wide cutscene shot sees
	   the other stage areas hundreds of metres away. While a cinematic owns the camera
	   only the stage areas around the camera, what it looks at and the local player are
	   drawn; every other area is suppressed through an overlay flag that never touches
	   the logical visibility gameplay and Sequences own, and the flag is cleared the
	   moment the cinematic ends. */
	void Build_CinematicStageAreas();
	void Update_CinematicSurroundings();
	void Apply_CinematicSurroundings(const std::vector<uint8_t>& keptAreas);
	void Restore_CinematicSurroundings();

#ifdef _DEBUG
	/* The three arena-side `_go` boxes are the only way into the Mario
	   stages and each is 2x1x2m of empty air, so nobody can find them
	   without a wire. This draws them through the same authoring box the
	   Map Editor and Bern already use; it reads the authored document and
	   owns no gameplay state. */
	bool_t Ready_DebugStageEntryTriggers(const std::string& areaId);
#endif

private:
	CMapPlacementRuntime m_MapRuntime;
#ifdef _DEBUG
	bool_t m_bMapAuthoringActive = false;
#endif
	/* The authored deploy catalog carries both paper levers and both paper
	   stage bridges. A bridge stays DESPAWNED until its lever is pulled, so
	   suppress the bridges before the first rendered frame instead of letting
	   them appear already unfolded. */
	CDeployPropRuntime m_DeployRuntime;
	MAP_LIGHT_COMPARISON m_eMapLightComparison = MAP_LIGHT_COMPARISON::CURRENT;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightComparisonSource;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightComparisonGate;
#ifdef _DEBUG
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightComparisonPopup;
#endif
	size_t m_iMapLightComparisonGate = 0u;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightAuthoringOverride;
#ifdef _DEBUG
	std::shared_ptr<CMapLightPresentationRuntime> m_pCompositionMapLightPreview;
	std::optional<CMapLightDocument> m_CompositionMapLightSource;
	bool_t m_bCompositionMapLightPreviewActive = false;
	std::string m_strCompositionWorldPreviewFailurePattern;
	std::string m_strCompositionWorldPreviewFailure;
	void Debug_InvalidateCompositionMapLights();
#endif
	CWorldSequencePlayer m_SequencePlayer;
    struct COMPLETE_PLAY_PREPARATION final
    {
        std::vector<std::string> selectedPatterns, selectedBundles;
        KOUKU_SAYDON_PLAY_RESOURCES resources;
        std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft;
        uint32_t sourceRevision = 0u;
        bool wholeRaid = false;
        uint64_t v1Revision = 0u, v2Generation = 0u, worldRevision = 0u;
        struct WORLD_CLONE_RESERVATION final
        {
            std::string instanceId;
            uint32_t copies = 0u;
        };
        std::vector<WORLD_CLONE_RESERVATION> worldCloneReservations;
        std::vector<std::string> worldSubsetIds;
        size_t worldCloneCount = 0u;
        size_t actorIndex = 0u, v2Index = 0u, worldIndex = 0u;
        size_t worldCloneIndex = 0u, worldSubsetIndex = 0u;
    };
    std::optional<COMPLETE_PLAY_PREPARATION> m_CompletePlayPreparation;

	bool_t m_bWorldObjectReloadPending = false;
	struct OWNED_WORLD_CUE final
	{
		std::uint32_t runEpoch = 0, patternSequence = 0, startTick = 0, durationMs = 0;
		std::string memberId, cueId, occurrenceId, sequenceId;
		float clockMs = 0.f;
		bool untilDestroyed = false;
		LostArk::Shared::NET_ENTITY_ID combatBodyNetEntityId = LostArk::Shared::INVALID_NET_ENTITY_ID;
		WORLD_EMISSION_ANCHOR emissionAnchor;
		std::shared_ptr<CWorldSequencePlayer> player;
	};
	WORLD_EMISSION_RESOLVER m_WorldEmissionResolver;
	std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_PendingOwnedWorldCues;
	std::map<std::string, OWNED_WORLD_CUE> m_OwnedWorldCues;
	std::set<std::string> m_StoppedWorldOwners;
	std::set<std::string> m_FinishedWorldOwners;
	std::set<std::string> m_ConsumedWorldCueIds;
	std::uint32_t m_iLatestWorldRunEpoch = 0u;
	void Consume_OwnedWorldCue(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play,
		const CWorldSequencePlayer::TARGET_SET& targets);
#ifdef _DEBUG
	unique_ptr<CWorldSequencePlayer> m_pWorldObjectPreview;
	std::vector<std::string> m_WorldObjectPreviewInstances;
	struct WORLD_OBJECT_PREVIEW_SCREEN_EFFECT final
	{
		uint32_t handle = 0u;
		f32_t startDelayMs = 0.f, playbackSpeed = 1.f, durationMs = 0.f;
	};
	std::vector<WORLD_OBJECT_PREVIEW_SCREEN_EFFECT> m_WorldObjectPreviewScreenEffects;
	std::string m_strWorldObjectPreviewNotice;
#endif
	struct GATE_OBJECT_PRESENTATION final
	{
		size_t gateIndex = NO_ACTIVE_DEBUG_GATE;
		unique_ptr<CWorldSequencePlayer> player;
		std::vector<std::pair<std::string, f32_t>> instances;
		struct VISIBILITY final { uint64_t placementId; bool_t previous, applied; };
		std::vector<VISIBILITY> visibility;
		std::optional<DEPLOY_PROP_STATE> previousLegacyBook;
		bool_t suspended = true;
		bool_t serverRaidPrepared = false;
	};
	unique_ptr<GATE_OBJECT_PRESENTATION> m_pPendingGateObjects;
	unique_ptr<GATE_OBJECT_PRESENTATION> m_pGateObjects;
	// Borrow only the existing G1 owner; no second World playback path is created.
	GATE_OBJECT_PRESENTATION* m_pServerRaidCinematicBorrowedGateObjects = nullptr;
#ifdef _DEBUG
	GATE_OBJECT_PRESENTATION* m_pWorldObjectPreviewBorrowedGateObjects = nullptr;
	bool_t m_bCompositionWorldPreviewBorrowsGateObjects = false;
#endif
	bool_t Debug_PrepareGateObjects(size_t gateIndex, std::string& status);
	bool_t Debug_CommitGateObjects(size_t gateIndex, std::string& status);
	void Debug_CancelGateObjects();
	void Debug_StopGateObjects();
	void Debug_UpdateGateObjects(f32_t delta);
	bool_t Debug_StartGateObjectPresentation(GATE_OBJECT_PRESENTATION& state, std::string& status);
	bool_t Debug_ReleaseGateObjectPresentation(GATE_OBJECT_PRESENTATION& state, std::string& status);
	bool_t Debug_SetGateObjectsSuspended(bool_t suspended, std::string& status);
#ifdef _DEBUG
	struct COMPOSITION_WORLD_PREVIEW_PLAYBACK final
	{
		COMPOSITION_WORLD_PREVIEW_CUE cue;
		unique_ptr<CWorldSequencePlayer> player;
	};
	std::map<std::string, COMPOSITION_WORLD_PREVIEW_PLAYBACK> m_CompositionWorldPreviewCues;
	struct COMPOSITION_WORLD_PREVIEW_DEPLOY_STATE final
	{
		uint64_t placementId = 0u;
		DEPLOY_PROP_STATE previousState = DEPLOY_PROP_STATE::INTACT;
		DEPLOY_PROP_STATE appliedState = DEPLOY_PROP_STATE::INTACT;
	};
	// Preview borrows bound Deploy states, a replaced book and exclusive arena visibility.
	std::vector<COMPOSITION_WORLD_PREVIEW_DEPLOY_STATE> m_CompositionWorldPreviewDeployStates;
	std::vector<std::pair<uint64_t, bool_t>> m_CompositionWorldPreviewArenaVisibility;
	std::string m_strCompositionWorldPreviewPattern;
	bool_t m_bCompositionWorldPreviewClockBound = false;
	bool_t m_bCompositionWorldPreviewStandingArenaVisible = false;
#endif
	bool_t m_bCutsceneBossVisible = false;
	bool_t m_bCutsceneSetVisible = false;
	std::unordered_set<uint64_t> m_RaisedPaperBridges;
	shared_ptr<CCamera_Free> m_pCamera;
	weak_ptr<CCharacter> m_pCameraTarget;
	ARENA_CAMERA_PROFILE m_FollowCameraProfile =
		CArenaCameraProfile::Default(ARENA_CAMERA_MAP::KOUKU_SAYDON);
	ARENA_CAMERA_PROFILE m_EffectiveFollowCameraProfile = m_FollowCameraProfile;
	bool_t m_bSourceCameraInitialized = false;
	bool_t m_bInsideSourceCameraEntrance = false;
	float3_t m_vSourceCameraPreviousPlayer{};
	f32_t m_fSourceCameraBlendFromDistance = 16.f;
	f32_t m_fSourceCameraBlendElapsed = 3.f;
	std::string m_strFollowCameraProfileStatus;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
	/* Same over-head name + HP gauge as Bern/Valtan (this room has nicknames too). */
	CWorldPlayerNameplateView m_PlayerNameplateView;
	CWorldPlayerChatBubbleView m_ChatBubbleView;
	CPartyInteractionView m_PartyTransferNotice;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	std::vector<KAKUL_STAGE_MARKER> m_StageMarkers;
	std::unordered_set<std::string> m_StageMarkerPlacementIds;
	std::vector<KAKUL_CAMERA_SHOT> m_CameraShots;
	std::vector<KAKUL_CAMERA_SHOT> m_AuthoringCameraShots;
	std::string m_strCameraAuthoringBaseline;
	std::set<std::string> m_DirtyCameraShotIds;
	bool_t m_bCameraAuthoringLoaded = false;
	// A failed first load is retried only by the authoring Reload command.
	bool_t m_bCameraAuthoringLoadAttempted = false;
	std::string m_strCameraAuthoringLoadFailure;
	struct COMPOSITION_CAMERA_TRANSITION final
	{
		std::string ownerKey;
		std::string shotId;
		bool_t cinematicTrack = false;
		std::string cancelledOwnerKey;
		std::string finishedOwnerKey; // Suppress Area camera reacquisition while this row drains.
		VALTAN_CINEMATIC_CAMERA_POSE fromPose;
		VALTAN_CINEMATIC_CAMERA_POSE entryPose;
		f32_t lastSeconds = -1.f;
		VALTAN_CINEMATIC_CAMERA_POSE appliedPose;
		VALTAN_CINEMATIC_CAMERA_EASING easing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		uint32_t blendOutMs = 0u;
		f32_t returnSeconds = 0.f;
		bool_t returning = false;
		bool_t followAtStart = true;
	} m_CompositionCamera;
	std::string m_strActiveCameraShotId;
	std::string m_strCinematicDiagnosticKey;
	struct CINEMATIC_STAGE_AREA final
	{
		f32_t fMinX = 0.f;
		f32_t fMaxX = 0.f;
		f32_t fMinZ = 0.f;
		f32_t fMaxZ = 0.f;
	};
	std::vector<CINEMATIC_STAGE_AREA> m_CinematicStageAreas;
	std::unordered_map<uint64_t, uint32_t> m_CinematicAreaOfPlacement;
	std::vector<uint8_t> m_CinematicKeptAreas;
	std::vector<uint8_t> m_CinematicKeptScratch;
	bool_t m_bCinematicSurroundingsApplied = false;
	std::size_t m_iCinematicSuppressedCount = 0u;
	uint64_t m_iCinematicOwnedSignature = 0u;
	/* The pose written last frame. A hand-over starts from this, so entering,
	   swapping and leaving all begin at what the player already sees. */
	float3_t m_vCameraEyeApplied = {};
	float3_t m_vCameraLookApplied = {};
	f32_t m_fCameraFovApplied = 60.f;
	float3_t m_vCameraEyeFrom = {};
	float3_t m_vCameraLookFrom = {};
	f32_t m_fCameraFovFrom = 60.f;
	float3_t m_vCameraEyeTo = {};
	float3_t m_vCameraLookTo = {};
	f32_t m_fCameraFovTo = 60.f;
	f32_t m_fCameraBlendSeconds = 0.f;
	f32_t m_fCameraBlendElapsed = 0.f;
	bool_t m_bCameraShotHeld = false;
	std::string m_strCameraShotStatus;
	/* Presented gate; a completed Server raid can retain this scene after
	   despawning its bosses. Debug approval is tracked separately below. */
	size_t m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	string m_strGatePresentationProfileId;
    size_t m_iGateLightingIndex = NO_ACTIVE_DEBUG_GATE;
    std::shared_ptr<CMapLightPresentationRuntime> m_pGateMapLightPresentation;
    std::uint32_t m_iServerRaidGatePresentationEpoch = 0u;
    std::optional<CMapLightDocument> m_GateMapLightSource;
    std::shared_ptr<CMapLightPresentationRuntime> m_pPendingGateMapLights;
    std::optional<CMapLightDocument> m_PendingGateMapLightSource;
    struct SERVER_RAID_ENVIRONMENT_BASELINE final
    {
        size_t gateLightingIndex = NO_ACTIVE_DEBUG_GATE;
        string profileId;
        std::shared_ptr<CMapLightPresentationRuntime> lights;
        std::optional<CMapLightDocument> source;
    };
    std::optional<SERVER_RAID_ENVIRONMENT_BASELINE> m_ServerRaidEnvironmentBaseline;
    struct SERVER_ENCORE_VIEW final
    {
        uint32_t runEpoch = 0u, startTick = 0u;
        VALTAN_CINEMATIC_CAMERA_POSE heldPose;
        VALTAN_CINEMATIC_CAMERA_CUE authoredTrack;
        uint32_t blendOutMs = 0u;
        VALTAN_CINEMATIC_CAMERA_EASING easing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
    };
    std::optional<SERVER_ENCORE_VIEW> m_ServerEncoreView;
    bool_t Begin_ServerEncoreView(std::string& status);
    bool_t Acquire_ServerEncoreView(std::string& status);

	bool_t m_bSequenceCombatPending = false;
	bool_t m_bSequenceCombatFadeHeld = false;
	std::string m_strDebugGateStatus =
		"Choose a gate. The Server raises its bosses and moves only your player.";
	/* Debug gate command sequence and the accumulated Server replies shown in
	   the F1 arena panel. Session state only; never persisted. */
	bool m_bDebugGazeView = false;
	float m_fDebugGazeHalfAngle = 45.f;
	float m_fDebugGazeDistance = 30.f;
	struct DEBUG_GATE_APPROVAL_SCOPE
	{
		std::uint64_t iWorldGeneration = 0u;
		std::uint32_t iRaidEpoch = 0u;
	};
	DEBUG_GATE_APPROVAL_SCOPE m_PendingDebugGateApproval, m_DebugGateApproval;
	std::uint32_t m_iNextDebugGateRequestSequence = 1u;
	size_t m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
	std::map<std::string, std::uint64_t> m_DebugGatePendingPlacements;
	bool_t m_bDebugGateFailed = false;
	bool_t m_bDebugGatePreservesPlayerPosition = false;
	bool_t m_bDebugStartPending = false;
	bool_t m_bDebugStartSucceeded = false;
	f32_t m_fDebugGatePendingSeconds = 0.f;
	/* Last F1 status-word preview serial already turned into a word. */
	std::uint32_t m_iStatusEffectTextPreviewSerial = 0u;
	/* One full-screen slot, black, whose alpha is the whole effect. Built
	   hidden so the first rendered frame after activation cannot flash it. */
	unique_ptr<CUILayoutRuntime> m_pTriggerMoveFadeView;
	unique_ptr<CUILayoutRuntime> m_pDeadSceneView;
	/* Madness gauge under the local character. Reads CCombatHUDViewModel's
	   KoukuSaydon gimmick state only; hidden while that state is invalid. */
	unique_ptr<CKoukuMadnessGaugeView> m_pMadnessGaugeView;
	/* The same gauge over every other player in the room (a room holds four), fed from the
	   replicated per-player madness of the world snapshot. */
	std::array<unique_ptr<CKoukuMadnessGaugeView>, 3> m_OtherMadnessGaugeViews;
	/* Floating status word over a head (currently the Server FEAR state). Owns no
	   gameplay truth: Update submits one word per replicated FEAR occurrence and
	   Render draws whatever is still inside its motion. */
	CStatusEffectTextView m_StatusEffectTextView;
	/* Raid-clear MVP award page. Preview only for now: nothing in this Level
	   shows it, the F1 Developer Tools do. */
	unique_ptr<CMvpResultView> m_pMvpResultView;
	/* Dungeon-clear celebration. KoukuSaydon drives its own keyframe document
	   rather than the fixed-rect one Valtan uses, because every layer of the Set
	   animates its position, size and tint frame by frame. */
	unique_ptr<CUILayoutRuntime> m_pRaidClearView;
	/* Negative until a clear starts. */
	f32_t m_fRaidClearElapsedSeconds = -1.f;
	bool_t m_bRaidClearShowMvp = true;
	uint8_t m_iPendingRaidMvpGate = 0u;
	void Update_RaidClear(f32_t fTimeDelta);
	/* Commander raid gate progress. The Server owns the cleared mask, the vote and the gate
	   switch (S2C_GATE_PROGRESS_STATE); this Level shows the panel, starts the clear mark
	   when a gate clears, offers the proceed / vote prompt after the award page, and applies
	   the presentation of whichever gate the Server raised. */
	CRaidGateProgressView m_GateProgressView;
	/* Retail "G" keycap over the interact-gated trigger box the player walks up to. */
	CInteractKeyPromptView m_InteractKeyPrompt;
	LostArk::Shared::S2C_GATE_PROGRESS_STATE m_GateProgress{};
	bool_t m_bGateProgressKnown = false;
	bool_t m_bGateVoteAnswered = false;
	bool_t m_bMvpWasVisible = false;
	/* The Server's last raid-clear award input (S2C_RAID_MVP_RESULT). Fresh until the
	   clear's award page shows it; kept afterwards so the Debug page can replay it. */
	LostArk::Shared::S2C_RAID_MVP_RESULT m_RaidMvpResult{};
	bool_t m_bHasRaidMvpResult = false;
	bool_t m_bRaidMvpResultFresh = false;
	/* The character each award panel shows (0 = MVP, 1..3 the columns), resolved from
	   the result's players when the page opens. */
	weak_ptr<CCharacter> m_MvpStageCharacters[4];
	/* Opens the award page from the Server result; bReplayLast reuses an already shown
	   one. With no result at all only a Debug build shows the sample page. */
	void Show_MvpResult(bool_t bReplayLast);
	std::uint32_t m_iNextGateRequestSequence = 1u;
	std::array<EFFECT_WORLD_ROOT_HANDLE, 2> m_Gate3AuraHandles{};
	std::array<std::string, 2> m_Gate3AuraAttempted{};
	std::string m_strGate3AuraFailure;
	std::uint32_t m_iGate3AuraStartTick = 0u;
	std::uint32_t m_iGate3AuraRunEpoch = 0u;
	bool_t m_bGate3AuraOccupied = false;
	std::vector<LostArk::Shared::NET_ENTITY_ID> m_Gate3AuraParticipants;
	f32_t m_fGate3AuraSecondsLeft = 10.f;
	void Update_Gate3EntryAura(bool_t entryAvailable);
	void Submit_Gate3Auras();
	void Clear_Gate3Auras();
	// Attempt once on each playback edge; missing media never retries every frame.
	bool_t m_bRaidBgmInitialized = false;
	std::wstring m_strRaidBgmWanted;
	bool_t m_bLocalSequencePlaybackActive = false;
	bool_t m_bRaidBgmStarted = false;
	std::uint32_t m_iReadyTerraceObservedRunEpoch = 0u;
	LostArk::Shared::KOUKUSAYDON_RAID_PHASE m_eReadyTerraceObservedPhase = LostArk::Shared::KOUKUSAYDON_RAID_PHASE::INACTIVE;
	bool_t Try_GetReplicatedLocalPlayerPosition(float3_t& outPosition) const;
	void Start_RaidBgm(const wchar_t* assetId);
	void Stop_RaidBgm();
	void Update_RaidBgm();
	void Update_GateProgress(f32_t fTimeDelta);
	void Apply_GateProgressState(const LostArk::Shared::S2C_GATE_PROGRESS_STATE& State);
	/* Both Server gate routes share the same object / lighting commit. Active Raid
	   presentation waits for its cinematic clock before this owner changes. */
	void Apply_ServerGate(size_t gateIndex);
	bool_t Prepare_GatePresentation(size_t gateIndex, std::string& status);
	bool_t Commit_GatePresentation(size_t gateIndex, std::string& status);
	bool_t Is_ServerRaidActive() const;
	bool_t Is_LocalGateParticipant() const;
	bool_t Can_InteractGateProgress() const;
	bool_t Is_LocalRaidLeader() const;
	bool_t Is_GateVotePromptOpen() const;
	static CRaidGateProgressView::PROMPT Gate_VotePrompt(LostArk::Shared::GATE_PROGRESS_KIND eKind);
	wstring_t Find_PlayerNickname(LostArk::Shared::NET_ENTITY_ID iNetEntityId) const;
	/* 1-based gate for the award headline. The debug gate index is 0-based and
	   NO_ACTIVE_DEBUG_GATE means none was entered, which reads as gate 1. */
	int32_t Current_GateNumber() const;
	f32_t m_fTriggerMoveFadeAlpha = 0.f;
	/* Speed gate. The short hops share TRIGGER_MOVE with the stage
	   transition, so the fade arms only once the character is seen moving
	   far faster than any hop can. */
	float3_t m_vTriggerMoveFadeLastPosition = {};
	bool_t m_bTriggerMoveFadeHasLastPosition = false;
	bool_t m_bTriggerMoveFadeArmed = false;
	struct ENTRANCE_TRIGGER_MARKER final
	{
		std::string placementId;
		std::string sequenceInstanceId;
		EFFECT_WORLD_ROOT_HANDLE handle;
		float4x4_t rootWorld{};
		f32_t seconds = 0.f;
		bool_t started = false;
		bool_t clockStarted = false;
		bool_t active = false;
		bool_t retired = false;
	};
	std::vector<ENTRANCE_TRIGGER_MARKER> m_EntranceTriggerMarkers;

#ifdef _DEBUG
	std::vector<shared_ptr<CTrigger_Box>> m_DebugStageEntryTriggers;
#endif

	static CLevel_KakulSaydonArena* s_pActiveInstance;

public:
	/* F1 Developer Tools only -- the award page has no gameplay trigger yet. */
	/* Plays the dungeon-clear overlay and hands off to the award page when it ends,
	   the order retail runs them in. */
	/* Starts the dungeon-clear screen and plays its cue. One owner for "the clear
	begins", the way CLevel_ValtanArena::Trigger_RaidClear already is, so the cue
	cannot go missing again when the product path starts calling it. */
	void Trigger_RaidClear();
	void Debug_Play_ClearThenMvp();
	void Debug_Show_MvpResult();
	void Debug_Hide_MvpResult();
	bool_t Debug_Is_MvpResultVisible() const;
	/* The award page's character panels are off-screen draws, so they run in
	   CMainApp's portrait phase rather than with the rest of this level. */
	void Render_MvpPortraits();

public:
	static unique_ptr<CLevel_KakulSaydonArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
```

### C:/Users/tnest/Desktop/LostArk/Client/Private/Level_KakulSaydonArena.cpp 전체 코드

```cpp
#include "Level_KakulSaydonArena.h"
#include "UITextOcclusion.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")
#include "WorldSequenceObject.h"
#include "Effect_PresentationService.h"
#include "EffectFailureDiagnostic.h"
#include "ActorCatalog.h"
#include "KoukuSaydonPresentationAssetService.h"
#include "KoukuSaydonBossTool.h"
#include "KoukuSaydonPresentationPlayer.h"
#include "EffectV2_Runtime.h"
#include "Effect_Catalog.h"
#include "Npc.h"
#include "Model.h"
#include "ActionPresentationTimeline.h"
#include "AnimationTargetService.h"

#include "Camera_Free.h"
#include "CameraTool.h"
#include "MapTool.h"
#include "Character.h"
#include "CombatHUDViewModel.h"
#include "EstherActionSoundCueDocument.h"
#include "DataJson.h"
#include "GameInstance.h"
#include "RuntimeAssetRoot.h"
#include "Profiler.h"
#include "KakulArenaHiddenPlacements.h"
#include "KoukuSaydonPatternAuditionService.h"
#include "KoukuMadnessGaugeView.h"
#include "MvpAwardCatalog.h"
#include "MvpResultView.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MapAssetCatalog.h"
#include "MapAssetRenderUtils.h"
#include "ValtanCinematicCameraController.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "ProjectDataRoot.h"
#include "UILayoutRuntime.h"
#include "UIInputRouter.h"
#include "UILabelFont.h"
#include "MainApp.h"
#include "HitAreaWire.h"
#include "Transform.h"
#include "Trigger_Box.h"
#include "WorldGameplayDocument.h"
#include "Gameplay/KoukuArenaReadyAreas.h"

#include <algorithm>
#include <cstring>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <limits>
#include <unordered_set>
#include <unordered_map>

namespace
{
	/* Three gate icons remain visible; the Server may also expose the Bingo encore. */
	constexpr uint8_t KOUKU_GATE_COUNT = 3u;

	bool_t CinematicShotAllowsGameplay(const std::string_view shotId)
	{
		// The Gate 2 giant Saydon appearance is a combat bundle camera, not a raid transition.
		return shotId == "camera.kouku.pattern.1";
	}

	bool_t CinematicShotShowsPlayers(const std::string_view shotId)
	{
		// Only these authored scenes include the replicated party. The combined
		// Gate 2 clear / Gate 3 entry reveals players only in entry shots 11-18.
		return CinematicShotAllowsGameplay(shotId) ||
			shotId == "1Stage.finale" || shotId == "2Stage.book" ||
			shotId == "kouku.gate1.authored.finale" || shotId == "kouku.gate1.authored.portal" ||
			shotId == "kouku.gate1.authored.book" || shotId.starts_with("kouku.gate1.full.camera.") ||
			shotId.starts_with("kouku.gate2.maze.camera.") ||
			shotId == "kouku.gate2.clear.camera.11" || shotId == "kouku.gate2.clear.camera.12" ||
			shotId == "kouku.gate2.clear.camera.13" || shotId == "kouku.gate2.clear.camera.14" ||
			shotId == "kouku.gate2.clear.camera.15" || shotId == "kouku.gate2.clear.camera.16" ||
			shotId == "kouku.gate2.clear.camera.17" || shotId == "kouku.gate2.clear.camera.18" ||
			shotId.starts_with("kouku.gate3.intro.camera.");
	}

	constexpr const wchar_t* KOUKU_READY_TERRACE_BGM_ASSET_ID =
		L"Sound/KoukuSaton/S_BGM_COMMANDERRAID/bgm_midnightc_ed_m12_ready_terrace_2ndcircus__559227263.wav";

    const wchar_t* Resolve_KoukuRaidBgmAsset(const bool suppressed, const bool readyArea,
        const LostArk::Shared::KOUKUSAYDON_RAID_PHASE phase, const std::string_view gate,
        const std::uint8_t marioStage, const bool maze)
    {
        using PHASE = LostArk::Shared::KOUKUSAYDON_RAID_PHASE;
        if (suppressed || phase == PHASE::COMPLETE || phase == PHASE::ABORTED) return nullptr;
        // Mario/maze come only from this player's replicated mechanic state.
        static constexpr const wchar_t* mario[] = {
            L"Sound/KoukuSaton/Raid/mario1.wav", L"Sound/KoukuSaton/Raid/mario2.wav",
            L"Sound/KoukuSaton/Raid/mario3.wav", L"Sound/KoukuSaton/Raid/mario4.wav" };
        if (marioStage >= 1u && marioStage <= 4u) return mario[marioStage - 1u];
        if (maze) return L"Sound/KoukuSaton/Raid/maze.wav";
        // A G3 entry terrace remains preparation even while the previous gate drains.
        if (readyArea) return KOUKU_READY_TERRACE_BGM_ASSET_ID;
        if (phase == PHASE::COMBAT || phase == PHASE::WAIT_MINIGAME || phase == PHASE::WAIT_GATE)
        {
            if (gate == "GATE1" || gate == "GATE2" || gate == "BINGO")
                return L"Sound/KoukuSaton/S_BGM_COMMANDERRAID/midnightc_ed__398225682.wav";
            if (gate == "GATE3") return L"Sound/KoukuSaton/S_BGM_COMMANDERRAID/midnightc_ed__1053752270.wav";
            return nullptr;
        }
        // The approach spans several G-key jumps below the initial terrace. Its music
        // belongs to the preparation phase, not the small initial spawn volume.
        return KOUKU_READY_TERRACE_BGM_ASSET_ID;
    }

	std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> WorldPlacementFromCue(
		const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play)
	{
		if (!play.bHasPlacement) return {};
		return CWorldSequencePlayer::OBJECT_PLACEMENT{
			{play.fWorldPositionX, play.fWorldPositionY, play.fWorldPositionZ},
			{play.fWorldRotationXDegrees, play.fWorldRotationYDegrees, play.fWorldRotationZDegrees},
			{play.fWorldScaleX, play.fWorldScaleY, play.fWorldScaleZ}};
	}

	// A Composition cue keeps one owner/clock while the existing World group
	// expands into the same independent motions used by the World Object tool.
	std::vector<std::string> CompositionWorldMotions(const CWorldSequenceDocument& document, const std::string& id)
	{
		const auto* group = document.Find_ObjectResource(id);
		if (!group || group->motionInstanceIds.empty()) return {id};
		std::vector<std::string> result;
		for (const auto& member : group->motionInstanceIds)
		{
			const auto* motion = document.Find_Instance(member);
			const auto* sequence = motion ? document.Find_Template(motion->templateId) : nullptr;
			const auto* model = motion && motion->bindings.size() == 1u ?
				document.Find_ObjectResource(motion->bindings.front().targetId) : nullptr;
			if (!motion || !sequence || motion->anchorKind != "WORLD" || motion->walkableSurface ||
				!model || model->modelAssetId.empty() || model->combatBody ||
				!model->motionInstanceIds.empty() || motion->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE ||
				(motion->motionEnd != WORLD_SEQUENCE_MOTION_END::STOP && motion->motionEnd != WORLD_SEQUENCE_MOTION_END::LOOP))
				return {}; // A visual group cannot create unreplicated gameplay objects.
			// Collider tracks are baked into Server pattern geometry; the Client only
			// validates their model bones and draws Debug previews. They remain valid
			// metadata on an otherwise visual group motion.
			if (motion->enabled) result.push_back(member);
		}
		return result;
	}

	void BindCompositionGroupOrigin(const CWorldSequenceDocument& document, const std::string& id,
		CWorldSequencePlayer::TARGET_SET& targets)
	{
		const auto* group = document.Find_ObjectResource(id);
		if (!group || group->motionInstanceIds.empty() || !targets.objectEmissionAnchor) return;
		const auto source = targets.objectEmissionAnchor;
		const auto captured = std::make_shared<std::optional<float4x4_t>>();
		// Child offsets are authored from their parent's endpoint. All member
		// providers share the first successful birth sample; a pending anchor retries.
		targets.objectEmissionAnchor = [source, captured](float, float4x4_t& world)
		{
			if (!*captured)
			{
				float4x4_t first;
				if (!source(0.f, first)) return false;
				*captured = first;
			}
			world = **captured;
			return true;
		};
	}

	enum class WORLD_EFFECT_PREPARATION { READY, PENDING, FAILED };
	WORLD_EFFECT_PREPARATION PrepareCompositionWorldEffects(const CWorldSequenceDocument& document,
		const std::string& id, std::string& status)
	{
		std::set<std::string> assets;
		for (const auto& member : CompositionWorldMotions(document, id))
		{
			const auto* motion = document.Find_Instance(member);
			for (uint32_t depth = 0; motion && depth <= 32u; ++depth)
			{
				const auto* sequence = document.Find_Template(motion->templateId);
				if (!sequence) break; // Normal sequence validation reports malformed data.
				for (const auto& effect : sequence->effectTracks)
					if (effect.resourceKind == "V1_EFFECT") assets.insert(effect.resourceId);
				if (motion->motionEnd != WORLD_SEQUENCE_MOTION_END::NEXT) break;
				motion = document.Find_Instance(motion->nextMotionId);
			}
		}
		if (assets.empty()) return WORLD_EFFECT_PREPARATION::READY;
		const std::vector<std::string> requested(assets.begin(), assets.end());
		std::vector<std::string> queued;
		if (!CEffectPresentationService::Queue_ProductTargets_Priority(requested, queued, status))
			return WORLD_EFFECT_PREPARATION::FAILED;
		const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(requested);
		if (probe.iFailedCount || probe.iUnavailableCount)
		{
			status = "World Object Effect preparation failed: " + id;
			for (const auto& asset : requested)
			{
				const auto failure = CEffectPresentationService::Get_ProductCuePreparationFailure(asset);
				if (!failure.empty()) status += "; " + asset + ": " + failure;
			}
			return WORLD_EFFECT_PREPARATION::FAILED;
		}
		return probe.bCatalogRevisionCurrent && probe.bSettled ?
			WORLD_EFFECT_PREPARATION::READY : WORLD_EFFECT_PREPARATION::PENDING;
	}

	f32_t CompositionWorldSpan(const CWorldSequencePlayer& player, const std::string& id,
		const f32_t speed, const uint32_t duration)
	{
		f32_t span = 0.f;
		for (const auto& member : CompositionWorldMotions(player.Get_Document(), id))
			span = (std::max)(span, player.Get_InstanceElapsedSpanMs(member, speed, duration));
		return span;
	}

	bool_t PrepareCompositionWorld(CWorldSequencePlayer& player, const std::string& id,
		const CWorldSequencePlayer::TARGET_SET& targets,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement, std::string& status)
	{
		const auto members = CompositionWorldMotions(player.Get_Document(), id);
		if (members.empty()) { status = "WORLD group has no enabled motions: " + id; return false; }
		for (const auto& member : members)
			if (!player.Prepare_InstanceResources(member, targets) ||
				!player.Validate_ObjectPlacement(member, placement, status))
			{ if (status.empty()) status = player.Get_Status(); return false; }
		return true;
	}

	bool_t PlayCompositionWorld(CWorldSequencePlayer& player, const std::string& id,
		const CWorldSequencePlayer::TARGET_SET& targets, const f32_t speed,
		const float3_t& offset, const uint32_t duration,
		const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement)
	{
		const auto members = CompositionWorldMotions(player.Get_Document(), id);
		if (members.empty()) return false;
		for (const auto& member : members)
			if (!player.Is_Playing(member) && !player.Play(member, targets, speed, offset, duration, placement))
			{ player.Stop_All(targets, true); return false; }
		return true;
	}

	bool_t CompositionWorldPivot(const CWorldSequencePlayer& player, const std::string& id,
		float4x4_t& out, const uint32_t emissionIndex = 0u, const std::string& bone = {},
        const bool_t boneRotation = false, const std::string& effectTrackId = {})
	{
		const auto* group = player.Get_Document().Find_ObjectResource(id);
		if (!group || group->motionInstanceIds.empty()) return player.Try_GetSequencePivot(id, out, emissionIndex, bone, boneRotation, effectTrackId);
		bool_t found = false;
		for (const auto& member : CompositionWorldMotions(player.Get_Document(), id))
		{
			float4x4_t visible;
			if (!player.Try_GetObjectPivot(member, visible)) continue;
			float4x4_t pivot;
			if (!player.Try_GetSequencePivot(member, pivot, emissionIndex, bone, boneRotation, effectTrackId)) continue;
			if (found) return false; // Parallel visible motions have no unique effect anchor.
			out = pivot; found = true;
		}
		return found;
	}

	// Each arena remembers its own chosen speed for this process session.
	f32_t g_KakulSaydonFreeCameraSpeed = CCamera_Free::DEFAULT_ARENA_MOVE_SPEED;
	constexpr std::string_view KAKULSAYDON_AREA_ID =
		"LV_LUT_MIDNIGHTC_ED";
	/* Runtime placement IDs from the authored deploy placements. Rows 1 and 4
	   are the paper stage bridges; rows 2 and 3 are the levers that raise them
	   and stay visible from the first frame. */
	constexpr std::array<uint64_t, 2> KAKULSAYDON_PAPER_BRIDGE_PLACEMENT_IDS = {
		1ull,
		4ull,
	};
	/* One lever raises exactly one bridge. The authored sequence instances play
	   the lever pull and the bridge unfold, so the level only decides when. */
	struct PAPER_BRIDGE_LINK final
	{
		uint64_t leverPlacementId;
		uint64_t bridgePlacementId;
		std::string_view leverSequenceInstanceId;
		std::string_view bridgeSequenceInstanceId;
	};
	constexpr std::array<PAPER_BRIDGE_LINK, 2> KAKULSAYDON_PAPER_BRIDGE_LINKS = {
		PAPER_BRIDGE_LINK{ 2ull, 1ull,
			"world.sequence.instance.3", "world.sequence.instance.1" },
		PAPER_BRIDGE_LINK{ 3ull, 4ull,
			"world.sequence.instance.6", "world.sequence.instance.5" },
	};
	/* Map placement IDs the circus finale raises. They are authored standing so
	   the Map Tool can edit them in place, so the level suppresses them here
	   instead of letting the arena open with the finale already assembled. The
	   paper wall is deliberately absent: it must stand until the sequence
	   topples it. */
	constexpr std::array<uint64_t, 22> KAKULSAYDON_CIRCUS_FINALE_PLACEMENT_IDS = {
		8ull, 10ull, 11ull, 12ull, 13ull, 14ull, 15ull, 16ull, 18ull,
		19ull, 20ull, 21ull, 23ull, 24ull, 25ull, 26ull, 27ull, 28ull,
		/* The four stage curtains sweep in at the end of the finale, so they
		   stay hidden with the rest instead of framing an empty plaza. */
		33ull, 35ull, 38ull, 39ull,
	};
	constexpr std::string_view STAGE_MARKER_SCHEMA =
		"lostark.kakul-stage-markers-runtime";
	constexpr std::string_view STAGE_SEMANTIC_STATUS =
		"SOURCE_LEVEL_ID_ONLY";

	constexpr std::string_view CAMERA_SHOT_SCHEMA = "lostark.camera-shots";
	constexpr size_t CAMERA_SHOT_MAX_COUNT = 128u;
	constexpr uint32_t CAMERA_SHOT_MAX_BLEND_MS = 10000u;
	constexpr uint32_t CAMERA_SHOT_MAX_PRIORITY = 1000u;
	constexpr f32_t CAMERA_SHOT_MAX_HALF_EXTENT = 1000.f;
	constexpr f32_t CAMERA_SHOT_MAX_COORDINATE = 100000.f;
	/* A shot is released only once the Character stands this far outside its
	   box, so walking the boundary cannot flip the camera every frame. */
	constexpr f32_t CAMERA_SHOT_EXIT_MARGIN = 0.5f;
	/* A cue longer than the cutscene it rides is authoring nonsense, and a
	   key list longer than this is past what one shot can be read as. */
	constexpr uint32_t CAMERA_TRACK_MAX_DURATION_MS = 120000u;
	constexpr size_t CAMERA_TRACK_MAX_KEYFRAMES = 128u;
	constexpr f32_t CAMERA_TRACK_MIN_LOOK_DISTANCE = 0.01f;
	/* Distinct from the Bern and Valtan cinematic owners so the engine's
	   single-owner override never confuses this arena with theirs. */
	constexpr uint64_t KAKULSAYDON_CAMERA_SHOT_OWNER_ID = 0x4B414B554C534854ull;
	/* The framing the telescope owner holds while the card maze runs; it
	   follows the Server role rather than a box or a sequence. */
	constexpr const char* CARD_MAZE_TELESCOPE_SHOT_ID = "cardmaze.telescope";

	bool_t Is_LocalMarioLightingActive()
	{
		// Only the selected camera subject chooses Mario lighting; other remote props do not.
		if (const auto* arena = CLevel_KakulSaydonArena::Get_Active())
			if (const auto* subject = arena->Get_CameraPlayerSnapshot())
				return subject->iMarioStage >= 1u && subject->iMarioStage <= 4u;
		const auto& player = CCombatHUDViewModel::Get().Get_Player();
		return player.isValid && !player.isPreview && player.iMarioStage >= 1u && player.iMarioStage <= 4u;
	}

	bool Is_SequenceSoundAudience(const std::string& instanceId)
    {
        const auto& listener = CCombatHUDViewModel::Get().Get_Player();
        if (!listener.isValid || listener.isPreview) return true;
        constexpr std::array<std::string_view, 4> marioIntros = {
            "world.sequence.instance.mario_m1_intro", "world.sequence.instance.mario_m2_intro",
            "world.sequence.instance.mario_m3_intro", "world.sequence.instance.mario_m4_intro" };
        const auto intro = std::find(marioIntros.begin(), marioIntros.end(), instanceId);
        if (intro != marioIntros.end())
            return listener.iMarioStage == static_cast<std::uint8_t>(intro - marioIntros.begin() + 1u);
        return listener.iMarioStage == 0u;
    }

	bool_t Is_SequenceCameraAudience(const std::string_view instanceId, const std::uint8_t localMarioStage)
	{
		// Shared stage props play for the room; only its admitted participant
		// owns the Mario intro camera. A remote player's stage is not local state.
		constexpr std::array<std::string_view, 4> marioIntros = {
			"world.sequence.instance.mario_m1_intro", "world.sequence.instance.mario_m2_intro",
			"world.sequence.instance.mario_m3_intro", "world.sequence.instance.mario_m4_intro" };
		const auto intro = std::find(marioIntros.begin(), marioIntros.end(), instanceId);
		return intro == marioIntros.end() ||
			localMarioStage == static_cast<std::uint8_t>(intro - marioIntros.begin() + 1u);
	}
	/* The pop-up book cutscene and the boss prop it stages. The boss is
	   presentation only, so it leaves the arena when this sequence ends. */
	constexpr const char* KAKULSAYDON_CUTSCENE_SEQUENCE_ID =
		"world.sequence.instance.original_kouku";
	constexpr uint64_t KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID = 5ull;
	constexpr uint64_t KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID = 7ull;
	/* The card maze telescope visual and the Server clown box archetype that
	   must be broken before it appears (original triggers 2201/2202). */
	constexpr uint64_t KAKULSAYDON_CARD_MAZE_TELESCOPE_PLACEMENT_ID = 8ull;
	constexpr const char* CARD_MAZE_CLOWN_BOX_ARCHETYPE_ID = "MONSTER_KOUKU_CLOWN_BOX";
	/* Every instance whose id starts with this belongs to the same show. */
	constexpr const char* KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX =
		"world.sequence.instance.original_";
	/* The unfolding copy of the tent. These placements are hidden while the
	   arena stands and take over for the length of the cutscene. */
	constexpr uint64_t KAKULSAYDON_CUTSCENE_SET_FIRST_ID = 41ull;
	constexpr uint64_t KAKULSAYDON_CUTSCENE_SET_END_ID = 300ull;
#ifdef _DEBUG
	bool_t Is_PopupBookHoldInstance(const WORLD_SEQUENCE_INSTANCE& instance)
	{
		return instance.motionEnd == WORLD_SEQUENCE_MOTION_END::HOLD &&
			instance.bindings.size() == 1u &&
			instance.bindings.front().targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
			instance.bindings.front().targetId == "world.object.kouku.popup.book";
	}

#endif

	bool_t Same_MapLightSource(const CMapLightDocument& a, const CMapLightDocument& b)
	{
		if (a.Is_Ready() != b.Is_Ready() || a.Get_AreaId() != b.Get_AreaId() ||
			a.Get_FormatVersion() != b.Get_FormatVersion() || a.Get_NextLightOrdinal() != b.Get_NextLightOrdinal() ||
			a.Get_Provenance() != b.Get_Provenance() || a.Get_Lights().size() != b.Get_Lights().size()) return false;
		const auto same3 = [](const float3_t& x, const float3_t& y)
		{ return x.x == y.x && x.y == y.y && x.z == y.z; };
		for (size_t i = 0; i < a.Get_Lights().size(); ++i)
		{
			const auto& x = a.Get_Lights()[i]; const auto& y = b.Get_Lights()[i];
			if (x.lightId != y.lightId || x.sourceLevel != y.sourceLevel || x.sourceObjectId != y.sourceObjectId ||
				x.displayName != y.displayName || x.groupId != y.groupId || x.kind != y.kind || x.receiver != y.receiver ||
				x.staticShadowChannel != y.staticShadowChannel || x.enabled != y.enabled ||
				!same3(x.rotationDegrees, y.rotationDegrees) || !same3(x.position, y.position) ||
				x.innerConeDegrees != y.innerConeDegrees || x.outerConeDegrees != y.outerConeDegrees ||
				x.radiusMeters != y.radiusMeters || x.falloffExponent != y.falloffExponent || x.brightness != y.brightness ||
				x.color.x != y.color.x || x.color.y != y.color.y || x.color.z != y.color.z || x.color.w != y.color.w)
				return false;
		}
		return true;
	}

	bool_t Prepare_PopupMapLights(const CMapLightDocument& source,
		std::shared_ptr<CMapLightPresentationRuntime>& result, std::string& status,
		const bool_t sourceOnly = false)
	{
		if (!source.Is_Ready() || source.Get_FormatVersion() != 2u || source.Get_AreaId() != KAKULSAYDON_AREA_ID)
		{ status = "Popup lighting requires the authored Kouku Area light document."; return false; }
		auto lights = source.Get_Lights();
		if (lights.size() + 32u > CMapLightDocument::MAX_LIGHT_COUNT)
		{ status = "Popup lighting exceeds the Area light limit."; return false; }
		std::unordered_map<std::string, size_t> indices;
		for (size_t i = 0; i < lights.size(); ++i)
			if (!indices.emplace(lights[i].lightId, i).second)
			{ status = "Popup lighting source ID is duplicated: " + lights[i].lightId; return false; }
		std::vector<std::string> copies;
		// The source reference excludes this project-authored spotlight by stable ID.
		if (!sourceOnly) copies.push_back("light.LV_LUT_MIDNIGHTC_ED.1");
		for (unsigned i = 151u; i <= 179u; ++i) copies.push_back("light.kouku.source.sl05." + std::to_string(i));
		copies.push_back("light.kouku.source.sl05.227"); copies.push_back("light.kouku.source.sl05.228");
		std::vector<std::string> excluded = {"light.kouku.source.ps.265", "light.kouku.source.ps.267", "light.kouku.source.ps.268"};
		for (unsigned i = 105u; i <= 110u; ++i) excluded.push_back("light.kouku.source.sl04." + std::to_string(i));
		for (unsigned i = 113u; i <= 117u; ++i) excluded.push_back("light.kouku.source.sl04." + std::to_string(i));
		for (const auto& id : excluded)
		{
			const auto found = indices.find(id);
			if (found == indices.end()) { status = "Popup lighting exclusion is missing: " + id; return false; }
			lights[found->second].enabled = false;
		}
		for (const auto& id : copies)
		{
			const auto found = indices.find(id);
			if (found == indices.end()) { status = "Popup lighting source is missing: " + id; return false; }
			auto copy = lights[found->second];
			copy.lightId = "popup." + id;
			if (!indices.emplace(copy.lightId, lights.size()).second)
			{ status = "Popup lighting copy ID already exists: " + copy.lightId; return false; }
			copy.position.z -= 204.8f;
			if (!std::isfinite(copy.position.z)) { status = "Popup lighting offset is non-finite: " + id; return false; }
			lights.push_back(std::move(copy));
		}
		auto document = source;
		if (!document.Replace_Authored(lights, source.Get_NextLightOrdinal(), status)) return false;
		auto staged = std::make_shared<CMapLightPresentationRuntime>();
		if (!staged->Replace_Document(document)) { status = staged->Get_Status(); return false; }
		result = std::move(staged);
		return true;
	}
    bool_t Prepare_GateMapLights(const CMapLightDocument& source, const size_t gateIndex,
        std::shared_ptr<CMapLightPresentationRuntime>& result, std::string& status,
        const bool_t sourceOnly = false)
    {
        result.reset();
        if (gateIndex != 0u && gateIndex != 2u) return true;
        if (!source.Is_Ready() || source.Get_AreaId() != KAKULSAYDON_AREA_ID)
        { status = "Gate lighting requires the current Kouku Area light document."; return false; }
        auto document = source;
        if (gateIndex == 0u)
        {
            std::shared_ptr<CMapLightPresentationRuntime> popup;
            if (!Prepare_PopupMapLights(source, popup, status, sourceOnly)) return false;
            document = popup->Get_Document();
        }
        auto lights = document.Get_Lights();
        const std::string popupSpotlightId = "popup.light.LV_LUT_MIDNIGHTC_ED.1";
        bool foundPopupSpotlight = false;
        for (auto& light : lights)
        {
            if (gateIndex == 2u)
            {
                // Keep the dark-stage background mask without overriding authored lights.
                const bool backgroundLight = light.groupId == "source_baked_character" ||
                    light.groupId == "source_direct" ||
                    light.lightId == "light.LV_LUT_MIDNIGHTC_ED.1" ||
                    light.lightId == "light.LV_LUT_MIDNIGHTC_ED.2" ||
                    light.lightId == "light.LV_LUT_MIDNIGHTC_ED.3" ||
                    light.lightId == "light.LV_LUT_MIDNIGHTC_ED.4" ||
                    light.lightId == "light.LV_LUT_MIDNIGHTC_ED.5";
                if (backgroundLight) light.enabled = false;
                continue;
            }
            if (light.lightId == "light.LV_LUT_MIDNIGHTC_ED.1" ||
                light.lightId == "light.LV_LUT_MIDNIGHTC_ED.6") light.enabled = false;
            if (light.lightId == popupSpotlightId) { light.enabled = true; foundPopupSpotlight = true; }
        }
        if (gateIndex == 0u && !sourceOnly && !foundPopupSpotlight)
        { status = "Gate spotlight is missing: " + popupSpotlightId; return false; }
        if (!document.Replace_Authored(lights, source.Get_NextLightOrdinal(), status)) return false;
        auto staged = std::make_shared<CMapLightPresentationRuntime>();
        if (!staged->Replace_Document(document)) { status = staged->Get_Status(); return false; }
        result = std::move(staged);
        return true;
    }

	/* The follow camera this level installs. Reused when a shot hands the
	   camera back so the released pose matches the follow pose exactly. */

	const Client::DATA_JSON_VALUE* Required(
		const Client::DATA_JSON_VALUE& object,
		const char* name,
		const Client::DATA_JSON_TYPE type)
	{
		const Client::DATA_JSON_VALUE* value = object.Find(name);
		return nullptr != value && value->Get_Type() == type ? value : nullptr;
	}

	bool Has_ExactProperties(
		const Client::DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> names)
	{
		if (!object.Is_Object() || object.Get_Object().size() != names.size())
			return false;
		for (const std::string_view name : names)
		{
			if (nullptr == object.Find(name))
				return false;
		}
		return true;
	}

	/* The shot object carries optional blocks. Counting the required names
	   and allowing only the known optional ones rejects unknown properties
	   just as strictly as one exact list per combination would. */
	bool Has_ShotProperties(
		const Client::DATA_JSON_VALUE& object,
		const std::initializer_list<std::string_view> required,
		const std::initializer_list<std::string_view> optional)
	{
		if (!object.Is_Object())
			return false;
		size_t known = 0u;
		for (const std::string_view name : required)
		{
			if (nullptr == object.Find(name))
				return false;
			++known;
		}
		for (const std::string_view name : optional)
		{
			if (nullptr != object.Find(name))
				++known;
		}
		return object.Get_Object().size() == known;
	}

	bool Is_StableId(const std::string_view value)
	{
		if (value.empty() || value.size() > 128u ||
			value == "." || value == "..")
		{
			return false;
		}
		return std::all_of(value.begin(), value.end(), [](const unsigned char c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
		});
	}

	bool Is_DisplayText(const std::string_view value)
	{
		return !value.empty() && value.size() <= 512u &&
			std::none_of(value.begin(), value.end(), [](const unsigned char c)
			{
				return c < 0x20u;
			});
	}

	std::filesystem::path Find_CameraShotDocument()
	{
		return Client::CMapAssetCatalog::Get_MapDataRoot() /
			(std::filesystem::path(std::string(KAKULSAYDON_AREA_ID)).wstring() +
				L".camerashots.json");
	}

	bool Read_Float3(
		const Client::DATA_JSON_VALUE* value,
		const f32_t limit,
		float3_t& out)
	{
		if (nullptr == value || !value->Is_Array() ||
			3u != value->Get_Array().size())
		{
			return false;
		}
		f32_t parts[3]{};
		for (size_t index = 0; index < 3u; ++index)
		{
			const Client::DATA_JSON_VALUE& part = value->Get_Array()[index];
			if (!part.Is_Number() || !std::isfinite(part.Get_Number()) ||
				std::abs(part.Get_Number()) > limit)
			{
				return false;
			}
			parts[index] = static_cast<f32_t>(part.Get_Number());
		}
		out = float3_t(parts[0], parts[1], parts[2]);
		return true;
	}

	bool Read_Uint(
		const Client::DATA_JSON_VALUE* value,
		const uint32_t maximum,
		uint32_t& out)
	{
		if (nullptr == value || !value->Is_Number())
			return false;
		const double number = value->Get_Number();
		if (!std::isfinite(number) || number < 0.0 || number > maximum ||
			std::floor(number) != number)
		{
			return false;
		}
		out = static_cast<uint32_t>(number);
		return true;
	}

	/* Same yawed box test the Server applies to trigger boxes, so a shot
	   authored with the trigger tools covers the ground it appears to. */
	bool Read_CameraTrack(
		const Client::DATA_JSON_VALUE& value,
		const std::string& shotId,
		Client::VALTAN_CINEMATIC_CAMERA_CUE& outCue,
		std::string& outStatus)
	{
		if (!Has_ExactProperties(value,
			{ "durationMs", "interpolation", "easing", "keyframes" }))
		{
			outStatus = "KoukuSaydon camera track shape is invalid: " + shotId;
			return false;
		}
		const Client::DATA_JSON_VALUE* interpolation =
			Required(value, "interpolation", Client::DATA_JSON_TYPE::STRING);
		const Client::DATA_JSON_VALUE* easing =
			Required(value, "easing", Client::DATA_JSON_TYPE::STRING);
		const Client::DATA_JSON_VALUE* keyframes =
			Required(value, "keyframes", Client::DATA_JSON_TYPE::ARRAY);
		uint32_t durationMs = 0u;
		if (nullptr == interpolation || nullptr == easing || nullptr == keyframes ||
			!Read_Uint(value.Find("durationMs"), CAMERA_TRACK_MAX_DURATION_MS,
				durationMs) ||
			0u == durationMs ||
			keyframes->Get_Array().empty() ||
			keyframes->Get_Array().size() > CAMERA_TRACK_MAX_KEYFRAMES)
		{
			outStatus = "KoukuSaydon camera track values are invalid: " + shotId;
			return false;
		}
		if ("LINEAR" == interpolation->Get_String())
		{
			outCue.eInterpolation =
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
		}
		else if ("CATMULL_ROM" == interpolation->Get_String())
		{
			outCue.eInterpolation =
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
		}
		else
		{
			outStatus = "KoukuSaydon camera track interpolation is unknown: " + shotId;
			return false;
		}
		if ("LINEAR" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		else if ("SMOOTHSTEP" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		else if ("HOLD" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
		else
		{
			outStatus = "KoukuSaydon camera track easing is unknown: " + shotId;
			return false;
		}
		outCue.strCueId = shotId;
		outCue.strPatternId.clear();
		outCue.strStageId.clear();
		outCue.strStageActionId.clear();
		outCue.iStageIndex = 0u;
		outCue.iDurationMs = durationMs;
		outCue.iTransitionInMs = 0u;
		outCue.iTransitionOutMs = 0u;
		/* The cutscene has no replicated actor to track; the shot is authored
		   in world space and the level's own blend owns the hand-over. */
		outCue.eTrackingMode = Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
		outCue.vTrackingOrigin = float3_t(0.f, 0.f, 0.f);
		outCue.fShakeAmplitude = 0.f;
		outCue.iShakeDurationMs = 0u;
		outCue.Keyframes.clear();
		outCue.Keyframes.reserve(keyframes->Get_Array().size());
		uint32_t previousTimeMs = 0u;
		std::unordered_set<std::string> sceneIds;
		for (const Client::DATA_JSON_VALUE& entry : keyframes->Get_Array())
		{
			if (!Has_ShotProperties(entry,
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }, { "up" }))
			{
				outStatus = "KoukuSaydon camera keyframe shape is invalid: " + shotId;
				return false;
			}
			Client::VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			const Client::DATA_JSON_VALUE* sceneId =
				Required(entry, "sceneId", Client::DATA_JSON_TYPE::STRING);
			const Client::DATA_JSON_VALUE* fov =
				Required(entry, "fovYDegrees", Client::DATA_JSON_TYPE::NUMBER);
			uint32_t timeMs = 0u;
			if (nullptr == sceneId || !Is_StableId(sceneId->Get_String()) ||
				!sceneIds.emplace(sceneId->Get_String()).second ||
				!Read_Uint(entry.Find("timeMs"), durationMs, timeMs) ||
				!Read_Float3(entry.Find("eye"), CAMERA_SHOT_MAX_COORDINATE,
					keyframe.vEye) ||
				!Read_Float3(entry.Find("lookAt"), CAMERA_SHOT_MAX_COORDINATE,
					keyframe.vLookAt) ||
				nullptr == fov || !std::isfinite(fov->Get_Number()) ||
				fov->Get_Number() <= 1.0 || fov->Get_Number() >= 179.0)
			{
				outStatus = "KoukuSaydon camera keyframe values are invalid: " + shotId;
				return false;
			}
			if (outCue.Keyframes.empty())
			{
				if (0u != timeMs)
				{
					outStatus = "KoukuSaydon camera track must start at 0ms: " + shotId;
					return false;
				}
			}
			else if (timeMs <= previousTimeMs)
			{
				outStatus = "KoukuSaydon camera keyframes must advance: " + shotId;
				return false;
			}
			const f32_t dx = keyframe.vLookAt.x - keyframe.vEye.x;
			const f32_t dy = keyframe.vLookAt.y - keyframe.vEye.y;
			const f32_t dz = keyframe.vLookAt.z - keyframe.vEye.z;
			if (CAMERA_TRACK_MIN_LOOK_DISTANCE >
				std::sqrt(dx * dx + dy * dy + dz * dz))
			{
				outStatus = "KoukuSaydon camera keyframe has no view direction: " +
					shotId;
				return false;
			}
			keyframe.strSceneId = sceneId->Get_String();
			if (const auto* up = entry.Find("up"))
			{
				if (!Read_Float3(up, 1.f, keyframe.vUp))
				{ outStatus = "Camera up must be a finite direction: " + shotId; return false; }
				const auto forward = XMVector3Normalize(XMLoadFloat3(&keyframe.vLookAt) - XMLoadFloat3(&keyframe.vEye));
				if (XMVectorGetX(XMVector3LengthSq(XMVector3Cross(XMLoadFloat3(&keyframe.vUp), forward))) < 0.000001f)
				{ outStatus = "Camera up must not be parallel to its view: " + shotId; return false; }
				keyframe.hasUp = true;
			}
			keyframe.iTimeMs = timeMs;
			keyframe.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
			previousTimeMs = timeMs;
			outCue.Keyframes.push_back(std::move(keyframe));
		}
		if (outCue.Keyframes.size() > 1u && outCue.Keyframes.back().iTimeMs != durationMs)
		{
			outStatus = "KoukuSaydon camera track must end at its duration: " + shotId;
			return false;
		}
		return true;
	}

	bool Contains_CameraShot(
		const Client::CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT& shot,
		const float3_t& position,
		const f32_t margin)
	{
		const f32_t deltaX = position.x - shot.vCenter.x;
		const f32_t deltaZ = position.z - shot.vCenter.z;
		const f32_t yaw = XMConvertToRadians(shot.fYawDegrees);
		const f32_t cosine = std::cos(yaw);
		const f32_t sine = std::sin(yaw);
		const f32_t localX = cosine * deltaX - sine * deltaZ;
		const f32_t localZ = sine * deltaX + cosine * deltaZ;
		return std::abs(localX) <= shot.vHalfExtents.x + margin &&
			std::abs(position.y - shot.vCenter.y) <= shot.vHalfExtents.y + margin &&
			std::abs(localZ) <= shot.vHalfExtents.z + margin;
	}

	float3_t Lerp_Float3(const float3_t& from, const float3_t& to, const f32_t t)
	{
		return float3_t(
			from.x + (to.x - from.x) * t,
			from.y + (to.y - from.y) * t,
			from.z + (to.z - from.z) * t);
	}

	std::filesystem::path Find_StageMarkerDocument()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == length || length >= std::size(modulePath))
			return {};

		const std::filesystem::path moduleDirectory =
			std::filesystem::path(modulePath).parent_path();
		const std::filesystem::path fileName =
			L"KAKULSAYDON_ARENA.stagemarkers.json";
		const std::filesystem::path adjacent = moduleDirectory /
			L"DataFiles" / L"World" / fileName;
		if (std::filesystem::is_regular_file(adjacent))
			return adjacent;
		const std::filesystem::path parent = moduleDirectory.parent_path() /
			L"DataFiles" / L"World" / fileName;
		return std::filesystem::is_regular_file(parent) ? parent : adjacent;
	}
}

Client::CLevel_KakulSaydonArena*
	Client::CLevel_KakulSaydonArena::s_pActiveInstance = nullptr;

Client::CLevel_KakulSaydonArena::CLevel_KakulSaydonArena(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel{ pDevice, pContext }
{
	s_pActiveInstance = this;
}

Client::CLevel_KakulSaydonArena::~CLevel_KakulSaydonArena()
{
	Stop_RaidBgm();
	Stop_CompositionCamera(true);
	Clear_EntranceTriggerMarkers();
	Clear_Gate3Auras();
	Clear_JokerTargetMarker();
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	Debug_StopWorldObjectPreview();
	Debug_StopCompositionWorldPreview();
#endif
    Debug_StopGateObjects();
	// The gate focus is this arena's session state; the next level starts neutral.
	CCombatHUDViewModel::Get().Clear_BossFocus();
	for (const auto& player : m_NameplatePlayers)
		if (const auto character = player.pCharacter.lock())
			character->Set_CinematicPresentationSuppressed(false);
	m_PlayerController.Set_LocalCharacter(nullptr);
	m_PlayerController.Set_CommandSink(nullptr);
	m_Replication.Reset();
	m_pWorldEntityCommandSink.reset();
	m_pPlayerCommandSink.reset();
	m_pCameraTarget.reset();
	m_pCamera.reset();
	for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(Make_WorldSequenceTargets(), true);
	m_OwnedWorldCues.clear();
	if (m_pMarioBombPlayer) m_pMarioBombPlayer->Stop_All(Make_WorldSequenceTargets(), true);
	m_pMarioBombPlayer.reset();
	m_MarioBombEmitters.clear();
	m_SequencePlayer.Clear();
	m_pMapLightAuthoringOverride.reset();
	m_pMapLightPresentation.reset();
	m_DeployRuntime.Clear();
	m_MapRuntime.Clear();
}


bool_t Client::CLevel_KakulSaydonArena::Create_CompositionPreviewActor(
    const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern, std::shared_ptr<CNpc>& outActor,
    std::string& status)
{
    CWorldGameplayDocument world;
    if (!world.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
            std::string(KAKULSAYDON_AREA_ID) / "Gameplay.world.json"),
            std::string(KAKULSAYDON_AREA_ID), status)) return false;
    const auto* placement = world.Find(pattern.strTargetBossPlacementId);
    if (!placement || placement->eKind != WORLD_PLACEMENT_KIND::BOSS ||
        CKoukuSaydonCompositionDocument::Resolve_ActorProfileForPlacement(placement->placementId) != pattern.strActorProfileId)
    { status = "Bundle preview target/model is unavailable: " + pattern.strTargetBossPlacementId; return false; }
    const auto* actor = CActorCatalog::Find_Boss(placement->archetypeId);
    const auto level = ETOUI(LEVEL::KAKULSAYDON_ARENA);
    if (!actor || FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(
        m_pDevice, m_pContext, level, placement->archetypeId)))
    { status = "Bundle preview boss model admission failed: " + placement->archetypeId; return false; }
    CNpc::NPC_DESC desc{};
    desc.iPrototypeLevelIndex = level;
    desc.strModelTag = CKoukuSaydonPresentationAssetService::Get_ModelPrototypeTag(placement->archetypeId);
    desc.strShaderTag = L"Prototype_Component_Shader_VtxAnimMeshBinary";
    desc.pIdleClip = actor->presentationClips.idle.c_str();
    desc.vPosition = placement->position;
    desc.fYawDegree = placement->yawDegrees;
    // The Server preserves the live facing even when only position is reset.
    // Start preview from that same authoritative pose, rather than an unrelated spawn yaw.
    std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
    std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
    m_Replication.Collect_KoukuPresentationViews(bosses, players);
    for (const auto& live : bosses)
    {
        if (live.strArchetypeId != placement->archetypeId || live.iOwnerBossNetEntityId ||
            !live.Snapshot.iCurrentHp) continue;
        desc.fYawDegree = live.Snapshot.fYawDegrees;
        if (!pattern.bResetBossToSpawn)
            desc.vPosition = {live.Snapshot.fPositionX, live.Snapshot.fPositionY, live.Snapshot.fPositionZ};
        break;
    }
    if (pattern.ResetBossYawDegrees) desc.fYawDegree = float(*pattern.ResetBossYawDegrees);
    if (pattern.BossMotion)
    {
        const auto& motion = *pattern.BossMotion;
        desc.vPosition = {float(motion.StartPosition[0]), float(motion.StartPosition[1]), float(motion.StartPosition[2])};
        desc.fYawDegree = float(motion.fYawDegrees);
    }
    desc.bSuppressRootMotion = true;
    desc.strWeaponModelTag = CKoukuSaydonPresentationAssetService::Get_WeaponModelPrototypeTag(placement->archetypeId);
    if (!desc.strWeaponModelTag.empty()) desc.pWeaponSocketBone = CKoukuSaydonPresentationAssetService::Get_WeaponSocketBone();
    std::shared_ptr<CGameObject> object;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(level,
        CKoukuSaydonPresentationAssetService::Get_GameObjectPrototypeTag(), level,
        L"Layer_KoukuCompositionPreview", &desc, &object)))
    { status = "Bundle preview actor clone failed: " + placement->placementId; return false; }
    const auto npc = std::dynamic_pointer_cast<CNpc>(object);
    if (!npc || !npc->Get_Model() || !npc->Get_Transform())
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(level, L"Layer_KoukuCompositionPreview", object);
        status = "Bundle preview actor has no model/transform: " + placement->placementId;
        return false;
    }
    npc->Get_Model()->Set_AnimPaused(true);
    outActor = npc;
    return true;
}

void Client::CLevel_KakulSaydonArena::Release_CompositionPreviewActor(const std::shared_ptr<CNpc>& actor)
{
    if (actor) CGameInstance::Get().Remove_GameObject_from_Layer(
        ETOUI(LEVEL::KAKULSAYDON_ARENA), L"Layer_KoukuCompositionPreview", actor);
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Debug_BeginCompositionWorldPreview(
	const std::string& patternId, std::vector<COMPOSITION_WORLD_PREVIEW_CUE> cues,
	std::string& status, const CWorldSequenceDocument* sourceDocument)
{
	if (cues.empty())
	{
		Debug_StopCompositionWorldPreview();
		if (!m_CompositionWorldPreviewDeployStates.empty() || !m_CompositionWorldPreviewArenaVisibility.empty())
		{ status = "Previous WORLD preview state could not be restored."; return false; }
		return true;
	}
	auto targets = Make_WorldSequenceTargets();
	const auto& document = sourceDocument ? *sourceDocument : m_SequencePlayer.Get_Document();
	std::map<std::string, COMPOSITION_WORLD_PREVIEW_PLAYBACK> staged;
	std::set<std::pair<WORLD_SEQUENCE_TARGET_KIND, std::string>> placementBindings;
	bool previewsResourceBook = false;
	bool previewsPopupBook = false;
	for (const auto& cue : cues)
	{
		const auto members = CompositionWorldMotions(document, cue.instanceId);
		const char* rejection = nullptr;
		if (members.empty()) rejection = "group has no enabled motions";
		else if (std::any_of(members.begin(), members.end(), [&](const auto& id) {
			const auto* motion = document.Find_Instance(id); return !motion || !motion->enabled; }))
			rejection = "instance is missing or disabled";
		else if (0u == cue.durationMs) rejection = "duration is zero";
		else if (!std::isfinite(cue.playbackSpeed) || cue.playbackSpeed <= 0.f)
			rejection = "playback speed must be finite and positive";
		else if (!Is_StableId(cue.occurrenceId)) rejection = "occurrence ID is invalid";
		else if (staged.contains(cue.occurrenceId)) rejection = "occurrence ID is duplicated";
		if (rejection)
		{
			status = "WORLD preview " + cue.occurrenceId + ": " + rejection +
				" [instance=" + cue.instanceId + ", revision=" + std::to_string(document.Get_Revision()) +
				", document=" + (sourceDocument ? "supplied snapshot" : "runtime") + "]";
			return false;
		}
		if (!Can_StartCompositionWorld(cue.instanceId, status, &document)) return false;
		for (const auto& member : members)
		{
			const auto* instance = document.Find_Instance(member);
			previewsPopupBook |= Is_PopupBookHoldInstance(*instance);
			for (const auto& binding : instance->bindings)
			{
				if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
					binding.targetId == "world.object.kouku.popup.book") previewsResourceBook = true;
				if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
					!placementBindings.emplace(binding.targetKind, binding.targetId).second)
				{
					status = "WORLD preview occurrences share a mutable map/deploy target: " + cue.occurrenceId;
					return false;
				}
			}
		}
		auto player = make_unique<CWorldSequencePlayer>();
		auto stagedCue = cue;
		auto groupTargets = targets;
		groupTargets.objectEmissionAnchor = cue.emissionAnchor;
		BindCompositionGroupOrigin(document, cue.instanceId, groupTargets);
		stagedCue.emissionAnchor = std::move(groupTargets.objectEmissionAnchor);
		staged.emplace(cue.occurrenceId, COMPOSITION_WORLD_PREVIEW_PLAYBACK{std::move(stagedCue), std::move(player)});
	}
	for (auto& [id, playback] : staged)
	{
		auto& player = *playback.player;
		const auto& cue = playback.cue;
        CWorldSequenceDocument selected;
        // Keep a cue's dependency closure only. Cinematic stop releases these
        // small documents instead of one full Area document per WORLD lane.
        if (!document.Build_PlaybackSubset({cue.instanceId}, selected, status) ||
            !player.Set_Document(selected, targets, status)) return false;
		if (!PrepareCompositionWorld(player, cue.instanceId, targets, cue.placement, status))
		{
			status = "WORLD preview " + cue.occurrenceId + ": " + player.Get_Status();
			return false;
		}
	}
	std::shared_ptr<CMapLightPresentationRuntime> stagedMapLights;
	std::optional<CMapLightDocument> stagedLightSource;
	if (previewsPopupBook)
	{
		const auto& source = m_pMapLightAuthoringOverride ? m_pMapLightAuthoringOverride : m_pMapLightPresentation;
		if (!source) { status = "Popup lighting has no active Area source."; return false; }
		stagedLightSource = source->Get_Document();
		if (!Prepare_PopupMapLights(*stagedLightSource, stagedMapLights, status)) return false;
	}
	Debug_StopCompositionWorldPreview();
	if (!m_CompositionWorldPreviewDeployStates.empty() || !m_CompositionWorldPreviewArenaVisibility.empty())
	{ status = "Previous WORLD preview state could not be restored."; return false; }

	std::vector<COMPOSITION_WORLD_PREVIEW_DEPLOY_STATE> previousDeployStates;
	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> visibleDeployStates;
	bool_t previewsCutsceneSet = false;
	for (const auto& [kind, target] : placementBindings)
	{
		WORLD_SEQUENCE_BINDING binding;
		binding.targetKind = kind;
		binding.targetId = target;
		uint64_t targetId = 0u;
		if (!CWorldSequencePlayer::Try_ParseTargetId(binding, targetId))
		{ status = "WORLD preview target identity is invalid."; return false; }
		if (kind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT)
		{
			const auto object = m_DeployRuntime.Find(targetId);
			if (!object || object->Is_AnimationAuthoringPreviewActive())
			{ status = "WORLD preview Deploy target is missing or already owned."; return false; }
			previousDeployStates.push_back({targetId, object->Get_State(), DEPLOY_PROP_STATE::INTACT});
			visibleDeployStates.emplace_back(targetId, DEPLOY_PROP_STATE::INTACT);
		}
		else if (kind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
			KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= targetId && targetId < KAKULSAYDON_CUTSCENE_SET_END_ID)
			previewsCutsceneSet = true;
	}
	if (previewsResourceBook)
	{
		if (placementBindings.contains({WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT,
			std::to_string(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID)}))
		{ status = "WORLD preview cannot own both the restored and legacy popup book."; return false; }
		const auto book = m_DeployRuntime.Find(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID);
		if (book)
		{
			if (book->Is_AnimationAuthoringPreviewActive())
			{ status = "WORLD preview legacy popup book is already owned."; return false; }
			previousDeployStates.push_back({KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID,
				book->Get_State(), DEPLOY_PROP_STATE::DESPAWNED});
			visibleDeployStates.emplace_back(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
		}
	}
	std::vector<std::pair<uint64_t, bool_t>> previousArenaVisibility;
	if (previewsCutsceneSet)
	{
		// Reuse the Level's exclusive unfolded/standing arena membership. The
		// World player reveals its bound copy; only the standing copy is borrowed.
		for (auto& entry : m_MapRuntime.Get_MutablePlacements())
		{
			if (placementBindings.contains({WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT,
				std::to_string(entry.record.placementId)}) ||
				std::find(KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.begin(), KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end(),
					entry.record.placementId) == KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end()) continue;
			bool_t visible = false;
			if (!CMapPlacementRuntime::Try_GetRuntimeVisible(entry, visible))
			{ status = "WORLD preview could not capture standing arena visibility."; return false; }
			previousArenaVisibility.emplace_back(entry.record.placementId, visible);
		}
	}
	m_CompositionWorldPreviewDeployStates = std::move(previousDeployStates);
	m_CompositionWorldPreviewArenaVisibility = std::move(previousArenaVisibility);
	m_bCompositionWorldPreviewStandingArenaVisible = false;
	m_bCompositionWorldPreviewBorrowsGateObjects = previewsResourceBook || previewsCutsceneSet ||
		placementBindings.contains({WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT,
			std::to_string(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID)});
	if (m_bCompositionWorldPreviewBorrowsGateObjects && !Debug_SetGateObjectsSuspended(true, status))
	{ Debug_StopCompositionWorldPreview(); return false; }
	if (!visibleDeployStates.empty() && !m_DeployRuntime.Set_States(visibleDeployStates))
	{
		status = "WORLD preview could not apply its Deploy states: " + m_DeployRuntime.Get_Status();
		Debug_StopCompositionWorldPreview();
		return false;
	}
	for (const auto& previous : m_CompositionWorldPreviewArenaVisibility)
	{
		auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), previous.first);
		if (!entry || !CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
		{
			status = "WORLD preview could not hide the standing arena.";
			Debug_StopCompositionWorldPreview();
			return false;
		}
	}
	m_pCompositionMapLightPreview = std::move(stagedMapLights);
	m_CompositionMapLightSource = std::move(stagedLightSource);
	m_bCompositionMapLightPreviewActive = false;
	m_CompositionWorldPreviewCues = std::move(staged);
	m_strCompositionWorldPreviewPattern = patternId;
	m_bCompositionWorldPreviewClockBound = false;
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_HasVisibleCompositionWorldBox(const std::string_view occurrenceId) const
{
    const auto found = m_CompositionWorldPreviewCues.find(std::string(occurrenceId));
    float4x4_t pivot;
    if (found == m_CompositionWorldPreviewCues.end()) return false;
    for (const auto& member : CompositionWorldMotions(found->second.player->Get_Document(), found->second.cue.instanceId))
        if (found->second.player->Try_GetObjectPivot(member, pivot)) return true;
    return false;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_SetCompositionWorldPlacement(
	const std::string& occurrenceId, const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement,
	std::string& status)
{
	const auto found = m_CompositionWorldPreviewCues.find(occurrenceId);
	if (found == m_CompositionWorldPreviewCues.end())
	{ status = "WORLD preview occurrence is unavailable: " + occurrenceId; return false; }
	auto& playback = found->second;
	const auto members = CompositionWorldMotions(playback.player->Get_Document(), playback.cue.instanceId);
	for (const auto& member : members)
		if (!playback.player->Validate_ObjectPlacement(member, placement, status)) return false;
	const auto targets = Make_WorldSequenceTargets();
	std::vector<std::string> applied;
	for (const auto& member : members)
		if (playback.player->Is_Playing(member))
		{
			if (!playback.player->Set_ObjectPlacement(member, placement, targets))
			{
				status = playback.player->Get_Status();
				for (const auto& prior : applied)
					(void)playback.player->Set_ObjectPlacement(prior, playback.cue.placement, targets);
				return false;
			}
			applied.push_back(member);
		}
	playback.cue.placement = placement;
	return true;
}

void Client::CLevel_KakulSaydonArena::Debug_StopCompositionWorldPreview()
{
	// Frame providers keep their document until submission releases the last reference.
	m_pCompositionMapLightPreview.reset();
	m_CompositionMapLightSource.reset();
	m_bCompositionMapLightPreviewActive = false;
	m_strCompositionWorldPreviewFailurePattern.clear();
	m_strCompositionWorldPreviewFailure.clear();
	auto targets = Make_WorldSequenceTargets();
	for (auto& [id, playback] : m_CompositionWorldPreviewCues)
		playback.player->Stop_All(targets, true);
	m_CompositionWorldPreviewCues.clear();
	// Release the animation borrow before restoring state: Set_State rejects
	// writes while an animation preview owns the model. Respect later external state.
	for (auto row = m_CompositionWorldPreviewDeployStates.begin(); row != m_CompositionWorldPreviewDeployStates.end();)
	{
		const auto object = m_DeployRuntime.Find(row->placementId);
		if (object && object->Get_State() == row->appliedState &&
			object->Get_State() != row->previousState &&
			!m_DeployRuntime.Set_State(row->placementId, row->previousState))
		{
			OutputDebugStringA(("[KoukuWorldPreview] Deploy restore pending: " + m_DeployRuntime.Get_Status() + "\n").c_str());
			++row;
		}
		else row = m_CompositionWorldPreviewDeployStates.erase(row);
	}
	for (auto row = m_CompositionWorldPreviewArenaVisibility.begin(); row != m_CompositionWorldPreviewArenaVisibility.end();)
	{
		auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), row->first);
		bool_t visible = false;
		if (entry && (!CMapPlacementRuntime::Try_GetRuntimeVisible(*entry, visible) ||
			(visible == m_bCompositionWorldPreviewStandingArenaVisible && visible != row->second &&
				!CMapPlacementRuntime::Set_RuntimeVisible(*entry, row->second))))
		{
			OutputDebugStringA("[KoukuWorldPreview] Standing arena visibility restore pending.\n");
			++row;
		}
		else row = m_CompositionWorldPreviewArenaVisibility.erase(row);
	}
	m_strCompositionWorldPreviewPattern.clear();
	m_bCompositionWorldPreviewClockBound = false;
	if (std::exchange(m_bCompositionWorldPreviewBorrowsGateObjects, false))
	{
		std::string restore;
		if (!Debug_SetGateObjectsSuspended(false, restore))
			OutputDebugStringA(("[KoukuWorldPreview] Gate baseline restore failed: " + restore + "\n").c_str());
	}
}

bool_t Client::CLevel_KakulSaydonArena::Debug_SampleCompositionWorldPreview(
	const std::string& patternId, const bool_t playing, const uint32_t clockMs, std::string& status,
	const decltype(CWorldSequencePlayer::TARGET_SET::bossAnchor)& bossAnchorOverride)
{
	status.clear();
	if (!m_strCompositionWorldPreviewFailure.empty())
	{
		const bool failedOwner = patternId == m_strCompositionWorldPreviewFailurePattern;
		if (failedOwner) status = std::move(m_strCompositionWorldPreviewFailure);
		m_strCompositionWorldPreviewFailure.clear();
		m_strCompositionWorldPreviewFailurePattern.clear();
		if (failedOwner) return false;
	}
	if (m_CompositionWorldPreviewCues.empty()) return true;
	if (!playing || patternId != m_strCompositionWorldPreviewPattern)
	{
		// A pending Animation target admission takes one frame. Wait for that
		// first matching clock; after binding, a changed owner releases WORLD.
		if (m_bCompositionWorldPreviewClockBound) Debug_StopCompositionWorldPreview();
		return true;
	}
	m_bCompositionWorldPreviewClockBound = true;
	auto targets = Make_WorldSequenceTargets();
	const auto replicatedBossAnchor = targets.bossAnchor;
	std::string pendingAnchorStatus;
	bool_t cutsceneMapPending = false;
	bool_t popupBookActive = false;
	bool_t gateBorrowPending = false;
	if (m_bCompositionWorldPreviewBorrowsGateObjects)
	{
		for (const auto& [id, playback] : m_CompositionWorldPreviewCues)
		{
			const auto& cue = playback.cue;
			const auto* instance = playback.player->Get_Document().Find_Instance(cue.instanceId);
			const auto span = CompositionWorldSpan(*playback.player, cue.instanceId, cue.playbackSpeed, cue.durationMs);
			if (!instance || (clockMs >= cue.startMs && clockMs - cue.startMs >= span)) continue;
			for (const auto& binding : instance->bindings)
			{
				uint64_t targetId = 0u;
				gateBorrowPending |= (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE &&
					binding.targetId == "world.object.kouku.popup.book") ||
					(CWorldSequencePlayer::Try_ParseTargetId(binding, targetId) &&
						((binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
							KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= targetId && targetId < KAKULSAYDON_CUTSCENE_SET_END_ID) ||
						 (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::DEPLOY_PLACEMENT && targetId == KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID)));
			}
		}
		if (gateBorrowPending && !Debug_SetGateObjectsSuspended(true, status))
		{ Debug_StopCompositionWorldPreview(); return false; }
	}
	for (auto& [id, playback] : m_CompositionWorldPreviewCues)
	{
		const auto& cue = playback.cue;
		auto& player = *playback.player;
		targets.objectEmissionAnchor = cue.emissionAnchor;
		// Each cue selects its own actor; a previous Model View resolver cannot leak.
		targets.bossAnchor = bossAnchorOverride ? bossAnchorOverride : replicatedBossAnchor;
		if (!bossAnchorOverride && !cue.actorProfileId.empty())
			targets.bossAnchor = [&cue](const std::string& archetype, const std::string& bone,
				CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
			{
				ANIMATION_MODEL_TARGET_VIEW view;
				if (archetype != cue.bossArchetypeId ||
					CKoukuSaydonCompositionDocument::Resolve_ActorProfileId(CAnimationTargetService::Resolve_AssetName()) != cue.actorProfileId ||
					!CAnimationTargetService::Resolve_ModelTarget(ANIMATION_BONE_TARGET::BODY, view))
				{ status = "World Object Boss anchor is waiting for its matching Model View actor: " + archetype; return false; }
				return CWorldSequencePlayer::Resolve_BossBoneAnchor(view.Model, view.BoneRoot, bone, out, status);
			};
		// A cutscene BODY is a World actor, distinct from the selected or replicated boss.
		const auto actorFallback = targets.bossAnchor;
		targets.bossAnchor = [this, actorFallback](const std::string& archetype, const std::string& bone,
			CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& status)
		{
			if (Try_GetCinematicWorldBossAnchor(archetype, bone, out, status)) return true;
			if (!status.empty()) return false;
			return actorFallback && actorFallback(archetype, bone, out, status);
		};
		bool_t bossAnchorPending = false;
		const auto resolveBossAnchor = targets.bossAnchor;
		targets.bossAnchor = [resolveBossAnchor, &bossAnchorPending](const std::string& archetype,
			const std::string& bone, CWorldSequencePlayer::PLAYER_ANCHOR& out, std::string& reason)
		{
			const bool_t resolved = resolveBossAnchor && resolveBossAnchor(archetype, bone, out, reason);
			bossAnchorPending |= !resolved;
			return resolved;
		};
		const auto span = CompositionWorldSpan(player, cue.instanceId, cue.playbackSpeed, cue.durationMs);
		const auto* instance = player.Get_Document().Find_Instance(cue.instanceId);
		if (instance && Is_PopupBookHoldInstance(*instance) &&
			clockMs >= cue.startMs && clockMs - cue.startMs < span) popupBookActive = true;
		if (instance && (clockMs < cue.startMs || clockMs - cue.startMs < span))
			for (const auto& binding : instance->bindings)
			{
				uint64_t targetId = 0u;
				if (binding.targetKind == WORLD_SEQUENCE_TARGET_KIND::MAP_PLACEMENT &&
					CWorldSequencePlayer::Try_ParseTargetId(binding, targetId) &&
					KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= targetId &&
					targetId < KAKULSAYDON_CUTSCENE_SET_END_ID)
					cutsceneMapPending = true;
			}
		if (clockMs < cue.startMs || clockMs - cue.startMs >= span)
		{
			player.Stop_All(targets, true);
			continue;
		}
		if (!PlayCompositionWorld(player, cue.instanceId, targets, cue.playbackSpeed, cue.positionOffset, cue.durationMs, cue.placement))
		{
			status = "WORLD preview failed: " + cue.occurrenceId + ": " + player.Get_Status();
			OutputDebugStringA(("[KoukuWorldPreview] " + status + "\n").c_str());
			Debug_StopCompositionWorldPreview();
			return false;
		}
		if (!player.Seek_AllToMs(static_cast<f32_t>(clockMs - cue.startMs), targets))
		{
			status = "WORLD preview failed: " + cue.occurrenceId + ": " + player.Get_Status();
			OutputDebugStringA(("[KoukuWorldPreview] " + status + "\n").c_str());
			Debug_StopCompositionWorldPreview();
			return false;
		}
		if (bossAnchorPending)
		{
			if (!pendingAnchorStatus.empty()) pendingAnchorStatus += " | ";
			pendingAnchorStatus += "WORLD preview " + cue.occurrenceId + ": " + player.Get_ObjectSampleStatus(cue.instanceId);
		}
	}
	// Finished unfold boxes release their animated copies before the real arena
	// (including its placement lighting) is shown. Scrubbing reverses the swap.
	const bool_t showStandingArena = !cutsceneMapPending;
	if (!m_CompositionWorldPreviewArenaVisibility.empty() &&
		showStandingArena != m_bCompositionWorldPreviewStandingArenaVisible)
	{
		std::vector<MAP_RUNTIME_PLACED_ENTRY*> changed;
		for (const auto& previous : m_CompositionWorldPreviewArenaVisibility)
		{
			auto* entry = CWorldSequencePlayer::Find_Placement(m_MapRuntime.Get_MutablePlacements(), previous.first);
			if (!entry || !CMapPlacementRuntime::Set_RuntimeVisible(*entry, showStandingArena))
			{
				for (auto* applied : changed)
					(void)CMapPlacementRuntime::Set_RuntimeVisible(*applied, m_bCompositionWorldPreviewStandingArenaVisible);
				status = "WORLD preview could not switch the unfolded and standing arena.";
				Debug_StopCompositionWorldPreview();
				return false;
			}
			changed.push_back(entry);
		}
		m_bCompositionWorldPreviewStandingArenaVisible = showStandingArena;
	}
	m_bCompositionMapLightPreviewActive = popupBookActive && m_pCompositionMapLightPreview != nullptr;
	if (m_bCompositionWorldPreviewBorrowsGateObjects && !gateBorrowPending &&
		!Debug_SetGateObjectsSuspended(false, status))
	{ Debug_StopCompositionWorldPreview(); return false; }
	if (!pendingAnchorStatus.empty()) status = std::move(pendingAnchorStatus);
	return true;
}
#endif

HRESULT Client::CLevel_KakulSaydonArena::Initialize()
{
    m_SequencePlayer.Set_SoundAudience(Is_SequenceSoundAudience);
	const auto arenaStarted = GetTickCount64();
	const EFFECT_SLOW_SCOPE_DIAGNOSTIC arenaTiming{"Kouku.Arena.Initialize", {}, {}};
	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope initializeScope(pProfiler, "Level.Kouku.Initialize");
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::KAKULSAYDON_ARENA);
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.MapPlacementCommit");
		if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
			KAKULSAYDON_AREA_ID != pEntry->pMapAreaId ||
			!m_MapRuntime.Load_Area(
				ETOUI(LEVEL::KAKULSAYDON_ARENA),
				pEntry->pMapAreaId,
				pEntry->MapLoadScope))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena] " +
				m_MapRuntime.Get_Status() + "\n").c_str());
			return E_FAIL;
		}
	}
	/* Loader admits this Area's deploy prototypes before activation. Clone the
	   prepared models here; repeating admission would both stall this frame
	   and reject the duplicate prototype tags. */
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.DeployCommit");
		if (!m_DeployRuntime.Load_Area(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			pEntry->pMapAreaId))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][DeployProp] " +
				m_DeployRuntime.Get_Status() + "\n").c_str());
			m_MapRuntime.Clear();
			return E_FAIL;
		}
	}
	if (!Reload_MapLights())
	{
		m_DeployRuntime.Clear();m_MapRuntime.Clear();return E_FAIL;
	}
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.InitialVisibility");
		/* Levers stay INTACT so the player can find them. Each paper stage bridge
		   only exists once its lever is pulled, so suppress it here rather than
		   waiting for the first sequence frame and flashing an unfolded bridge. */
		std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> hiddenBridges;
		hiddenBridges.reserve(KAKULSAYDON_PAPER_BRIDGE_PLACEMENT_IDS.size() + 3u);
		for (const uint64_t placementId : KAKULSAYDON_PAPER_BRIDGE_PLACEMENT_IDS)
			hiddenBridges.emplace_back(placementId, DEPLOY_PROP_STATE::DESPAWNED);
		// The Sequence owns these cinematic copies; no idle duplicate is placed in the arena.
		hiddenBridges.emplace_back(KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
		hiddenBridges.emplace_back(KAKULSAYDON_CUTSCENE_BOOK_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
		// The telescope appears only after the clown box is broken.
		if (m_DeployRuntime.Find(KAKULSAYDON_CARD_MAZE_TELESCOPE_PLACEMENT_ID))
			hiddenBridges.emplace_back(KAKULSAYDON_CARD_MAZE_TELESCOPE_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
		if (!m_DeployRuntime.Set_States(hiddenBridges))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][PaperBridge] " +
				m_DeployRuntime.Get_Status() + "\n").c_str());
			m_DeployRuntime.Clear();
			m_MapRuntime.Clear();
			return E_FAIL;
		}

		// Resolve the initial visibility batch once. emplace preserves the same
		// first-match identity as Find_Placement without 483 full vector scans.
		std::unordered_map<uint64_t, MAP_RUNTIME_PLACED_ENTRY*> placementIndex;
		auto& placements = m_MapRuntime.Get_MutablePlacements();
		placementIndex.reserve(placements.size());
		for (auto& entry : placements)
			placementIndex.emplace(entry.record.placementId, &entry);
		const auto FindInitialPlacement = [&placementIndex](const uint64_t placementId)
		{
			const auto found = placementIndex.find(placementId);
			return found != placementIndex.end() ? found->second : nullptr;
		};

		/* The finale reveals each of these on its own keyframe. Hiding them now
		   costs nothing if the sequence never runs, and a placement the runtime
		   cannot address is reported rather than silently left standing. */
		for (const uint64_t placementId : KAKULSAYDON_CIRCUS_FINALE_PLACEMENT_IDS)
		{
			MAP_RUNTIME_PLACED_ENTRY* const entry = FindInitialPlacement(placementId);
			if (nullptr == entry ||
				!CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
			{
				OutputDebugStringA((
					"[Level_KakulSaydonArena][CircusFinale] placement not hidden: " +
					std::to_string(placementId) + "\n").c_str());
			}
		}

		/* The pop-up book cutscene raises the tent arena, so it must not already be
		   standing when the level opens. The generated list is every placement
		   within 80m of the roulette floor; the circus plaza is 800m away and never
		   overlaps it. */
		for (const uint64_t placementId : KAKUL_ARENA_HIDDEN_PLACEMENT_IDS)
		{
			MAP_RUNTIME_PLACED_ENTRY* const entry = FindInitialPlacement(placementId);
			if (nullptr == entry ||
				!CMapPlacementRuntime::Set_RuntimeVisible(*entry, false))
			{
				OutputDebugStringA((
					"[Level_KakulSaydonArena][Arena] placement not hidden: " +
					std::to_string(placementId) + "\n").c_str());
			}
		}
	}
	Build_CinematicStageAreas();
	// Release prepares every raid prop before gameplay; Debug uses the same
	// Prepare_InstanceResources path when a World sequence is requested.
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.WorldSequence.Load");
		if (FAILED(CGameInstance::Get().Add_Prototype(ETOUI(LEVEL::KAKULSAYDON_ARENA),
			CWorldSequenceObject::PROTOTYPE_TAG, CWorldSequenceObject::Create(m_pDevice, m_pContext))))
			return E_FAIL;
		auto sequenceTargets = Make_WorldSequenceTargets();
		if (!m_SequencePlayer.Load_PreparedArea(pEntry->pMapAreaId, sequenceTargets))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][WorldSequence] " +
				m_SequencePlayer.Get_Status() + "\n").c_str());
			return E_FAIL;
		}
		else
		{
#ifdef _DEBUG
			CProfilerScope prepareScope(pProfiler, "Level.Kouku.JokerCards.Prewarm");
            // Six cards and one joker have no asynchronous Effect dependency.
            // Shared cue players borrow these exact clones from m_SequencePlayer.
			if (!m_SequencePlayer.Prewarm_ObjectInstances("world.object.instance.kouku.card", 6u, sequenceTargets) ||
				!m_SequencePlayer.Prewarm_ObjectInstances("world.object.instance.kouku.joker_card", 1u, sequenceTargets))
			{
				OutputDebugStringA(("[Level_KakulSaydonArena][JokerPrewarm] " +
					m_SequencePlayer.Get_Status() + "\n").c_str());
				return E_FAIL;
			}
#endif
		}
	}

	std::string stageStatus;
	if (!Load_StageMarkers(stageStatus))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] " + stageStatus + "\n").c_str());
		return E_FAIL;
	}

	/* A missing or rejected shot document costs the arena its authored camera
	   only. Entry never depends on it, so report and keep the follow view. */
	if (!Load_CameraShots(m_strCameraShotStatus))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][CameraShot] " +
			m_strCameraShotStatus + "\n").c_str());
	}

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	CClientReplication::DESC replicationDesc{};
	/* Built here rather than on first use so a trigger move never waits on a
	   JSON load, and hidden immediately because Render() can run before the
	   first Update() on the frame this Level is activated. */
	/* Most of this stage's life is EFActorMotion rather than matinee: cards
	   rocking, floor pieces turning. An absent document is not an error. */
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.SelfMotion.Load");
		if (!m_MapRuntime.Load_SelfMotions(std::string(KAKULSAYDON_AREA_ID)))
		{
			OutputDebugStringA(
				"[Level_KakulSaydonArena] Self-motion document was rejected.\n");
		}
	}

	{
		CProfilerScope scope(pProfiler, "Level.Kouku.UI.Create");
		m_pTriggerMoveFadeView = std::make_unique<CUILayoutRuntime>(
			m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
			L"UI/KakulFade/KakulFadeUI.json");
		m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", false);
		m_pTriggerMoveFadeView->Set_SlotCinematicOverlay("KakulFade_Screen", true);
		m_pMadnessGaugeView = std::make_unique<CKoukuMadnessGaugeView>(
			m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));
		for (unique_ptr<CKoukuMadnessGaugeView>& pOther : m_OtherMadnessGaugeViews)
			pOther = std::make_unique<CKoukuMadnessGaugeView>(
				m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));

		m_pDeadSceneView = std::make_unique<CUILayoutRuntime>(
			m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
			L"UI/DeadScene/DeadSceneUI.json");
		m_pDeadSceneView->Set_AllSlotsVisible(false);
	}

	/* Built hidden; only the F1 Developer Tools show it so far. */
	m_pMvpResultView = std::make_unique<CMvpResultView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));

	/* KoukuSaydon's own document rather than the one Valtan drives. Every layer of
	   epicGateCommanderClearSuccess_Set02 animates its position, size and alpha frame by
	   frame, and the light layers are authored white with each Set's colorTransform
	   supplying the raid colour (Kouku pulls blue to 0, which is what makes it gold), so
	   the whole Set is carried as a keyframe document instead of fixed rects.
	   epicgatecommonclear.gfx's MainTimeline places the frame at translateX -6400 twips on
	   a 1920x1080 stage, so local x maps to x - 320 and then the usual 2/3 onto 1280x720;
	   that mapping is already baked into the generated keys. */
	m_pRaidClearView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA), TEXT("Layer_UI"),
		L"UI/RaidClear/RaidClear_Kouku_Layout.json");
	m_pRaidClearView->Set_AllSlotsVisible(false);

	/* Gate progress panel (top-left): three gates of the raid this arena is. */
	m_GateProgressView.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));
	/* GameMsg tip.name.scene_group_index_name_contents_commanderraid_37081_1: the commander
	   name, not the zone name the award page uses. */
	m_GateProgressView.Set_Raid(L"\xAD11\xAE30\xAD70\xB2E8\xC7A5 \xCFE0\xD06C\xC138\xC774\xD2BC",
		CMvpAwardCatalog::Get().Find_DifficultyText("normal"), KOUKU_GATE_COUNT);
	m_GateProgressView.Set_Progress(1u, 0u);
	m_InteractKeyPrompt.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA),
		"LV_LUT_MIDNIGHTC_ED");

	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::KAKULSAYDON_ARENA);
	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::KAKULSAYDON_ARENA);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag = TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag = TEXT("Layer_WorldEntity");
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.Replication.Initialize");
		if (!m_Replication.Initialize(replicationDesc))
			return E_FAIL;
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_pWorldEntityCommandSink = make_shared<CNetworkWorldEntityCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	m_PlayerController.Set_MovementSurfaceResolver([this](const float3_t& origin,
		const float3_t& direction, float3_t& surface)
	{
		const bool_t mapHit = m_MapRuntime.Try_PickMovementSurface(origin, direction, surface);
		const f32_t limit = mapHit ? XMVectorGetX(XMVector3Length(
			XMLoadFloat3(&surface) - XMLoadFloat3(&origin))) :
			(std::numeric_limits<f32_t>::max)();
		// Breakable floors and unfolding bridges are owned by Deploy, not Map placements.
		return m_DeployRuntime.Try_PickMovementSurface(origin, direction, limit, surface) || mapHit;
	});
	m_PlayerController.Set_ItemTargetResolver([this](const float3_t& origin, const float3_t& direction)
	{ return m_Replication.Find_ItemTargetPlayerFromRay(origin, direction); });
	m_ChatBubbleView.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::KAKULSAYDON_ARENA));
#ifdef _DEBUG
	m_PlayerController.Set_DebugMarioJumpEnabled(true);
#endif
	{
		CProfilerScope scope(pProfiler, "Level.Kouku.Controller.Prepare");
		if (!m_PlayerController.Initialize_TargetingPreview(
				ETOUI(LEVEL::KAKULSAYDON_ARENA)) ||
			!m_PlayerController.Initialize_ClickMoveEffect(
				ETOUI(LEVEL::KAKULSAYDON_ARENA)))
		{
			return E_FAIL;
		}
	}

	(void)Load_EntranceTriggerMarkers();

#ifdef _DEBUG
	/* A missing wire is a missing wire, not a reason to keep the arena
	   shut, so this reports and carries on. */
	if (!Ready_DebugStageEntryTriggers(pEntry->pMapAreaId))
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Debug stage entry Trigger Box "
			"presentation failed.\n");
	}
#endif

#ifndef _DEBUG
    std::string raidPreparationStatus;
    if (!Prepare_EntryRaidResources(raidPreparationStatus))
    {
        Write_EffectFailureDiagnostic("Kouku.Loading.RaidPreparationFailed", raidPreparationStatus);
        OutputDebugStringA(("[Level_KakulSaydonArena][RaidPrewarm] " + raidPreparationStatus + "\n").c_str());
        return E_FAIL;
    }
#endif

	Write_EffectFailureDiagnostic("Kouku.Arena.Ready",
		"elapsed_ms=" + std::to_string(GetTickCount64() - arenaStarted));
	return S_OK;
}

void Client::CLevel_KakulSaydonArena::Start_RaidBgm(const wchar_t* assetId)
{
	if (this != s_pActiveInstance) return;
	const std::filesystem::path path = CRuntimeAssetRoot::Resolve(assetId);
	if (!path.empty() && std::filesystem::is_regular_file(path) &&
		SUCCEEDED(CGameInstance::Get().Play_Music(path.wstring(), 1.f, true)))
	{
		m_bRaidBgmStarted = true;
		return;
	}
	// Play_Music stages replacement transactionally; preserve an existing owner
	// if this edge could not load or start its exact resource.
	OutputDebugStringA("[Level_KakulSaydonArena] Raid BGM is unavailable; this playback edge was isolated.\n");
}

void Client::CLevel_KakulSaydonArena::Stop_RaidBgm()
{
	// A newer arena can already be initialized before the previous Level dies.
	// Only this active Level may release the Music channel that it started.
	if (m_bRaidBgmStarted && this == s_pActiveInstance)
		CGameInstance::Get().Stop_Music();
	m_bRaidBgmStarted = false;
}

bool_t Client::CLevel_KakulSaydonArena::Try_GetReplicatedLocalPlayerPosition(float3_t& outPosition) const
{
	const auto localCharacter = m_Replication.Get_LocalCharacter();
	if (!localCharacter) return false;
	std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
	std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
	m_Replication.Collect_KoukuPresentationViews(bosses, players);
	for (const auto& view : players)
	{
		if (view.pCharacter.lock() != localCharacter) continue;
		outPosition = {view.Snapshot.fPositionX, view.Snapshot.fPositionY, view.Snapshot.fPositionZ};
		return std::isfinite(outPosition.x) && std::isfinite(outPosition.y) && std::isfinite(outPosition.z);
	}
	return false;
}

bool_t Client::CLevel_KakulSaydonArena::Is_AtGate3EntryTerrace() const
{
	float3_t position{};
	return Try_GetReplicatedLocalPlayerPosition(position) &&
		LostArk::Shared::Is_KoukuGate3EntryTerrace(position.x, position.y, position.z);
}

void Client::CLevel_KakulSaydonArena::Notify_SequencePlaybackStarted()
{
	m_bLocalSequencePlaybackActive = true;
	Update_RaidBgm();
}

void Client::CLevel_KakulSaydonArena::Notify_SequencePlaybackEnded()
{
	m_bLocalSequencePlaybackActive = false;
	Update_RaidBgm();
}

void Client::CLevel_KakulSaydonArena::Update_RaidBgm()
{
    using PHASE = LostArk::Shared::KOUKUSAYDON_RAID_PHASE;
    const auto& state = Get_KoukuRaidState();
    const auto& player = CCombatHUDViewModel::Get().Get_Player();
    const bool_t localPlayer = player.isValid && !player.isPreview;
    const bool_t cameraPlaying = std::any_of(m_CameraShots.begin(), m_CameraShots.end(),
        [this, &player](const KAKUL_CAMERA_SHOT& shot)
        {
            return m_bCameraShotHeld && shot.strShotId == m_strActiveCameraShotId &&
                shot.hasCameraTrack && !shot.followsPlayer && !shot.strSequenceInstanceId.empty() &&
                Is_SequenceCameraAudience(shot.strSequenceInstanceId, player.iMarioStage) &&
                m_SequencePlayer.Is_Playing(shot.strSequenceInstanceId);
        });
    const bool_t compositionCameraPlaying = m_pCamera && m_CompositionCamera.cinematicTrack &&
        !m_CompositionCamera.ownerKey.empty() &&
        m_pCamera->Is_PresentationOverrideOwnedBy(0x4b4f554b55434f4dull);
    float3_t position{};
    const bool_t inReadyArea = Try_GetReplicatedLocalPlayerPosition(position) &&
        (LostArk::Shared::Is_KoukuArenaStartArea(position.x, position.y, position.z) ||
         LostArk::Shared::Is_KoukuGate3EntryTerrace(position.x, position.y, position.z));
    auto musicPhase = state.ePhase;
    std::string_view musicGate = state.strGateId;
    if (musicPhase == PHASE::INACTIVE && m_iActiveDebugGate < Get_DebugGates().size())
    {
        // F1 and the Release entry UI commit the same Server-approved gate scene.
        // A room without a raid epoch still uses that admitted gate's battle music.
        const auto* id = Get_DebugGates()[m_iActiveDebugGate].pAuditionPlacementId;
        const std::string_view placement = id ? id : "";
        musicGate = placement == "boss.kakulsaydon.g1.saydon" ? "GATE1" :
            placement == "boss.kakulsaydon.g2.kouku" ? "GATE2" :
            placement == "boss.kakulsaydon.g3.saydon" ? "GATE3" :
            placement == "boss.kakulsaydon.bingo.saydon" ? "BINGO" : "";
        if (!musicGate.empty()) musicPhase = PHASE::COMBAT;
    }
    const wchar_t* assetId = Resolve_KoukuRaidBgmAsset(
        !localPlayer || m_bLocalSequencePlaybackActive || m_bSequenceCombatPending || cameraPlaying ||
        compositionCameraPlaying || state.ePhase == PHASE::CINEMATIC,
        inReadyArea, musicPhase, musicGate,
        localPlayer ? player.iMarioStage : 0u,
        localPlayer && player.eKoukuHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE);
    const std::wstring wanted = assetId ? assetId : L"";
    const bool_t newWaitingRun = !wanted.empty() && state.iRunEpoch && state.ePhase == PHASE::WAIT_ENTRY &&
        (state.iRunEpoch != m_iReadyTerraceObservedRunEpoch || m_eReadyTerraceObservedPhase != PHASE::WAIT_ENTRY);
    const bool_t playbackEdge = !m_bRaidBgmInitialized || wanted != m_strRaidBgmWanted || newWaitingRun;
    m_bRaidBgmInitialized = true;
    m_strRaidBgmWanted = wanted;
    m_iReadyTerraceObservedRunEpoch = state.iRunEpoch;
    m_eReadyTerraceObservedPhase = state.ePhase;
    if (!playbackEdge) return;
    // One Level owns the Music channel; cinematic SOUND occurrences retain their own handles.
    // Missing media is isolated once per edge, never retried on every snapshot.
    if (assetId) Start_RaidBgm(assetId);
    else Stop_RaidBgm();
}

void Client::CLevel_KakulSaydonArena::Update(const f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
			LEVEL::KAKULSAYDON_ARENA))
	{
		return;
	}

	if (!m_Replication.Update())
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Failed to apply replication event.\n");
	}
	CEstherActionSoundCueDocument::Update_SoundAudience();
	m_PartyTransferNotice.Update_TransferNotice(m_Replication);
	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_InteractKeyPrompt.Update(fTimeDelta, m_Replication.Get_LocalCharacter(),
		CCombatHUDViewModel::Get().Get_InteractPromptTriggerId(),
		nullptr == m_pMvpResultView || !m_pMvpResultView->Is_Visible());
	if (m_Replication.Has_PendingConnectionLoss())
	{
		CLevelTransitionService::Report_NetworkRecovery(
			"level-kakul-saydon.network-connection-lost",
			"KoukuSaydon replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Lobby recovery request was rejected; retrying.\n");
	}

	if (!Bind_CameraToLocalCharacter())
	{
		OutputDebugStringA(
			"[Level_KakulSaydonArena] Failed to bind local character camera.\n");
	}
	Update_SourceFollowCamera(fTimeDelta);
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	// Avatar replacement keeps the same Server player and command sequences.
	m_PlayerController.Rebind_LocalCharacter(localCharacter);
#ifdef _DEBUG
	if (m_bDebugGazeView && localCharacter && localCharacter->Get_Transform())
	{
		HIT_AREA_SHAPE cone{};
		cone.iAreaType = 3;
		cone.iAreaRange = static_cast<int32_t>(m_fDebugGazeDistance * 100.f);
		cone.iAreaAngle = static_cast<int32_t>(m_fDebugGazeHalfAngle * 2.f);
		CHitAreaWire::Draw(*localCharacter->Get_Transform()->Get_WorldMatrixPtr(), cone,
			40u | (220u << 8u) | (255u << 16u) | (255u << 24u));
	}
#endif
	Update_DeadScene(fTimeDelta);
	Update_RaidClear(fTimeDelta);
	if (nullptr != m_pMvpResultView)
	{
		/* The award page has no characters of its own: every panel is a host
		   render target in mvp.gfx, so the page is handed whichever character
		   each one should draw -- the player that panel names (Show_MvpResult). */
		for (size_t iStageSlot = 0; iStageSlot < 4u; ++iStageSlot)
			m_pMvpResultView->Set_StageCharacter(
				iStageSlot, m_MvpStageCharacters[iStageSlot].lock());
		m_pMvpResultView->Update(fTimeDelta);
	}
	Update_GateProgress(fTimeDelta);

	/* Gate spawn replies arrive one per requested placement. They are Debug
	   status only; the presentation itself follows the reliable spawn stream. */
	LostArk::Shared::S2C_WORLD_ENTITY_SPAWN_RESULT spawnResult{};
	std::uint64_t spawnRequestToken = 0u;
	while (CNetworkManager::Get().Try_Consume_WorldEntitySpawnResult(spawnResult, &spawnRequestToken))
	{
		const auto pending = m_DebugGatePendingPlacements.find(spawnResult.strPlacementId);
		if (pending == m_DebugGatePendingPlacements.end() ||
			spawnRequestToken == 0u || pending->second != spawnRequestToken)
			continue;
		m_DebugGatePendingPlacements.erase(pending);
		const char_t* pResult = "unsupported result";
		switch (spawnResult.eResult)
		{
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::SPAWNED:
			pResult = "spawned"; break;
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS:
			pResult = "already exists"; break;
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ACTIVATED:
			pResult = "activated"; break;
		case LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::REJECTED:
			pResult = "rejected by Server"; break;
		default: break;
		}
		if (LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::SPAWNED != spawnResult.eResult &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS != spawnResult.eResult &&
			LostArk::Shared::WORLD_ENTITY_SPAWN_RESULT::ACTIVATED != spawnResult.eResult)
			m_bDebugGateFailed = true;
		m_strDebugGateStatus += "\n" + spawnResult.strPlacementId + ": " + pResult;
	}
	if (m_iPendingDebugGate != NO_ACTIVE_DEBUG_GATE)
	{
		m_fDebugGatePendingSeconds += fTimeDelta;
		if (m_fDebugGatePendingSeconds >= 15.f)
		{
			Debug_RetireGateActivation(
				"Server Gate activation timed out before all spawn and movement approvals arrived.");
		}
	}
	if (m_iPendingDebugGate != NO_ACTIVE_DEBUG_GATE && m_DebugGatePendingPlacements.empty() &&
		!m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		if (!m_bDebugGateFailed && (m_bDebugGatePreservesPlayerPosition || m_PlayerController.Did_DebugPlayerPlacementSucceed()) &&
		m_PendingDebugGateApproval.iWorldGeneration == CNetworkManager::Get().Get_WorldInboundGeneration() &&
		m_PendingDebugGateApproval.iRaidEpoch == Get_KoukuRaidState().iRunEpoch)
		{
			const KAKUL_DEBUG_GATE& gate = Get_DebugGates()[m_iPendingDebugGate];
			CCombatHUDViewModel::Get().Set_BossFocusArchetype(
				nullptr != gate.pHudFocusArchetypeId ? gate.pHudFocusArchetypeId : "");
			/* Each gate is its own combat-analysis total (retail counts per 관문). */
			CCombatHUDViewModel::Get().Reset_CombatAnalysis();
			CCombatHUDViewModel::Get().Set_BossHidden(m_bSequenceCombatPending || nullptr == gate.pHudFocusArchetypeId);
			CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss(
				nullptr != gate.pAuditionPlacementId ? gate.pAuditionPlacementId : "",
				nullptr != gate.pHudFocusArchetypeId ? gate.pHudFocusArchetypeId : "");
			m_iActiveDebugGate = m_iPendingDebugGate;
			m_DebugGateApproval = m_PendingDebugGateApproval;
            std::string gatePresentationStatus;
            if (Debug_CommitGateObjects(m_iActiveDebugGate, gatePresentationStatus))
            {
                m_iGateLightingIndex = m_iActiveDebugGate;
                m_pGateMapLightPresentation = std::move(m_pPendingGateMapLights);
                m_GateMapLightSource = std::move(m_PendingGateMapLightSource);
                m_strGatePresentationProfileId = m_iActiveDebugGate == 0u ? "scene.kakulsaydon.g1.book-open.v1" :
                    m_iActiveDebugGate == 2u ? "scene.kakulsaydon.g3.dark.v1" : "";
            }
            else m_strDebugGateStatus += "\nServer approved the gate, but its presentation failed: " + gatePresentationStatus;

			// Commit the gimmick mode only after the Server-approved gate move.
			using LostArk::Shared::KOUKU_HUD_MODE;
			(void)m_PlayerController.Request_DebugKoukuHudMode(
				m_iActiveDebugGate == 3u ? KOUKU_HUD_MODE::MARIO :
				(m_iActiveDebugGate == 7u ? KOUKU_HUD_MODE::MAZE : KOUKU_HUD_MODE::NONE));
			m_strDebugGateStatus += "\nGate activation confirmed by Server.";
		}
		else
		{
			m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
            Debug_CancelGateObjects(); m_pPendingGateMapLights.reset(); m_PendingGateMapLightSource.reset();
			m_strDebugGateStatus += "\nGate activation failed; correct the reported cause and retry.";
		}
		m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
		m_PendingDebugGateApproval = {};
		m_bDebugGatePreservesPlayerPosition = false;
		CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(false);
	}
	if (m_bDebugStartPending && !m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		m_bDebugStartPending = false;
		if (m_PlayerController.Did_DebugPlayerPlacementSucceed())
		{
			m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
            Debug_StopGateObjects();
            m_iGateLightingIndex = NO_ACTIVE_DEBUG_GATE;
            m_strGatePresentationProfileId.clear();
            m_pGateMapLightPresentation.reset(); m_pPendingGateMapLights.reset();
            m_GateMapLightSource.reset(); m_PendingGateMapLightSource.reset();

			m_bSequenceCombatPending = false; m_bSequenceCombatFadeHeld = false;
			CCombatHUDViewModel::Get().Clear_BossFocus();
			CCombatHUDViewModel::Get().Set_BossHidden(true);
			CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss("", "");
#ifdef _DEBUG
			Debug_StopCompositionWorldPreview();
#endif
			auto startTargets = Make_WorldSequenceTargets();
			m_SequencePlayer.Stop_All(startTargets, true);
			for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(startTargets, true);
			m_OwnedWorldCues.clear(); m_PendingOwnedWorldCues.clear();
#ifdef _DEBUG
			Debug_StopWorldObjectPreview();
#endif
			m_bCutsceneBossVisible = true;
			Update_CutsceneBossRetire(startTargets);
			(void)Load_EntranceTriggerMarkers();
			Stop_CompositionCamera(true);
			m_fTriggerMoveFadeAlpha = 0.f; m_bTriggerMoveFadeArmed = false; m_bTriggerMoveFadeHasLastPosition = false;
			m_bDebugStartSucceeded = true;
			m_strDebugGateStatus = "Server reset arena bosses and entry triggers; returned to the authored start.";
		}
		else m_strDebugGateStatus = "Arena start rejected; previous scene retained. " + m_PlayerController.Get_DebugPlayerPlacementStatus();
		CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(false);
	}

	auto targets = Make_WorldSequenceTargets();
	const auto& pendingRun = m_Replication.Get_KoukuBundleState();
	if (pendingRun.iRunEpoch > m_iLatestWorldRunEpoch)
	{
		for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(targets, true);
		m_OwnedWorldCues.clear(); m_PendingOwnedWorldCues.clear();
		m_StoppedWorldOwners.clear(); m_FinishedWorldOwners.clear(); m_ConsumedWorldCueIds.clear();
		m_iLatestWorldRunEpoch = pendingRun.iRunEpoch;
	}
	// Product presentation is prepared by MainApp after the first arena update.
	// Keep reliable cues until the matching pinned revision is ready or their span expires.
	auto pendingWorldCues = std::move(m_PendingOwnedWorldCues);
	m_PendingOwnedWorldCues.clear();
	for (const auto& play : pendingWorldCues) Consume_OwnedWorldCue(play, targets);
	/* The Server decided these started; this level only resolves each stable
	   instance ID against what it loaded and plays the presentation. */
	for (const auto& play : m_Replication.Consume_WorldSequencePlays())
	{
        if (Queue_MarioBombContactStop(play)) continue;
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP_OWNER ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::FINISH_OWNER ||
			play.iRunEpoch != 0u)
		{
			Consume_OwnedWorldCue(play, targets);
			continue;
		}
		const std::string& instanceId = play.strSequenceInstanceId;
		// Reject stale legacy cues before they cancel an active Pattern preview
		// or take the exact-motion branch around normal sequence admission.
		if ((play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::REPLAY) &&
			(instanceId == KAKULSAYDON_CUTSCENE_SEQUENCE_ID ||
			 play.strTargetSequenceInstanceId == KAKULSAYDON_CUTSCENE_SEQUENCE_ID))
		{
			OutputDebugStringA("[Level_KakulSaydonArena] Retired Saydon cutscene cue rejected; use the authored Sequence Pattern.\n");
			continue;
		}
#ifdef _DEBUG
		Debug_StopWorldObjectPreview();
		Debug_StopCompositionWorldPreview();
#endif
		std::string status;
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY &&
			!play.strTargetSequenceInstanceId.empty())
		{
			if (!m_SequencePlayer.Apply_ObjectMotion(play.strTargetSequenceInstanceId, instanceId, targets))
				OutputDebugStringA(("[Level_KakulSaydonArena][WorldMotion] " +
					m_SequencePlayer.Get_Status() + "\n").c_str());
			continue;
		}
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::REPLAY ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP)
		{
			m_SequencePlayer.Stop_Instance(instanceId, targets, true);
			for (const auto& link : KAKULSAYDON_PAPER_BRIDGE_LINKS)
			{
				if (link.bridgeSequenceInstanceId != instanceId) continue;
				m_SequencePlayer.Stop_Instance(std::string(link.leverSequenceInstanceId), targets, true);
				m_RaisedPaperBridges.erase(link.bridgePlacementId);
				m_DeployRuntime.Set_State(link.bridgePlacementId, DEPLOY_PROP_STATE::DESPAWNED);
			}
			if (instanceId == KAKULSAYDON_CUTSCENE_SEQUENCE_ID)
			{
				for (const auto& instance : m_SequencePlayer.Get_Document().Get_Instances())
					if (instance.instanceId.starts_with(KAKULSAYDON_CUTSCENE_INSTANCE_PREFIX))
						m_SequencePlayer.Stop_Instance(instance.instanceId, targets, true);
				m_bCutsceneBossVisible = false;
				Apply_CutsceneSetVisible(false);
				m_DeployRuntime.Set_State(KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED);
			}
			const auto activeShot = std::find_if(m_CameraShots.begin(), m_CameraShots.end(),
				[&](const KAKUL_CAMERA_SHOT& shot) { return shot.strShotId == m_strActiveCameraShotId &&
					shot.strSequenceInstanceId == instanceId; });
			if (activeShot != m_CameraShots.end()) Release_CameraShot();
			if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP) continue;
		}
		if (play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::PLAY ||
			play.eOperation == LostArk::Shared::WORLD_SEQUENCE_OPERATION::REPLAY)
			Retire_EntranceTriggerMarker(instanceId);
		if (!Start_ServerRequestedSequence(instanceId, play.fPlaybackSpeed,
			float3_t(play.fPositionOffsetX, play.fPositionOffsetY, play.fPositionOffsetZ), targets, status, play.iDurationMs,
			WorldPlacementFromCue(play)))
		{
			OutputDebugStringA((
				"[Level_KakulSaydonArena][WorldSequence] " + instanceId +
				": " + status + "\n").c_str());
		}
	}
	// Persistent terminal state also closes owners for reconnect/late packet ordering.
	const auto& runState = m_Replication.Get_KoukuBundleState();
	using RUN_STATE = LostArk::Shared::KOUKUSAYDON_PATTERN_AUDITION_LIFECYCLE_STATE;
	const auto stopOwner = [&](const std::string& memberId, const RUN_STATE state, const std::uint32_t patternSequence = 0u)
	{
		LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY stop;
		stop.eOperation = state == RUN_STATE::COMPLETED ? LostArk::Shared::WORLD_SEQUENCE_OPERATION::FINISH_OWNER :
			LostArk::Shared::WORLD_SEQUENCE_OPERATION::STOP_OWNER;
		stop.iRunEpoch = runState.iRunEpoch; stop.strMemberId = memberId; stop.iPatternSequence = patternSequence;
		Consume_OwnedWorldCue(stop, targets);
	};
	if (runState.iRunEpoch)
	{
		if (runState.eState == RUN_STATE::COMPLETED || runState.eState == RUN_STATE::ABORTED) stopOwner({}, runState.eState);
		else for (const auto& member : runState.Members)
			if (member.eState == RUN_STATE::COMPLETED || member.eState == RUN_STATE::ABORTED) stopOwner(member.strMemberId, member.eState, member.iPatternSequence);
	}
	m_SequencePlayer.Update(fTimeDelta, targets);
    Debug_UpdateGateObjects(fTimeDelta);
	Update_CardMazePresentation(fTimeDelta);
	Update_MarioBallBouncePresentation(fTimeDelta);
	Update_MarioBallPresentation(fTimeDelta);
	Update_MarioBombPresentation(fTimeDelta);
	if (m_bWorldObjectReloadPending && !m_SequencePlayer.Has_ActiveInstances())
	{
		std::string status;
		if (!Reload_WorldObjectRuntime(status))
			OutputDebugStringA(("[WorldObjectReload] " + status + "\n").c_str());
	}
	for (auto cue = m_OwnedWorldCues.begin(); cue != m_OwnedWorldCues.end();)
	{
		float seconds = 0.f;
		auto& value = cue->second;
		if (CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
			m_Replication.Get_LastServerTick(), value.startTick, 30.f, seconds))
			value.clockMs = (std::max)(seconds * 1000.f, value.clockMs + fTimeDelta * 1000.f);
		else value.clockMs += fTimeDelta * 1000.f;
		auto cueTargets = targets;
		cueTargets.objectEmissionAnchor = value.emissionAnchor;
		(void)value.player->Seek_AllToMs(value.clockMs, cueTargets, false);
		value.player->Update(0.f, cueTargets);
		if (!value.player->Has_ActiveInstances()) cue = m_OwnedWorldCues.erase(cue);
		else ++cue;
	}
	std::vector<CClientReplication::WORLD_COMBAT_TARGET> combatTargets;
	for (const auto& [key, cue] : m_OwnedWorldCues)
	{
		if (!cue.combatBodyNetEntityId) continue;
		std::vector<std::shared_ptr<CWorldSequenceObject>> objects;
		cue.player->Collect_VisibleObjects(objects);
		for (auto& object : objects) combatTargets.push_back({ cue.combatBodyNetEntityId, std::move(object) });
	}
	m_Replication.Set_WorldCombatTargets(combatTargets);
	Update_CutsceneBossRetire(targets);
	Update_CompositionCamera(fTimeDelta);
	Update_CameraShots(fTimeDelta);
	Update_CinematicSurroundings();
	Sync_CinematicPlayerVisibility();
	Update_RaidBgm();
	// Consume this frame's Server-started camera sequence before accepting input.
	// The giant Saydon combat shot keeps ordinary Server-authorized picking available.
	const bool_t cinematicInputBlocked = Is_CinematicInputBlocked();
	bool_t sequenceInputReady = !m_bSequenceCombatPending;
#ifdef _DEBUG
	sequenceInputReady = sequenceInputReady && !Is_DebugGatePending();
#endif
	m_PlayerController.Update(
		sequenceInputReady && nullptr != m_pCamera && m_pCamera->Is_FollowEnabled() && !cinematicInputBlocked,
		sequenceInputReady && nullptr != m_pCamera && !m_pCamera->Is_FollowRequested() &&
		!m_pCamera->Is_PresentationOverrideActive());
	Update_TriggerMoveFade(fTimeDelta);
	Update_EntranceTriggerMarkerClocks(fTimeDelta);
#ifdef _DEBUG
	if (!m_bMapAuthoringActive)
#endif
		m_MapRuntime.Update_SelfMotions(fTimeDelta);
	// Gate progress is room-wide and survives Return to Start. Use each player's
	// Server position so the waiting platform stays hidden on initial entry and return.
	std::vector<KOUKU_BOSS_PRESENTATION_VIEW> madnessBosses;
	std::vector<KOUKU_CARD_PRESENTATION_VIEW> madnessPlayers;
	m_Replication.Collect_KoukuPresentationViews(madnessBosses, madnessPlayers);
	const bool_t gateEntered = m_bGateProgressKnown && 0u != m_GateProgress.iCurrentGate;
	const auto& gates = Get_DebugGates();
	const bool_t playerOnlyEntry = m_iActiveDebugGate < gates.size() &&
		nullptr == gates[m_iActiveDebugGate].BossPlacementIds[0];
	const auto canShowMadnessGauge = [this, &madnessPlayers, &localCharacter, gateEntered, playerOnlyEntry](
		const shared_ptr<CCharacter>& character)
	{
		if (!character) return false;
		for (const auto& view : madnessPlayers)
		{
			if (view.pCharacter.lock() != character) continue;
			const auto& player = view.Snapshot;
			if (!m_Replication.Should_ShowKoukuPlayerWorldUI(player.iNetEntityId)) return false;
			// Server-approved player-only Debug entry and Mario have no boss gate.
			if (!gateEntered && LostArk::Shared::KOUKU_HUD_MODE::MARIO != player.eKoukuHudMode &&
				!(playerOnlyEntry && character == localCharacter))
				return false;
			return !LostArk::Shared::Is_KoukuArenaStartArea(
				player.fPositionX, player.fPositionY, player.fPositionZ);
		}
		return false;
	};
	HUD_KOUKU_GIMMICK_STATE madnessState = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	madnessState.isValid = madnessState.isValid && canShowMadnessGauge(localCharacter);
	if (LostArk::Shared::KOUKU_HUD_MODE::MAZE == CCombatHUDViewModel::Get().Get_Player().eKoukuHudMode)
		madnessState.eHudMode = HUD_KOUKU_HUD_MODE::MAZE;
	if (m_Replication.Get_KoukuWorldUISubjectId() != LostArk::Shared::INVALID_NET_ENTITY_ID)
	{
		madnessState.eHudMode = HUD_KOUKU_HUD_MODE::MARIO;
		madnessState.eCardMazeRole = LostArk::Shared::CARD_MAZE_ROLE::NONE;
	}
	if (nullptr != m_pMadnessGaugeView)
		m_pMadnessGaugeView->Update(fTimeDelta, localCharacter, madnessState);
	/* Teammates: their own madness from the snapshot, drawn the same way over them. */
	{
		size_t iOther = 0u;
		for (const REPLICATED_PLAYER_VIEW& Player : m_NameplatePlayers)
		{
			if (Player.isLocal || iOther >= m_OtherMadnessGaugeViews.size())
				continue;
			const REPLICATED_PLAYER_HEALTH Health = m_Replication.Get_PlayerHealth().Find(Player.iNetEntityId);
			// Keep the local maze presentation policy for every world-space gauge.
			HUD_KOUKU_GIMMICK_STATE State = madnessState;
			const auto character = Player.pCharacter.lock();
			State.isValid = Health.Has_Madness() && canShowMadnessGauge(character);
			State.iMadnessGauge = Health.iCurrentMadness;
			State.iMadnessMaximum = Health.iMaximumMadness;
			if (nullptr != m_OtherMadnessGaugeViews[iOther])
				m_OtherMadnessGaugeViews[iOther]->Update(fTimeDelta, character, State);
			++iOther;
		}
		for (; iOther < m_OtherMadnessGaugeViews.size(); ++iOther)
			if (nullptr != m_OtherMadnessGaugeViews[iOther])
				m_OtherMadnessGaugeViews[iOther]->Hide();
	}
	Update_StatusEffectText(fTimeDelta);
}

bool_t Client::CLevel_KakulSaydonArena::Get_MadnessGaugePosition(
	float2_t& screenOffset, f32_t& feetOffsetMeters) const
{
	if (!m_pMadnessGaugeView) return false;
	m_pMadnessGaugeView->Get_Position(screenOffset, feetOffsetMeters);
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Set_MadnessGaugePosition(
	const float2_t& screenOffset, const f32_t feetOffsetMeters)
{
	if (!m_pMadnessGaugeView || !m_pMadnessGaugeView->Set_Position(screenOffset, feetOffsetMeters)) return false;
	for (auto& view : m_OtherMadnessGaugeViews)
		if (view) view->Set_Position(screenOffset, feetOffsetMeters);
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Save_MadnessGaugePosition(std::string& status)
{
	if (!m_pMadnessGaugeView) { status = "Madness view is unavailable."; return false; }
	if (!m_pMadnessGaugeView->Save_Position(status)) return false;
	float2_t offset; f32_t head;
	m_pMadnessGaugeView->Get_Position(offset, head);
	return Set_MadnessGaugePosition(offset, head);
}

bool_t Client::CLevel_KakulSaydonArena::Reload_MadnessGaugePosition(std::string& status)
{
	if (!m_pMadnessGaugeView) { status = "Madness view is unavailable."; return false; }
	if (!m_pMadnessGaugeView->Reload_Position(status)) return false;
	float2_t offset; f32_t head;
	m_pMadnessGaugeView->Get_Position(offset, head);
	return Set_MadnessGaugePosition(offset, head);
}

void Client::CLevel_KakulSaydonArena::Update_StatusEffectText(const f32_t fTimeDelta)
{
	/* "gongpo" (fear). The word and its colour are retail data, not a code
	   decision: EFTable_GameMsg tip.name.skillbuffdmgfont_<buffId> spells it and
	   EFTable_SkillBuff.FontColor gives 0x8041D9 on all 66 fear buff rows that show
	   one. Those rows also carry FontShow 1, which is the movie motion the view
	   draws. Written with universal character names so this file keeps the
	   ASCII bytes its codepage needs, exactly like the card maze suit names below. */
	static const std::wstring FEAR_WORD = L"\uACF5\uD3EC";
	constexpr std::uint32_t FEAR_COLOR_RGB = 0x8041D9u;
	static const std::wstring SAFE_ZONE_WORD = L"\uBB34\uC801";
	constexpr std::uint32_t SAFE_ZONE_COLOR_RGB = 0x3399FFu;

	std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
	std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
	Collect_KoukuPresentationViews(bosses, players);
	for (const KOUKU_CARD_PRESENTATION_VIEW& view : players)
	{
		if (view.Snapshot.iCurrentHp != 0u && view.Snapshot.iInvulnerabilityZonePulseTick != 0u)
		{
			CStatusEffectTextView::REQUEST safeZone{};
			safeZone.iOwnerEntityId = view.Snapshot.iNetEntityId;
			// Entry and the two-second repeat are Server occurrences, not Client timers.
			safeZone.iOccurrenceKey = view.Snapshot.iInvulnerabilityZonePulseTick;
			safeZone.strWord = SAFE_ZONE_WORD;
			safeZone.iColorRgb = SAFE_ZONE_COLOR_RGB;
			safeZone.pAnchor = view.pCharacter;
			m_StatusEffectTextView.Submit(safeZone);
		}
		if (LostArk::Shared::PLAYER_ACTION_STATE::FEAR != view.Snapshot.eAction ||
			0u == view.Snapshot.iCurrentHp || 0u == view.Snapshot.iActionStartTick)
		{
			continue;
		}
		CStatusEffectTextView::REQUEST request{};
		request.iOwnerEntityId = view.Snapshot.iNetEntityId;
		/* The Server owns the window, so its start tick is the occurrence: one
		   word per FEAR, and a second FEAR pops a second word. */
		request.iOccurrenceKey = view.Snapshot.iActionStartTick;
		request.strWord = FEAR_WORD;
		request.iColorRgb = FEAR_COLOR_RGB;
		request.pAnchor = view.pCharacter;
		m_StatusEffectTextView.Submit(request);
	}

#ifdef _DEBUG
	/* F1 preview: the same word over the local character with no Server truth,
	   keyed by the button's own serial so repeated presses keep firing. */
	const std::uint32_t previewSerial =
		CCombatHUDViewModel::Get().Get_StatusEffectTextPreviewSerial();
	const auto previewAnchor = m_Replication.Get_CameraCharacter();
	/* Only consume the serial once there is a character to hang the word on, so
	   a press made before the local character is up is not swallowed. */
	if (previewSerial != m_iStatusEffectTextPreviewSerial && nullptr != previewAnchor)
	{
		m_iStatusEffectTextPreviewSerial = previewSerial;
		CStatusEffectTextView::REQUEST request{};
		const auto* subject = m_Replication.Get_CameraPlayerSnapshot();
		request.iOwnerEntityId = subject ? subject->iNetEntityId : 0u;
		request.iOccurrenceKey = previewSerial;
		request.strWord = FEAR_WORD;
		request.iColorRgb = FEAR_COLOR_RGB;
		request.pAnchor = previewAnchor;
		m_StatusEffectTextView.Submit(request);
	}
#endif

	m_StatusEffectTextView.Update(fTimeDelta);
}

void Client::CLevel_KakulSaydonArena::Apply_CutsceneSetVisible(
	const bool_t cutsceneVisible)
{
	if (m_bCutsceneSetVisible == cutsceneVisible)
		return;
	m_bCutsceneSetVisible = cutsceneVisible;
	/* The unfolding copy and the standing arena occupy the same space, so
	   exactly one of them is on screen at a time. */
	for (MAP_RUNTIME_PLACED_ENTRY& entry :
		m_MapRuntime.Get_MutablePlacements())
	{
		const uint64_t placementId = entry.record.placementId;
		const bool_t isCutsceneSet =
			KAKULSAYDON_CUTSCENE_SET_FIRST_ID <= placementId &&
			placementId < KAKULSAYDON_CUTSCENE_SET_END_ID;
		if (isCutsceneSet)
		{
			(void)CMapPlacementRuntime::Set_RuntimeVisible(
				entry, cutsceneVisible);
			continue;
		}
		const bool_t isHiddenArena = std::find(
			KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.begin(),
			KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end(),
			placementId) != KAKUL_ARENA_HIDDEN_PLACEMENT_IDS.end();
		if (isHiddenArena)
		{
			(void)CMapPlacementRuntime::Set_RuntimeVisible(
				entry, !cutsceneVisible);
		}
	}
}

void Client::CLevel_KakulSaydonArena::Update_CutsceneBossRetire(
	const CWorldSequencePlayer::TARGET_SET& targets)
{
	if (!targets.Is_Complete())
		return;
	const bool_t playing =
		m_SequencePlayer.Is_Playing(KAKULSAYDON_CUTSCENE_SEQUENCE_ID);
	if (playing)
	{
		m_bCutsceneBossVisible = true;
		return;
	}
	if (!m_bCutsceneBossVisible)
		return;
	/* One retire per cutscene: the flag clears whether or not the prop was
	   still there, so a missing prop never retries every frame. */
	m_bCutsceneBossVisible = false;
	/* The show is over: the arena the cutscene built takes over from the
	   unfolding copy, and the presentation boss leaves with it. */
	Apply_CutsceneSetVisible(false);
	if (!targets.pDeployRuntime->Set_State(
		KAKULSAYDON_CUTSCENE_BOSS_PLACEMENT_ID, DEPLOY_PROP_STATE::DESPAWNED))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][Cutscene] boss retire failed: " +
			targets.pDeployRuntime->Get_Status() + "\n").c_str());
	}
}

bool_t Client::CLevel_KakulSaydonArena::Start_ServerRequestedSequence(
	const std::string& instanceId, const f32_t playbackSpeed, const float3_t& positionOffset,
	const CWorldSequencePlayer::TARGET_SET& targets,
	std::string& outStatus, const uint32_t durationMs,
	const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& placement)
{
	/* A bridge unfold is more than its sequence: the Deploy prop must leave
	   DESPAWNED first. Route those through the bridge contract so the reveal
	   and the animation stay one decision. */
	const auto link = std::find_if(
		KAKULSAYDON_PAPER_BRIDGE_LINKS.begin(),
		KAKULSAYDON_PAPER_BRIDGE_LINKS.end(),
		[&instanceId](const PAPER_BRIDGE_LINK& value)
		{
			return value.bridgeSequenceInstanceId == instanceId;
		});
	if (KAKULSAYDON_PAPER_BRIDGE_LINKS.end() != link)
		return Request_PaperBridgeUnfold(link->leverPlacementId, outStatus);

	// Saydon is owned by a Composition Pattern. Old saved trigger messages
	// must not resurrect the separate Deploy actor or start every original_* map.
	if (KAKULSAYDON_CUTSCENE_SEQUENCE_ID == instanceId)
	{
		outStatus = "Legacy Saydon cutscene is retired; play the authored Sequence Pattern.";
		return false;
	}

	if (!m_SequencePlayer.Play(instanceId, targets, playbackSpeed, positionOffset, durationMs, placement))
	{
		outStatus = m_SequencePlayer.Get_Status();
		return false;
	}
	outStatus = "World sequence started";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Request_PaperBridgeUnfold(
	const uint64_t leverPlacementId,
	std::string& outStatus)
{
	const auto link = std::find_if(
		KAKULSAYDON_PAPER_BRIDGE_LINKS.begin(),
		KAKULSAYDON_PAPER_BRIDGE_LINKS.end(),
		[leverPlacementId](const PAPER_BRIDGE_LINK& value)
		{
			return value.leverPlacementId == leverPlacementId;
		});
	if (KAKULSAYDON_PAPER_BRIDGE_LINKS.end() == link)
	{
		outStatus = "Unknown paper lever placement";
		return false;
	}
	if (!m_RaisedPaperBridges.insert(link->bridgePlacementId).second)
	{
		outStatus = "Paper bridge is already raised";
		return true;
	}

	auto targets = Make_WorldSequenceTargets();

	/* Reveal before the first sample so the unfold plays from its own opening
	   frame. A failed reveal leaves the bridge hidden and stays retryable. */
	if (!m_DeployRuntime.Set_State(
		link->bridgePlacementId, DEPLOY_PROP_STATE::INTACT))
	{
		m_RaisedPaperBridges.erase(link->bridgePlacementId);
		outStatus = m_DeployRuntime.Get_Status();
		return false;
	}
	if (!m_SequencePlayer.Play(
			std::string(link->bridgeSequenceInstanceId), targets))
	{
		m_DeployRuntime.Set_State(
			link->bridgePlacementId, DEPLOY_PROP_STATE::DESPAWNED);
		m_RaisedPaperBridges.erase(link->bridgePlacementId);
		outStatus = m_SequencePlayer.Get_Status();
		return false;
	}
	/* The lever pull is decoration on top of the bridge contract: losing it
	   must not undo a bridge that is already unfolding. */
	if (!m_SequencePlayer.Play(
		std::string(link->leverSequenceInstanceId), targets))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena][PaperLever] " +
			m_SequencePlayer.Get_Status() + "\n").c_str());
	}
	outStatus = "Paper bridge unfold started";
	return true;
}

void Client::CLevel_KakulSaydonArena::Update_DeadScene(
	const f32_t fTimeDelta)
{
	if (nullptr == m_pDeadSceneView)
		return;

	m_pDeadSceneView->Update(fTimeDelta);

	using LostArk::Shared::PLAYER_ACTION_STATE;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t isDead = player.isValid &&
		PLAYER_ACTION_STATE::DEAD == player.eAction;

	/* Real Render_DeadScene's own whole-screen AddRectFilled(IM_COL32(0,0,0,160)), now a real
	slot (DeadScene_Dim, White1x1 tinted) instead of a raw ImGui draw call. */
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Dim", isDead && !m_Replication.Is_Spectating());
	m_pDeadSceneView->Set_SlotVisible("DeadScene_PanelBg", isDead && !m_Replication.Is_Spectating());
	m_pDeadSceneView->Set_SlotVisible("DeadScene_WingedArch", isDead && !m_Replication.Is_Spectating());
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Effect", isDead && !m_Replication.Is_Spectating());
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveButton", isDead);
	// Spectating changes the presentation target; revive remains a typed gameplay command.
	m_pDeadSceneView->Set_SlotVisible("DeadScene_SpectateButton", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_SpectateBorder", isDead);
	/* Tool-authoring placeholders only (mark where RenderDeadSceneText's labels land) --
	never shown in real gameplay, regardless of death state. */
	m_pDeadSceneView->Set_SlotVisible("DeadScene_TitleTextMarker", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveMessageMarker", false);
	if (!isDead)
	{
		CCombatHUDViewModel::Get().Set_DeadSceneTextRects({});
		return;
	}

	/* RenderDeadSceneText() (CMainApp, after EndFrame()) has no access to this Level's
	m_pDeadSceneView -- push the live, Tool-editable rects through the same Level -> ViewModel ->
	UI path the rest of the combat HUD uses instead of hand-copying these numbers into
	MainApp.cpp, which is exactly what went stale and made the title/button text drift off after
	the panel was repositioned in the Tool. The "부활"/"관전하기" labels are drawn ON their own
	buttons, so those two read the button slots' own rects directly -- DeadScene_ReviveMessageMarker
	is a separate free-standing box above the revive button, unrelated to that label. */
	{
		HUD_DEADSCENE_TEXT_RECTS textRects;
		textRects.isSpectating = m_Replication.Is_Spectating();
		textRects.isValid =
			m_pDeadSceneView->Get_SlotRect("DeadScene_TitleTextMarker",
				textRects.fTitleX, textRects.fTitleY,
				textRects.fTitleWidth, textRects.fTitleHeight) &&
			m_pDeadSceneView->Get_SlotRect("DeadScene_ReviveButton",
				textRects.fReviveTextX, textRects.fReviveTextY,
				textRects.fReviveTextWidth, textRects.fReviveTextHeight) &&
			m_pDeadSceneView->Get_SlotRect("DeadScene_SpectateButton",
				textRects.fSpectateX, textRects.fSpectateY,
				textRects.fSpectateWidth, textRects.fSpectateHeight) &&
			m_pDeadSceneView->Get_SlotRect("DeadScene_ReviveMessageMarker",
				textRects.fMessageX, textRects.fMessageY,
				textRects.fMessageWidth, textRects.fMessageHeight);
		CCombatHUDViewModel::Get().Set_DeadSceneTextRects(textRects);
	}

	f32_t spectateX = 0.f, spectateY = 0.f, spectateWidth = 0.f, spectateHeight = 0.f;
	if (m_pDeadSceneView->Get_SlotRect("DeadScene_SpectateButton", spectateX, spectateY, spectateWidth, spectateHeight))
	{
		auto& router = CUIInputRouter::Get();
		const auto width = m_pDeadSceneView->Get_ResolutionWidth();
		const auto height = m_pDeadSceneView->Get_ResolutionHeight();
		if (router.Is_Hovered(spectateX, spectateY, spectateWidth, spectateHeight, width, height))
		{
			router.Claim_Mouse_This_Frame();
			if (router.Is_Clicked(spectateX, spectateY, spectateWidth, spectateHeight, width, height) &&
				m_Replication.Cycle_SpectateTarget())
			{
				CMainApp::Play_UIButtonClickSound();
				Bind_CameraToLocalCharacter();
			}
		}
	}
	f32_t fX = 0.f, fY = 0.f, fWidth = 0.f, fHeight = 0.f;
	if (!m_pDeadSceneView->Get_SlotRect(
		"DeadScene_ReviveButton", fX, fY, fWidth, fHeight))
	{
		return;
	}
	CUIInputRouter& Router = CUIInputRouter::Get();
	const f32_t fResolutionWidth = m_pDeadSceneView->Get_ResolutionWidth();
	const f32_t fResolutionHeight = m_pDeadSceneView->Get_ResolutionHeight();
	if (Router.Is_Hovered(fX, fY, fWidth, fHeight, fResolutionWidth, fResolutionHeight))
	{
		Router.Claim_Mouse_This_Frame();
		if (Router.Is_Clicked(fX, fY, fWidth, fHeight, fResolutionWidth, fResolutionHeight))
		{
			CMainApp::Play_UIButtonClickSound();
			m_PlayerController.Request_Revive();
		}
	}
}

HRESULT Client::CLevel_KakulSaydonArena::Render()
{
	const HRESULT drawn = __super::Render();
	if (FAILED(drawn))
		return drawn;
	if (Is_CinematicPresentationActive()) return drawn;
	/* The award page is a full-screen modal: no world text at all while it is up. */
	if (nullptr == m_pMvpResultView || !m_pMvpResultView->Is_Visible())
	{
		auto worldUIPlayers = m_NameplatePlayers;
		std::erase_if(worldUIPlayers, [this](const auto& player) {
			return !m_Replication.Should_ShowKoukuPlayerWorldUI(player.iNetEntityId);
		});
		m_PlayerNameplateView.Render(worldUIPlayers, &m_Replication.Get_PartyRoster());
		m_ChatBubbleView.Render(m_Replication, worldUIPlayers);
	}
#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(
		TEXT("KoukuSaydon arena loading complete"));
#endif
	/* Drawn last so it sits over the scene. The text only reports what the
	   Server is offering -- pressing the shown key submits a command and the Server
	   decides, so nothing here can move the player by itself. */
	/* The offered box itself shows the retail key prompt over the player's head
	   (CInteractKeyPromptView). On the Mario lanes Up answers the same offer too, which the
	   retail prompt does not say, so that hint stays as text. */
	m_InteractKeyPrompt.Render_Text();
	const float2_t viewport = CGameInstance::Get().Get_ViewportSize();
	const f32_t promptScale = (std::min)(viewport.x / 1280.f, viewport.y / 720.f);
	const f32_t promptLineSpacing = CGameInstance::Get().Measure_Text(
		TEXT("Font_YoonGasiIIM"), L"0").y;
	const auto drawPrompt = [&](const wchar_t* text, const f32_t heightRatio,
		const f32_t sizeMultiplier, const vector_t tint)
	{
		UILabelFont::Draw_Centered(TEXT("Font_YoonGasiIIM"), text,
			viewport.x * 0.5f, viewport.y * heightRatio,
			promptLineSpacing * promptScale * sizeMultiplier, tint);
	};
	if (m_bGate3AuraOccupied)
		CRaidGateProgressView::Render_AssemblyCountdown(m_fGate3AuraSecondsLeft);
	const std::string& offered =
		CCombatHUDViewModel::Get().Get_InteractPromptTriggerId();
	if (!offered.empty() && 0u != CCombatHUDViewModel::Get().Get_Player().iMarioStage)
	{
		/* ASCII only: this file carries no other non-ASCII byte and has no BOM,
		   so a UTF-8 Korean literal here is read back in the system codepage. */
		const tchar_t* const PROMPT = TEXT("[ Up ]");
		drawPrompt(PROMPT, 0.62f, 1.f, Colors::White);
	}
	/* Card maze: the suit this player hunts and the count, or the telescope
	   role. ASCII for the same codepage reason as the prompt above. */
	const HUD_KOUKU_GIMMICK_STATE& maze = CCombatHUDViewModel::Get().Get_KoukuGimmick();
	if (LostArk::Shared::CARD_MAZE_ROLE::NONE != maze.eCardMazeRole ||
		CCombatHUDViewModel::Get().Get_Player().eKoukuHudMode == LostArk::Shared::KOUKU_HUD_MODE::MAZE)
	{
		std::wstring text;
		if (LostArk::Shared::CARD_MAZE_ROLE::NONE == maze.eCardMazeRole && m_bCardMazeClownBoxAlive)
			text = L"[ Q ] Break the clown box at the maze center";
		else if (LostArk::Shared::CARD_MAZE_ROLE::NONE == maze.eCardMazeRole && m_bCardMazeTelescopeShown)
			text = L"[ Q ] Strike the telescope at the maze center (G is not used)";
		if (maze.CardMaze.flags & 1u) text = L"[ TELESCOPE ON ]";
		else if (maze.CardMaze.flags & 2u) text = L"[ ESCAPED / WAITING FOR THE OTHER HUNTERS ]";
		/* The Debug solo owner hunts as well, so both parts can show at once.
		   The suit name is Korean, written with universal character names so this
		   file keeps the ASCII bytes its codepage needs; the YoonGasiIIM sprite
		   font carries every Hangul syllable used here. */
		if (LostArk::Shared::MECHANIC_CARD_SYMBOL::NONE != maze.eCardMazeSuit)
		{
			if (!text.empty())
				text += L" ";
			switch (maze.eCardMazeSuit)
			{
			/* hateu */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::HEART:
				text += L"\uD558\uD2B8"; break;
			/* seupeideu */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::SPADE:
				text += L"\uC2A4\uD398\uC774\uB4DC"; break;
			/* keullobeo */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::CLUB:
				text += L"\uD074\uB85C\uBC84"; break;
			/* daia */
			case LostArk::Shared::MECHANIC_CARD_SYMBOL::DIAMOND:
				text += L"\uB2E4\uC774\uC544"; break;
			/* munyang */
			default:
				text += L"\uBB38\uC591"; break;
			}
			/* "<suit> jogak x N": the shards this hunter has collected. */
			text += L" \uC870\uAC01 x " + std::to_wstring(maze.iCardMazeKills);
		}
		if (maze.CardMaze.flags & 4u)
			text += L" EXIT (" + std::to_wstring(static_cast<int>(maze.CardMaze.exitX)) + L", " +
				std::to_wstring(static_cast<int>(maze.CardMaze.exitZ)) + L")";
		drawPrompt(text.c_str(), 0.68f, 1.f, Colors::White);
	}
	/* The Server reports the entrant's matching count used by the exit check.
	   The overhead marker may belong to a different player outside Mario. */
	const auto& mario = CCombatHUDViewModel::Get().Get_Player();
	if (m_fMarioProgressNoticeSeconds > 0.f && mario.isValid &&
		mario.iMarioStage >= 1u && mario.iMarioStage <= 4u &&
		m_iMarioProgressColor >= 1u && m_iMarioProgressColor <= 3u)
	{
		static constexpr const wchar_t* BALL_NAMES[3] = {
			L"\uBE68\uAC04\uC0C9 \uACF5", L"\uD30C\uB780\uC0C9 \uACF5", L"\uB178\uB780\uC0C9 \uACF5" };
		const auto color = m_iMarioProgressColor;
		const std::wstring progress = L"[" + std::to_wstring(m_iMarioProgressCount) +
			L" / 3] " + BALL_NAMES[color - 1u];
		const vector_t tint = 1u == color ? Colors::Red :
			2u == color ? Colors::DeepSkyBlue : Colors::Gold;
		const f32_t alpha = std::clamp(m_fMarioProgressNoticeSeconds / MARIO_PROGRESS_FADE_SECONDS, 0.f, 1.f);
		drawPrompt(progress.c_str(), 0.68f, 1.f, XMVectorSetW(XMVectorScale(tint, alpha), alpha));
	}
	/* Mario: a colour's curse lifts when its last source ball pops. Korean by
	   universal character names for the same codepage reason as above. */
	if (m_iMarioCurseNoticeColor >= 0 && m_iMarioCurseNoticeColor < 3)
	{
		/* "<ppalgan|paran|noran> inhyeong-ui jeoju haeje": the red/blue/yellow
		   doll's curse is released. */
		static constexpr const tchar_t* NOTICES[3] = {
			TEXT("\uBE68\uAC04 \uC778\uD615\uC758 \uC800\uC8FC \uD574\uC81C"),
			TEXT("\uD30C\uB780 \uC778\uD615\uC758 \uC800\uC8FC \uD574\uC81C"),
			TEXT("\uB178\uB780 \uC778\uD615\uC758 \uC800\uC8FC \uD574\uC81C") };
		const tchar_t* const notice = NOTICES[m_iMarioCurseNoticeColor];
		const vector_t tint = 0 == m_iMarioCurseNoticeColor ? Colors::Red :
			1 == m_iMarioCurseNoticeColor ? Colors::DeepSkyBlue : Colors::Gold;
		drawPrompt(notice, 0.5f, 2.f, tint);
	}
	/* Floating status words last, over the scene and over the two prompts above,
	   the way the retail damage-text canvas sits on its own top layer. */
	m_StatusEffectTextView.Render(m_Replication.Get_KoukuWorldUISubjectId());
	/* Award page labels sit over everything else this Level draws, the status
	   words included. Its own image layers are CUI_Sprite objects on Layer_UI,
	   so they need no call. */
	/* Gate progress panel and prompt text, under the award page's labels. */
	m_GateProgressView.Render_Text();
	if (nullptr != m_pMvpResultView)
	{
		CUITextLayerScope PageText(UI_TEXT_LAYER::PAGE);
		m_pMvpResultView->Render();
	}
	return drawn;
}

namespace
{
	/* KoukuSaydon is a four-player raid -- the award page seats one MVP and three
	   party columns -- so the four-player cutoffs apply. */
	constexpr int32_t MVP_PARTY_SIZE = 4;

	/* EFTable_ZoneEpicGate.GroupId for KoukuSaydon; Valtan is 101, and the
	   headline follows whichever raid is handed in. SecondaryKey 0 on that row is
	   the normal difficulty, 2 the hard one. */
	constexpr int32_t KOUKU_RAID_GROUP_ID = 103;
	constexpr const char* KOUKU_DIFFICULTY_ID = "normal";

	/* Sample page for the Debug award button when no Server result has arrived. */
	Client::MVP_RESULT_DATA Build_MvpResultPreviewData(const int32_t iGate)
	{
		return Client::CMvpAwardCatalog::Get().Build_PreviewPage(
			KOUKU_RAID_GROUP_ID, iGate, KOUKU_DIFFICULTY_ID, MVP_PARTY_SIZE);
	}
}

void Client::CLevel_KakulSaydonArena::Show_MvpResult(const bool_t bReplayLast)
{
	if (nullptr == m_pMvpResultView)
		return;
	for (weak_ptr<CCharacter>& pStaged : m_MvpStageCharacters)
		pStaged.reset();
	const bool_t matchesClear = !m_iPendingRaidMvpGate ||
		(m_RaidMvpResult.eWorldId == LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA &&
		 m_RaidMvpResult.iGate == m_iPendingRaidMvpGate);
	if (m_bHasRaidMvpResult && matchesClear && (m_bRaidMvpResultFresh || bReplayLast))
	{
		vector<LostArk::Shared::PLAYER_ID> StagePlayerIds;
		const MVP_RESULT_DATA Data = CMvpAwardCatalog::Get().Build_ServerPage(
			KOUKU_RAID_GROUP_ID, KOUKU_DIFFICULTY_ID, MVP_PARTY_SIZE,
			m_RaidMvpResult, StagePlayerIds);
		vector<REPLICATED_PLAYER_VIEW> Players;
		m_Replication.Collect_PlayerViews(Players);
		for (size_t iSlot = 0; iSlot < StagePlayerIds.size() && iSlot < 4u; ++iSlot)
		{
			for (const REPLICATED_PLAYER_VIEW& Player : Players)
			{
				if (Player.iPlayerId == StagePlayerIds[iSlot])
				{
					m_MvpStageCharacters[iSlot] = Player.pCharacter;
					break;
				}
			}
		}
		m_bRaidMvpResultFresh = false;
		m_iPendingRaidMvpGate = 0u;
		m_pMvpResultView->Show(Data);
		return;
	}
	// A Server clear waits for its matching award packet, including late arrival.
	if (m_iPendingRaidMvpGate) return;
#ifdef _DEBUG
	m_MvpStageCharacters[0] = m_Replication.Get_LocalCharacter();
	m_pMvpResultView->Show(Build_MvpResultPreviewData(Current_GateNumber()));
#endif
}

namespace
{
	/* epicgatecommonclear.gfx runs at 40fps and every Set variant is 309 frames, so elapsed
	   seconds * 40 is the Set's own current frame and the keyframe document plays on the
	   same clock. EpicGateCommonClearFrame picks its variant by an integer the client hands
	   it -- result_<ClearNoticeImage> -- and EFTable_ZoneEpicGate gives KoukuSaydon 103,
	   which is epicGateCommanderClearSuccess_Set02, the Set this document was built from. */
	constexpr f32_t CLEAR_FPS = 40.f;
	/* The Set sprite is authored 309 frames and fades itself out over 300..308, but retail
	   never gets there: in the reference capture the clear screen is still at full strength
	   when it is cut outright, and the award page starts in the same instant. Anchoring the
	   capture to the document (its light enters at f108, crest f118, caption f126) puts
	   document frame 1 at capture frame 1038.5 and the cut at capture 1401, i.e. f243.
	   Document-to-document sequencing lives in the client's C++ and is not in the .gfx, so
	   this one number is measured rather than extracted. */
	constexpr f32_t CLEAR_END_FRAME = 243.f;
	// Keep a visual tail while admission waits. At the Server's 5s handoff,
	// cinematic suppression yields this UI to the encore's source-5s sample.
	constexpr f32_t ENCORE_CLEAR_END_FRAME = 320.f;

	/* Layer entry, position, size, alpha and tint all live in the keyframe document now,
	   so nothing is listed here. The one thing the document cannot carry is the caption:
	   the clear title is a DefineEditText (char 364, 66pt, scale 1.3 settling to 1.0 over
	   frames 126..136) and text is drawn by CMainApp::RenderRaidClearText from a rect.
	   HUD_RAIDCLEAR_TEXT_RECTS has no alpha field, so the caption is gated on at the frame
	   its own alphaMultTerm leaves 0; its 126..136 scale-in is not reproduced yet. */
	constexpr f32_t CLEAR_CAPTION_IN_FRAME = 126.f;

	/* The document's first 91 frames are empty -- nothing in it has a non-zero alpha
	until the white flash at frame 92 -- so the clear cue waits for that frame too.
	Firing it when the clock starts put the whole 5.77s sound 2.3s ahead of the
	picture. A delay on screen is a delay on the sound. */
	constexpr f32_t CLEAR_FLASH_IN_FRAME = 92.f;
	const wchar_t* const CLEAR_CUE =
		L"Sound/UI/System/sys_raid_success1__457395004.wav";
}

void Client::CLevel_KakulSaydonArena::Render_MvpPortraits()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Render_Portraits();
}

int32_t Client::CLevel_KakulSaydonArena::Current_GateNumber() const
{
	return (NO_ACTIVE_DEBUG_GATE == m_iActiveDebugGate)
		? 1 : static_cast<int32_t>(m_iActiveDebugGate) + 1;
}

void Client::CLevel_KakulSaydonArena::Update_RaidClear(const f32_t fTimeDelta)
{
	if (nullptr == m_pRaidClearView || m_fRaidClearElapsedSeconds < 0.f)
		return;

	const f32_t fPrevious = m_fRaidClearElapsedSeconds;
	m_fRaidClearElapsedSeconds += fTimeDelta;
	const f32_t fFrame = m_fRaidClearElapsedSeconds * CLEAR_FPS;
	const f32_t endFrame = m_bRaidClearShowMvp ? CLEAR_END_FRAME : ENCORE_CLEAR_END_FRAME;
	const bool_t isShowing = fFrame < endFrame;

	if (0.f == fPrevious)
	{
		m_pRaidClearView->Set_SlotVisible("RaidClear_Kouku_Frame", true);
		m_pRaidClearView->Play_KeyframeAnimation("RaidClear_Kouku_Frame", "intro");
	}
	/* epicgatecommonclear.gfx embeds no audio, so the cue is the host's to fire, and
	it belongs on the flash rather than on the clock. Same frame-crossing test the
	document's own end uses below. */
	if (fPrevious * CLEAR_FPS < CLEAR_FLASH_IN_FRAME && fFrame >= CLEAR_FLASH_IN_FRAME)
	{
		const std::filesystem::path SoundPath = CRuntimeAssetRoot::Resolve(CLEAR_CUE);
		if (!SoundPath.empty())
			CGameInstance::Get().Play_Sound(SoundPath.wstring(), 1.f);
	}
	m_pRaidClearView->Set_SlotVisible("RaidClear_Kouku_Frame", isShowing);
	/* Authoring-only marker; the caption itself is drawn from the text pass. */
	m_pRaidClearView->Set_SlotVisible("RaidClear_Kouku_TitleTextBox", false);
	m_pRaidClearView->Update(fTimeDelta);

	/* The light enters first, the crest lands on it, the caption follows. */
	HUD_RAIDCLEAR_TEXT_RECTS TextRects;
	TextRects.isValid = isShowing &&
		fFrame >= CLEAR_CAPTION_IN_FRAME &&
		m_pRaidClearView->Get_SlotRect("RaidClear_Kouku_TitleTextBox",
			TextRects.fTitleX, TextRects.fTitleY,
			TextRects.fTitleWidth, TextRects.fTitleHeight);
	CCombatHUDViewModel::Get().Set_RaidClearTextRects(TextRects);

	/* callbackFrameActionEnd: the document hides itself at its last frame and hands the
	   screen to whatever comes next. */
	if (fPrevious * CLEAR_FPS < endFrame && fFrame >= endFrame)
	{
		m_pRaidClearView->Set_AllSlotsVisible(false);
		if (m_bRaidClearShowMvp) Show_MvpResult(false);
	}
}

void Client::CLevel_KakulSaydonArena::Trigger_RaidClear()
{
	m_bRaidClearShowMvp = true;
	m_iPendingRaidMvpGate = 0u;
	m_fRaidClearElapsedSeconds = 0.f;
}

bool_t Client::CLevel_KakulSaydonArena::Is_ServerRaidActive() const
{
	using LostArk::Shared::KOUKUSAYDON_RAID_PHASE;
	const auto& state = Get_KoukuRaidState();
	return state.iRunEpoch != 0u && state.ePhase != KOUKUSAYDON_RAID_PHASE::INACTIVE &&
		state.ePhase != KOUKUSAYDON_RAID_PHASE::COMPLETE && state.ePhase != KOUKUSAYDON_RAID_PHASE::ABORTED &&
		state.ePhase < KOUKUSAYDON_RAID_PHASE::END;
}

bool_t Client::CLevel_KakulSaydonArena::Is_LocalGateParticipant() const
{
	if (!Is_ServerRaidActive()) return true;
	const auto& participants = Get_KoukuRaidState().ParticipantPlayerIds;
	const auto localId = CNetworkManager::Get().Get_LocalPlayerId();
	return std::find(participants.begin(), participants.end(), localId) != participants.end();
}

bool_t Client::CLevel_KakulSaydonArena::Can_InteractGateProgress() const
{
	using LostArk::Shared::KOUKUSAYDON_RAID_PHASE;
	const auto phase = Get_KoukuRaidState().ePhase;
    if (m_bLocalSequencePlaybackActive) return false;
	// Gate 3 clear is an automatic Server-owned Encore transition.
	if (m_GateProgress.iCurrentGate == 3u && m_GateProgress.iGateCount > 3u &&
		0u != (m_GateProgress.iClearedMask & 4u)) return false;
	if (!Is_LocalGateParticipant() || (Is_ServerRaidActive() &&
		(phase == KOUKUSAYDON_RAID_PHASE::PREPARING || phase == KOUKUSAYDON_RAID_PHASE::CINEMATIC ||
            (phase == KOUKUSAYDON_RAID_PHASE::WAIT_GATE && Get_KoukuRaidState().strGateId == "GATE3" && Get_KoukuRaidState().iEndTick)))) return false;
	// The clear mark hands over to MVP before offering the next gate vote.
	if (m_iPendingRaidMvpGate) return false;
	return !(m_fRaidClearElapsedSeconds >= 0.f && m_fRaidClearElapsedSeconds * CLEAR_FPS < CLEAR_END_FRAME) &&
		(!m_pMvpResultView || !m_pMvpResultView->Is_Visible());
}

bool_t Client::CLevel_KakulSaydonArena::Is_LocalRaidLeader() const
{
	if (Is_ServerRaidActive())
		return Is_LocalGateParticipant() && Get_KoukuRaidState().iOwnerPlayerId == CNetworkManager::Get().Get_LocalPlayerId();
	/* Outside an admitted Raid run, keep the existing solo / party leader rule. */
	const auto& Roster = m_Replication.Get_PartyRoster();
	return Roster.Members.empty() ||
		Roster.Members.front().iNetEntityId == CNetworkManager::Get().Get_LocalEntityId();
}

wstring_t Client::CLevel_KakulSaydonArena::Find_PlayerNickname(
	const LostArk::Shared::NET_ENTITY_ID iNetEntityId) const
{
	for (const REPLICATED_PLAYER_VIEW& Player : m_NameplatePlayers)
	{
		if (Player.iNetEntityId != iNetEntityId)
			continue;
		std::wstring strWide;
		if (CWorldPlayerNameplateView::Try_ConvertUtf8(Player.strNickname, strWide))
			return strWide;
	}
	return wstring_t();
}

void Client::CLevel_KakulSaydonArena::Apply_ServerGate(const size_t serverGateIndex)
{
	// Raid composition and gate-progress messages share a room, but only the
	// Raid clock commits its scene after the cinematic has finished.
	if (serverGateIndex >= 4u || Is_ServerRaidActive()) return;
    size_t gateIndex = serverGateIndex;
    if (serverGateIndex == 3u)
    {
        const auto& gates = Get_DebugGates();
        const auto bingo = std::find_if(gates.begin(), gates.end(), [](const auto& gate) {
            return gate.pAuditionPlacementId && std::string_view(gate.pAuditionPlacementId) == "boss.kakulsaydon.bingo.saydon"; });
        if (bingo == gates.end()) return;
        gateIndex = static_cast<size_t>(std::distance(gates.begin(), bingo));
    }
#ifdef _DEBUG
	if (m_iPendingDebugGate == gateIndex) return;
#endif
	std::string status;
	if (!Commit_GatePresentation(gateIndex, status))
	{
		m_strDebugGateStatus = "Server gate presentation failed: " + status;
		OutputDebugStringA((m_strDebugGateStatus + "\n").c_str());
		return;
	}
	m_iServerRaidGatePresentationEpoch = 0u;
}

void Client::CLevel_KakulSaydonArena::Apply_GateProgressState(
	const LostArk::Shared::S2C_GATE_PROGRESS_STATE& State)
{
	using namespace LostArk::Shared;
	const S2C_GATE_PROGRESS_STATE Previous = m_GateProgress;
	const bool_t bHadState = m_bGateProgressKnown;
	m_GateProgress = State;
	m_bGateProgressKnown = true;
	if (Previous.iProposalId != State.iProposalId) m_bGateVoteAnswered = false;

	/* The gate the Server raised changed (advance, a Debug button) or a restart vote re-raised
	   the same gate: present it. */
	const bool_t bRestarted = State.bClosed && GATE_PROGRESS_VOTE_RESULT::ALL_ACCEPTED == State.eResult &&
        (GATE_PROGRESS_KIND::RESTART == State.eKind || GATE_PROGRESS_KIND::ENTER_GATE3 == State.eKind);
	if (0u != State.iCurrentGate && (!bHadState || Previous.iCurrentGate != State.iCurrentGate || bRestarted))
	{
		const bool_t bClearedNow = 0u != (State.iClearedMask & (1u << (State.iCurrentGate - 1u)));
		if (!bClearedNow)
		{
			m_GateProgressView.Close_Prompt();
			if (nullptr != m_pMvpResultView)
				m_pMvpResultView->Hide();
			m_fRaidClearElapsedSeconds = -1.f;
			m_iPendingRaidMvpGate = 0u;
			if (nullptr != m_pRaidClearView)
				m_pRaidClearView->Set_AllSlotsVisible(false);
			m_bGateVoteAnswered = false;
			Apply_ServerGate(static_cast<size_t>(State.iCurrentGate - 1u));
		}
	}
	/* This gate just cleared: the clear mark, then the award page (Update_RaidClear). */
	if (0u != State.iCurrentGate)
	{
		const uint8_t iBit = static_cast<uint8_t>(1u << (State.iCurrentGate - 1u));
		const bool_t bWasCleared = bHadState && Previous.iCurrentGate == State.iCurrentGate &&
			0u != (Previous.iClearedMask & iBit);
		if (0u != (State.iClearedMask & iBit) && !bWasCleared)
		{
			if (nullptr != m_pMvpResultView)
				m_pMvpResultView->Hide();
			m_bGateVoteAnswered = false;
			Trigger_RaidClear();
            m_bRaidClearShowMvp = !(State.iCurrentGate == 3u && State.iGateCount > 3u);
			m_iPendingRaidMvpGate = m_bRaidClearShowMvp ? State.iCurrentGate : 0u;
		}
	}
	/* Vote: a member gets the accept / decline prompt once; the proposer waits. */
	if (0u != State.iProposalId && !State.bClosed)
	{
		const bool_t bMine = State.iProposerNetEntityId == CNetworkManager::Get().Get_LocalEntityId();
		if (GATE_PROGRESS_KIND::ENTER_GATE3 == State.eKind || bMine || !Can_InteractGateProgress())
		{
			if (Is_GateVotePromptOpen())
				m_GateProgressView.Close_Prompt();
		}
		else if (!m_bGateVoteAnswered && !Is_GateVotePromptOpen())
		{
			m_GateProgressView.Open_Prompt(Gate_VotePrompt(State.eKind),
				Find_PlayerNickname(State.iProposerNetEntityId));
		}
	}
	if (State.bClosed)
	{
		m_GateProgressView.Close_Prompt();
		m_bGateVoteAnswered = false;
		if (GATE_PROGRESS_KIND::ENTER_GATE3 == State.eKind &&
			GATE_PROGRESS_VOTE_RESULT::ALL_ACCEPTED != State.eResult)
		{
			m_GateProgressView.Show_Notice(
				L"\uC785\uC7A5\uD560 \uC218 \uC5C6\uC2B5\uB2C8\uB2E4. \uC9D1\uACB0 \uC9C0\uC810\uC5D0\uC11C \uB098\uAC14\uB2E4\uAC00 \uB2E4\uC2DC \uB4E4\uC5B4\uC640 \uC8FC\uC138\uC694.", 4.f);
		}
		else if (GATE_PROGRESS_VOTE_RESULT::ALL_ACCEPTED != State.eResult)
		{
			/* sys.commander.progress_vote_fail_dialog_desc */
			const wstring_t strOutcome = GATE_PROGRESS_KIND::EXIT == State.eKind ?
				L"\xB098\xAC00\xAE30\xAC00 \xCDE8\xC18C\xB418\xC5C8\xC2B5\xB2C8\xB2E4." :
				L"\xB2E4\xC74C \xAD00\xBB38 \xC785\xC7A5\xC774 \xCDE8\xC18C\xB418\xC5C8\xC2B5\xB2C8\xB2E4.";
			m_GateProgressView.Show_Notice(
				wstring_t(L"\xD22C\xD45C\xC5D0 \xC751\xB2F5\xD558\xC9C0 \xC54A\xC558\xAC70\xB098 \xAC70\xC808\xD55C \xC778\xC6D0\xC774 \xC788\xC5B4 ") +
				strOutcome, 4.f);
		}
	}
}

void Client::CLevel_KakulSaydonArena::Update_Gate3EntryAura(const bool_t entryAvailable)
{
	using namespace LostArk::Shared;
	const auto& raid = Get_KoukuRaidState();
	const auto epoch = Is_ServerRaidActive() ? raid.iRunEpoch : 0u;
	std::vector<REPLICATED_PLAYER_VIEW> players;
	m_Replication.Collect_PlayerViews(players);
	std::vector<NET_ENTITY_ID> participants;
	if (Is_ServerRaidActive())
	{
		auto playerIds = raid.ParticipantPlayerIds;
		const auto owner = std::find(playerIds.begin(), playerIds.end(), raid.iOwnerPlayerId);
		if (owner != playerIds.end())
		{
			std::rotate(playerIds.begin(), owner, owner + 1);
			for (const auto id : playerIds)
			{
				const auto player = std::find_if(players.begin(), players.end(),
					[&](const auto& view) { return view.iPlayerId == id; });
				participants.push_back(player != players.end() ? player->iNetEntityId : INVALID_NET_ENTITY_ID);
			}
		}
	}
	else
	{
		for (const auto& member : m_Replication.Get_PartyRoster().Members)
			participants.push_back(member.iNetEntityId);
		if (participants.empty()) participants.push_back(CNetworkManager::Get().Get_LocalEntityId());
	}
	if (epoch != m_iGate3AuraRunEpoch || participants != m_Gate3AuraParticipants)
	{
		m_iGate3AuraRunEpoch = epoch;
		m_Gate3AuraParticipants = participants;
		m_bGate3AuraOccupied = false;
	}
	std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
	std::vector<KOUKU_CARD_PRESENTATION_VIEW> snapshots;
	m_Replication.Collect_KoukuPresentationViews(bosses, snapshots);
	bool_t occupied = entryAvailable && !participants.empty() && participants.size() <= 4u;
	for (size_t i = 0u; occupied && i < participants.size(); ++i)
	{
		const auto player = std::find_if(snapshots.begin(), snapshots.end(),
			[&](const auto& view) { return view.Snapshot.iNetEntityId == participants[i]; });
		if (player == snapshots.end()) { occupied = false; break; }
		const auto& state = player->Snapshot;
		occupied = state.eControlKind == PLAYER_CONTROL_KIND::HUMAN && state.iCurrentHp > 0u &&
			state.eAction != PLAYER_ACTION_STATE::DEAD && state.eAction != PLAYER_ACTION_STATE::FALLING &&
			state.eAction != PLAYER_ACTION_STATE::TRIGGER_MOVE;
		if (i == 0u)
			occupied = occupied && Is_KoukuGate3EntryAura(state.fPositionX, state.fPositionY, state.fPositionZ);
	}
	if (!occupied)
	{
		m_bGate3AuraOccupied = false;
		m_fGate3AuraSecondsLeft = 10.f;
		return;
	}
	const auto tick = m_Replication.Get_LastServerTick();
	if (!m_bGate3AuraOccupied) m_iGate3AuraStartTick = tick;
	m_bGate3AuraOccupied = true;
	// Presentation only: the Server owns the 300-tick hold and commits entry without a vote.
	const auto elapsed = tick - m_iGate3AuraStartTick;
	if (elapsed > 0x7fffffffu) { m_iGate3AuraStartTick = tick; m_fGate3AuraSecondsLeft = 10.f; return; }
	m_fGate3AuraSecondsLeft = (std::max)(0.f, 10.f - static_cast<float>(elapsed) / 30.f);
}

void Client::CLevel_KakulSaydonArena::Clear_Gate3Auras()
{
	for (auto& handle : m_Gate3AuraHandles)
	{
		CEffectPresentationService::Stop_WorldRoot(handle);
		handle = {};
	}
	m_Gate3AuraAttempted = {};
	m_strGate3AuraFailure.clear();
}

void Client::CLevel_KakulSaydonArena::Submit_Gate3Auras()
{
	if (!Is_AtGate3EntryTerrace() || Is_CinematicPresentationActive())
	{
		Clear_Gate3Auras();
		return;
	}
	const char* assets[] = { m_bGate3AuraOccupied ? "effect.world.entry_aura.active" : "effect.world.entry_aura",
		"effect.world.respawn_aura" };
	const float3_t positions[] = { { -22.20617676f, 25.59f, 954.59429688f }, { -11.9999292f, 25.59f, 964.54328125f } };
	const float rotations[] = { -67.5f, -22.5f };
	for (size_t i = 0; i < m_Gate3AuraHandles.size(); ++i)
	{
		if (m_Gate3AuraAttempted[i] == assets[i]) continue;
		m_Gate3AuraAttempted[i] = assets[i];
		EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
		desc.iLevelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
		desc.strPlacementId = i == 0 ? "kouku.gate3.entry_aura" : "kouku.gate3.respawn_aura";
		desc.strEffectAssetId = assets[i];
		desc.bOwnerSustainedSourceLoops = true;
		XMStoreFloat4x4(&desc.RootWorld, XMMatrixRotationY(XMConvertToRadians(rotations[i])) *
			XMMatrixTranslation(positions[i].x, positions[i].y, positions[i].z));
		EFFECT_WORLD_ROOT_HANDLE staged;
		std::string status;
		if (!CEffectPresentationService::Spawn_LevelPlacement(desc, staged, status))
		{
			m_strGate3AuraFailure = status;
			OutputDebugStringA(("[Gate3Aura] " + desc.strEffectAssetId + ": " + status + "\n").c_str());
			continue;
		}
		CEffectPresentationService::Commit_PendingWorldRootSpawns({ staged });
		if (!CEffectPresentationService::Update_WorldRoot(staged, desc.RootWorld))
		{
			m_strGate3AuraFailure = CEffectPresentationService::Get_Status();
			CEffectPresentationService::Stop_WorldRoot(staged);
			OutputDebugStringA(("[Gate3Aura] Activation failed; previous aura preserved: " +
				m_strGate3AuraFailure + "\n").c_str());
			continue;
		}
		CEffectPresentationService::Stop_WorldRoot(m_Gate3AuraHandles[i]);
		m_Gate3AuraHandles[i] = staged;
	}
}

void Client::CLevel_KakulSaydonArena::Update_GateProgress(const f32_t fTimeDelta)
{
	using namespace LostArk::Shared;
	S2C_GATE_PROGRESS_STATE State{};
	while (nullptr != m_pPlayerCommandSink && m_pPlayerCommandSink->Consume_GateProgressState(State))
		Apply_GateProgressState(State);
	S2C_RAID_MVP_RESULT MvpResult{};
	while (nullptr != m_pPlayerCommandSink && m_pPlayerCommandSink->Consume_RaidMvpResult(MvpResult))
	{
		m_RaidMvpResult = std::move(MvpResult);
		m_bHasRaidMvpResult = true;
		m_bRaidMvpResultFresh = true;
	}
	if (m_iPendingRaidMvpGate && m_bRaidClearShowMvp &&
		m_fRaidClearElapsedSeconds * CLEAR_FPS >= CLEAR_END_FRAME)
		Show_MvpResult(false);

	/* Defer votes under a cinematic, clear mark or MVP; late joins observe only. */
	const bool_t canInteract = Can_InteractGateProgress();
	if (!canInteract || (!Is_GateVotePromptOpen() && !Is_LocalRaidLeader()))
		m_GateProgressView.Close_Prompt();
	const bool_t bMvpVisible = nullptr != m_pMvpResultView && m_pMvpResultView->Is_Visible();
	const bool_t bVoteOpen = 0u != m_GateProgress.iProposalId && !m_GateProgress.bClosed &&
		m_GateProgress.eKind != GATE_PROGRESS_KIND::ENTER_GATE3;
	if (canInteract && bVoteOpen && !m_bGateVoteAnswered &&
		CRaidGateProgressView::PROMPT::NONE == m_GateProgressView.Get_Prompt() &&
		m_GateProgress.iProposerNetEntityId != CNetworkManager::Get().Get_LocalEntityId())
	{
		m_GateProgressView.Open_Prompt(Gate_VotePrompt(m_GateProgress.eKind),
			Find_PlayerNickname(m_GateProgress.iProposerNetEntityId));
	}
	// Closing the cleared Gate 2 award page submits the existing Server vote once.
	// Party members still consent through the typed gate-progress contract.
	if (m_bMvpWasVisible && !bMvpVisible && canInteract && !bVoteOpen &&
		m_GateProgress.iCurrentGate == 2u && (m_GateProgress.iClearedMask & 2u) &&
		Is_LocalRaidLeader() && nullptr != m_pPlayerCommandSink)
		(void)m_pPlayerCommandSink->Request_GateProgressPropose(
			m_iNextGateRequestSequence++, GATE_PROGRESS_KIND::ADVANCE);
	m_bMvpWasVisible = bMvpVisible;

	/* The panel button follows the raid: restart while a gate is up, leave (an exit vote back
	   to Bern) in the gate 3 waiting deck, dungeon progress once a gate short of the last is
	   cleared, exit once the last one is. The leader presses everything but the final exit,
	   which every player can use to go back on their own, and nothing while a vote is running
	   or the award page is up. */
	/* Before any gate is raised (fresh room, or before the F1 button) the panel treats gate 1
	   as current: the restart button then raises it, as the Server's RESTART rule does. */
	const uint8_t iShownGate = (std::max<uint8_t>)(m_GateProgress.iCurrentGate, 1u);
	const uint8_t iShownCount = (std::max<uint8_t>)(m_GateProgress.iGateCount, KOUKU_GATE_COUNT);
	const bool_t bCleared = 0u != (m_GateProgress.iClearedMask & (1u << (iShownGate - 1u)));
	CRaidGateProgressView::BUTTON eButton = CRaidGateProgressView::BUTTON::RESTART;
	if (bCleared)
		eButton = iShownGate < iShownCount ? CRaidGateProgressView::BUTTON::PROGRESS : CRaidGateProgressView::BUTTON::EXIT;
	if (bCleared && iShownGate == 3u && iShownCount > 3u)
		eButton = CRaidGateProgressView::BUTTON::NONE;
	/* The waiting deck before gate 3: the panel offers the exit vote there, and the party enters
	   the gate by standing on the entry aura. */
	const bool_t bAtGate3Deck = eButton != CRaidGateProgressView::BUTTON::NONE &&
		(Get_KoukuRaidState().ePhase == KOUKUSAYDON_RAID_PHASE::WAIT_ENTRY ||
        (!Is_ServerRaidActive() && Is_AtGate3EntryTerrace()));
	if (bAtGate3Deck)
		eButton = CRaidGateProgressView::BUTTON::LEAVE;
	const bool_t canPropose = canInteract && Is_LocalRaidLeader() && !bVoteOpen;
	/* The final exit is the player's own trip back to Bern, so it is not the leader's alone. */
	const bool_t canExit = canInteract && !bVoteOpen;
	Update_Gate3EntryAura(bAtGate3Deck && canInteract);
	m_GateProgressView.Set_Button(eButton,
		CRaidGateProgressView::BUTTON::EXIT == eButton ? canExit : canPropose);
	// Bingo is the encore after the three displayed gate icons.
	m_GateProgressView.Set_Progress(iShownGate, iShownGate > 3u ?
		static_cast<uint8_t>(m_GateProgress.iClearedMask | 0x7u) : m_GateProgress.iClearedMask);
	const CRaidGateProgressView::INTENT eIntent = m_GateProgressView.Update(fTimeDelta);
	if (nullptr == m_pPlayerCommandSink || !canInteract)
		return;
	switch (eIntent)
	{
	case CRaidGateProgressView::INTENT::PROPOSE_ADVANCE:
		if (!canPropose) break;
		(void)m_pPlayerCommandSink->Request_GateProgressPropose(m_iNextGateRequestSequence++, GATE_PROGRESS_KIND::ADVANCE);
		break;
	case CRaidGateProgressView::INTENT::PROPOSE_RESTART:
		if (!canPropose) break;
		(void)m_pPlayerCommandSink->Request_GateProgressPropose(m_iNextGateRequestSequence++, GATE_PROGRESS_KIND::RESTART);
		break;
	case CRaidGateProgressView::INTENT::PROPOSE_EXIT:
		if (!canPropose) break;
		(void)m_pPlayerCommandSink->Request_GateProgressPropose(m_iNextGateRequestSequence++, GATE_PROGRESS_KIND::EXIT);
		break;
	case CRaidGateProgressView::INTENT::EXIT:
		if (!canExit) break;
		(void)m_pPlayerCommandSink->Request_ReturnToBern(m_iNextGateRequestSequence++);
		break;
	case CRaidGateProgressView::INTENT::ACCEPT:
	case CRaidGateProgressView::INTENT::DECLINE:
		if (!bVoteOpen || m_bGateVoteAnswered ||
			m_GateProgress.iProposerNetEntityId == CNetworkManager::Get().Get_LocalEntityId()) break;
		m_bGateVoteAnswered = true;
		(void)m_pPlayerCommandSink->Request_GateProgressRespond(m_iNextGateRequestSequence++,
			m_GateProgress.iProposalId, CRaidGateProgressView::INTENT::ACCEPT == eIntent);
		break;
	default:
		break;
	}
}

bool_t Client::CLevel_KakulSaydonArena::Is_GateVotePromptOpen() const
{
	const CRaidGateProgressView::PROMPT ePrompt = m_GateProgressView.Get_Prompt();
	return CRaidGateProgressView::PROMPT::VOTE_ADVANCE == ePrompt ||
		CRaidGateProgressView::PROMPT::VOTE_RESTART == ePrompt ||
		CRaidGateProgressView::PROMPT::VOTE_EXIT == ePrompt ||
        CRaidGateProgressView::PROMPT::VOTE_ENTER_GATE3 == ePrompt;
}

Client::CRaidGateProgressView::PROMPT Client::CLevel_KakulSaydonArena::Gate_VotePrompt(
	const LostArk::Shared::GATE_PROGRESS_KIND eKind)
{
    if (LostArk::Shared::GATE_PROGRESS_KIND::ENTER_GATE3 == eKind)
        return CRaidGateProgressView::PROMPT::VOTE_ENTER_GATE3;
	if (LostArk::Shared::GATE_PROGRESS_KIND::EXIT == eKind)
		return CRaidGateProgressView::PROMPT::VOTE_EXIT;
	return LostArk::Shared::GATE_PROGRESS_KIND::RESTART == eKind ?
		CRaidGateProgressView::PROMPT::VOTE_RESTART : CRaidGateProgressView::PROMPT::VOTE_ADVANCE;
}

void Client::CLevel_KakulSaydonArena::Debug_Play_ClearThenMvp()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Hide();
	Trigger_RaidClear();
}

void Client::CLevel_KakulSaydonArena::Debug_Show_MvpResult()
{
	Show_MvpResult(true);
}

void Client::CLevel_KakulSaydonArena::Debug_Hide_MvpResult()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Hide();
	m_fRaidClearElapsedSeconds = -1.f;
	m_iPendingRaidMvpGate = 0u;
	if (nullptr != m_pRaidClearView)
		m_pRaidClearView->Set_AllSlotsVisible(false);
}

bool_t Client::CLevel_KakulSaydonArena::Debug_Is_MvpResultVisible() const
{
	return nullptr != m_pMvpResultView && m_pMvpResultView->Is_Visible();
}

bool_t Client::CLevel_KakulSaydonArena::Load_StageMarkers(
	std::string& outStatus)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.StageMarkers.Load");
	const std::filesystem::path path = Find_StageMarkerDocument();
	std::error_code fileError;
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (path.empty() || fileError || 0u == fileBytes || fileBytes > 256u * 1024u)
	{
		outStatus = "KoukuSaydon StageMarkers document is missing or exceeds 256 KiB.";
		return false;
	}

	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "KoukuSaydon StageMarkers document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "KoukuSaydon StageMarkers document could not be read completely.";
		return false;
	}

	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 256u * 1024u;
	limits.iMaximumDepth = 12u;
	limits.iMaximumValues = 4096u;
	if (!CDataJson::Parse(text, root, parseError, limits) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "worldId", "areaId", "revision",
				"semanticStatus", "stages" }))
	{
		outStatus = "KoukuSaydon StageMarkers root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* world = Required(root, "worldId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* area = Required(root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(root, "revision", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* semanticStatus = Required(
		root, "semanticStatus", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* stages = Required(root, "stages", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || STAGE_MARKER_SCHEMA != schema->Get_String() ||
		nullptr == version || version->Get_Number() != 1.0 ||
		nullptr == world || world->Get_String() != "KAKULSAYDON_ARENA" ||
		nullptr == area || KAKULSAYDON_AREA_ID != area->Get_String() ||
		nullptr == revision || !std::isfinite(revision->Get_Number()) ||
		revision->Get_Number() < 1.0 ||
		std::floor(revision->Get_Number()) != revision->Get_Number() ||
		nullptr == semanticStatus ||
		STAGE_SEMANTIC_STATUS != semanticStatus->Get_String() ||
		nullptr == stages || stages->Get_Array().empty() ||
		stages->Get_Array().size() > 64u)
	{
		outStatus = "KoukuSaydon StageMarkers header is invalid.";
		return false;
	}

	std::vector<KAKUL_STAGE_MARKER> stagedMarkers;
	std::unordered_set<std::string> stagedIds;
	std::unordered_set<std::string> stagedPlacementIds;
	stagedMarkers.reserve(stages->Get_Array().size());
	for (const DATA_JSON_VALUE& value : stages->Get_Array())
	{
		if (!Has_ExactProperties(value,
			{ "stageId", "placementId", "displayNameKo", "sourceLevelId" }))
		{
			outStatus = "KoukuSaydon StageMarkers stage has unexpected properties.";
			return false;
		}
		const DATA_JSON_VALUE* stageId = Required(value, "stageId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* placementId = Required(value, "placementId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* displayName = Required(value, "displayNameKo", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* sourceLevelId = Required(value, "sourceLevelId", DATA_JSON_TYPE::STRING);
		if (nullptr == stageId || nullptr == placementId || nullptr == displayName ||
			nullptr == sourceLevelId ||
			!Is_StableId(stageId->Get_String()) ||
			!stageId->Get_String().starts_with("stage.kakul.") ||
			stageId->Get_String() != placementId->Get_String() ||
			!Is_DisplayText(displayName->Get_String()) ||
			!Is_StableId(sourceLevelId->Get_String()) ||
			!stagedIds.emplace(stageId->Get_String()).second ||
			!stagedPlacementIds.emplace(placementId->Get_String()).second)
		{
			outStatus = "KoukuSaydon StageMarkers stage identity or evidence is invalid.";
			return false;
		}
		stagedMarkers.push_back({
			stageId->Get_String(), placementId->Get_String(),
			displayName->Get_String(), sourceLevelId->Get_String() });
	}

	m_StageMarkers = std::move(stagedMarkers);
	m_StageMarkerPlacementIds = std::move(stagedPlacementIds);
	outStatus = "KoukuSaydon StageMarkers loaded.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Try_Get_AuthoringPreviewPlacement(
	float3_t& outPosition, std::string& outStatus) const
{
	const shared_ptr<CCharacter> localCharacter = m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		outStatus = "Waiting for the replicated local player in the KoukuSaydon arena.";
		return false;
	}
	const shared_ptr<CTransform> transform = localCharacter->Get_Transform();
	if (nullptr == transform)
	{
		outStatus = "The replicated local player has no transform for preview placement.";
		return false;
	}
	const vector_t playerPosition = transform->Get_State(STATE::POSITION);
	float3_t position{};
	XMStoreFloat3(&position, playerPosition);
	if (!std::isfinite(position.x) || !std::isfinite(position.y) ||
		!std::isfinite(position.z))
	{
		outStatus = "The replicated local player position is not finite.";
		return false;
	}

	vector_t screenRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	if (nullptr != m_pCamera)
	{
		const shared_ptr<CTransform> cameraTransform = dynamic_pointer_cast<CTransform>(
			m_pCamera->Get_Component(g_strTransformComTag));
		if (nullptr != cameraTransform)
		{
			vector_t candidate = cameraTransform->Get_State(STATE::RIGHT);
			candidate = XMVectorSetW(XMVectorSetY(candidate, 0.f), 0.f);
			const f32_t lengthSquared = XMVectorGetX(XMVector3LengthSq(candidate));
			if (std::isfinite(lengthSquared) && lengthSquared > 0.000001f)
				screenRight = XMVector3Normalize(candidate);
		}
	}

	constexpr f32_t PREVIEW_OFFSET_METERS = 3.25f;
	for (const f32_t direction : std::array<f32_t, 2>{ 1.f, -1.f })
	{
		float3_t candidate{};
		XMStoreFloat3(&candidate,
			playerPosition + screenRight * (PREVIEW_OFFSET_METERS * direction));
		float3_t sampled{};
		if (localCharacter->Try_SampleTargetGround(candidate.x, candidate.z, sampled) &&
			std::isfinite(sampled.x) && std::isfinite(sampled.y) && std::isfinite(sampled.z))
		{
			outPosition = sampled;
			outStatus = direction > 0.f ?
				"replicated local player / camera-right / Navigation" :
				"replicated local player / camera-left / Navigation";
			return true;
		}
	}

	// Navigation is optional for this collision-off view; retain the player's height.
	XMStoreFloat3(&outPosition, playerPosition + screenRight * PREVIEW_OFFSET_METERS);
	outStatus = "replicated local player / camera-right / unclamped";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Try_Get_AuthoringForwardPlacement(
	float3_t& outPosition, std::string& outStatus) const
{
	const auto character = m_Replication.Get_LocalCharacter();
	const auto transform = character ? character->Get_Transform() : nullptr;
	if (!transform) { outStatus = "WORLD placement requires the replicated local player."; return false; }
	const vector_t origin = transform->Get_State(STATE::POSITION);
	vector_t forward = XMVectorSetW(XMVectorSetY(transform->Get_State(STATE::LOOK), 0.f), 0.f);
	const float lengthSquared = XMVectorGetX(XMVector3LengthSq(forward));
	if (!std::isfinite(lengthSquared) || lengthSquared <= .000001f)
	{ outStatus = "The local player has no finite horizontal facing for WORLD placement."; return false; }
	forward = XMVector3Normalize(forward);
	float3_t candidate;
	XMStoreFloat3(&candidate, origin + forward * 3.25f);
	if (!std::isfinite(candidate.x) || !std::isfinite(candidate.y) || !std::isfinite(candidate.z))
	{ outStatus = "The local player's WORLD placement is not finite."; return false; }
	float3_t sampled;
	if (character->Try_SampleTargetGround(candidate.x, candidate.z, sampled) &&
		std::isfinite(sampled.x) && std::isfinite(sampled.y) && std::isfinite(sampled.z))
		candidate = sampled;
	outPosition = candidate;
	outStatus = "Placed ahead of the current player; the saved world position remains fixed.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Request_StageTeleport(
	const std::uint32_t requestSequence,
	const std::string_view placementId,
	std::string& outStatus)
{
	if (0u == requestSequence || placementId.empty())
	{
		outStatus = "KoukuSaydon stage teleport request identity is invalid.";
		return false;
	}
	if (m_StageMarkerPlacementIds.empty())
	{
		outStatus = "KoukuSaydon StageMarkers are not authored; teleport is isolated.";
		return false;
	}
	if (!m_StageMarkerPlacementIds.contains(std::string(placementId)))
	{
		outStatus = "KoukuSaydon stage marker placement ID is not authored.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		!m_pWorldEntityCommandSink->Request_StageTeleport(
			requestSequence, placementId))
	{
		outStatus = "KoukuSaydon stage teleport command was rejected.";
		return false;
	}
	outStatus = "KoukuSaydon stage teleport command submitted.";
	return true;
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Set_DebugCameraSpeed(const f32_t metersPerSecond)
{
	if (nullptr == m_pCamera || !m_pCamera->Set_FreeMoveSpeed(metersPerSecond))
		return false;
	g_KakulSaydonFreeCameraSpeed = metersPerSecond;
	return true;
}

#endif

bool_t Client::CLevel_KakulSaydonArena::Debug_ReturnToStart(std::string& outStatus)
{
	if (Is_DebugGatePending() || m_PlayerController.Is_DebugPlayerPlacementPending())
	{ outStatus = m_strDebugGateStatus = "Wait for the pending Server placement before returning to start."; return false; }
	if (!m_PlayerController.Request_DebugReturnToKoukuStart())
	{ outStatus = m_strDebugGateStatus = m_PlayerController.Get_DebugPlayerPlacementStatus(); return false; }
	m_bDebugStartPending = true; m_bDebugStartSucceeded = false;
	m_fDebugGatePendingSeconds = 0.f;
	CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(true);
	outStatus = m_strDebugGateStatus = "Waiting for Server reset of this arena's bosses and entry triggers; only your player returns to start.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Is_DebugGateApprovedForServerPlay(const size_t gateIndex) const
{
	// A retained raid scene is not a spawn receipt. A new raid invalidates
	// the old Debug approval even before its final despawn is presented.
	return gateIndex != NO_ACTIVE_DEBUG_GATE && m_iActiveDebugGate == gateIndex &&
		m_DebugGateApproval.iWorldGeneration != 0u &&
		m_DebugGateApproval.iWorldGeneration == CNetworkManager::Get().Get_WorldInboundGeneration() &&
		m_DebugGateApproval.iRaidEpoch == Get_KoukuRaidState().iRunEpoch;
}

bool_t Client::CLevel_KakulSaydonArena::Debug_ActivateGate(
	const size_t gateIndex, std::string& outStatus, const bool_t preservePlayerPosition)
{
	const auto& gates = Get_DebugGates();
	if (gateIndex >= gates.size())
	{
		outStatus = m_strDebugGateStatus = "Unknown KoukuSaydon gate index.";
		return false;
	}
	const KAKUL_DEBUG_GATE& gate = gates[gateIndex];
	const std::string label = nullptr != gate.pLabel ? gate.pLabel : "gate";
	if (nullptr != gate.pDeferredReason)
	{
		outStatus = m_strDebugGateStatus = label + ": " + gate.pDeferredReason;
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink ||
		nullptr == m_Replication.Get_LocalCharacter())
	{
		outStatus = m_strDebugGateStatus =
			label + ": the replicated local player or command sink is unavailable.";
		return false;
	}
	/* Pre-check before any command leaves: a gate change while the previous
	   player move is still unanswered would replace the bosses but leave the
	   player at the old gate. Refusing here keeps boss, HUD and player on the
	   gate that is already in flight. */
	if (Is_DebugGatePending() || m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		outStatus = m_strDebugGateStatus =
			label + ": the previous gate's player move is still awaiting the Server; wait for its reply.";
		return false;
	}
	if (0u == m_iNextDebugGateRequestSequence)
	{
		outStatus = m_strDebugGateStatus =
			label + ": gate request sequence is exhausted; restart the Client.";
		return false;
	}
    if (!Debug_PrepareGateObjects(gateIndex, outStatus))
    { m_strDebugGateStatus = label + ": " + outStatus; return false; }
    const auto lightSource = m_pMapLightAuthoringOverride ? m_pMapLightAuthoringOverride : m_pMapLightPresentation;
    m_pPendingGateMapLights.reset(); m_PendingGateMapLightSource.reset();
    if ((gateIndex == 0u || gateIndex == 2u) &&
        (!lightSource || !Prepare_GateMapLights(lightSource->Get_Document(), gateIndex, m_pPendingGateMapLights, outStatus)))
    {
        Debug_CancelGateObjects();
        if (!lightSource) outStatus = "Gate Area lighting is unavailable.";
        m_strDebugGateStatus = label + ": " + outStatus; return false;
    }
    if (lightSource) m_PendingGateMapLightSource = lightSource->Get_Document();
	const std::uint32_t requestSequence = m_iNextDebugGateRequestSequence;
	if ((std::numeric_limits<std::uint32_t>::max)() == m_iNextDebugGateRequestSequence)
		m_iNextDebugGateRequestSequence = 0u;
	else
		++m_iNextDebugGateRequestSequence;

	/* These ordered commands have separate Server results. Only their
	   confirmed success commits the active gate, HUD and audition target. */
	if (!m_pWorldEntityCommandSink->Request_DespawnAllWorldEntities(requestSequence))
	{
        Debug_CancelGateObjects(); m_pPendingGateMapLights.reset(); m_PendingGateMapLightSource.reset();
		outStatus = m_strDebugGateStatus = label + ": despawn command was rejected.";
		return false;
	}
	CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(true);
	m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	m_iPendingDebugGate = gateIndex;
	m_DebugGateApproval = {};
	m_PendingDebugGateApproval = { CNetworkManager::Get().Get_WorldInboundGeneration(), Get_KoukuRaidState().iRunEpoch };
	m_bDebugGatePreservesPlayerPosition = preservePlayerPosition;
	m_fDebugGatePendingSeconds = 0.f;
	m_bDebugGateFailed = false;
	m_DebugGatePendingPlacements.clear();
	std::size_t spawnRequests = 0u;
	for (const char_t* pPlacementId : gate.BossPlacementIds)
	{
		if (nullptr == pPlacementId)
			continue;
		std::uint64_t requestToken = 0u;
		if (!m_pWorldEntityCommandSink->Request_SpawnWorldEntity(pPlacementId, &requestToken))
		{
			m_bDebugGateFailed = true;
			outStatus = m_strDebugGateStatus =
				label + ": spawn command was rejected for " + pPlacementId;
			return false;
		}
		m_DebugGatePendingPlacements.emplace(pPlacementId, requestToken);
		++spawnRequests;
	}
	const bool_t teleportSubmitted = preservePlayerPosition || m_PlayerController.Request_DebugTeleportToPosition(
		LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA,
		gate.vPlayerPosition.x, gate.vPlayerPosition.y, gate.vPlayerPosition.z);
	m_bDebugGateFailed = !teleportSubmitted;
	char_t summary[256]{};
	sprintf_s(summary,
		": despawn + %zu spawn request(s) sent; player -> (%.2f, %.2f, %.2f) %s",
		spawnRequests, gate.vPlayerPosition.x, gate.vPlayerPosition.y,
		gate.vPlayerPosition.z, teleportSubmitted ? "submitted" : "not submitted");
	m_strDebugGateStatus = preservePlayerPosition ?
		label + ": boss activation submitted; preserving Server-approved sequence arrival positions." : label + summary;
	if (!teleportSubmitted)
	{
		m_strDebugGateStatus += " (" +
			m_PlayerController.Get_DebugPlayerPlacementStatus() + ")";
	}
	outStatus = m_strDebugGateStatus;
	return teleportSubmitted;
}

void Client::CLevel_KakulSaydonArena::Debug_RetireGateActivation(const std::string& reason)
{
	// Start placement keeps the Controller's pending request until its Server reply.
	if (m_iPendingDebugGate == NO_ACTIVE_DEBUG_GATE) return;
	const std::string finalReason = reason;
    Debug_CancelGateObjects(); m_pPendingGateMapLights.reset(); m_PendingGateMapLightSource.reset();
	m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
	m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	m_DebugGatePendingPlacements.clear();
	m_fDebugGatePendingSeconds = 0.f;
	m_bDebugGateFailed = true;
	m_bDebugGatePreservesPlayerPosition = false;
	m_strDebugGateStatus = finalReason;
	m_PlayerController.Retire_DebugPlayerPlacementRequest(finalReason);
	CCombatHUDViewModel::Get().Set_BossFocusArchetype("");
	CCombatHUDViewModel::Get().Set_BossHidden(true);
	CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss("", "");
	CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(false);
}

bool_t Client::CLevel_KakulSaydonArena::Debug_DespawnArenaBosses(std::string& outStatus)
{
	if (Is_DebugGatePending() || m_PlayerController.Is_DebugPlayerPlacementPending())
	{
		outStatus = m_strDebugGateStatus = "Wait for the pending gate request before despawning.";
		return false;
	}
	if (nullptr == m_pWorldEntityCommandSink || 0u == m_iNextDebugGateRequestSequence)
	{
		outStatus = m_strDebugGateStatus =
			"Despawn requires the command sink and an available request sequence.";
		return false;
	}
	const std::uint32_t requestSequence = m_iNextDebugGateRequestSequence;
	if ((std::numeric_limits<std::uint32_t>::max)() == m_iNextDebugGateRequestSequence)
		m_iNextDebugGateRequestSequence = 0u;
	else
		++m_iNextDebugGateRequestSequence;
	if (!m_pWorldEntityCommandSink->Request_DespawnAllWorldEntities(requestSequence))
	{
		outStatus = m_strDebugGateStatus = "Despawn command was rejected.";
		return false;
	}
	CCombatHUDViewModel::Get().Clear_BossFocus();
	CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss("", "");
	m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	outStatus = m_strDebugGateStatus =
		"Despawn of Debug-activated arena bosses submitted; HUD focus and audition target reset.";
	return true;
}

const std::array<Client::CLevel_KakulSaydonArena::KAKUL_DEBUG_GATE, 9>&
Client::CLevel_KakulSaydonArena::Get_DebugGates()
{
	/* Boss positions are the disabled placements in
	   Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json; only the player
	   position, the HUD focus and the audition target are Client Debug
	   values. Labels are UTF-8 byte escapes so the source encoding never
	   changes them. */
	static const std::array<KAKUL_DEBUG_GATE, 9> gates = { {
		// 1관문 - 세이튼
		KAKUL_DEBUG_GATE{ "1" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.g1.saydon", nullptr } },
			float3_t(-2.45f, 1.32f, 740.37f),
			"BOSS_KAKULSAYDON_G1_SAYDON", "boss.kakulsaydon.g1.saydon", nullptr },
		// 2관문 - 대형 세이튼, 쿠크 (HUD and audition follow Kouku)
		KAKUL_DEBUG_GATE{ "2" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEB\x8C\x80\xED\x98\x95" " " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC" ", " "\xEC\xBF\xA0\xED\x81\xAC",
			{ { "boss.kakulsaydon.g2.big-saydon", "boss.kakulsaydon.g2.kouku" } },
			float3_t(3.38f, 10.56f, 323.92f),
			"BOSS_KAKULSAYDON_G2_KOUKU", "boss.kakulsaydon.g2.kouku", nullptr },
		// 3관문 - 세이튼
		KAKUL_DEBUG_GATE{ "3" "\xEA\xB4\x80\xEB\xAC\xB8" " - " "\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.g3.saydon", nullptr } },
			float3_t(-2.45f, 1.32f, 945.17f),
			"BOSS_KAKULSAYDON_G3_SAYDON", "boss.kakulsaydon.g3.saydon", nullptr },
		// 1마리오 - player only
		KAKUL_DEBUG_GATE{ "1" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4" " (" "\xED\x94\x8C\xEB\xA0\x88\xEC\x9D\xB4\xEC\x96\xB4\xEB\xA7\x8C" ")",
			{ { nullptr, nullptr } },
			float3_t(-1150.f, -11.52f, -909.28f),
			nullptr, nullptr, nullptr },
		// Mario2/3/4_go destinations use their published detail navigation grids.
		KAKUL_DEBUG_GATE{ "2" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } },
			float3_t(-1434.48999f, -9.02000999f, -1175.96997f), nullptr, nullptr, nullptr },
		KAKUL_DEBUG_GATE{ "3" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } },
			float3_t(-1889.68994f, -11.5299997f, -1646.20996f), nullptr, nullptr, nullptr },
		KAKUL_DEBUG_GATE{ "4" "\xEB\xA7\x88\xEB\xA6\xAC\xEC\x98\xA4", { { nullptr, nullptr } },
			float3_t(-1632.57f, -20.49f, -1400.92f), nullptr, nullptr, nullptr },
		// Card maze: main's admitted Debug entry destination.
		KAKUL_DEBUG_GATE{ "\xEC\xB9\xB4\xEB\x93\x9C\xEB\xAF\xB8\xEB\xA1\x9C", { { nullptr, nullptr } }, float3_t(0.09f, -0.01f, 1351.48f),
			nullptr, nullptr, nullptr },
		// 빙고 - 앵콜을 외친 쿠크세이튼 (Saydon holding the hammer)
		KAKUL_DEBUG_GATE{ "\xEB\xB9\x99\xEA\xB3\xA0" " - " "\xEC\x95\xB5\xEC\xBD\x9C\xEC\x9D\x84" " " "\xEC\x99\xB8\xEC\xB9\x9C" " " "\xEC\xBF\xA0\xED\x81\xAC\xEC\x84\xB8\xEC\x9D\xB4\xED\x8A\xBC",
			{ { "boss.kakulsaydon.bingo.saydon", nullptr } },
			float3_t(-3.4f, 0.f, 1147.44f),
			"BOSS_KAKULSAYDON_BINGO_SAYDON", "boss.kakulsaydon.bingo.saydon", nullptr },
	} };
	return gates;
}

void Client::CLevel_KakulSaydonArena::Debug_ReturnToPlayerCamera()
{
	// Release presentation ownership even if the replicated player disappeared.
	Stop_CompositionCamera(true);
	Release_CameraShot();
	const auto character = m_Replication.Get_LocalCharacter();
	if (!m_pCamera || !character || !character->Get_Transform()) return;
	m_pCamera->Set_FollowTarget(character->Get_Transform());
	m_pCamera->Set_FollowEnabled(true);
	Update_SourceFollowCamera(0.f, true);
	(void)m_pCamera->Set_FollowPose(m_EffectiveFollowCameraProfile.positionOffset,
		CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile), m_EffectiveFollowCameraProfile.rotationDegrees.z,
		m_EffectiveFollowCameraProfile.fovYDegrees, m_EffectiveFollowCameraProfile.followResponse);
}

void Client::CLevel_KakulSaydonArena::Debug_SetSequenceCombatPending(const bool_t pending)
{
	// Restart keeps the admitted run epoch, but the next cinematic owns a new scene cycle.
	if (pending && !m_bSequenceCombatPending) m_iServerRaidGatePresentationEpoch = 0u;
	m_bSequenceCombatPending = pending;
	if (!pending) m_bSequenceCombatFadeHeld = false;
	CCombatHUDViewModel::Get().Set_BossHidden(pending || m_iActiveDebugGate == NO_ACTIVE_DEBUG_GATE);
	if (!Is_DebugGatePending())
		CKoukuSaydonPatternAuditionService::Get().Set_TargetTransitionPending(pending);
}

bool_t Client::CLevel_KakulSaydonArena::Prepare_ServerRaidGatePresentation(const std::string& gateId, std::string& status)
{
    const auto& gates = Get_DebugGates();
    const auto bingo = std::find_if(gates.begin(), gates.end(), [](const auto& gate) {
        return gate.pAuditionPlacementId && std::string_view(gate.pAuditionPlacementId) == "boss.kakulsaydon.bingo.saydon"; });
    const size_t index = gateId == "GATE1" ? 0u : gateId == "GATE2" ? 1u : gateId == "GATE3" ? 2u :
        gateId == "BINGO" && bingo != gates.end() ? static_cast<size_t>(std::distance(gates.begin(), bingo)) : NO_ACTIVE_DEBUG_GATE;
    if (index == NO_ACTIVE_DEBUG_GATE) { status = "Unknown Server raid gate."; return false; }
    if (!Prepare_GatePresentation(index, status)) return false;
    m_pPendingGateObjects->serverRaidPrepared = true;
    return true;
}

bool_t Client::CLevel_KakulSaydonArena::Apply_ServerRaidGatePresentation(const std::string& gateId, const std::uint32_t epoch, std::string& status)
{
    const auto& gates = Get_DebugGates();
    const auto bingo = std::find_if(gates.begin(), gates.end(), [](const auto& gate) {
        return gate.pAuditionPlacementId && std::string_view(gate.pAuditionPlacementId) == "boss.kakulsaydon.bingo.saydon"; });
    const size_t index = gateId == "GATE1" ? 0u : gateId == "GATE2" ? 1u : gateId == "GATE3" ? 2u :
        gateId == "BINGO" && bingo != gates.end() ? static_cast<size_t>(std::distance(gates.begin(), bingo)) : NO_ACTIVE_DEBUG_GATE;
    if (!epoch || index == NO_ACTIVE_DEBUG_GATE) { status = "Unknown Server raid gate."; return false; }
    // A same-gate restart still owns a newly prepared cinematic handoff.
    if (m_iActiveDebugGate == index && m_iServerRaidGatePresentationEpoch == epoch &&
        !(m_pPendingGateObjects && m_pPendingGateObjects->serverRaidPrepared)) return true;
    if (!Commit_GatePresentation(index, status)) return false;
    m_iServerRaidGatePresentationEpoch = epoch;
    return true;
}

bool_t Client::CLevel_KakulSaydonArena::Is_LocalMarioStageActive() const
{
    return Is_LocalMarioLightingActive();
}

const string& Client::CLevel_KakulSaydonArena::Get_GatePresentationProfileId() const
{
	static const string marioProfile = "scene.kakulsaydon.source-rendering.v1";
	// Stage zero (return, failure or reset) exposes the existing gate profile again.
	return Is_LocalMarioLightingActive() ? marioProfile : m_strGatePresentationProfileId;
}

bool_t Client::CLevel_KakulSaydonArena::Prepare_GatePresentation(const size_t index, std::string& status)
{
    if (index >= Get_DebugGates().size()) { status = "Unknown Server gate."; return false; }
    // The cinematic pins this prepared owner until its combat handoff. Preparing
    // does not play objects or change visibility, lighting or the current HUD.
    if (m_pPendingGateObjects && m_pPendingGateObjects->serverRaidPrepared && m_pPendingGateObjects->gateIndex == index &&
        ((index != 0u && index != 2u) || m_pPendingGateMapLights)) return true;
    if (!Debug_PrepareGateObjects(index, status)) return false;
    const auto lightSource = m_pMapLightAuthoringOverride ? m_pMapLightAuthoringOverride : m_pMapLightPresentation;
    m_pPendingGateMapLights.reset(); m_PendingGateMapLightSource.reset();
    if ((index == 0u || index == 2u) && (!lightSource ||
        !Prepare_GateMapLights(lightSource->Get_Document(), index, m_pPendingGateMapLights, status)))
    { Debug_CancelGateObjects(); if (!lightSource) status = "Gate Area lighting is unavailable."; return false; }
    if (lightSource) m_PendingGateMapLightSource = lightSource->Get_Document();
    return true;
}

bool_t Client::CLevel_KakulSaydonArena::Commit_GatePresentation(const size_t index, std::string& status)
{
    if (!Prepare_GatePresentation(index, status)) return false;
    if (!Debug_CommitGateObjects(index, status)) { Debug_CancelGateObjects(); return false; }
    m_iActiveDebugGate = index; m_iGateLightingIndex = index;
    m_pGateMapLightPresentation = std::move(m_pPendingGateMapLights);
    m_GateMapLightSource = std::move(m_PendingGateMapLightSource);
    m_strGatePresentationProfileId = index == 0u ? "scene.kakulsaydon.g1.book-open.v1" : index == 2u ? "scene.kakulsaydon.g3.dark.v1" : "";
    const auto& gate = Get_DebugGates()[index];
    CCombatHUDViewModel::Get().Set_BossFocusArchetype(gate.pHudFocusArchetypeId);
    CCombatHUDViewModel::Get().Reset_CombatAnalysis();
    CCombatHUDViewModel::Get().Set_BossHidden(m_bSequenceCombatPending || nullptr == gate.pHudFocusArchetypeId);
    CKoukuSaydonPatternAuditionService::Get().Set_TargetBoss(gate.pAuditionPlacementId, gate.pHudFocusArchetypeId);
    m_strDebugGateStatus = "Server gate committed: " + std::string(gate.pAuditionPlacementId ? gate.pAuditionPlacementId : "unknown");
    return true;
}

void Client::CLevel_KakulSaydonArena::Debug_HoldSequenceCombatFade()
{
	m_bSequenceCombatFadeHeld = true;
	m_fTriggerMoveFadeAlpha = 1.f;
	if (m_pTriggerMoveFadeView)
	{
		m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", true);
		m_pTriggerMoveFadeView->Set_SlotTint("KakulFade_Screen", float4_t(0.f, 0.f, 0.f, 1.f));
	}
}

HRESULT Client::CLevel_KakulSaydonArena::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.Camera.Create");
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::KOUKU_SAYDON,
		m_FollowCameraProfile, m_strFollowCameraProfileStatus))
	{
		OutputDebugStringA(("[Level_KakulSaydonArena][FollowCamera] " +
			m_strFollowCameraProfileStatus + "\n").c_str());
	}
	m_EffectiveFollowCameraProfile = m_FollowCameraProfile.useSourceCameraRegions ?
		CArenaCameraProfile::KoukuSourceProfile(m_FollowCameraProfile, false) : m_FollowCameraProfile;
	float3_t positionOffset = m_EffectiveFollowCameraProfile.positionOffset;
	float3_t lookOffset = CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile);
	float3_t minimum{};
	float3_t maximum{};
	float3_t focus(0.f, 0.f, 0.f);
	f32_t span = 80.f;
	if (m_MapRuntime.Try_Get_PlacementBounds(minimum, maximum))
	{
		focus = float3_t(
			(minimum.x + maximum.x) * 0.5f,
			(minimum.y + maximum.y) * 0.5f,
			(minimum.z + maximum.z) * 0.5f);
		span = (std::clamp)(
			(std::max)(maximum.x - minimum.x, maximum.z - minimum.z),
			40.f,
			5000.f);
	}

	const f32_t distance = (std::max)(40.f, span * 0.7f);
	float3_t initialEye(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);
	float3_t initialAt = focus;
	LostArk::Shared::S2C_PLAYER_SPAWNED approvedSpawn{};
	if (CNetworkManager::Get().Try_Get_LocalSpawn(approvedSpawn))
	{
		if (m_FollowCameraProfile.useSourceCameraRegions)
		{
			const float3_t position(approvedSpawn.fPositionX, approvedSpawn.fPositionY, approvedSpawn.fPositionZ);
			m_EffectiveFollowCameraProfile = CArenaCameraProfile::KoukuSourceProfile(m_FollowCameraProfile,
				CArenaCameraProfile::Contains_KoukuSourceEntrance(position));
			positionOffset = m_EffectiveFollowCameraProfile.positionOffset;
			lookOffset = CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile);
		}
		initialEye = float3_t(
			approvedSpawn.fPositionX + positionOffset.x,
			approvedSpawn.fPositionY + positionOffset.y,
			approvedSpawn.fPositionZ + positionOffset.z);
		initialAt = float3_t(
			approvedSpawn.fPositionX + lookOffset.x,
			approvedSpawn.fPositionY + lookOffset.y,
			approvedSpawn.fPositionZ + lookOffset.z);
	}

	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = initialEye;
	cameraDesc.vAt = initialAt;
	cameraDesc.fFovy = m_EffectiveFollowCameraProfile.fovYDegrees;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = g_KakulSaydonFreeCameraSpeed;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.pFollowTarget = nullptr;
	cameraDesc.vPositionOffset = positionOffset;
	cameraDesc.vLookOffset = lookOffset;
	cameraDesc.fFollowResponse = m_EffectiveFollowCameraProfile.followResponse;
	cameraDesc.fFollowRollDegrees = m_EffectiveFollowCameraProfile.rotationDegrees.z;
	cameraDesc.isFollowEnabled = false;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::KAKULSAYDON_ARENA),
		strLayerTag,
		&cameraDesc,
		&gameObject)))
	{
		return E_FAIL;
	}

	m_pCamera = dynamic_pointer_cast<CCamera_Free>(gameObject);
	if (nullptr == m_pCamera)
	{
		CGameInstance::Get().Remove_GameObject_from_Layer(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			strLayerTag,
			gameObject);
		return E_FAIL;
	}
	return S_OK;
}

bool_t Client::CLevel_KakulSaydonArena::Set_FollowCameraProfile(
	const ARENA_CAMERA_PROFILE& profile,
	std::string& outStatus)
{
	if (!CArenaCameraProfile::Validate(profile, outStatus))
		return false;
	auto effective = profile;
	if (profile.useSourceCameraRegions)
	{
		float3_t position{};
		const auto character = Get_LocalCharacter();
		const bool_t positioned = character && character->Get_Transform();
		if (positioned) XMStoreFloat3(&position, character->Get_Transform()->Get_State(STATE::POSITION));
		effective = CArenaCameraProfile::KoukuSourceProfile(profile,
			positioned && CArenaCameraProfile::Contains_KoukuSourceEntrance(position));
	}
	if (nullptr == m_pCamera || !m_pCamera->Set_FollowPose(
		effective.positionOffset, CArenaCameraProfile::LookOffset(effective),
		effective.rotationDegrees.z, effective.fovYDegrees, effective.followResponse))
	{
		outStatus = "The active follow camera could not apply these settings.";
		return false;
	}
	m_FollowCameraProfile = profile;
	m_EffectiveFollowCameraProfile = effective;
	m_bSourceCameraInitialized = false;
	Update_SourceFollowCamera(0.f, true);
	if (const auto character = Get_LocalCharacter())
		CCharacter::Set_MapPresentationSizeProfile(profile);
	outStatus = "Applied to this map's follow camera. Save to keep these settings.";
	m_strFollowCameraProfileStatus = outStatus;
	return true;
}

void Client::CLevel_KakulSaydonArena::Update_SourceFollowCamera(const f32_t timeDelta, bool_t immediate)
{
	if (!m_FollowCameraProfile.useSourceCameraRegions || !m_pCamera) return;
	const auto transform = m_Replication.Get_CameraTarget();
	if (!transform)
	{ m_bSourceCameraInitialized = false; return; }
	float3_t position{};
	XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return;
	const auto delta = XMLoadFloat3(&position) - XMLoadFloat3(&m_vSourceCameraPreviousPlayer);
	immediate = immediate || !m_bSourceCameraInitialized || XMVectorGetX(XMVector3LengthSq(delta)) > 144.f;
	const bool_t inside = CArenaCameraProfile::Contains_KoukuSourceEntrance(position,
		!immediate && m_bInsideSourceCameraEntrance ? .25f : 0.f);
	if (immediate || inside != m_bInsideSourceCameraEntrance)
	{
		m_fSourceCameraBlendFromDistance = m_EffectiveFollowCameraProfile.focusDistance;
		m_fSourceCameraBlendElapsed = immediate ? 3.f : 0.f;
	}
	m_bInsideSourceCameraEntrance = inside;
	m_bSourceCameraInitialized = true;
	m_vSourceCameraPreviousPlayer = position;
	const auto target = CArenaCameraProfile::KoukuSourceProfile(m_FollowCameraProfile, inside);
	m_fSourceCameraBlendElapsed = (std::min)(3.f, m_fSourceCameraBlendElapsed +
		(std::isfinite(timeDelta) ? (std::max)(0.f, timeDelta) : 0.f));
	// Source BlendParam is 3 seconds. Smoothstep and exit hysteresis are project transition policy.
	const f32_t t = m_fSourceCameraBlendElapsed / 3.f;
	const f32_t alpha = t * t * (3.f - 2.f * t);
	const f32_t distance = immediate ? target.focusDistance :
		m_fSourceCameraBlendFromDistance + (target.focusDistance - m_fSourceCameraBlendFromDistance) * alpha;
	m_EffectiveFollowCameraProfile = target;
	m_EffectiveFollowCameraProfile.focusDistance = distance;
	m_EffectiveFollowCameraProfile.positionOffset = {-distance * .5f,
		distance * std::sqrt(.5f) - .1f, distance * .5f};
	if (immediate)
		(void)m_pCamera->Set_FollowPose(m_EffectiveFollowCameraProfile.positionOffset,
			CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile), target.rotationDegrees.z,
			target.fovYDegrees, target.followResponse);
	else
		// Preserve existing follow damping, cinematic ownership and the user's F6 mode.
		m_pCamera->Set_PositionOffset(m_EffectiveFollowCameraProfile.positionOffset);
}

bool_t Client::CLevel_KakulSaydonArena::Bind_CameraToLocalCharacter()
{
	if (nullptr == m_pCamera)
		return false;
	m_pCamera->Set_SpectateFrozen(m_Replication.Is_SpectateTargetDead());
	const auto transform = m_Replication.Resolve_CameraTarget();
	const auto localCharacter = m_Replication.Get_CameraCharacter();
	if (nullptr == transform)
	{
		m_pCameraTarget.reset();
		m_bSourceCameraInitialized = false;
		m_pCamera->Set_FollowTarget(nullptr);
		m_pCamera->Set_FollowEnabled(false);
		return true;
	}
	CCharacter::Set_MapPresentationSizeProfile(m_FollowCameraProfile);
	if (m_pCamera->Get_FollowTarget() == transform)
		return true;

	m_pCameraTarget = localCharacter;
	m_pCamera->Set_FollowTarget(transform);
	Update_SourceFollowCamera(0.f, true);
	m_pCamera->Set_FollowEnabled(true);
	return true;
}

namespace
{
	std::filesystem::path Camera_AuthoringPath()
	{
		return CProjectDataRoot::Resolve(L"Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json");
	}
	std::string Camera_JsonText(const DATA_JSON_VALUE& value, const unsigned depth = 0u)
	{
		if (value.Is_String()) return "\"" + CDataJson::Escape(value.Get_String()) + "\"";
		if (value.Is_Number()) { std::ostringstream stream; stream << std::setprecision(17) << value.Get_Number(); return stream.str(); }
		if (value.Is_Boolean()) return value.Get_Boolean() ? "true" : "false";
		if (value.Is_Null()) return "null";
		const bool object = value.Is_Object();
		std::string text = object ? "{" : "[";
		bool first = true;
		const auto append = [&](const std::string& item) {
			text += first ? "\n" : ",\n"; first = false;
			text += std::string((depth + 1u) * 2u, ' ') + item;
		};
		if (object)
		{
			std::set<std::string> written;
			for (const auto& key : value.Get_ObjectInsertionOrder())
				if (const auto* child = value.Find(key); child && written.insert(key).second)
					append("\"" + CDataJson::Escape(key) + "\": " + Camera_JsonText(*child, depth + 1u));
			for (const auto& [key, child] : value.Get_Object())
				if (written.insert(key).second) append("\"" + CDataJson::Escape(key) + "\": " + Camera_JsonText(child, depth + 1u));
		}
		else for (const auto& child : value.Get_Array()) append(Camera_JsonText(child, depth + 1u));
		if (!first) text += "\n" + std::string(depth * 2u, ' ');
		return text + (object ? "}" : "]");
	}
	DATA_JSON_VALUE Camera_TrackJson(const VALTAN_CINEMATIC_CAMERA_CUE& cue);
	DATA_JSON_VALUE Camera_ShotJson(const CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT& shot,
		const DATA_JSON_VALUE* existing)
	{
		using J = DATA_JSON_VALUE;
		const auto vec = [](const float3_t& v) { return J::Array({ J::Number(v.x), J::Number(v.y), J::Number(v.z) }); };
		J::OBJECT fields = existing ? existing->Get_Object() : J::OBJECT{};
		fields["shotId"] = J::String(shot.strShotId);
		fields["displayName"] = J::String(shot.strDisplayName);
		fields["sequenceInstanceId"] = J::String(shot.strSequenceInstanceId);
		fields["box"] = J::Object({ {"center", vec(shot.vCenter)}, {"halfExtents", vec(shot.vHalfExtents)}, {"yawDegrees", J::Number(shot.fYawDegrees)} });
		fields["eye"] = vec(shot.vEye); fields["lookAt"] = vec(shot.vLookAt);
		fields["fovYDegrees"] = J::Number(shot.fFovYDegrees);
		fields["blendInMs"] = J::Number(shot.iBlendInMs); fields["blendOutMs"] = J::Number(shot.iBlendOutMs);
		fields["defaultHoldMs"] = J::Number(shot.iDefaultHoldMs); fields["priority"] = J::Number(shot.iPriority);
		fields["activation"] = J::String(shot.bPatternOnly ? "PATTERN_ONLY" : "AUTO");
		fields["transitionEasing"] = J::String(shot.eTransitionEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? "LINEAR" : "SMOOTHSTEP");
		if (shot.followsPlayer) fields["follow"] = J::Object({ {"eyeOffset", vec(shot.vFollowEyeOffset)}, {"lookAtOffset", vec(shot.vFollowLookAtOffset)} });
		else fields.erase("follow");
		if (shot.hasCameraTrack) fields["cameraTrack"] = Camera_TrackJson(shot.CameraTrack);
		else fields.erase("cameraTrack");
		return J::Object(std::move(fields), existing ? existing->Get_ObjectInsertionOrder() : std::vector<std::string>{});
	}
	DATA_JSON_VALUE Camera_TrackJson(const VALTAN_CINEMATIC_CAMERA_CUE& cue)
	{
		using J = DATA_JSON_VALUE;
		const auto vec = [](const float3_t& v) { return J::Array({J::Number(v.x), J::Number(v.y), J::Number(v.z)}); };
		J::ARRAY keys;
		for (const auto& key : cue.Keyframes)
		{
			J::OBJECT fields{{"sceneId", J::String(key.strSceneId)}, {"timeMs", J::Number(key.iTimeMs)},
				{"eye", vec(key.vEye)}, {"lookAt", vec(key.vLookAt)}, {"fovYDegrees", J::Number(key.fFovYDegrees)}};
			if (key.hasUp) fields["up"] = vec(key.vUp);
			keys.push_back(J::Object(std::move(fields)));
		}
		return J::Object({{"durationMs", J::Number(cue.iDurationMs)},
			{"interpolation", J::String(cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR ? "LINEAR" :
				cue.eInterpolation == VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM ? "CATMULL_ROM" : "INVALID")},
			{"easing", J::String(cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::LINEAR ? "LINEAR" :
				cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP ? "SMOOTHSTEP" :
				cue.eEasing == VALTAN_CINEMATIC_CAMERA_EASING::HOLD ? "HOLD" : "INVALID")}, {"keyframes", J::Array(std::move(keys))}});
	}
	std::string Camera_EmptyDocument()
	{
		return "{\"schema\":\"lostark.camera-shots\",\"formatVersion\":1,\"areaId\":\"LV_LUT_MIDNIGHTC_ED\",\"revision\":1,\"shots\":[]}";
	}
}

Client::VALTAN_CINEMATIC_CAMERA_CUE Client::CLevel_KakulSaydonArena::CameraShot_ToCue(const KAKUL_CAMERA_SHOT& shot)
{
	auto cue = shot.hasCameraTrack ? shot.CameraTrack : VALTAN_CINEMATIC_CAMERA_CUE{};
	cue.strCueId = shot.strShotId;
	cue.iTransitionInMs = shot.iBlendInMs; cue.iTransitionOutMs = shot.iBlendOutMs;
	if (!shot.hasCameraTrack)
	{
		cue.iDurationMs = (std::clamp)(shot.iBlendInMs + shot.iDefaultHoldMs, 1u, CAMERA_TRACK_MAX_DURATION_MS);
		cue.Keyframes = {{shot.strShotId + ".p1", 0u, shot.vEye, shot.vLookAt, shot.fFovYDegrees}};
	}
	return cue;
}

bool_t Client::CLevel_KakulSaydonArena::Stage_PatternCameraTracks(const std::string_view baseline,
	const std::vector<VALTAN_CINEMATIC_CAMERA_CUE>& cues, const std::map<std::string, std::string>& names,
	std::string& outText, std::string& outStatus)
{
	using J = DATA_JSON_VALUE;
	std::vector<KAKUL_CAMERA_SHOT> original;
	J root;
	if (!Parse_CameraShots(baseline, original, outStatus) || !CDataJson::Parse(baseline, root, outStatus)) return false;
	auto rows = root.Find("shots")->Get_Array();
	std::set<std::string> ids;
	bool changed = false;
	for (const auto& cue : cues)
	{
		const auto name = names.find(cue.strCueId);
		if (!ids.insert(cue.strCueId).second || name == names.end() || cue.eTrackingMode != VALTAN_CINEMATIC_TRACKING_MODE::WORLD ||
			cue.fShakeAmplitude != 0.f || cue.iShakeDurationMs != 0u)
		{ outStatus = "Pattern Camera requires a unique shot, display name and WORLD track."; return false; }
		const auto old = std::find_if(original.begin(), original.end(), [&](const auto& shot) { return shot.strShotId == cue.strCueId; });
		if (old != original.end() && !old->bPatternOnly)
		{ outStatus = "Automatic Area cameras stay in their existing Map Tool owner."; return false; }
		const bool trackChanged = old == original.end() ||
			Camera_JsonText(Camera_TrackJson(CameraShot_ToCue(*old))) != Camera_JsonText(Camera_TrackJson(cue));
		if (!trackChanged && old->strDisplayName == name->second && old->iBlendInMs == cue.iTransitionInMs &&
			old->iBlendOutMs == cue.iTransitionOutMs) continue;
		KAKUL_CAMERA_SHOT shot = old != original.end() ? *old : KAKUL_CAMERA_SHOT{};
		shot.strShotId = cue.strCueId; shot.strDisplayName = name->second; shot.bPatternOnly = true;
		shot.iBlendInMs = cue.iTransitionInMs; shot.iBlendOutMs = cue.iTransitionOutMs;
		if (trackChanged || old->iBlendInMs != cue.iTransitionInMs)
			shot.iDefaultHoldMs = cue.iDurationMs > cue.iTransitionInMs ? cue.iDurationMs - cue.iTransitionInMs : 0u;
		if (old == original.end())
		{
			shot.vHalfExtents = {1.f, 1.f, 1.f};
			shot.iDefaultHoldMs = cue.iDurationMs > cue.iTransitionInMs ? cue.iDurationMs - cue.iTransitionInMs : 0u;
		}
		if (trackChanged && !cue.Keyframes.empty())
		{ shot.vEye = cue.Keyframes.front().vEye; shot.vLookAt = cue.Keyframes.front().vLookAt; shot.fFovYDegrees = cue.Keyframes.front().fFovYDegrees; }
		const auto target = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.Find("shotId")->Get_String() == cue.strCueId; });
		auto json = Camera_ShotJson(shot, target == rows.end() ? nullptr : &*target);
		auto fields = json.Get_Object();
		if (trackChanged) { fields.erase("follow"); fields["cameraTrack"] = Camera_TrackJson(cue); }
		json = J::Object(std::move(fields), json.Get_ObjectInsertionOrder());
		if (target == rows.end()) rows.push_back(std::move(json)); else *target = std::move(json);
		changed = true;
	}
	std::erase_if(rows, [&](const auto& row) {
		const auto id = row.Find("shotId")->Get_String();
		const auto old = std::find_if(original.begin(), original.end(), [&](const auto& shot) { return shot.strShotId == id; });
		const bool removed = old != original.end() && old->bPatternOnly && !ids.contains(id);
		changed |= removed; return removed;
	});
	if (!changed) { outText = std::string(baseline); return true; }
	const double revision = root.Find("revision")->Get_Number();
	if (revision >= 4294967295.0) { outStatus = "Camera revision is exhausted."; return false; }
	auto fields = root.Get_Object(); fields["shots"] = J::Array(std::move(rows)); fields["revision"] = J::Number(revision + 1.0);
	auto text = Camera_JsonText(J::Object(std::move(fields), root.Get_ObjectInsertionOrder())) + "\n";
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus)) return false;
	outText = std::move(text); return true;
}

bool_t Client::CLevel_KakulSaydonArena::Save_CameraShotSource(const std::string_view expectedSource,
	const std::string& text, std::string& outStatus)
{
	if (!m_DirtyCameraShotIds.empty())
	{ outStatus = "Save the Action Workbench Camera draft before saving from Cinematic Camera Tool."; return false; }
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus) ||
		!CMapTool::Save_CameraShotDocumentAtomic(Camera_AuthoringPath(), std::string(expectedSource), text, outStatus)) return false;
	m_AuthoringCameraShots = std::move(staged); m_strCameraAuthoringBaseline = text; m_bCameraAuthoringLoaded = true;
	outStatus = "Saved Kouku Area Camera source. Preview is ready; publish before Complete Play.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Ensure_CameraShotAuthoring(std::string& outStatus)
{
	if (m_bCameraAuthoringLoaded) return true;
	if (m_bCameraAuthoringLoadAttempted)
	{
		outStatus = m_strCameraAuthoringLoadFailure;
		return false;
	}
	return Reload_CameraShotAuthoring(outStatus);
}

bool_t Client::CLevel_KakulSaydonArena::Reload_CameraShotAuthoring(std::string& outStatus)
{
	if (!m_DirtyCameraShotIds.empty())
	{ outStatus = "Save the Camera draft before Reload Cameras; unsaved shots were preserved."; return false; }
	Engine::CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Kouku.CameraAuthoring.Load");
	m_bCameraAuthoringLoadAttempted = true;
	const auto fail = [&](std::string reason) {
		m_strCameraAuthoringLoadFailure = std::move(reason);
		outStatus = m_strCameraAuthoringLoadFailure;
		return false;
	};
	const auto path = Camera_AuthoringPath();
	std::error_code error;
	std::string text;
	if (std::filesystem::exists(path, error))
	{
		const auto bytes = std::filesystem::file_size(path, error);
		if (error || bytes > 2u * 1024u * 1024u)
			return fail("Camera authoring source exceeds 2 MiB or cannot be read.");
		if (bytes == 0u) return fail("Camera authoring source is empty or unreadable.");
		std::ifstream input(path, std::ios::binary);
		if (!input) return fail("Cannot open Camera authoring source.");
		text.resize(static_cast<std::size_t>(bytes));
		input.read(text.data(), static_cast<std::streamsize>(text.size()));
		if (!input || input.peek() != std::char_traits<char>::eof())
			return fail("Camera authoring source changed while reading or is unreadable.");
	}
	else if (error) return fail("Cannot inspect Camera authoring source.");
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text.empty() ? Camera_EmptyDocument() : text, staged, outStatus))
		return fail(outStatus);
	m_AuthoringCameraShots = std::move(staged);
	m_strCameraAuthoringBaseline = std::move(text);
	m_bCameraAuthoringLoaded = true;
	m_strCameraAuthoringLoadFailure.clear();
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Create_CameraShot(const std::string_view name,
	std::string& outShotId, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	if (m_AuthoringCameraShots.size() >= CAMERA_SHOT_MAX_COUNT)
	{ outStatus = "Camera shot limit is 128."; return false; }
	KAKUL_CAMERA_SHOT shot;
	for (uint32_t ordinal = 1u; ordinal <= CAMERA_SHOT_MAX_COUNT + 1u; ++ordinal)
	{
		shot.strShotId = "camera.kouku.pattern." + std::to_string(ordinal);
		if (std::none_of(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
			[&](const auto& item) { return item.strShotId == shot.strShotId; })) break;
	}
	shot.strDisplayName = std::string(name);
	shot.bPatternOnly = true; shot.eTransitionEasing = VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
	shot.iBlendInMs = 500u; shot.iBlendOutMs = 500u; shot.vHalfExtents = float3_t(1.f, 1.f, 1.f);
	VALTAN_CINEMATIC_CAMERA_POSE pose;
	if (!CCameraTool::Capture_ViewPose(pose)) { outStatus = "Current Camera pose is unavailable."; return false; }
	shot.vEye = pose.vEye; shot.vLookAt = pose.vLookAt; shot.fFovYDegrees = pose.fFovYDegrees;
	shot.CameraTrack = CameraShot_ToCue(shot);
	shot.CameraTrack.Keyframes.front().vUp = pose.vUp;
	shot.CameraTrack.Keyframes.front().hasUp = true;
	shot.hasCameraTrack = true;
	DATA_JSON_VALUE root; std::string ignored;
	(void)CDataJson::Parse(Camera_EmptyDocument(), root, ignored);
	auto fields = root.Get_Object(); fields["shots"] = DATA_JSON_VALUE::Array({ Camera_ShotJson(shot, nullptr) });
	std::vector<KAKUL_CAMERA_SHOT> validated;
	if (!Parse_CameraShots(Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields))), validated, outStatus)) return false;
	outShotId = shot.strShotId;
	m_DirtyCameraShotIds.insert(shot.strShotId);
	m_AuthoringCameraShots.push_back(std::move(shot));
	outStatus = "Camera created in the authoring draft. Set Camera Pos, then Save Camera.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Update_CameraShot(const KAKUL_CAMERA_SHOT& shot, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	const auto found = std::find_if(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
		[&](const auto& value) { return value.strShotId == shot.strShotId; });
	if (found == m_AuthoringCameraShots.end()) { outStatus = "Camera shot was not found."; return false; }
	// Validate the exact track being committed, including times and orientation.
	DATA_JSON_VALUE root; std::string ignored;
	(void)CDataJson::Parse(Camera_EmptyDocument(), root, ignored);
	auto fields = root.Get_Object();
	fields["shots"] = DATA_JSON_VALUE::Array({ Camera_ShotJson(shot, nullptr) });
	std::vector<KAKUL_CAMERA_SHOT> validated;
	if (!Parse_CameraShots(Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields))), validated, outStatus)) return false;
	auto adjusted = shot;
	if (adjusted.followsPlayer && !found->followsPlayer)
	{
		const auto character = m_Replication.Get_LocalCharacter();
		if (!character || !character->Get_Transform()) { outStatus = "PLAYER anchor requires the local replicated Character."; return false; }
		float3_t position; XMStoreFloat3(&position, character->Get_Transform()->Get_State(STATE::POSITION));
		adjusted.vFollowEyeOffset = float3_t(shot.vEye.x - position.x, shot.vEye.y - position.y, shot.vEye.z - position.z);
		adjusted.vFollowLookAtOffset = float3_t(shot.vLookAt.x - position.x, shot.vLookAt.y - position.y, shot.vLookAt.z - position.z);
	}
	*found = adjusted; m_DirtyCameraShotIds.insert(shot.strShotId);
	outStatus = "Camera draft changed. Save Camera writes the Area source.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Capture_CameraShot(const std::string_view shotId, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	const auto found = std::find_if(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
		[&](const auto& value) { return value.strShotId == shotId; });
	if (found == m_AuthoringCameraShots.end()) { outStatus = "Camera shot was not found."; return false; }
	VALTAN_CINEMATIC_CAMERA_POSE pose;
	if (!CCameraTool::Capture_ViewPose(pose)) { outStatus = "Current Camera pose is unavailable."; return false; }
	auto shot = *found;
	shot.vEye = pose.vEye; shot.vLookAt = pose.vLookAt; shot.fFovYDegrees = pose.fFovYDegrees;
	shot.hasCameraTrack = false;
	if (shot.followsPlayer)
	{
		const auto character = m_Replication.Get_LocalCharacter();
		if (!character || !character->Get_Transform()) { outStatus = "PLAYER capture requires the local replicated Character."; return false; }
		float3_t position; XMStoreFloat3(&position, character->Get_Transform()->Get_State(STATE::POSITION));
		shot.vFollowEyeOffset = float3_t(pose.vEye.x - position.x, pose.vEye.y - position.y, pose.vEye.z - position.z);
		shot.vFollowLookAtOffset = float3_t(pose.vLookAt.x - position.x, pose.vLookAt.y - position.y, pose.vLookAt.z - position.z);
	}
	return Update_CameraShot(shot, outStatus);
}

bool_t Client::CLevel_KakulSaydonArena::Duplicate_CameraShot(const std::string_view sourceShotId,
	const std::string_view name, std::string& outShotId, std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	const auto source = std::find_if(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
		[&](const auto& value) { return value.strShotId == sourceShotId; });
	if (source == m_AuthoringCameraShots.end()) { outStatus = "Camera shot was not found."; return false; }
	if (m_AuthoringCameraShots.size() >= CAMERA_SHOT_MAX_COUNT) { outStatus = "Camera shot limit is 128."; return false; }
	auto shot = *source;
	for (uint32_t ordinal = 1u; ordinal <= CAMERA_SHOT_MAX_COUNT + 1u; ++ordinal)
	{
		shot.strShotId = "camera.kouku.pattern." + std::to_string(ordinal);
		if (std::none_of(m_AuthoringCameraShots.begin(), m_AuthoringCameraShots.end(),
			[&](const auto& item) { return item.strShotId == shot.strShotId; })) break;
	}
	// The copy belongs to one Sequence box; the Area trigger keeps playing the source shot.
	shot.strDisplayName = std::string(name);
	shot.bPatternOnly = true;
	shot.strSequenceInstanceId.clear();
	if (shot.hasCameraTrack) shot.CameraTrack.strCueId = shot.strShotId;
	DATA_JSON_VALUE root; std::string ignored;
	(void)CDataJson::Parse(Camera_EmptyDocument(), root, ignored);
	auto fields = root.Get_Object(); fields["shots"] = DATA_JSON_VALUE::Array({ Camera_ShotJson(shot, nullptr) });
	std::vector<KAKUL_CAMERA_SHOT> validated;
	if (!Parse_CameraShots(Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields))), validated, outStatus)) return false;
	outShotId = shot.strShotId;
	m_DirtyCameraShotIds.insert(shot.strShotId);
	m_AuthoringCameraShots.push_back(std::move(shot));
	outStatus = "Dedicated Camera shot created in the draft. Save Camera writes it to the Area source.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Discard_UnsavedCameraShot(const std::string_view shotId, std::string& outStatus)
{
	std::vector<KAKUL_CAMERA_SHOT> saved;
	if (!Parse_CameraShots(m_strCameraAuthoringBaseline.empty() ? Camera_EmptyDocument() : m_strCameraAuthoringBaseline,
		saved, outStatus)) return false;
	if (std::any_of(saved.begin(), saved.end(), [&](const auto& shot) { return shot.strShotId == shotId; }))
	{ outStatus = "A saved Camera shot is never discarded here."; return false; }
	std::erase_if(m_AuthoringCameraShots, [&](const auto& shot) { return shot.strShotId == shotId; });
	m_DirtyCameraShotIds.erase(std::string(shotId));
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Save_CameraShots(std::string& outStatus)
{
	if (!Ensure_CameraShotAuthoring(outStatus)) return false;
	if (m_DirtyCameraShotIds.empty()) { outStatus = "Camera source is already saved."; return true; }
	DATA_JSON_VALUE root;
	if (!CDataJson::Parse(m_strCameraAuthoringBaseline.empty() ? Camera_EmptyDocument() : m_strCameraAuthoringBaseline, root, outStatus)) return false;
	auto fields = root.Get_Object(); auto rows = root.Find("shots")->Get_Array();
	for (const auto& shot : m_AuthoringCameraShots)
	{
		if (!m_DirtyCameraShotIds.contains(shot.strShotId)) continue;
		auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.Find("shotId")->Get_String() == shot.strShotId; });
		if (found == rows.end()) rows.push_back(Camera_ShotJson(shot, nullptr));
		else *found = Camera_ShotJson(shot, &*found);
	}
	const double revision = root.Find("revision")->Get_Number();
	if (revision >= 4294967295.0) { outStatus = "Camera revision is exhausted."; return false; }
	fields["shots"] = DATA_JSON_VALUE::Array(std::move(rows));
	fields["revision"] = DATA_JSON_VALUE::Number(revision + 1.0);
	const auto text = Camera_JsonText(DATA_JSON_VALUE::Object(std::move(fields), root.Get_ObjectInsertionOrder())) + "\n";
	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus) ||
		!CMapTool::Save_CameraShotDocumentAtomic(Camera_AuthoringPath(), m_strCameraAuthoringBaseline, text, outStatus)) return false;
	m_AuthoringCameraShots = std::move(staged); m_strCameraAuthoringBaseline = text; m_DirtyCameraShotIds.clear();
	outStatus = "Camera saved to the Area source; Preview is ready. Publish the Area before Complete Play.";
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Load_CameraShots(
	std::string& outStatus)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.CameraShots.Load");
	const std::filesystem::path path = Find_CameraShotDocument();
	std::error_code fileError;
	if (!std::filesystem::is_regular_file(path, fileError) || fileError)
	{
		outStatus = "KoukuSaydon camera shot document is absent; follow view only.";
		return true;
	}
	const std::uintmax_t fileBytes = std::filesystem::file_size(path, fileError);
	if (fileError || 0u == fileBytes || fileBytes > 2u * 1024u * 1024u)
	{
		outStatus = "KoukuSaydon camera shot document is empty or exceeds 2 MiB.";
		return false;
	}
	std::ifstream input(path, std::ios::binary);
	if (!input)
	{
		outStatus = "KoukuSaydon camera shot document could not be opened.";
		return false;
	}
	const std::string text{
		std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>() };
	if (input.bad() || text.size() != fileBytes)
	{
		outStatus = "KoukuSaydon camera shot document could not be read completely.";
		return false;
	}

	std::vector<KAKUL_CAMERA_SHOT> staged;
	if (!Parse_CameraShots(text, staged, outStatus)) return false;
	Release_CameraShot();
	m_CameraShots = std::move(staged);
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Parse_CameraShots(
	const std::string_view text, std::vector<KAKUL_CAMERA_SHOT>& outShots, std::string& outStatus)
{
	DATA_JSON_VALUE root;
	std::string parseError;
	DATA_JSON_PARSE_LIMITS limits{};
	limits.iMaximumBytes = 2u * 1024u * 1024u;
	limits.iMaximumDepth = 12u;
	// The document supports 128 shots with 64 keys each, including eye/lookAt/up.
	// Keep the byte/depth and per-shot/key bounds; 4096 values rejected valid tracks.
	limits.iMaximumValues = 128u * 1024u;
	if (!CDataJson::Parse(text, root, parseError, limits) ||
		!Has_ExactProperties(root,
			{ "schema", "formatVersion", "areaId", "revision", "shots" }))
	{
		outStatus = "KoukuSaydon camera shot root is invalid: " + parseError;
		return false;
	}

	const DATA_JSON_VALUE* schema = Required(root, "schema", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* version = Required(root, "formatVersion", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* area = Required(root, "areaId", DATA_JSON_TYPE::STRING);
	const DATA_JSON_VALUE* revision = Required(root, "revision", DATA_JSON_TYPE::NUMBER);
	const DATA_JSON_VALUE* shots = Required(root, "shots", DATA_JSON_TYPE::ARRAY);
	if (nullptr == schema || CAMERA_SHOT_SCHEMA != schema->Get_String() ||
		nullptr == version || version->Get_Number() != 1.0 ||
		nullptr == area || KAKULSAYDON_AREA_ID != area->Get_String() ||
		nullptr == revision || !std::isfinite(revision->Get_Number()) ||
		revision->Get_Number() < 1.0 ||
		std::floor(revision->Get_Number()) != revision->Get_Number() ||
		nullptr == shots || shots->Get_Array().size() > CAMERA_SHOT_MAX_COUNT)
	{
		outStatus = "KoukuSaydon camera shot header is invalid.";
		return false;
	}

	std::vector<KAKUL_CAMERA_SHOT> stagedShots;
	std::unordered_set<std::string> stagedIds;
	stagedShots.reserve(shots->Get_Array().size());
	for (const DATA_JSON_VALUE& value : shots->Get_Array())
	{
		if (!Has_ShotProperties(value,
			{ "shotId", "sequenceInstanceId", "box", "eye", "lookAt",
				"fovYDegrees", "blendInMs", "blendOutMs", "priority" },
			{ "cameraTrack", "follow", "displayName", "defaultHoldMs", "transitionEasing", "activation" }))
		{
			outStatus = "KoukuSaydon camera shot has unexpected properties.";
			return false;
		}
		KAKUL_CAMERA_SHOT shot;
		const DATA_JSON_VALUE* shotId = Required(value, "shotId", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* box = Required(value, "box", DATA_JSON_TYPE::OBJECT);
		if (nullptr == shotId || !Is_StableId(shotId->Get_String()) ||
			!stagedIds.emplace(shotId->Get_String()).second ||
			nullptr == box ||
			!Has_ExactProperties(*box, { "center", "halfExtents", "yawDegrees" }))
		{
			outStatus = "KoukuSaydon camera shot identity or box is invalid.";
			return false;
		}
		shot.strShotId = shotId->Get_String();
		const DATA_JSON_VALUE* sequenceId =
			Required(value, "sequenceInstanceId", DATA_JSON_TYPE::STRING);
		if (nullptr == sequenceId ||
			(!sequenceId->Get_String().empty() &&
				!Is_StableId(sequenceId->Get_String())))
		{
			outStatus = "KoukuSaydon camera shot sequence binding is invalid: " +
				shot.strShotId;
			return false;
		}
		shot.strSequenceInstanceId = sequenceId->Get_String();

		const DATA_JSON_VALUE* yaw = Required(*box, "yawDegrees", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* fov = Required(value, "fovYDegrees", DATA_JSON_TYPE::NUMBER);
		if (!Read_Float3(box->Find("center"), CAMERA_SHOT_MAX_COORDINATE, shot.vCenter) ||
			!Read_Float3(box->Find("halfExtents"), CAMERA_SHOT_MAX_HALF_EXTENT,
				shot.vHalfExtents) ||
			shot.vHalfExtents.x <= 0.f || shot.vHalfExtents.y <= 0.f ||
			shot.vHalfExtents.z <= 0.f ||
			nullptr == yaw || !std::isfinite(yaw->Get_Number()) ||
			std::abs(yaw->Get_Number()) > 360.0 ||
			!Read_Float3(value.Find("eye"), CAMERA_SHOT_MAX_COORDINATE, shot.vEye) ||
			!Read_Float3(value.Find("lookAt"), CAMERA_SHOT_MAX_COORDINATE, shot.vLookAt) ||
			nullptr == fov || !std::isfinite(fov->Get_Number()) ||
			fov->Get_Number() <= 1.0 || fov->Get_Number() >= 179.0 ||
			!Read_Uint(value.Find("blendInMs"), CAMERA_SHOT_MAX_BLEND_MS, shot.iBlendInMs) ||
			!Read_Uint(value.Find("blendOutMs"), CAMERA_SHOT_MAX_BLEND_MS, shot.iBlendOutMs) ||
			!Read_Uint(value.Find("priority"), CAMERA_SHOT_MAX_PRIORITY, shot.iPriority))
		{
			outStatus = "KoukuSaydon camera shot values are out of range: " +
				shot.strShotId;
			return false;
		}
		shot.strDisplayName = shot.strShotId;
		if (const auto* name = value.Find("displayName"))
		{
			if (!name->Is_String() || name->Get_String().empty() || name->Get_String().size() > 128u || name->Get_String().find('\0') != std::string::npos ||
				MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name->Get_String().data(),
					static_cast<int>(name->Get_String().size()), nullptr, 0) <= 0)
			{ outStatus = "Camera displayName requires 1..128 UTF-8 bytes."; return false; }
			shot.strDisplayName = name->Get_String();
		}
		if (const auto* hold = value.Find("defaultHoldMs"))
			if (!Read_Uint(hold, 600000u, shot.iDefaultHoldMs) || shot.iBlendInMs + shot.iDefaultHoldMs > 600000u ||
				shot.iBlendInMs + shot.iDefaultHoldMs == 0u)
			{ outStatus = "Camera entry plus default hold must be 1..600000 ms."; return false; }
		if (const auto* easing = value.Find("transitionEasing"))
		{
			if (!easing->Is_String() || (easing->Get_String() != "LINEAR" && easing->Get_String() != "SMOOTHSTEP"))
			{ outStatus = "Camera transitionEasing must be LINEAR or SMOOTHSTEP."; return false; }
			shot.eTransitionEasing = easing->Get_String() == "LINEAR" ?
				VALTAN_CINEMATIC_CAMERA_EASING::LINEAR : VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		}
		if (const auto* activation = value.Find("activation"))
		{
			if (!activation->Is_String() || (activation->Get_String() != "AUTO" && activation->Get_String() != "PATTERN_ONLY"))
			{ outStatus = "Camera activation must be AUTO or PATTERN_ONLY."; return false; }
			shot.bPatternOnly = activation->Get_String() == "PATTERN_ONLY";
		}
		shot.fYawDegrees = static_cast<f32_t>(yaw->Get_Number());
		shot.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
		const DATA_JSON_VALUE* cameraTrack = value.Find("cameraTrack");
		if (nullptr != cameraTrack)
		{
			if (DATA_JSON_TYPE::OBJECT != cameraTrack->Get_Type() ||
				!Read_CameraTrack(*cameraTrack, shot.strShotId,
					shot.CameraTrack, outStatus))
			{
				return false;
			}
			shot.hasCameraTrack = true;
		}
		const DATA_JSON_VALUE* follow = value.Find("follow");
		if (nullptr != follow)
		{
			if (DATA_JSON_TYPE::OBJECT != follow->Get_Type() ||
				!Has_ExactProperties(*follow, { "eyeOffset", "lookAtOffset" }) ||
				!Read_Float3(follow->Find("eyeOffset"),
					CAMERA_SHOT_MAX_COORDINATE, shot.vFollowEyeOffset) ||
				!Read_Float3(follow->Find("lookAtOffset"),
					CAMERA_SHOT_MAX_COORDINATE, shot.vFollowLookAtOffset))
			{
				outStatus = "KoukuSaydon camera shot follow offsets are invalid: " +
					shot.strShotId;
				return false;
			}
			const float3_t followForward(
				shot.vFollowLookAtOffset.x - shot.vFollowEyeOffset.x,
				shot.vFollowLookAtOffset.y - shot.vFollowEyeOffset.y,
				shot.vFollowLookAtOffset.z - shot.vFollowEyeOffset.z);
			if (followForward.x * followForward.x +
				followForward.y * followForward.y +
				followForward.z * followForward.z <= 0.000001f)
			{
				outStatus = "KoukuSaydon camera shot follow offsets coincide: " +
					shot.strShotId;
				return false;
			}
			shot.followsPlayer = true;
		}
		/* A pose whose eye sits on its own target has no direction, and the
		   engine would reject it every frame. Refuse it at load instead. */
		const float3_t forward(
			shot.vLookAt.x - shot.vEye.x,
			shot.vLookAt.y - shot.vEye.y,
			shot.vLookAt.z - shot.vEye.z);
		if (forward.x * forward.x + forward.y * forward.y +
			forward.z * forward.z <= 0.000001f)
		{
			outStatus = "KoukuSaydon camera shot eye and lookAt coincide: " +
				shot.strShotId;
			return false;
		}
		stagedShots.push_back(std::move(shot));
	}

	outShots = std::move(stagedShots);
	outStatus = "KoukuSaydon camera shots loaded: " +
		std::to_string(outShots.size());
	return true;
}

const Client::CLevel_KakulSaydonArena::KAKUL_CAMERA_SHOT*
Client::CLevel_KakulSaydonArena::Find_ActiveCameraShot(
	const float3_t& vPosition) const
{
	const KAKUL_CAMERA_SHOT* best = nullptr;
	for (const KAKUL_CAMERA_SHOT& shot : m_CameraShots)
	{
		if (shot.bPatternOnly || !Is_SequenceCameraAudience(shot.strSequenceInstanceId,
			(m_Replication.Get_CameraPlayerSnapshot() ? m_Replication.Get_CameraPlayerSnapshot()->iMarioStage : 0u))) continue;
		const bool_t isHeldNow = shot.strShotId == m_strActiveCameraShotId;
		bool_t isActive = false;
		if (shot.strShotId == CARD_MAZE_TELESCOPE_SHOT_ID)
		{
			const auto* subject = m_Replication.Get_CameraPlayerSnapshot();
			isActive = subject && (subject->CardMaze.flags & 1u) != 0u;
		}
		else if (!shot.strSequenceInstanceId.empty())
		{
			/* The sequence starts the shot on the frame its trigger fires, even
			   though the party is still far from the box. Once the sequence
			   ends the box keeps the framing until they walk on to the next
			   stage, so the camera does not snap back mid scene. */
			isActive = m_SequencePlayer.Is_Playing(shot.strSequenceInstanceId);
			if (!isActive && isHeldNow)
			{
				isActive = Contains_CameraShot(
					shot, vPosition, CAMERA_SHOT_EXIT_MARGIN);
			}
		}
		else
		{
			isActive = Contains_CameraShot(shot, vPosition,
				isHeldNow ? CAMERA_SHOT_EXIT_MARGIN : 0.f);
		}
		if (!isActive)
			continue;
		if (nullptr == best || shot.iPriority > best->iPriority)
			best = &shot;
	}
	return best;
}

void Client::CLevel_KakulSaydonArena::Release_CameraShot()
{
	if (m_bCameraShotHeld && nullptr != m_pCamera)
	{
		m_pCamera->End_PresentationOverride(
			KAKULSAYDON_CAMERA_SHOT_OWNER_ID);
	}
	m_bCameraShotHeld = false;
	m_strActiveCameraShotId.clear();
	m_fCameraBlendSeconds = 0.f;
	m_fCameraBlendElapsed = 0.f;
}

void Client::CLevel_KakulSaydonArena::Update_CardMazePresentation(f32_t dt)
{
	/* Telescope visibility from replicated truth only: a clown box seen alive and
	   then gone, or a dealt maze role, shows it; leaving MAZE mode forgets it. */
	{
		const bool localMaze = LostArk::Shared::KOUKU_HUD_MODE::MAZE ==
			CCombatHUDViewModel::Get().Get_Player().eKoukuHudMode;
		const bool running = LostArk::Shared::CARD_MAZE_ROLE::NONE !=
			CCombatHUDViewModel::Get().Get_KoukuGimmick().eCardMazeRole;
		bool boxAlive = false;
		if (localMaze || running)
		{
			m_Replication.Collect_KoukuMazeTargets(m_CardMazeTargetScratch);
			boxAlive = std::any_of(m_CardMazeTargetScratch.begin(), m_CardMazeTargetScratch.end(),
				[](const KOUKU_MAZE_TARGET_VIEW& target)
				{ return target.archetypeId == CARD_MAZE_CLOWN_BOX_ARCHETYPE_ID; });
		}
		if (m_bCardMazeClownBoxAlive && !boxAlive) m_bCardMazeClownBoxDefeated = true;
		if (boxAlive || (!localMaze && !running)) m_bCardMazeClownBoxDefeated = false;
		m_bCardMazeClownBoxAlive = boxAlive;
		const bool showTelescope = running || (localMaze && m_bCardMazeClownBoxDefeated);
		if (showTelescope != m_bCardMazeTelescopeShown)
		{
			// One attempt per change: a missing row or rejected state is logged, not retried per frame.
			m_bCardMazeTelescopeShown = showTelescope;
			if (m_DeployRuntime.Find(KAKULSAYDON_CARD_MAZE_TELESCOPE_PLACEMENT_ID) &&
				!m_DeployRuntime.Set_State(KAKULSAYDON_CARD_MAZE_TELESCOPE_PLACEMENT_ID,
					showTelescope ? DEPLOY_PROP_STATE::INTACT : DEPLOY_PROP_STATE::DESPAWNED))
				OutputDebugStringA(("[CardMaze][Telescope] " + m_DeployRuntime.Get_Status() + "\n").c_str());
		}
	}
	const auto tick = m_Replication.Get_LastServerTick();
	if (m_iCardMazeLastSnapshotTick != tick)
	{ m_iCardMazeLastSnapshotTick = tick; m_fCardMazeSnapshotSeconds = 0.f; }
	else m_fCardMazeSnapshotSeconds = (std::min)(.1f, m_fCardMazeSnapshotSeconds + dt);
	const auto& state = CCombatHUDViewModel::Get().Get_KoukuGimmick().CardMaze;
	const bool playing = state.marchStartTick && state.marchCycleMs;
	if (!playing && !m_bCardMazeMarchPlaying) return;
	const auto targets = Make_WorldSequenceTargets();
	const float elapsed = playing ? std::fmod(
		float(tick - state.marchStartTick) * (1000.f / 30.f) + m_fCardMazeSnapshotSeconds * 1000.f,
		float(state.marchCycleMs)) : 0.f;
	for (const auto& instance : m_SequencePlayer.Get_Document().Get_Instances())
	{
		if (!instance.instanceId.starts_with("cardmiro.march.instance.")) continue;
		const auto* sequence = m_SequencePlayer.Get_Document().Find_Template(instance.templateId);
		const bool active = playing && instance.enabled && sequence &&
			elapsed >= float(instance.startDelayMs) && elapsed < float(instance.startDelayMs + sequence->durationMs);
		if (!active) { m_SequencePlayer.Stop_Instance(instance.instanceId, targets, true); continue; }
		if (!m_SequencePlayer.Is_Playing(instance.instanceId) && !m_SequencePlayer.Play(instance.instanceId, targets)) continue;
		(void)m_SequencePlayer.Seek_InstanceToMs(instance.instanceId, elapsed, targets, false);
	}
	m_bCardMazeMarchPlaying = playing;
}

void Client::CLevel_KakulSaydonArena::Clear_JokerTargetMarker()
{
	CEffectPresentationService::Stop_WorldRoot(m_JokerTargetMarker.handle);
	m_JokerTargetMarker = {};
}

void Client::CLevel_KakulSaydonArena::Submit_JokerTargetMarker()
{
	using namespace LostArk::Shared;
	std::vector<KOUKU_BOSS_PRESENTATION_VIEW> bosses;
	std::vector<KOUKU_CARD_PRESENTATION_VIEW> players;
	m_Replication.Collect_KoukuPresentationViews(bosses, players);
	// Every room member consumes the same Server-selected target. Never infer
	// it from facing, local player identity, or the input ping's lifetime.
	const auto boss = std::find_if(bosses.begin(), bosses.end(), [this](const auto& view) {
		const auto& state = view.Snapshot;
		return view.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON" &&
			m_pTargetedCombatPresentationPlayer && m_pTargetedCombatPresentationPlayer->Is_RandomTargetActive(
                state.strPatternId, view.iServerTick, state.iPatternStartTick) && state.iCurrentHp &&
			state.eAction != WORLD_ENTITY_ACTION::DEAD && state.iPatternSequence &&
			state.iPatternTargetNetEntityId != INVALID_NET_ENTITY_ID;
	});
	const auto target = boss == bosses.end() ? players.end() :
		std::find_if(players.begin(), players.end(), [&](const auto& view) {
			return view.Snapshot.iNetEntityId == boss->Snapshot.iPatternTargetNetEntityId &&
				view.Snapshot.iCurrentHp && view.Snapshot.eAction != PLAYER_ACTION_STATE::DEAD;
		});
	const auto character = target == players.end() ? nullptr : target->pCharacter.lock();
	float3_t head{};
	if (m_Replication.Has_PendingConnectionLoss() || !character ||
		!CWorldPlayerNameplateView::Try_GetHeadAnchor(*character, head))
	{
		Clear_JokerTargetMarker();
		return;
	}
	head.y += 0.55f;
	const auto& state = boss->Snapshot;
	if (m_JokerTargetMarker.bossId != state.iNetEntityId ||
		m_JokerTargetMarker.targetId != state.iPatternTargetNetEntityId ||
		m_JokerTargetMarker.patternSequence != state.iPatternSequence)
	{
		// Retire the former target before preparing its replacement: a missing
		// optional Effect must not keep identifying the wrong player.
		Clear_JokerTargetMarker();
		m_JokerTargetMarker.bossId = state.iNetEntityId;
		m_JokerTargetMarker.targetId = state.iPatternTargetNetEntityId;
		m_JokerTargetMarker.patternSequence = state.iPatternSequence;
	}
	if (m_JokerTargetMarker.failed) return;
	float4x4_t world{};
	XMStoreFloat4x4(&world, XMMatrixTranslation(head.x, head.y, head.z));
	if (!m_JokerTargetMarker.handle.Is_Valid())
	{
		EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
		desc.iLevelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
		desc.strPlacementId = "kouku.joker.target." + std::to_string(state.iNetEntityId);
		desc.strEffectAssetId = "effect.world.target_reticle";
		desc.RootWorld = world;
		desc.bOwnerSustainedSourceLoops = true;
		std::string status;
		if (!CEffectPresentationService::Spawn_LevelPlacement(desc, m_JokerTargetMarker.handle, status))
		{
			m_JokerTargetMarker.failed = true;
			OutputDebugStringA(("[KoukuJokerTarget] Optional target marker unavailable: " + status + "\n").c_str());
			return;
		}
		// This submission runs after object iteration, like the existing auras.
		CEffectPresentationService::Commit_PendingWorldRootSpawns({ m_JokerTargetMarker.handle });
	}
	if (!CEffectPresentationService::Update_WorldRoot(m_JokerTargetMarker.handle, world))
	{
		CEffectPresentationService::Stop_WorldRoot(m_JokerTargetMarker.handle);
		m_JokerTargetMarker.handle = {};
		m_JokerTargetMarker.failed = true;
		OutputDebugStringA("[KoukuJokerTarget] Target marker update failed; stale marker retired.\n");
	}
}

bool_t Client::CLevel_KakulSaydonArena::Load_EntranceTriggerMarkers()
{
	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
		std::string(KAKULSAYDON_AREA_ID) / "Gameplay.world.json"),
		std::string(KAKULSAYDON_AREA_ID), status))
	{
		OutputDebugStringA(("[KoukuEntranceMarker] " + status + "\n").c_str());
		return false;
	}
	static constexpr std::array<std::string_view, 23> triggerIds = {
		"jump.1", "jump.2", "jump.3", "paper.1", "paper.2",
		"Mario1_Trigger_1", "Mario1_Trigger_3", "Mario1_Trigger_5", "Mario2_Trigger_2", "Mario2_Trigger_4",
		"Mario2_Trigger_7", "Mario3_Trigger_4", "Mario3_Trigger_5", "Mario3_Trigger_6", "Mario3_Trigger_8",
		"Mario3_Trigger_10", "Mario3_Trigger_12", "Mario4_Tigger_2", "Mario4_Tigger_3", "Mario4_Tigger_5",
		"Mario4_Tigger_6", "Mario4_Tigger_7", "Mario4_Tigger_13" };
	std::vector<ENTRANCE_TRIGGER_MARKER> staged;
	for (const std::string_view id : triggerIds)
	{
		const auto* placement = document.Find(std::string(id));
		if (!placement || placement->eKind != WORLD_PLACEMENT_KIND::TRIGGER_BOX ||
			placement->triggerEvents.size() != 1u)
		{
			OutputDebugStringA(("[KoukuEntranceMarker] Invalid trigger: " +
				std::string(id) + "\n").c_str());
			return false;
		}
		if (!placement->isEnabled) continue;
		const auto& event = placement->triggerEvents.front();
		if (event.eKind != WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER &&
			event.eKind != WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE)
			return false;
		ENTRANCE_TRIGGER_MARKER marker;
		marker.placementId = placement->placementId;
		if (placement->isTriggerOnce && event.eKind == WORLD_TRIGGER_EVENT_KIND::PLAY_SEQUENCE)
			marker.sequenceInstanceId = event.targetId;
		// Trigger_Box::Rebuild_Bounds uses placement.position as its exact center.
		// No authoring position copy, ground guess, or collider-local offset.
		XMStoreFloat4x4(&marker.rootWorld, XMMatrixTranslation(
			placement->position.x, placement->position.y, placement->position.z));
		staged.push_back(std::move(marker));
	}
	Clear_EntranceTriggerMarkers();
	m_EntranceTriggerMarkers = std::move(staged);
	return true;
}

void Client::CLevel_KakulSaydonArena::Clear_EntranceTriggerMarkers()
{
	for (auto& marker : m_EntranceTriggerMarkers)
		CEffectPresentationService::Stop_WorldRoot(marker.handle);
	m_EntranceTriggerMarkers.clear();
}

void Client::CLevel_KakulSaydonArena::Retire_EntranceTriggerMarker(
	const std::string& sequenceInstanceId)
{
	for (auto& marker : m_EntranceTriggerMarkers)
	{
		if (marker.sequenceInstanceId.empty() || marker.sequenceInstanceId != sequenceInstanceId)
			continue;
		CEffectPresentationService::Stop_WorldRoot(marker.handle);
		marker.handle = {};
		marker.retired = true;
	}
}

void Client::CLevel_KakulSaydonArena::Update_EntranceTriggerMarkerClocks(const f32_t deltaSeconds)
{
	if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.f) return;
	for (auto& marker : m_EntranceTriggerMarkers)
	{
		if (marker.retired) continue;
		if (marker.clockStarted)
			marker.seconds = std::fmod(marker.seconds + deltaSeconds, 7.f);
		marker.clockStarted = true;
	}
}

void Client::CLevel_KakulSaydonArena::Submit_EntranceTriggerMarkers()
{
	CProfilerScope profile(CGameInstance::Get().Get_Profiler(), "Level.Kouku.Markers.Prepare");
	if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::KAKULSAYDON_ARENA))
		return;
	Submit_Gate3Auras();
	Submit_JokerTargetMarker();
	for (auto& marker : m_EntranceTriggerMarkers)
	{
		if (marker.retired) continue;
		// The complete fixed marker footprint stays inside the existing 8m sphere.
		const float3_t center{ marker.rootWorld._41, marker.rootWorld._42, marker.rootWorld._43 };
		const bool_t visible = CEffectPresentationService::Is_WorldPresentationVisible(
			center, 8.f, marker.active);
		if (!visible)
		{
			if (marker.active && marker.handle.Is_Valid())
				(void)CEffectPresentationService::Submit_LevelPlacementSample(marker.handle, false);
			marker.active = false;
			continue;
		}
		const bool_t firstSample = !marker.started;
		if (firstSample)
		{
			// Object iteration has finished. Reuse the Loader's prepared target
			// and commit only this marker, never unrelated pending gameplay cues.
			EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
			desc.iLevelIndex = ETOUI(LEVEL::KAKULSAYDON_ARENA);
			desc.strPlacementId = "kouku.entrance.trigger." + marker.placementId;
			desc.strEffectAssetId = "effect.world.move_destination";
			desc.RootWorld = marker.rootWorld;
			desc.bExternallySampled = true;
			std::string status;
			if (!CEffectPresentationService::Spawn_LevelPlacement(desc, marker.handle, status))
			{
				marker.retired = true;
				OutputDebugStringA(("[KoukuEntranceMarker] " + marker.placementId +
					": " + status + "\n").c_str());
				continue;
			}
			CEffectPresentationService::Commit_PendingWorldRootSpawns({ marker.handle });
			marker.started = true;
		}
		const EFFECT_FIXED_STEP_TRANSFORM_PROVIDER provider =
			[root = marker.rootWorld](f32_t, EFFECT_FIXED_STEP_TRANSFORM_SAMPLE& sample,
				std::string& status)
			{
				sample.RootWorld = root;
				sample.SourceAnchorWorlds.clear();
				status.clear();
				return true;
			};
		if (auto* profiler = CGameInstance::Get().Get_Profiler())
		{
			profiler->Add_Counter(EProfilerCounter::EffectMarkerSamples);
			if (firstSample || !marker.active)
				profiler->Add_Counter(EProfilerCounter::EffectMarkerHistoryRequests);
		}
		const bool_t sampled = CEffectPresentationService::Seek_WorldRoot(marker.handle,
			marker.seconds, provider, firstSample || !marker.active);
		const HRESULT submitted = sampled ?
			CEffectPresentationService::Submit_LevelPlacementSample(marker.handle, true) : E_FAIL;
		if (S_OK != submitted)
		{
			CEffectPresentationService::Stop_WorldRoot(marker.handle);
			marker.handle = {};
			marker.retired = true;
			marker.active = false;
			OutputDebugStringA(("[KoukuEntranceMarker] Sample/submission failed: " +
				marker.placementId + ": " + CEffectPresentationService::Get_Status() + "\n").c_str());
			continue;
		}
		marker.active = true;
	}
}

void Client::CLevel_KakulSaydonArena::Update_TriggerMoveFade(
	const f32_t fTimeDelta)
{
	if (nullptr == m_pTriggerMoveFadeView)
		return;

	if (m_bSequenceCombatFadeHeld)
	{
		Debug_HoldSequenceCombatFade();
		return;
	}
	using LostArk::Shared::PLAYER_ACTION_STATE;
	const auto& maze = CCombatHUDViewModel::Get().Get_KoukuGimmick().CardMaze;
	if (maze.transferStartTick)
	{
		const float ticks = float(m_Replication.Get_LastServerTick() - maze.transferStartTick) + m_fCardMazeSnapshotSeconds * 30.f;
		m_fTriggerMoveFadeAlpha = ticks < 12.f ? std::clamp(ticks / 12.f, 0.f, 1.f) :
			ticks < 24.f ? 1.f : std::clamp((36.f - ticks) / 12.f, 0.f, 1.f);
		m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", m_fTriggerMoveFadeAlpha > 0.f);
		m_pTriggerMoveFadeView->Set_SlotTint("KakulFade_Screen", float4_t(0.f, 0.f, 0.f, m_fTriggerMoveFadeAlpha));
		return;
	}
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t isMoving = player.isValid &&
		PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction;

	/* Every movePlayer trigger uses TRIGGER_MOVE, hops and stage transition
	   alike, so speed is what tells them apart: a 4-6 m hop stays under
	   15 m/s even with its arc, the 1.2 km transition runs at hundreds. */
	constexpr f32_t TRANSITION_SPEED_METRES_PER_SECOND = 40.f;
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	const shared_ptr<CTransform> transform =
		nullptr != localCharacter ? localCharacter->Get_Transform() : nullptr;
	if (nullptr == transform)
	{
		m_bTriggerMoveFadeHasLastPosition = false;
		m_bTriggerMoveFadeArmed = false;
	}
	else
	{
		float3_t position{};
		XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
		if (m_bTriggerMoveFadeHasLastPosition && isMoving &&
			fTimeDelta > 0.f)
		{
			const f32_t dx = position.x - m_vTriggerMoveFadeLastPosition.x;
			const f32_t dy = position.y - m_vTriggerMoveFadeLastPosition.y;
			const f32_t dz = position.z - m_vTriggerMoveFadeLastPosition.z;
			const f32_t speed =
				std::sqrt(dx * dx + dy * dy + dz * dz) / fTimeDelta;
			if (speed > TRANSITION_SPEED_METRES_PER_SECOND)
				m_bTriggerMoveFadeArmed = true;
		}
		m_vTriggerMoveFadeLastPosition = position;
		m_bTriggerMoveFadeHasLastPosition = true;
	}
	if (!isMoving)
		m_bTriggerMoveFadeArmed = false;

	/* Darkening is near-instant so the first frames of the transition are
	   covered; the arrival is revealed gently instead of snapping. */
	constexpr f32_t DARKEN_SECONDS = 0.08f;
	constexpr f32_t BRIGHTEN_SECONDS = 0.4f;
	const f32_t fStep = m_bTriggerMoveFadeArmed
		? fTimeDelta / DARKEN_SECONDS
		: -fTimeDelta / BRIGHTEN_SECONDS;
	m_fTriggerMoveFadeAlpha =
		std::clamp(m_fTriggerMoveFadeAlpha + fStep, 0.f, 1.f);

	const bool_t bVisible = m_fTriggerMoveFadeAlpha > 0.f;
	m_pTriggerMoveFadeView->Set_SlotVisible("KakulFade_Screen", bVisible);
	if (!bVisible)
		return;
	/* Set_SlotAlpha would rewrite RGB to white, which is the opposite of a
	   blackout, so the tint is written whole. */
	m_pTriggerMoveFadeView->Set_SlotTint("KakulFade_Screen",
		float4_t(0.f, 0.f, 0.f, m_fTriggerMoveFadeAlpha));
}

bool_t Client::CLevel_KakulSaydonArena::Try_GetCompositionWorldPivot(
 const std::string_view instanceId, float4x4_t& out, const std::string_view occurrenceId,
 const std::uint32_t emissionIndex, const std::string& bone, const bool_t boneRotation,
 const std::string& effectTrackId) const
{
#ifdef _DEBUG
 if (!m_CompositionWorldPreviewCues.empty())
 {
  const CWorldSequencePlayer* selected = nullptr;
  for (const auto& [id, playback] : m_CompositionWorldPreviewCues)
   if ((occurrenceId.empty() || id == occurrenceId) && playback.cue.instanceId == instanceId &&
    playback.player->Has_ActiveInstances())
   {
    if (selected) return false;
    selected = playback.player.get();
   }
  return selected && CompositionWorldPivot(*selected, std::string(instanceId), out, emissionIndex, bone, boneRotation, effectTrackId);
 }
#endif
 return m_SequencePlayer.Try_GetSequencePivot(std::string(instanceId), out, emissionIndex, bone, boneRotation, effectTrackId);
}

bool_t Client::CLevel_KakulSaydonArena::Is_CinematicPresentationActive() const
{
	if (m_bSequenceCombatPending) return true;
	if (!m_pCamera) return false;
	// Static/follow combat framing is not a cutscene. Only an owned timed track is.
	if (m_CompositionCamera.cinematicTrack && !m_CompositionCamera.ownerKey.empty() &&
		m_pCamera->Is_PresentationOverrideOwnedBy(0x4b4f554b55434f4dull)) return true;
	if (!m_bCameraShotHeld || !m_pCamera->Is_PresentationOverrideOwnedBy(KAKULSAYDON_CAMERA_SHOT_OWNER_ID)) return false;
	const auto shot = std::find_if(m_CameraShots.begin(), m_CameraShots.end(), [this](const auto& value) {
		return value.strShotId == m_strActiveCameraShotId;
	});
	return shot != m_CameraShots.end() && shot->hasCameraTrack && !shot->followsPlayer &&
		!shot->strSequenceInstanceId.empty() && m_SequencePlayer.Is_Playing(shot->strSequenceInstanceId);
}

bool_t Client::CLevel_KakulSaydonArena::Is_CinematicInputBlocked() const
{
	if (m_bSequenceCombatPending) return true;
	if (!Is_CinematicPresentationActive()) return false;
	if (m_pCamera && m_CompositionCamera.cinematicTrack && !m_CompositionCamera.ownerKey.empty() &&
		m_pCamera->Is_PresentationOverrideOwnedBy(0x4b4f554b55434f4dull))
		return !CinematicShotAllowsGameplay(m_CompositionCamera.shotId);
	if (m_pCamera && m_bCameraShotHeld &&
		m_pCamera->Is_PresentationOverrideOwnedBy(KAKULSAYDON_CAMERA_SHOT_OWNER_ID))
		return !CinematicShotAllowsGameplay(m_strActiveCameraShotId);
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Should_HideCinematicPlayers() const
{
	if (!Is_CinematicPresentationActive()) return false;
	if (m_pCamera && m_CompositionCamera.cinematicTrack && !m_CompositionCamera.ownerKey.empty() &&
		m_pCamera->Is_PresentationOverrideOwnedBy(0x4b4f554b55434f4dull))
		return !CinematicShotShowsPlayers(m_CompositionCamera.shotId);
	if (m_pCamera && m_bCameraShotHeld &&
		m_pCamera->Is_PresentationOverrideOwnedBy(KAKULSAYDON_CAMERA_SHOT_OWNER_ID))
		return !CinematicShotShowsPlayers(m_strActiveCameraShotId);

	// Keep an allowed Server sequence visible while its next camera row starts.
	// P5 also contains the preceding clear scene: its gaps stay hidden and
	// only the entry camera shots above reveal players. Unknown scenes hide.
	const auto& raid = m_Replication.Get_KoukuRaidState();
	if (raid.ePhase == LostArk::Shared::KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
		raid.strSequenceCompositionId == "boss.composition.kakulsaydon.sequencer")
	{
		const std::string_view pattern = raid.strSequencePatternId;
		if (pattern == "KAKULSAYDON_G1_PATTERN_4" || pattern == "KAKULSAYDON_G1_PATTERN_6" ||
			pattern == "KAKULSAYDON_G1_PATTERN_7") return false;
	}
	return true;
}

void Client::CLevel_KakulSaydonArena::Sync_CinematicPlayerVisibility()
{
	const bool_t suppressed = Should_HideCinematicPlayers();
	for (const auto& player : m_NameplatePlayers)
		if (const auto character = player.pCharacter.lock())
			character->Set_CinematicPresentationSuppressed(suppressed);
}

void Client::CLevel_KakulSaydonArena::Trace_CinematicPresentation(const std::string_view renderingProfile)
{
	const auto& raid = m_Replication.Get_KoukuRaidState();
	const std::string key = std::to_string(Is_CinematicPresentationActive()) + ":" +
		std::to_string(m_iCinematicSuppressedCount) + ":" +
		std::to_string(raid.iRunEpoch) + ":" + std::to_string(static_cast<int>(raid.ePhase)) + ":" +
		raid.strGateId + ":" + raid.strSequencePatternId + ":" + m_CompositionCamera.ownerKey + ":" +
		m_strActiveCameraShotId + ":" + std::string(renderingProfile) + ":" + m_strGatePresentationProfileId;
	if (key == m_strCinematicDiagnosticKey) return;
	m_strCinematicDiagnosticKey = key;
	std::size_t visible = 0u, unknown = 0u;
	for (const auto& entry : m_MapRuntime.Get_Placements())
	{
		bool_t shown = false;
		if (!CMapPlacementRuntime::Try_GetRuntimeVisible(entry, shown)) ++unknown;
		else if (shown) ++visible;
	}
	std::ostringstream detail;
	detail << "cinematic=" << Is_CinematicPresentationActive() << " playersHidden=" << Should_HideCinematicPlayers()
		<< " pending=" << m_bSequenceCombatPending
		<< " raidEpoch=" << raid.iRunEpoch << " phase=" << static_cast<int>(raid.ePhase)
		<< " gate=" << raid.strGateId << " sequence=" << raid.strSequencePatternId
		<< " scene=" << renderingProfile << " gateScene=" << m_strGatePresentationProfileId
		<< " mapVisible=" << visible << " mapTotal=" << m_MapRuntime.Get_Placements().size() << " mapUnknown=" << unknown
		<< " lightsBase=" << (m_pMapLightPresentation ? m_pMapLightPresentation->Get_Document().Get_Lights().size() : 0u)
		<< " lightsGate=" << (m_pGateMapLightPresentation ? m_pGateMapLightPresentation->Get_Document().Get_Lights().size() : 0u)
		<< " lightGateIndex=" << m_iGateLightingIndex << " lightAuthoring=" << bool(m_pMapLightAuthoringOverride)
		<< " cameraOwner=" << m_CompositionCamera.ownerKey << " cameraShot=" << m_CompositionCamera.shotId
		<< " areaShot=" << m_strActiveCameraShotId << " cameraReturning=" << m_CompositionCamera.returning
		<< " fade=" << m_fTriggerMoveFadeAlpha << " fadeHeld=" << m_bSequenceCombatFadeHeld
		<< " stageAreas=" << m_CinematicStageAreas.size() << " stageSuppressed=" << m_iCinematicSuppressedCount
		<< " stageKept=";
	{
		// Centre x,z of every kept area, so one log line shows which stages a cutscene kept drawn.
		bool_t anyKept = false;
		for (std::size_t area = 0u; area < m_CinematicKeptAreas.size() && area < m_CinematicStageAreas.size(); ++area)
		{
			if (0u == m_CinematicKeptAreas[area])
				continue;
			const CINEMATIC_STAGE_AREA& bounds = m_CinematicStageAreas[area];
			detail << (anyKept ? "|" : "")
				<< static_cast<int>(std::lround((bounds.fMinX + bounds.fMaxX) * 0.5f)) << ","
				<< static_cast<int>(std::lround((bounds.fMinZ + bounds.fMaxZ) * 0.5f));
			anyKept = true;
		}
		if (!anyKept)
			detail << "none";
	}
#ifdef _DEBUG
	detail << " lightComposition=" << m_bCompositionMapLightPreviewActive;
#endif
	CNetworkManager::Get().Record_SessionEvent("kouku.cinematic.transition", detail.str());
}

void Client::CLevel_KakulSaydonArena::Build_CinematicStageAreas()
{
	m_CinematicStageAreas.clear();
	m_CinematicAreaOfPlacement.clear();
	m_CinematicKeptAreas.clear();
	m_CinematicKeptScratch.clear();
	m_bCinematicSurroundingsApplied = false;
	m_iCinematicSuppressedCount = 0u;
	m_iCinematicOwnedSignature = 0u;
	const auto& placements = m_MapRuntime.Get_Placements();
	if (placements.empty())
		return;

	/* The stages sit in separate areas of one big map. Placements closer than the link
	   distance belong to the same area, so an area is a connected cluster of the authored
	   positions and needs no extra authoring data. Measured on the shipped map, 60 m splits
	   it into the arenas, the plaza and the Mario stages without bridging two of them. */
	constexpr f32_t LINK_METERS = 60.f;
	const std::size_t count = placements.size();
	std::vector<uint32_t> parent(count);
	for (std::size_t index = 0u; index < count; ++index)
		parent[index] = static_cast<uint32_t>(index);
	const auto findRoot = [&parent](uint32_t node)
	{
		while (parent[node] != node)
		{
			parent[node] = parent[parent[node]];
			node = parent[node];
		}
		return node;
	};
	const auto cellOf = [](const f32_t value)
	{
		return static_cast<int32_t>(std::floor(value / LINK_METERS));
	};
	const auto cellKey = [](const int32_t cellX, const int32_t cellZ)
	{
		return (static_cast<uint64_t>(static_cast<uint32_t>(cellX)) << 32) |
			static_cast<uint64_t>(static_cast<uint32_t>(cellZ));
	};
	std::unordered_map<uint64_t, std::vector<uint32_t>> grid;
	grid.reserve(count);
	for (std::size_t index = 0u; index < count; ++index)
	{
		const float3_t& position = placements[index].record.position;
		if (!std::isfinite(position.x) || !std::isfinite(position.z))
			continue;
		grid[cellKey(cellOf(position.x), cellOf(position.z))].push_back(static_cast<uint32_t>(index));
	}
	for (const auto& cell : grid)
	{
		const float3_t& origin = placements[cell.second.front()].record.position;
		const int32_t cellX = cellOf(origin.x);
		const int32_t cellZ = cellOf(origin.z);
		for (int32_t offsetX = -1; offsetX <= 1; ++offsetX)
		{
			for (int32_t offsetZ = -1; offsetZ <= 1; ++offsetZ)
			{
				const auto neighbour = grid.find(cellKey(cellX + offsetX, cellZ + offsetZ));
				if (neighbour == grid.end())
					continue;
				for (const uint32_t first : cell.second)
				{
					const float3_t& a = placements[first].record.position;
					for (const uint32_t second : neighbour->second)
					{
						if (second <= first)
							continue;
						const float3_t& b = placements[second].record.position;
						const f32_t deltaX = a.x - b.x;
						const f32_t deltaZ = a.z - b.z;
						if (deltaX * deltaX + deltaZ * deltaZ > LINK_METERS * LINK_METERS)
							continue;
						const uint32_t rootA = findRoot(first);
						const uint32_t rootB = findRoot(second);
						if (rootA != rootB)
							parent[rootA] = rootB;
					}
				}
			}
		}
	}

	std::unordered_map<uint32_t, uint32_t> areaOfRoot;
	m_CinematicAreaOfPlacement.reserve(count);
	for (const auto& cell : grid)
	{
		for (const uint32_t index : cell.second)
		{
			const float3_t& position = placements[index].record.position;
			const auto emplaced = areaOfRoot.try_emplace(
				findRoot(index), static_cast<uint32_t>(m_CinematicStageAreas.size()));
			if (emplaced.second)
				m_CinematicStageAreas.push_back({ position.x, position.x, position.z, position.z });
			CINEMATIC_STAGE_AREA& area = m_CinematicStageAreas[emplaced.first->second];
			area.fMinX = (std::min)(area.fMinX, position.x);
			area.fMaxX = (std::max)(area.fMaxX, position.x);
			area.fMinZ = (std::min)(area.fMinZ, position.z);
			area.fMaxZ = (std::max)(area.fMaxZ, position.z);
			m_CinematicAreaOfPlacement[placements[index].record.placementId] = emplaced.first->second;
		}
	}
}

void Client::CLevel_KakulSaydonArena::Update_CinematicSurroundings()
{
	if (m_CinematicStageAreas.empty())
		return;
#ifdef _DEBUG
	// The Map Tool edits and previews every placement, so nothing may be hidden under it.
	if (m_bMapAuthoringActive)
	{
		if (m_bCinematicSurroundingsApplied)
			Restore_CinematicSurroundings();
		return;
	}
#endif
	if (!Is_CinematicPresentationActive())
	{
		if (m_bCinematicSurroundingsApplied)
			Restore_CinematicSurroundings();
		return;
	}
	const float4x4_t* const cameraWorld = CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW);
	if (nullptr == cameraWorld)
		return;
	const float3_t eye(cameraWorld->_41, cameraWorld->_42, cameraWorld->_43);
	const float3_t look(cameraWorld->_31, cameraWorld->_32, cameraWorld->_33);
	const f32_t lookLength = std::sqrt(look.x * look.x + look.y * look.y + look.z * look.z);
	if (!std::isfinite(eye.x) || !std::isfinite(eye.z) || !std::isfinite(lookLength) || lookLength < 1e-4f)
		return;
	// Row 2 of the camera world matrix is the look direction; normalise so the sample
	// distances below are metres whatever scale the matrix carries.
	const float3_t forward(look.x / lookLength, look.y / lookLength, look.z / lookLength);

	/* The areas a cutscene may show: the ones around the camera and around what it looks at
	   (forward samples). The margin and the samples were fitted on the authored camera tracks:
	   of the 2920 keyframes in 92 tracks, the 2741 whose shot has a stage area within 100 m
	   never lost that area at 80 m with 40 m and 100 m samples, while 60 m and below did, and
	   the 22 static shots keep theirs too. The local player is deliberately NOT an anchor: it can
	   stand in another stage while a cutscene plays elsewhere (a Workbench preview, a gate
	   teleport in flight), and that whole stage then stayed drawn as a distant piece. */
	constexpr f32_t AREA_MARGIN_METERS = 80.f;
	constexpr std::array<f32_t, 2> LOOK_SAMPLE_METERS = { 40.f, 100.f };
	std::array<float3_t, 3> anchors{};
	std::size_t anchorCount = 0u;
	anchors[anchorCount++] = eye;
	for (const f32_t meters : LOOK_SAMPLE_METERS)
		anchors[anchorCount++] = float3_t(
			eye.x + forward.x * meters, eye.y + forward.y * meters, eye.z + forward.z * meters);

	std::vector<uint8_t>& kept = m_CinematicKeptScratch;
	kept.assign(m_CinematicStageAreas.size(), 0u);
	for (std::size_t area = 0u; area < m_CinematicStageAreas.size(); ++area)
	{
		const CINEMATIC_STAGE_AREA& bounds = m_CinematicStageAreas[area];
		for (std::size_t anchor = 0u; anchor < anchorCount; ++anchor)
		{
			const float3_t& point = anchors[anchor];
			if (point.x >= bounds.fMinX - AREA_MARGIN_METERS && point.x <= bounds.fMaxX + AREA_MARGIN_METERS &&
				point.z >= bounds.fMinZ - AREA_MARGIN_METERS && point.z <= bounds.fMaxZ + AREA_MARGIN_METERS)
			{
				kept[area] = 1u;
				break;
			}
		}
	}
	/* No kept area is a valid answer: the Gate 2 intro shots sit about 300 m outside every
	   area (and below the map), so there only what the Sequence owns is drawn and every area
	   placement is suppressed. */
	// The placements a Sequence owns change when one starts, holds a pose or stops, which must re-apply too.
	const uint64_t ownedSignature = m_SequencePlayer.Collect_OwnedPlacements(nullptr);
	if (m_bCinematicSurroundingsApplied && kept == m_CinematicKeptAreas &&
		ownedSignature == m_iCinematicOwnedSignature)
		return;
	Apply_CinematicSurroundings(kept);
}

void Client::CLevel_KakulSaydonArena::Apply_CinematicSurroundings(const std::vector<uint8_t>& keptAreas)
{
	/* A placement a Sequence that is playing (or holding a pose) manipulates is owned by that
	   Sequence, which reveals and moves it on its own clock, so it is never suppressed. Only those
	   count: a Sequence that is not running owns nothing, so its placements follow the area rule
	   like any other (exempting every authored binding kept other stages' props drawn). Backdrop-
	   size placements are shared by every area they tower over and stay as well. */
	constexpr f32_t BACKDROP_SCALE = 100.f;
	std::unordered_set<uint64_t> sequenceOwned;
	m_iCinematicOwnedSignature = m_SequencePlayer.Collect_OwnedPlacements(&sequenceOwned);
	std::size_t suppressed = 0u;
	for (MAP_RUNTIME_PLACED_ENTRY& entry : m_MapRuntime.Get_MutablePlacements())
	{
		bool_t suppress = false;
		const auto area = m_CinematicAreaOfPlacement.find(entry.record.placementId);
		if (area != m_CinematicAreaOfPlacement.end() && area->second < keptAreas.size() &&
			0u == keptAreas[area->second] && !sequenceOwned.contains(entry.record.placementId) &&
			(std::max)({ std::fabs(entry.record.signedScale.x), std::fabs(entry.record.signedScale.y),
				std::fabs(entry.record.signedScale.z) }) < BACKDROP_SCALE)
		{
			suppress = true;
		}
		(void)CMapPlacementRuntime::Set_RuntimeSuppressed(entry, suppress);
		if (suppress)
			++suppressed;
	}
	m_CinematicKeptAreas = keptAreas;
	m_bCinematicSurroundingsApplied = true;
	m_iCinematicSuppressedCount = suppressed;
}

void Client::CLevel_KakulSaydonArena::Restore_CinematicSurroundings()
{
	for (MAP_RUNTIME_PLACED_ENTRY& entry : m_MapRuntime.Get_MutablePlacements())
		(void)CMapPlacementRuntime::Set_RuntimeSuppressed(entry, false);
	m_CinematicKeptAreas.clear();
	m_bCinematicSurroundingsApplied = false;
	m_iCinematicSuppressedCount = 0u;
	m_iCinematicOwnedSignature = 0u;
}

bool_t Client::CLevel_KakulSaydonArena::Begin_ServerEncoreView(std::string& status)
{
    const auto& raid = m_Replication.Get_KoukuRaidState();
    const bool encore = raid.ePhase == LostArk::Shared::KOUKUSAYDON_RAID_PHASE::CINEMATIC &&
        raid.strGateId == "BINGO" && raid.strSequenceCompositionId == "boss.composition.kakulsaydon.sequencer" &&
        raid.strSequencePatternId == "KAKULSAYDON_G1_PATTERN_10";
    if (m_ServerEncoreView && (!encore || m_ServerEncoreView->runEpoch != raid.iRunEpoch ||
        m_ServerEncoreView->startTick != raid.iStartTick))
    {
        m_ServerEncoreView.reset();
        Stop_CompositionCamera(true);
    }
    if (!encore || m_ServerEncoreView) return true;
    if (!m_pPendingGateObjects || !m_pPendingGateObjects->serverRaidPrepared)
    { status = "Encore player view requires the prepared Server gate."; return false; }
    const auto shot = std::find_if(m_CameraShots.begin(), m_CameraShots.end(), [](const auto& row) {
        return row.strShotId == "kouku.bingo.encore.camera.1" && row.hasCameraTrack;
    });
    SERVER_ENCORE_VIEW candidate;
    candidate.runEpoch = raid.iRunEpoch; candidate.startTick = raid.iStartTick;
    if (shot == m_CameraShots.end() || !m_pCamera || !CCameraTool::Capture_ViewPose(candidate.heldPose))
    { status = "Encore published camera or current player view is unavailable."; return false; }
    candidate.authoredTrack = shot->CameraTrack;
    candidate.blendOutMs = shot->iBlendOutMs; candidate.easing = shot->eTransitionEasing;
    m_ServerEncoreView = std::move(candidate);
    if (m_pCamera->Is_FollowEnabled() && !Acquire_ServerEncoreView(status))
    { m_ServerEncoreView.reset(); return false; }
    return true;
}

bool_t Client::CLevel_KakulSaydonArena::Acquire_ServerEncoreView(std::string& status)
{
    constexpr uint64_t owner = 0x4b4f554b55434f4dull;
    if (!m_ServerEncoreView || !m_pCamera || !m_pCamera->Is_FollowEnabled()) return false;
    const auto& view = *m_ServerEncoreView;
    if (!m_pCamera->Begin_PresentationOverride(owner, Engine::CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW))
    { status = "Encore could not acquire the player camera."; return false; }
    Release_CameraShot();
    const auto& pose = view.heldPose;
    if (!m_pCamera->Apply_PresentationPoseWithUp(owner, pose.vEye, pose.vLookAt, pose.vUp, pose.fFovYDegrees))
    {
        (void)m_pCamera->End_PresentationOverride(owner);
        status = "Encore could not hold the captured player camera.";
        return false;
    }
    auto& transition = m_CompositionCamera;
    transition = {};
    transition.ownerKey = "server.encore." + std::to_string(view.runEpoch) + "." + std::to_string(view.startTick);
    transition.shotId = "kouku.bingo.encore.camera.1"; transition.cinematicTrack = true;
    transition.fromPose = transition.entryPose = transition.appliedPose = pose;
    transition.blendOutMs = view.blendOutMs; transition.easing = view.easing;
    transition.followAtStart = true;
    return true;
}

bool_t Client::CLevel_KakulSaydonArena::Is_CompositionCameraEnabled() const
{
	return m_pCamera && m_pCamera->Is_FollowEnabled();
}

bool_t Client::CLevel_KakulSaydonArena::Sample_CompositionCamera(
	const std::string_view shotId, const float seconds, const float3_t& offset,
	const std::string_view ownerKey, const uint32_t durationMs, const bool_t preview)
{
	constexpr uint64_t owner = 0x4b4f554b55434f4dull;
	if (!m_pCamera || ownerKey.empty() || !std::isfinite(seconds) || seconds < 0.f || durationMs == 0u ||
		!std::isfinite(offset.x) || !std::isfinite(offset.y) || !std::isfinite(offset.z)) return false;
	if (!Is_CompositionCameraEnabled()) return false;
    const bool serverEncore = m_ServerEncoreView && shotId == "kouku.bingo.encore.camera.1";
	if (preview && !serverEncore) { std::string status; if (!Ensure_CameraShotAuthoring(status)) return false; }
	const auto& shots = preview && !serverEncore ? m_AuthoringCameraShots : m_CameraShots;
	const auto found = std::find_if(shots.begin(), shots.end(), [shotId](const auto& shot) { return shot.strShotId == shotId; });
	if (found == shots.end() || durationMs < found->iBlendInMs) return false;
	if (!preview && !Is_SequenceCameraAudience(found->strSequenceInstanceId,
		(m_Replication.Get_CameraPlayerSnapshot() ? m_Replication.Get_CameraPlayerSnapshot()->iMarioStage : 0u))) return false;
	auto& transition = m_CompositionCamera;
	if (transition.cancelledOwnerKey == ownerKey) return false;
	// The Gate 1 book row includes a World tail after its authored camera has finished.
	// Other shots keep their explicitly authored row window (including inter-shot holds).
	const bool cameraTrackFinished = shotId == "kouku.gate1.authored.book" && found->hasCameraTrack &&
		!found->followsPlayer && found->CameraTrack.iDurationMs &&
		seconds * 1000.f >= float(found->CameraTrack.iDurationMs);
	if (cameraTrackFinished && transition.finishedOwnerKey == ownerKey) return true;
	if (transition.ownerKey != ownerKey || transition.returning || seconds + 0.01f < transition.lastSeconds)
	{
		VALTAN_CINEMATIC_CAMERA_POSE current;
        if (serverEncore) current = m_ServerEncoreView->heldPose;
		else if (!CCameraTool::Capture_ViewPose(current)) return false;
		if (!m_pCamera->Begin_PresentationOverride(owner, Engine::CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW)) return false;
		// Taking over an Area shot keeps the displayed pose but drops its stale owner state.
		Release_CameraShot();
		transition.ownerKey = std::string(ownerKey);
		transition.shotId = std::string(shotId);
		transition.cinematicTrack = found->hasCameraTrack && !found->followsPlayer;
		transition.cancelledOwnerKey.clear();
		transition.finishedOwnerKey.clear();
		transition.fromPose = current;
		transition.entryPose = current;
		transition.appliedPose = current;
		transition.blendOutMs = found->iBlendOutMs;
		transition.easing = found->eTransitionEasing;
		transition.returnSeconds = 0.f;
		transition.returning = false;
		transition.followAtStart = m_pCamera->Is_FollowEnabled();
	}
	if (cameraTrackFinished)
	{
		// Late joins/seeks first inherit the currently displayed Area/Composition pose.
		Stop_CompositionCamera();
		transition.finishedOwnerKey = std::string(ownerKey);
		return true;
	}
	if (!m_pCamera->Is_PresentationOverrideOwnedBy(owner) ||
		m_pCamera->Is_FollowEnabled() != transition.followAtStart)
	{ Stop_CompositionCamera(true); return false; }
	VALTAN_CINEMATIC_CAMERA_POSE target{ found->vEye, found->vLookAt, found->fFovYDegrees };
    if (serverEncore) target = m_ServerEncoreView->heldPose;
	else if (found->hasCameraTrack)
	{
		if (!CValtanCinematicCameraController::Sample_Cue(found->CameraTrack, seconds, target))
		{ Stop_CompositionCamera(true); return false; }
	}
	else if (found->followsPlayer)
	{
		const auto transform = m_Replication.Get_CameraTarget();
		if (!transform)
		{ Stop_CompositionCamera(true); return false; }
		float3_t player; XMStoreFloat3(&player, transform->Get_State(STATE::POSITION));
		target.vEye = float3_t(player.x + found->vFollowEyeOffset.x, player.y + found->vFollowEyeOffset.y, player.z + found->vFollowEyeOffset.z);
		target.vLookAt = float3_t(player.x + found->vFollowLookAtOffset.x, player.y + found->vFollowLookAtOffset.y, player.z + found->vFollowLookAtOffset.z);
	}
    if (!serverEncore)
    {
	    target.vEye.x += offset.x; target.vEye.y += offset.y; target.vEye.z += offset.z;
	    target.vLookAt.x += offset.x; target.vLookAt.y += offset.y; target.vLookAt.z += offset.z;
    }
	VALTAN_CINEMATIC_CAMERA_POSE applied = target;
	if (found->iBlendInMs && !CValtanCinematicCameraController::Sample_BoundedTransition(
		transition.fromPose, target, found->iBlendInMs, seconds, applied, found->eTransitionEasing))
	{ Stop_CompositionCamera(true); return false; }
	if (!(applied.hasUp ? m_pCamera->Apply_PresentationPoseWithUp(owner, applied.vEye, applied.vLookAt, applied.vUp, applied.fFovYDegrees) :
		m_pCamera->Apply_PresentationPose(owner, applied.vEye, applied.vLookAt, applied.fFovYDegrees)))
	{ Stop_CompositionCamera(true); return false; }
	transition.appliedPose = applied;
	transition.lastSeconds = seconds;
	return true;
}

bool_t Client::CLevel_KakulSaydonArena::Resolve_CompositionFollowPose(VALTAN_CINEMATIC_CAMERA_POSE& outPose) const
{
	const auto transform = m_Replication.Get_CameraTarget();
	if (!transform) return false;
	float3_t position; XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
	const auto eyeOffset = m_EffectiveFollowCameraProfile.positionOffset;
	const auto lookOffset = CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile);
	outPose.vEye = float3_t(position.x + eyeOffset.x, position.y + eyeOffset.y, position.z + eyeOffset.z);
	outPose.vLookAt = float3_t(position.x + lookOffset.x, position.y + lookOffset.y, position.z + lookOffset.z);
	outPose.fFovYDegrees = m_EffectiveFollowCameraProfile.fovYDegrees;
	const auto rotation = m_EffectiveFollowCameraProfile.rotationDegrees;
	const auto basis = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
	XMStoreFloat3(&outPose.vUp, basis.r[1]); outPose.hasUp = true;
	return true;
}

void Client::CLevel_KakulSaydonArena::Stop_CompositionCamera(const bool_t force)
{
	auto& transition = m_CompositionCamera;
    // The authored camera ends before the Encore sound tail and Server gate handoff.
    // F6/abort may still release the override immediately.
    if (!force && m_ServerEncoreView && transition.shotId == "kouku.bingo.encore.camera.1") return;
	transition.finishedOwnerKey.clear();
	if (transition.ownerKey.empty()) return;
	if (force)
	{
		// F6 suspends camera playback; returning to follow may resume this same row.
		transition.cancelledOwnerKey = m_pCamera && !m_pCamera->Is_FollowEnabled() ?
			std::string() : transition.ownerKey;
		if (m_pCamera)
		{
			auto pose = transition.appliedPose;
			if (m_pCamera->Is_FollowEnabled())
			{
				(void)Resolve_CompositionFollowPose(pose);
				(void)m_pCamera->End_PresentationOverrideToPose(0x4b4f554b55434f4dull, pose.vEye, pose.vLookAt, pose.fFovYDegrees);
			}
			else (void)m_pCamera->End_PresentationOverrideAtCurrentPose(0x4b4f554b55434f4dull);
		}
		transition.ownerKey.clear(); transition.returning = false;
		return;
	}
	if (!transition.returning)
	{
		transition.fromPose = transition.appliedPose;
		transition.returning = true; transition.returnSeconds = 0.f;
	}
}

void Client::CLevel_KakulSaydonArena::Update_CompositionCamera(const f32_t timeDelta)
{
	constexpr uint64_t owner = 0x4b4f554b55434f4dull;
	auto& transition = m_CompositionCamera;
	if (m_pCamera && !m_pCamera->Is_FollowEnabled())
	{
		Stop_CompositionCamera(true);
		transition.cancelledOwnerKey.clear();
		return;
	}
    if (m_ServerEncoreView && transition.ownerKey.empty() && m_pCamera && m_pCamera->Is_FollowEnabled())
    {
        // F6 may return after the authored Camera row has already ended.
        // The Server lease still owns its sound tail, using the same captured view.
        std::string status;
        if (!Acquire_ServerEncoreView(status)) return;
    }
	if (transition.ownerKey.empty()) return;
	if (!m_pCamera || !m_pCamera->Is_PresentationOverrideOwnedBy(owner) ||
		m_pCamera->Is_FollowEnabled() != transition.followAtStart)
	{ Stop_CompositionCamera(true); return; }
	if (!transition.returning || !std::isfinite(timeDelta) || timeDelta < 0.f) return;
	VALTAN_CINEMATIC_CAMERA_POSE target = transition.entryPose;
	if (transition.followAtStart && !Resolve_CompositionFollowPose(target)) { Stop_CompositionCamera(true); return; }
	transition.returnSeconds += timeDelta;
	VALTAN_CINEMATIC_CAMERA_POSE applied = target;
	if (transition.blendOutMs && !CValtanCinematicCameraController::Sample_BoundedTransition(
		transition.fromPose, target, transition.blendOutMs, transition.returnSeconds, applied, transition.easing))
	{ Stop_CompositionCamera(true); return; }
	if (!(applied.hasUp ? m_pCamera->Apply_PresentationPoseWithUp(owner, applied.vEye, applied.vLookAt, applied.vUp, applied.fFovYDegrees) :
		m_pCamera->Apply_PresentationPose(owner, applied.vEye, applied.vLookAt, applied.fFovYDegrees)))
	{ Stop_CompositionCamera(true); return; }
	transition.appliedPose = applied;
	if (transition.returnSeconds * 1000.f >= float(transition.blendOutMs))
	{
		if (transition.followAtStart) (void)m_pCamera->End_PresentationOverrideToPose(owner, target.vEye, target.vLookAt, target.fFovYDegrees);
		else (void)m_pCamera->End_PresentationOverride(owner);
		if (transition.followAtStart)
			(void)m_pCamera->Set_FollowPose(m_EffectiveFollowCameraProfile.positionOffset, CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile),
				m_EffectiveFollowCameraProfile.rotationDegrees.z, m_EffectiveFollowCameraProfile.fovYDegrees, m_EffectiveFollowCameraProfile.followResponse);
		transition.ownerKey.clear(); transition.returning = false;
	}
}

void Client::CLevel_KakulSaydonArena::Update_CameraShots(const f32_t fTimeDelta)
{
	if (!m_CompositionCamera.ownerKey.empty() || !m_CompositionCamera.finishedOwnerKey.empty()) return;
	if (nullptr == m_pCamera || m_CameraShots.empty())
		return;
	if (!m_pCamera->Is_FollowEnabled())
	{
		/* The free camera owns the view while it is on. */
		Release_CameraShot();
		return;
	}
	const auto transform = m_Replication.Get_CameraTarget();
	float3_t position{};
	if (nullptr != transform)
		XMStoreFloat3(&position, transform->Get_State(STATE::POSITION));
	else if (!m_bCameraShotHeld)
	{
		/* Without a Character there is no follow pose to hand back to, so a
		   shot may only start once the local player exists. */
		return;
	}

	// The same tuned framing is the destination when a camera shot ends.
	const float3_t positionOffset = m_EffectiveFollowCameraProfile.positionOffset;
	const float3_t lookOffset = CArenaCameraProfile::LookOffset(m_EffectiveFollowCameraProfile);
	const float3_t followEye(
		position.x + positionOffset.x,
		position.y + positionOffset.y,
		position.z + positionOffset.z);
	const float3_t followLook(
		position.x + lookOffset.x,
		position.y + lookOffset.y,
		position.z + lookOffset.z);

	const KAKUL_CAMERA_SHOT* shot = Find_ActiveCameraShot(position);
	const std::string shotId = nullptr != shot ? shot->strShotId : std::string();
	if (shotId != m_strActiveCameraShotId)
	{
		uint32_t blendMs = 0u;
		if (nullptr != shot)
		{
			blendMs = shot->iBlendInMs;
		}
		else
		{
			const auto previous = std::find_if(
				m_CameraShots.begin(), m_CameraShots.end(),
				[this](const KAKUL_CAMERA_SHOT& value)
				{
					return value.strShotId == m_strActiveCameraShotId;
				});
			blendMs = m_CameraShots.end() != previous ?
				previous->iBlendOutMs : 0u;
		}
		/* Freeze the starting pose once per hand-over. Advancing both the
		   start and the ratio would shorten every blend. */
		if (m_bCameraShotHeld)
		{
			m_vCameraEyeFrom = m_vCameraEyeApplied;
			m_vCameraLookFrom = m_vCameraLookApplied;
			m_fCameraFovFrom = m_fCameraFovApplied;
		}
		else
		{
			m_vCameraEyeFrom = followEye;
			m_vCameraLookFrom = followLook;
			m_fCameraFovFrom = m_EffectiveFollowCameraProfile.fovYDegrees;
		}
		m_strActiveCameraShotId = shotId;
		m_fCameraBlendSeconds = static_cast<f32_t>(blendMs) / 1000.f;
		m_fCameraBlendElapsed = 0.f;
	}

	if (nullptr != shot)
	{
		m_vCameraEyeTo = shot->vEye;
		m_vCameraLookTo = shot->vLookAt;
		m_fCameraFovTo = shot->fFovYDegrees;
		if (shot->followsPlayer && nullptr != transform)
		{
			/* The side scrolling stages keep this framing and slide it with
			   the Character, so the backdrop stays behind the run line. */
			m_vCameraEyeTo = float3_t(
				position.x + shot->vFollowEyeOffset.x,
				position.y + shot->vFollowEyeOffset.y,
				position.z + shot->vFollowEyeOffset.z);
			m_vCameraLookTo = float3_t(
				position.x + shot->vFollowLookAtOffset.x,
				position.y + shot->vFollowLookAtOffset.y,
				position.z + shot->vFollowLookAtOffset.z);
		}
		f32_t cueElapsedMs = 0.f;
		VALTAN_CINEMATIC_CAMERA_POSE cuePose{};
		if (shot->hasCameraTrack &&
			!shot->strSequenceInstanceId.empty() &&
			m_SequencePlayer.Try_GetElapsedMs(
				shot->strSequenceInstanceId, cueElapsedMs) &&
			CValtanCinematicCameraController::Sample_Cue(
				shot->CameraTrack, cueElapsedMs / 1000.f, cuePose))
		{
			/* The cue owns the framing for as long as the cutscene runs. The
			   authored single pose stays as the fallback so a rejected sample
			   never leaves the camera holding a stale frame. */
			m_vCameraEyeTo = cuePose.vEye;
			m_vCameraLookTo = cuePose.vLookAt;
			m_fCameraFovTo = cuePose.fFovYDegrees;
		}
	}
	else
	{
		if (!m_bCameraShotHeld)
			return;
		if (nullptr == transform)
		{
			Release_CameraShot();
			return;
		}
		m_vCameraEyeTo = followEye;
		m_vCameraLookTo = followLook;
		m_fCameraFovTo = m_EffectiveFollowCameraProfile.fovYDegrees;
	}

	if (!m_bCameraShotHeld)
	{
		if (!m_pCamera->Begin_PresentationOverride(
			KAKULSAYDON_CAMERA_SHOT_OWNER_ID))
		{
			/* A cinematic outranks an authored shot; try again once it ends. */
			m_strActiveCameraShotId.clear();
			return;
		}
		m_bCameraShotHeld = true;
	}

	f32_t ratio = 1.f;
	if (m_fCameraBlendSeconds > 0.f)
	{
		m_fCameraBlendElapsed = (std::min)(
			m_fCameraBlendSeconds,
			m_fCameraBlendElapsed + (std::max)(0.f, fTimeDelta));
		const f32_t linear = m_fCameraBlendElapsed / m_fCameraBlendSeconds;
		ratio = linear * linear * (3.f - 2.f * linear);
	}
	const bool_t isBlendFinished = m_fCameraBlendSeconds <= 0.f ||
		m_fCameraBlendElapsed >= m_fCameraBlendSeconds;
	const float3_t eye = Lerp_Float3(m_vCameraEyeFrom, m_vCameraEyeTo, ratio);
	const float3_t lookAt = Lerp_Float3(m_vCameraLookFrom, m_vCameraLookTo, ratio);
	const f32_t fov = m_fCameraFovFrom +
		(m_fCameraFovTo - m_fCameraFovFrom) * ratio;
	if (!m_pCamera->Apply_PresentationPose(
		KAKULSAYDON_CAMERA_SHOT_OWNER_ID, eye, lookAt, fov))
	{
		/* Ownership was taken or the pose was rejected: fall back rather than
		   hold a stale frame. */
		m_bCameraShotHeld = false;
		Release_CameraShot();
		return;
	}
	m_vCameraEyeApplied = eye;
	m_vCameraLookApplied = lookAt;
	m_fCameraFovApplied = fov;

	/* The hand-back finishes only once the blend has fully played. */
	if (nullptr == shot && isBlendFinished)
		Release_CameraShot();
}

#ifdef _DEBUG
void Client::CLevel_KakulSaydonArena::Debug_InvalidateCompositionMapLights()
{
	if (!m_pCompositionMapLightPreview) return;
	const auto owner = m_strCompositionWorldPreviewPattern;
	Debug_StopCompositionWorldPreview();
	// MainApp consumes the failure before sampling effects or starting the combat handoff.
	m_strCompositionWorldPreviewFailurePattern = owner;
	m_strCompositionWorldPreviewFailure = "Popup preview stopped because its Area light source changed. Play again to use the new source.";
}
#endif

void Client::CLevel_KakulSaydonArena::Set_MapLightAuthoringOverride(
	std::shared_ptr<CMapLightPresentationRuntime> lights)
{
#ifdef _DEBUG
	if (m_pCompositionMapLightPreview && (m_pMapLightAuthoringOverride != lights ||
		(lights && (!m_CompositionMapLightSource || !Same_MapLightSource(lights->Get_Document(), *m_CompositionMapLightSource)))))
		Debug_InvalidateCompositionMapLights();
#endif
	m_pMapLightAuthoringOverride = std::move(lights);
}

uint64_t Client::CLevel_KakulSaydonArena::Get_MapLightComparisonFingerprint() const
{
	// Hash named effective inputs without allocating/serializing the complete JSON during capture.
	uint64_t hash = 14695981039346656037ull;
	const auto add = [&hash](const auto& value)
	{ hash ^= static_cast<uint64_t>(std::hash<std::decay_t<decltype(value)>>{}(value)); hash *= 1099511628211ull; };
	const bool_t marioLighting = Is_LocalMarioLightingActive();
	add(static_cast<uint32_t>(m_eMapLightComparison)); add(m_iGateLightingIndex); add(marioLighting);
	auto lights = m_pMapLightAuthoringOverride ? m_pMapLightAuthoringOverride : m_pMapLightPresentation;
	if (!marioLighting && m_pGateMapLightPresentation) lights = m_pGateMapLightPresentation;
#ifdef _DEBUG
	add(m_bCompositionMapLightPreviewActive);
	if (m_bCompositionMapLightPreviewActive && m_pCompositionMapLightPreview) lights = m_pCompositionMapLightPreview;
#endif
	if (m_eMapLightComparison == MAP_LIGHT_COMPARISON::SOURCE_IMPORT)
	{
		lights = !marioLighting && m_pMapLightComparisonGate ? m_pMapLightComparisonGate : m_pMapLightComparisonSource;
#ifdef _DEBUG
		if (m_bCompositionMapLightPreviewActive && m_pMapLightComparisonPopup) lights = m_pMapLightComparisonPopup;
#endif
	}
	if (m_eMapLightComparison == MAP_LIGHT_COMPARISON::DISABLED) lights.reset();
	add(bool(lights));
	if (!lights) return hash;
	const auto& document = lights->Get_Document();
	add(document.Get_AreaId()); add(document.Get_Lights().size());
	for (const auto& light : document.Get_Lights())
	{
		add(light.lightId); add(light.enabled); add(static_cast<uint32_t>(light.kind));
		add(static_cast<uint32_t>(light.receiver)); add(light.staticShadowChannel);
		add(light.position.x); add(light.position.y); add(light.position.z);
		add(light.rotationDegrees.x); add(light.rotationDegrees.y); add(light.rotationDegrees.z);
		add(light.radiusMeters); add(light.falloffExponent); add(light.innerConeDegrees); add(light.outerConeDegrees);
		add(light.color.x); add(light.color.y); add(light.color.z); add(light.color.w); add(light.brightness);
	}
	return hash;
}

void Client::CLevel_KakulSaydonArena::Reset_MapLightComparison()
{
	m_eMapLightComparison = MAP_LIGHT_COMPARISON::CURRENT;
	m_pMapLightComparisonSource.reset();
	m_pMapLightComparisonGate.reset();
#ifdef _DEBUG
	m_pMapLightComparisonPopup.reset();
#endif
}

bool_t Client::CLevel_KakulSaydonArena::Set_MapLightComparison(
	const MAP_LIGHT_COMPARISON mode, std::string& outStatus)
{
	if (mode == MAP_LIGHT_COMPARISON::CURRENT || mode == MAP_LIGHT_COMPARISON::DISABLED)
	{
		Reset_MapLightComparison();
		m_eMapLightComparison = mode;
		outStatus = mode == MAP_LIGHT_COMPARISON::CURRENT ?
			"Current map lights restored." : "Area map lights disabled for this comparison.";
		return true;
	}
	if (mode != MAP_LIGHT_COMPARISON::SOURCE_IMPORT)
	{ outStatus = "Unknown map light comparison mode."; return false; }
	auto source = std::make_shared<CMapLightPresentationRuntime>();
	if (!source->Load(CProjectDataRoot::Resolve(
		L"Rendering/Reference/KoukuImportedSourceLights.maplights.json"), std::string(KAKULSAYDON_AREA_ID)))
	{ outStatus = source->Get_Status(); return false; }
	std::shared_ptr<CMapLightPresentationRuntime> gate;
	if (!Prepare_GateMapLights(source->Get_Document(), m_iGateLightingIndex, gate, outStatus, true))
		return false;
#ifdef _DEBUG
	std::shared_ptr<CMapLightPresentationRuntime> popup;
	if (!Prepare_PopupMapLights(source->Get_Document(), popup, outStatus, true))
		return false;
	m_pMapLightComparisonPopup = std::move(popup);
#endif
	m_pMapLightComparisonSource = std::move(source);
	m_pMapLightComparisonGate = std::move(gate);
	m_iMapLightComparisonGate = m_iGateLightingIndex;
	m_eMapLightComparison = mode;
	outStatus = "Comparing the 115 imported local lights from 2026-09-11.";
	return true;
}

void Client::CLevel_KakulSaydonArena::Submit_MapLightFrame()
{
	const bool_t marioLighting = Is_LocalMarioLightingActive();
	auto lights = m_pMapLightAuthoringOverride ? m_pMapLightAuthoringOverride : m_pMapLightPresentation;
    if (lights && (m_iGateLightingIndex == 0u || m_iGateLightingIndex == 2u) &&
        (!m_GateMapLightSource || !Same_MapLightSource(lights->Get_Document(), *m_GateMapLightSource)))
    {
        std::shared_ptr<CMapLightPresentationRuntime> staged;
        std::string status;
        if (Prepare_GateMapLights(lights->Get_Document(), m_iGateLightingIndex, staged, status))
        {
            m_GateMapLightSource = lights->Get_Document();
            m_pGateMapLightPresentation = std::move(staged);
        }
        else if (m_strDebugGateStatus != status)
        { m_strDebugGateStatus = status; OutputDebugStringA(("[GateLighting] " + status + "\n").c_str()); }
    }
    // Gate 3 suppresses Area source lights for its arena, but Mario occupies a separate stage.
    // Keep that gate cache staged so the local Server exit immediately restores it.
    if (!marioLighting && m_pGateMapLightPresentation) lights = m_pGateMapLightPresentation;
#ifdef _DEBUG
	if (m_bCompositionMapLightPreviewActive && m_pCompositionMapLightPreview)
		lights = m_pCompositionMapLightPreview;
#endif
	if (m_eMapLightComparison == MAP_LIGHT_COMPARISON::SOURCE_IMPORT && m_pMapLightComparisonSource)
	{
		if (m_iMapLightComparisonGate != m_iGateLightingIndex)
		{
			std::shared_ptr<CMapLightPresentationRuntime> gate;
			std::string status;
			if (Prepare_GateMapLights(m_pMapLightComparisonSource->Get_Document(),
				m_iGateLightingIndex, gate, status, true))
			{
				m_pMapLightComparisonGate = std::move(gate);
				m_iMapLightComparisonGate = m_iGateLightingIndex;
			}
			else
			{
				OutputDebugStringA(("[LightComparison] " + status + "\n").c_str());
				Reset_MapLightComparison();
			}
		}
		if (m_pMapLightComparisonSource)
			lights = !marioLighting && m_pMapLightComparisonGate ? m_pMapLightComparisonGate : m_pMapLightComparisonSource;
#ifdef _DEBUG
		// Composition popup lighting has its own placement, independent of the active gate.
		if (m_bCompositionMapLightPreviewActive && m_pMapLightComparisonPopup)
			lights = m_pMapLightComparisonPopup;
#endif
	}
	// Current gate and draft inputs keep staging while the session comparison is active.
	if (m_eMapLightComparison != MAP_LIGHT_COMPARISON::DISABLED && lights && !lights->Submit_Frame())
		OutputDebugStringA((lights->Get_Status() + "\n").c_str());
}

bool_t Client::CLevel_KakulSaydonArena::Reload_MapLights()
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.MapLights.Load");
	auto staged=std::make_shared<CMapLightPresentationRuntime>();
	if(!staged->Load_Runtime(std::string(KAKULSAYDON_AREA_ID)))
	{OutputDebugStringA(("[Level_KakulSaydonArena] "+staged->Get_Status()+"\n").c_str());return false;}
	OutputDebugStringA((staged->Get_Status()+"\n").c_str());
#ifdef _DEBUG
	if (!m_pMapLightAuthoringOverride) Debug_InvalidateCompositionMapLights();
#endif
	m_pMapLightPresentation=std::move(staged);return true;
}

#ifdef _DEBUG
bool_t Client::CLevel_KakulSaydonArena::Ready_DebugStageEntryTriggers(
	const std::string& areaId)
{
	CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.DebugTriggers.Load");
	/* Arena-side entrances only. Every stage also carries its own trigger
	   boxes a kilometre away, and drawing those here would say nothing
	   about where a player is supposed to stand when the arena opens. */
	static constexpr std::string_view STAGE_ENTRY_SUFFIX = "_go";

	const std::filesystem::path documentPath = CProjectDataRoot::Resolve(
		std::filesystem::path("Worlds") /
		areaId /
		"Gameplay.world.json");
	std::error_code pathError;
	if (documentPath.empty() ||
		!std::filesystem::is_regular_file(documentPath, pathError) ||
		pathError)
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] Debug gameplay document is "
			"unavailable: " + documentPath.string() + "\n").c_str());
		return false;
	}

	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(documentPath, areaId, status))
	{
		OutputDebugStringA((
			"[Level_KakulSaydonArena] Debug gameplay document rejected: " +
			status + "\n").c_str());
		return false;
	}

	std::vector<shared_ptr<CTrigger_Box>> staged;
	const auto rollback = [&staged]()
	{
		for (const shared_ptr<CTrigger_Box>& triggerBox : staged)
		{
			if (nullptr == triggerBox)
				continue;
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::KAKULSAYDON_ARENA),
				TEXT("Layer_DebugWorldGameplay"),
				static_pointer_cast<CGameObject>(triggerBox));
		}
		staged.clear();
	};

	for (const WORLD_GAMEPLAY_PLACEMENT& placement :
		document.Get_Placements())
	{
		const bool_t isStageEntryTrigger =
			placement.isEnabled &&
			WORLD_PLACEMENT_KIND::TRIGGER_BOX == placement.eKind &&
			1u == placement.triggerEvents.size() &&
			WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER ==
				placement.triggerEvents.front().eKind &&
			placement.placementId.ends_with(STAGE_ENTRY_SUFFIX);
		if (!isStageEntryTrigger)
			continue;

		CTrigger_Box::TRIGGER_BOX_DESC desc{};
		desc.placementId = placement.placementId;
		desc.position = placement.position;
		desc.halfExtents = placement.halfExtents;
		desc.yawDegrees = placement.yawDegrees;
		desc.isEnabled = true;

		shared_ptr<CGameObject> gameObject;
		if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			TEXT("Prototype_GameObject_TriggerBox"),
			ETOUI(LEVEL::KAKULSAYDON_ARENA),
			TEXT("Layer_DebugWorldGameplay"),
			&desc,
			&gameObject)))
		{
			rollback();
			OutputDebugStringA((
				"[Level_KakulSaydonArena] Debug Trigger Box clone failed: " +
				placement.placementId + "\n").c_str());
			return false;
		}

		shared_ptr<CTrigger_Box> triggerBox =
			dynamic_pointer_cast<CTrigger_Box>(gameObject);
		if (nullptr == triggerBox)
		{
			CGameInstance::Get().Remove_GameObject_from_Layer(
				ETOUI(LEVEL::KAKULSAYDON_ARENA),
				TEXT("Layer_DebugWorldGameplay"),
				gameObject);
			rollback();
			OutputDebugStringA((
				"[Level_KakulSaydonArena] Debug Trigger Box type mismatch: " +
				placement.placementId + "\n").c_str());
			return false;
		}

		triggerBox->Set_AuthoringVisible(true);
		staged.push_back(std::move(triggerBox));
	}

	m_DebugStageEntryTriggers = std::move(staged);
	OutputDebugStringA((
		"[Level_KakulSaydonArena] Debug stage entry Trigger Boxes ready: " +
		std::to_string(m_DebugStageEntryTriggers.size()) + "\n").c_str());
	return true;
}
#endif

unique_ptr<Client::CLevel_KakulSaydonArena>
Client::CLevel_KakulSaydonArena::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_KakulSaydonArena>(
		new CLevel_KakulSaydonArena(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}


void Client::CLevel_KakulSaydonArena::Consume_OwnedWorldCue(
    const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play,
    const CWorldSequencePlayer::TARGET_SET& targets)
{
    using LostArk::Shared::WORLD_SEQUENCE_OPERATION;
    if (play.iRunEpoch < m_iLatestWorldRunEpoch) return;
    if (play.iRunEpoch > m_iLatestWorldRunEpoch)
    {
        for (auto& [id, cue] : m_OwnedWorldCues) cue.player->Stop_All(targets, true);
        m_OwnedWorldCues.clear();
        m_PendingOwnedWorldCues.clear();
        m_StoppedWorldOwners.clear();
        m_FinishedWorldOwners.clear();
        m_ConsumedWorldCueIds.clear();
        m_iLatestWorldRunEpoch = play.iRunEpoch;
    }
    const auto owner = std::to_string(play.iRunEpoch) + ":" + play.strMemberId;
    const auto runOwner = std::to_string(play.iRunEpoch) + ":";
    // A Mario parent and its phase-2 follow-up share the run/member identity.
    // A scoped stop retires only the old pattern; zero retains whole-owner semantics.
    const auto patternOwner = owner + ":pattern:" + std::to_string(play.iPatternSequence);
    if (play.eOperation == WORLD_SEQUENCE_OPERATION::STOP_OWNER ||
        play.eOperation == WORLD_SEQUENCE_OPERATION::FINISH_OWNER)
    {
        (play.eOperation == WORLD_SEQUENCE_OPERATION::STOP_OWNER ? m_StoppedWorldOwners : m_FinishedWorldOwners)
            .insert(play.iPatternSequence ? patternOwner : owner);
        for (auto cue = m_OwnedWorldCues.begin(); cue != m_OwnedWorldCues.end();)
            if (cue->second.runEpoch == play.iRunEpoch &&
                (play.strMemberId.empty() || cue->second.memberId == play.strMemberId) &&
                (!play.iPatternSequence || cue->second.patternSequence == play.iPatternSequence))
            {
                // Natural Pattern completion releases boss progression only.
                // Every emitted World row retains its own authored end clock.
                if (play.eOperation == WORLD_SEQUENCE_OPERATION::FINISH_OWNER)
                { ++cue; continue; }
                cue->second.player->Stop_All(targets, true); cue = m_OwnedWorldCues.erase(cue);
            }
            else ++cue;
        return;
    }
    const std::string key = owner + ":" + play.strCueId;
    if (play.eOperation == WORLD_SEQUENCE_OPERATION::STOP_CUE)
    {
        // Death is idempotent and wins over a delayed reliable PLAY for this exact cue.
        m_ConsumedWorldCueIds.insert(key);
        std::erase_if(m_PendingOwnedWorldCues, [&](const auto& cue) {
            return cue.iRunEpoch == play.iRunEpoch && cue.strMemberId == play.strMemberId && cue.strCueId == play.strCueId; });
        if (const auto cue = m_OwnedWorldCues.find(key); cue != m_OwnedWorldCues.end())
        { cue->second.player->Stop_All(targets, true); m_OwnedWorldCues.erase(cue); }
        return;
    }
    // A reliable PLAY may follow a completion receipt while its row is still
    // live. Explicit STOP is terminal; FINISH keeps independent row clocks.
    if (m_StoppedWorldOwners.contains(owner) || m_StoppedWorldOwners.contains(runOwner) ||
        m_StoppedWorldOwners.contains(patternOwner)) return;
    if (m_ConsumedWorldCueIds.contains(key)) return; // reliable resend is idempotent
    if (m_ConsumedWorldCueIds.size() >= 65536u) { OutputDebugStringA("[KoukuWORLD] Run cue capacity exceeded.\n"); return; }
    const auto defer = [&]()
    {
        const bool pending = std::any_of(m_PendingOwnedWorldCues.begin(), m_PendingOwnedWorldCues.end(),
            [&](const auto& value) { return value.iRunEpoch == play.iRunEpoch &&
                value.strMemberId == play.strMemberId && value.strCueId == play.strCueId; });
        if (!pending && m_PendingOwnedWorldCues.size() < 1024u) m_PendingOwnedWorldCues.push_back(play);
        else if (!pending) OutputDebugStringA("[KoukuWORLD] Pending presentation cue capacity exceeded.\n");
    };
    float seconds = 0.f;
    const auto tick = (std::max)(play.iServerTick, m_Replication.Get_LastServerTick());
    if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(tick, play.iStartTick, 30.f, seconds)) return;
    const float ageMs = seconds * 1000.f;
    if (!play.strTargetSequenceInstanceId.empty() && play.iDurationMs && ageMs >= play.iDurationMs) return;
    if (!play.bUntilDestroyed && play.strTargetSequenceInstanceId.empty() && ageMs >= CompositionWorldSpan(m_SequencePlayer,
        play.strSequenceInstanceId, play.fPlaybackSpeed, play.iDurationMs)) return;
    std::string effectStatus;
    const auto preparation = PrepareCompositionWorldEffects(m_SequencePlayer.Get_Document(),
        play.strSequenceInstanceId, effectStatus);
    if (preparation == WORLD_EFFECT_PREPARATION::PENDING) { defer(); return; }
    if (preparation == WORLD_EFFECT_PREPARATION::FAILED)
    {
        m_ConsumedWorldCueIds.insert(key);
        Write_EffectFailureDiagnostic("Kouku.world.prepare", key + ": " + effectStatus);
        OutputDebugStringA(("[KoukuWORLD] " + effectStatus + "\n").c_str());
        return;
    }
    if (!play.strTargetSequenceInstanceId.empty())
    {
        OWNED_WORLD_CUE* target = nullptr;
        for (auto& [id, cue] : m_OwnedWorldCues)
            if (cue.runEpoch == play.iRunEpoch && cue.memberId == play.strMemberId &&
                cue.sequenceId == play.strTargetSequenceInstanceId &&
                (play.strTargetCueId.empty() || cue.cueId == play.strTargetCueId))
            {
                if (target) { OutputDebugStringA("[KoukuWORLD] Ambiguous owned motion target.\n"); return; }
                target = &cue;
            }
        if (!target)
        {
            // Its reliable birth can still be waiting for asynchronous Effect preparation.
            if (std::any_of(m_PendingOwnedWorldCues.begin(), m_PendingOwnedWorldCues.end(), [&](const auto& pending) {
                return pending.iRunEpoch == play.iRunEpoch && pending.strMemberId == play.strMemberId &&
                    pending.strSequenceInstanceId == play.strTargetSequenceInstanceId &&
                    (play.strTargetCueId.empty() || pending.strCueId == play.strTargetCueId); })) defer();
            else OutputDebugStringA("[KoukuWORLD] Owned motion target is unavailable.\n");
            return;
        }
        float motionSeconds = 0.f;
        if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(play.iStartTick, target->startTick, 30.f, motionSeconds)) return;
        const float present = (std::max)(target->clockMs, motionSeconds * 1000.f + ageMs);
        auto motionTargets = targets;
        motionTargets.objectEmissionAnchor = target->emissionAnchor;
        if (!target->player->Seek_InstanceToMs(target->sequenceId, motionSeconds * 1000.f, motionTargets) ||
            !target->player->Apply_ObjectMotion(target->sequenceId, play.strSequenceInstanceId, motionTargets))
            OutputDebugStringA(("[KoukuWORLD] " + target->player->Get_Status() + "\n").c_str());
        m_ConsumedWorldCueIds.insert(key);
        target->clockMs = present;
        (void)target->player->Seek_InstanceToMs(target->sequenceId, present, motionTargets);
        return;
    }
    for (auto old = m_OwnedWorldCues.begin(); old != m_OwnedWorldCues.end();)
    {
        float elapsed = 0.f;
        if (old->second.durationMs && CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
            tick, old->second.startTick, 30.f, elapsed) && elapsed * 1000.f >= old->second.player->Get_LongestElapsedSpanMs())
        { old->second.player->Stop_All(targets, true); old = m_OwnedWorldCues.erase(old); }
        else ++old;
    }
    std::string status;
    if (!Can_StartCompositionWorld(play.strSequenceInstanceId, status))
    { OutputDebugStringA(("[KoukuWORLD] " + status + "\n").c_str()); return; }
    auto cueTargets = targets;
    const auto& run = m_Replication.Get_KoukuBundleState();
    const auto member = std::find_if(run.Members.begin(), run.Members.end(),
        [&](const auto& value) { return value.strMemberId == play.strMemberId; });
    if (!play.bUntilDestroyed && (run.iRunEpoch != play.iRunEpoch || !m_WorldEmissionResolver ||
        !m_WorldEmissionResolver(run.iPinnedSourceRevision,
            member == run.Members.end() ? std::string{} : member->strPatternId,
            play.strOccurrenceId, cueTargets.objectEmissionAnchor)))
    { defer(); return; }
    BindCompositionGroupOrigin(m_SequencePlayer.Get_Document(), play.strSequenceInstanceId, cueTargets);
    auto player = std::make_shared<CWorldSequencePlayer>();
    player->Set_SoundAudience(Is_SequenceSoundAudience);
    const auto placement = WorldPlacementFromCue(play);
    if (!player->Set_PlaybackSubset(m_SequencePlayer, play.strSequenceInstanceId, targets, status) ||
        !PrepareCompositionWorld(*player, play.strSequenceInstanceId, targets, placement, status) ||
        !PlayCompositionWorld(*player, play.strSequenceInstanceId, cueTargets, play.fPlaybackSpeed,
            float3_t(play.fPositionOffsetX, play.fPositionOffsetY, play.fPositionOffsetZ), play.iDurationMs, placement) ||
        !player->Seek_AllToMs(ageMs, cueTargets))
    {
        const auto failure = status.empty() ? player->Get_Status() : status;
        player->Stop_All(targets, true);
        Write_EffectFailureDiagnostic("Kouku.world.play", key + ": " + play.strSequenceInstanceId + ": " + failure);
        OutputDebugStringA(("[KoukuWORLD] " + failure + "\n").c_str());
        return;
    }
    OWNED_WORLD_CUE cue;
    cue.runEpoch = play.iRunEpoch; cue.patternSequence = play.iPatternSequence;
    cue.startTick = play.iStartTick; cue.durationMs = play.iDurationMs;
    cue.untilDestroyed = play.bUntilDestroyed;
    cue.combatBodyNetEntityId = play.iCombatBodyNetEntityId;
    cue.memberId = play.strMemberId; cue.cueId = play.strCueId; cue.occurrenceId = play.strOccurrenceId; cue.sequenceId = play.strSequenceInstanceId;
    cue.emissionAnchor = std::move(cueTargets.objectEmissionAnchor);
    cue.clockMs = ageMs; cue.player = std::move(player);
    m_OwnedWorldCues.emplace(key, std::move(cue));
    m_ConsumedWorldCueIds.insert(key);
}

bool_t Client::CLevel_KakulSaydonArena::Try_GetOwnedCompositionWorldPivot(
    std::uint32_t runEpoch, const std::string& memberId, const std::string& sequenceId,
    const std::string& cueId, float4x4_t& out, const std::uint32_t emissionIndex,
    const std::uint32_t patternSequence, const std::string& bone, const bool_t boneRotation,
    const std::string& effectTrackId) const
{
    const OWNED_WORLD_CUE* found = nullptr;
    for (const auto& [id, cue] : m_OwnedWorldCues)
        if (cue.runEpoch == runEpoch && cue.memberId == memberId && cue.sequenceId == sequenceId &&
            (!patternSequence || cue.patternSequence == patternSequence) &&
            (cueId.empty() || cue.occurrenceId == cueId))
        {
            if (found) return false;
            found = &cue;
        }
    return found && CompositionWorldPivot(*found->player, sequenceId, out, emissionIndex, bone, boneRotation, effectTrackId);
}


bool_t Client::CLevel_KakulSaydonArena::Can_StartCompositionWorld(
    const std::string& instanceId, std::string& status, const CWorldSequenceDocument* sourceDocument) const
{
    const auto& document = sourceDocument ? *sourceDocument : m_SequencePlayer.Get_Document();
    const auto members = CompositionWorldMotions(document, instanceId);
    if (members.empty()) { status = "WORLD group has no enabled motions: " + instanceId; return false; }
    std::set<std::pair<WORLD_SEQUENCE_TARGET_KIND, std::string>> sharedTargets;
    for (const auto& member : members)
    {
        const auto* source = document.Find_Instance(member);
        if (!source || !source->enabled) { status = "WORLD instance is unavailable: " + member; return false; }
        for (const auto& binding : source->bindings)
            if (binding.targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE)
                sharedTargets.emplace(binding.targetKind, binding.targetId);
    }
    if (sharedTargets.empty()) return true;
    const auto conflicts = [&](const CWorldSequencePlayer& player)
    {
        for (const auto& active : player.Get_Document().Get_Instances())
            if (player.Is_Playing(active.instanceId))
                for (const auto& binding : active.bindings)
                    if (sharedTargets.contains({binding.targetKind, binding.targetId})) return true;
        return false;
    };
    if (conflicts(m_SequencePlayer))
    { status = "WORLD map/deploy target is already owned by an active level cue: " + instanceId; return false; }
    for (const auto& [id, cue] : m_OwnedWorldCues)
        if (conflicts(*cue.player))
        { status = "WORLD map/deploy target is already owned by another run/member cue: " + instanceId; return false; }
    return true;
}


bool Client::CLevel_KakulSaydonArena::Prepare_EntryRaidResources(std::string& status)
{
    const auto started = GetTickCount64();
    CProfilerScope scope(CGameInstance::Get().Get_Profiler(), "Level.Kouku.RaidResources.Prewarm");
    CKoukuSaydonBossTool published;
    if (!published.Reload(status)) return false;
    const auto& patterns = published.Get_PlayAllPatternIds();
    const auto sourceRevision = published.Get_SourceRevision();
    bool ready = false;
    // Loader already admitted this WORLD document. Preserve its models and pools;
    // the same selection will be consumed by the Server PREPARING acknowledgement.
    if (!Prepare_CompletePlayResources(patterns, {}, sourceRevision, ready, status, true, {}, false)) return false;
    const auto& resources = m_CompletePlayPreparation->resources;
    const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(resources.V1EffectIds);
    if (!resources.V1EffectIds.empty() && (!probe.bCatalogRevisionCurrent || !probe.bSettled ||
        probe.iFailedCount || probe.iUnavailableCount || !probe.strBlockingFailure.empty() ||
        probe.iPreparedCount != resources.V1EffectIds.size()))
    {
        status = "Release raid resources were not settled by the loading barrier: " + probe.strBlockingFailure;
        return false;
    }
    // Each call advances one actor, V2 or WORLD item, then at most one binding
    // archetype. Never wait for asynchronous work while Level activation owns the thread.
    const size_t maximumSteps = resources.BossArchetypeIds.size() + resources.V2Effects.size() +
        resources.WorldInstanceIds.size() + m_CompletePlayPreparation->worldCloneCount +
        m_CompletePlayPreparation->worldSubsetIds.size() + CActorCatalog::Get_Bosses().size() + 2u;
    for (size_t step = 0u; step < maximumSteps && !ready; ++step)
        if (!Prepare_CompletePlayResources(patterns, {}, sourceRevision, ready, status, true, {}, false)) return false;
    if (!ready)
    {
        status = "Release raid preparation did not finish within its dependency bound: " + status;
        return false;
    }
    if (!Prepare_ServerRaidGatePresentation("GATE1", status)) return false;
    Write_EffectFailureDiagnostic("Kouku.Loading.RaidPrepared",
        "elapsed_ms=" + std::to_string(GetTickCount64() - started) +
        " source_revision=" + std::to_string(sourceRevision) +
        " world_instances=" + std::to_string(resources.WorldInstanceIds.size()));
    return true;
}

bool Client::CLevel_KakulSaydonArena::Debug_PrepareCompletePlayResources(
    const std::vector<std::string>& patternIds, const std::vector<std::string>& bundleIds,
    const uint32_t sourceRevision, bool& ready, std::string& status, const bool wholeRaid,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft)
{
    return Prepare_CompletePlayResources(patternIds, bundleIds, sourceRevision, ready, status,
        wholeRaid, std::move(draft), true);
}

bool Client::CLevel_KakulSaydonArena::Prepare_CompletePlayResources(
    const std::vector<std::string>& patternIds, const std::vector<std::string>& bundleIds,
    const uint32_t sourceRevision, bool& ready, std::string& status, const bool wholeRaid,
    std::shared_ptr<const KOUKU_SAYDON_DRAFT_PRODUCT> draft, const bool reloadWorld)
{
    ready = false;
    const auto& document = m_SequencePlayer.Get_Document();
    const auto v1Revision = CEffectCatalog::Get_RuntimeRevision();
    const auto v2Generation = CEffectV2Runtime::Cache_Generation();
    if (!m_CompletePlayPreparation || m_CompletePlayPreparation->selectedPatterns != patternIds ||
        m_CompletePlayPreparation->selectedBundles != bundleIds || m_CompletePlayPreparation->sourceRevision != sourceRevision ||
        m_CompletePlayPreparation->wholeRaid != wholeRaid || m_CompletePlayPreparation->draft != draft)
    {
        // A publish can finish after arena entry. Refresh the idle runtime base once
        // per new request; active cues and editor drafts retain their own snapshots.
        if (m_SequencePlayer.Has_ActiveInstances())
        {
            // A late participant can consume an entry WORLD cue immediately before
            // the shared PREPARING state. Busy is pending, not PREPARE_FAILED:
            // the ordinary level update keeps advancing that Server-owned cue.
            // Do not read files or replace its document/model pools while it plays.
            status = "Waiting for the current WORLD sequence to finish before preparing Complete Play";
            size_t shown = 0u;
            for (const auto& instance : document.Get_Instances())
            {
                if (!m_SequencePlayer.Is_Playing(instance.instanceId)) continue;
                if (shown == 4u) { status += ", ..."; break; }
                status += (shown++ == 0u ? ": " : ", ") + instance.instanceId;
            }
            status += ". Preparation will continue automatically.";
            return true;
        }
        if (reloadWorld && !Reload_WorldObjectRuntime(status)) return false;
        COMPLETE_PLAY_PREPARATION staged;
        staged.selectedPatterns = patternIds; staged.selectedBundles = bundleIds; staged.sourceRevision = sourceRevision;
        if (!CKoukuSaydonPresentationAssetService::Collect_CompletePlayResources(patternIds, bundleIds,
            sourceRevision, staged.resources, status, draft)) return false;
        staged.draft = draft;
        staged.wholeRaid = wholeRaid;
        if (wholeRaid)
        {
            // Complete raid playback can cross later gates. Reuse the exact Release
            // effect closure, including original Sequence lanes and enabled WORLDs.
            if (!CKoukuSaydonPresentationPlayer::Collect_ProductEffectTargets(staged.resources.V1EffectIds,
                staged.resources.V2Effects, status)) return false;
            for (const auto& instance : document.Get_Instances())
                if (instance.enabled) staged.resources.WorldInstanceIds.push_back(instance.instanceId);
        }
        std::set<std::string> v1(staged.resources.V1EffectIds.begin(), staged.resources.V1EffectIds.end());
        std::set<std::pair<std::string, std::string>> v2(staged.resources.V2Effects.begin(), staged.resources.V2Effects.end());
        std::set<std::string> motions, queued(staged.resources.WorldInstanceIds.begin(), staged.resources.WorldInstanceIds.end());
        // Group children and NEXT states may start after the selected stage ends.
        // They still belong to the selected run's initial preparation barrier.
        while (!queued.empty())
        {
            const auto id = *queued.begin(); queued.erase(queued.begin());
            if (motions.contains(id)) continue;
            if (const auto* group = document.Find_ObjectResource(id); group && !group->motionInstanceIds.empty())
            {
                const auto members = CompositionWorldMotions(document, id);
                if (members.empty()) { status = "Complete Play WORLD group has no admitted motions: " + id; return false; }
                queued.insert(members.begin(), members.end());
                continue;
            }
            const auto* instance = document.Find_Instance(id);
            const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
            if (!instance || !instance->enabled || !sequence)
            { status = "Complete Play WORLD is absent, disabled or has no template: " + id; return false; }
            if (motions.size() >= 16384u) { status = "Complete Play WORLD closure exceeds its bounded capacity."; return false; }
            motions.insert(id);
            if (instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT) queued.insert(instance->nextMotionId);
            for (const auto& effect : sequence->effectTracks)
                if (effect.resourceKind == "V1_EFFECT") v1.insert(effect.resourceId);
                else v2.emplace(effect.resourceKind, effect.resourceId);
        }
        for (const auto& id : staged.resources.BossArchetypeIds)
        {
            const auto* actor = CActorCatalog::Find_Boss(id);
            if (!actor) { status = "Complete Play boss catalog entry is unavailable: " + id; return false; }
            for (const auto& effect : actor->defaultParticles) v1.insert(effect.effectAssetId);
            for (const auto& effect : actor->combatObjectVisuals)
            {
                if (effect.activeEffectKind == BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1) v1.insert(effect.effectAssetId);
                else v2.emplace("GROUP", effect.effectV2Group.groupId);
                if (!effect.hitEffectAssetId.empty()) v1.insert(effect.hitEffectAssetId);
                if (!effect.armedEffectAssetId.empty()) v1.insert(effect.armedEffectAssetId);
            }
        }
        staged.resources.V1EffectIds.assign(v1.begin(), v1.end()); staged.resources.V2Effects.assign(v2.begin(), v2.end());
        staged.resources.WorldInstanceIds.assign(motions.begin(), motions.end());
        const auto isPreparedMechanic = [](const std::string& objectId) {
            return objectId == "world.object.kouku.odd_doll.large" ||
                objectId == "world.object.kouku.mario_circus_ball" ||
                objectId == "world.object.kouku.cutting_blade" || objectId == "world.object.kouku.hook";
        };
        std::map<std::string, COMPLETE_PLAY_PREPARATION::WORLD_CLONE_RESERVATION> reservations;
        std::set<std::string> subsetRoots;
        for (const auto& group : staged.resources.WorldSpawnGroups)
        {
            std::map<std::string, uint32_t> groupCounts;
            for (const auto& root : group)
                for (const auto& id : CompositionWorldMotions(document, root))
                {
                    const auto* instance = document.Find_Instance(id);
                    if (!instance || !instance->enabled || instance->bindings.size() != 1u ||
                        instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
                    const auto& objectId = instance->bindings.front().targetId;
                    if (!isPreparedMechanic(objectId)) continue;
                    const auto* sequence = document.Find_Template(instance->templateId);
                    if (!sequence || instance->anchorKind != "WORLD")
                    { status = "Complete Play WORLD clone reservation has an invalid spawn: " + id; return false; }
                    const uint32_t copies = sequence->objectMotion.EmissionCount();
                    auto& count = groupCounts[objectId];
                    if (!copies || copies > 128u || count > 128u - copies)
                    { status = "Complete Play WORLD clone reservation exceeds 128 for " + objectId; return false; }
                    count += copies;
                    auto& reservation = reservations[objectId];
                    if (reservation.instanceId.empty()) reservation.instanceId = id;
                    reservation.copies = (std::max)(reservation.copies, count);
                    subsetRoots.insert(root);
                }
        }
        // Preserve state-driven minimums and reserve at least one full emitter
        // set for enabled mechanic resources reached outside an explicit spawn.
        for (const auto& id : staged.resources.WorldInstanceIds)
        {
            const auto* instance = document.Find_Instance(id);
            if (!instance || instance->bindings.size() != 1u ||
                instance->bindings.front().targetKind != WORLD_SEQUENCE_TARGET_KIND::OBJECT_RESOURCE) continue;
            const auto& objectId = instance->bindings.front().targetId;
            uint32_t copies = id == "world.object.instance.kouku.card" ? 6u :
                id == "world.object.instance.kouku.joker_card" ? 1u :
                (objectId == "world.object.kouku.mario_circus_ball" || objectId == "world.object.kouku.odd_doll.large") ? 4u : 0u;
            if (isPreparedMechanic(objectId))
            {
                const auto* sequence = document.Find_Template(instance->templateId);
                if (!sequence || instance->anchorKind != "WORLD")
                { status = "Complete Play WORLD clone reservation has an invalid motion: " + id; return false; }
                copies = (std::max)(copies, sequence->objectMotion.EmissionCount());
            }
            if (!copies) continue;
            if (copies > 128u)
            { status = "Complete Play WORLD clone reservation exceeds 128 for " + objectId; return false; }
            auto& reservation = reservations[objectId];
            if (reservation.instanceId.empty()) reservation.instanceId = id;
            reservation.copies = (std::max)(reservation.copies, copies);
        }
        for (auto& [objectId, reservation] : reservations)
        {
            if (staged.worldCloneCount + reservation.copies > 1024u)
            { status = "Complete Play WORLD clone reservations exceed the Level capacity of 1024."; return false; }
            staged.worldCloneCount += reservation.copies;
            staged.worldCloneReservations.push_back(std::move(reservation));
        }
        staged.worldSubsetIds.assign(subsetRoots.begin(), subsetRoots.end());
        std::vector<std::string> registered;
        if (!staged.resources.V1EffectIds.empty() && !CEffectPresentationService::Queue_ProductTargets_Priority(
            staged.resources.V1EffectIds, registered, status)) return false;
        staged.v1Revision = CEffectCatalog::Get_RuntimeRevision(); staged.v2Generation = v2Generation;
        staged.worldRevision = document.Get_Revision();
        m_CompletePlayPreparation = std::move(staged);
        status = "Complete Play is preparing the entire selected dependency closure; Server playback has not started.";
        return true;
    }
    auto& pending = *m_CompletePlayPreparation;
    if (pending.v1Revision != v1Revision || pending.v2Generation != v2Generation ||
        pending.worldRevision != document.Get_Revision())
    { status = "Complete Play resource catalog or WORLD revision changed during preparation. Start the updated selection explicitly."; return false; }
    const auto& resources = pending.resources;
    const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(resources.V1EffectIds);
    if (!resources.V1EffectIds.empty() && (!probe.strBlockingFailure.empty() || probe.iFailedCount || probe.iUnavailableCount))
    {
        status = "Complete Play Effect preparation failed. " + probe.strBlockingFailure;
        for (const auto& id : resources.V1EffectIds)
        {
            const auto reason = CEffectPresentationService::Get_ProductCuePreparationFailure(id);
            if (!reason.empty()) status += " " + id + ": " + reason;
        }
        if (probe.iUnavailableCount) status += " Unavailable targets=" + std::to_string(probe.iUnavailableCount);
        return false;
    }
    status = "Preparing Complete Play: V1 " + std::to_string(probe.iPreparedCount) + "/" + std::to_string(resources.V1EffectIds.size()) +
        ", V2 " + std::to_string(pending.v2Index) + "/" + std::to_string(resources.V2Effects.size()) +
        ", actors " + std::to_string(pending.actorIndex) + "/" + std::to_string(resources.BossArchetypeIds.size()) +
        ", WORLD " + std::to_string(pending.worldIndex) + "/" + std::to_string(resources.WorldInstanceIds.size()) +
        ", clone pools " + std::to_string(pending.worldCloneIndex) + "/" + std::to_string(pending.worldCloneReservations.size()) +
        ", spawn documents " + std::to_string(pending.worldSubsetIndex) + "/" + std::to_string(pending.worldSubsetIds.size()) + ". Server playback has not started.";
    // GPU/context owners stay on their existing main thread. At most one bounded
    // V2/model/WORLD item is prepared per update; V1 uses its existing worker queue.
    if (pending.actorIndex < resources.BossArchetypeIds.size())
    {
        const auto& id = resources.BossArchetypeIds[pending.actorIndex];
        if (FAILED(CKoukuSaydonPresentationAssetService::Ensure_Prototypes(m_pDevice, m_pContext,
            ETOUI(LEVEL::KAKULSAYDON_ARENA), id)))
        { status = "Complete Play boss preparation failed: " + id + "; " + CKoukuSaydonPresentationAssetService::Get_Status(); return false; }
        ++pending.actorIndex; return true;
    }
    if (pending.v2Index < resources.V2Effects.size())
    {
        std::string reason;
        if (!CKoukuSaydonPresentationPlayer::Prewarm_ProductEffectResources(m_pDevice, m_pContext,
            {resources.V2Effects[pending.v2Index]}, reason)) { status = std::move(reason); return false; }
        ++pending.v2Index; return true;
    }
    if (!resources.V1EffectIds.empty() && (!probe.bCatalogRevisionCurrent || !probe.bSettled ||
        probe.iPreparedCount != resources.V1EffectIds.size())) return true;
    if (pending.worldIndex < resources.WorldInstanceIds.size())
    {
        const auto& id = resources.WorldInstanceIds[pending.worldIndex];
        const auto targets = Make_WorldSequenceTargets();
        if (!m_SequencePlayer.Prepare_InstanceResources(id, targets))
        { status = "Complete Play WORLD preparation failed: " + id + "; " + m_SequencePlayer.Get_Status(); return false; }
        ++pending.worldIndex; return true;
    }
    if (pending.worldCloneIndex < pending.worldCloneReservations.size())
    {
        const auto& reservation = pending.worldCloneReservations[pending.worldCloneIndex];
        bool_t poolReady = false;
        if (!m_SequencePlayer.Prewarm_ObjectInstancesStep(reservation.instanceId, reservation.copies,
            Make_WorldSequenceTargets(), poolReady))
        { status = "Complete Play WORLD clone preparation failed: " + reservation.instanceId + "; " + m_SequencePlayer.Get_Status(); return false; }
        if (poolReady) ++pending.worldCloneIndex;
        return true;
    }
    if (pending.worldSubsetIndex < pending.worldSubsetIds.size())
    {
        const auto& id = pending.worldSubsetIds[pending.worldSubsetIndex];
        if (!m_SequencePlayer.Prepare_PlaybackSubset(id, Make_WorldSequenceTargets(), status)) return false;
        ++pending.worldSubsetIndex; return true;
    }
    bool bindingsReady = false;
    if (!CKoukuSaydonPresentationAssetService::Prepare_ProductBindings(
        ETOUI(LEVEL::KAKULSAYDON_ARENA), sourceRevision, bindingsReady, status, pending.draft)) return false;
    if (!bindingsReady) return true;
    ready = true;
    status = "Complete Play dependencies are fully prepared.";
    return true;
}
```

## G06. 현재 검증 범위와 재현 명령

제품에 추가하는 C++ 파일은 없으므로 기존 여섯 H/CPP의 프로젝트·filters 등록을 유지한다. out의 독립 CPU fixture는 제품 소스에서 실제 함수 본문을 추출해 검사하는 일회성 검증 파일이며 Client.vcxproj와 filters에 등록하지 않는다. 실제 GPU, CModel, renderer, 장면의 클론 동작과 시각 품질은 이 fixture로 검증하지 않는다.

Step fixture는 실제 세 함수의 증분·일괄 경로를 사용하고 문서 조회와 GameObject/Layer 작업을 CPU 계수·실패 주입 대역으로 제공한다. 신규 clone 최대1, ready 시점, capacity 증분,128/1024 전체 요청 예산 사전 거부, 일반 실패·타입 불일치·owner entry 할당 실패·예외의 해당 단계 rollback, 기존 pool 보존을 확인한다. Cache fixture도 실제 두 함수 본문을 쓰되 문서와 target은 통제된 대역이므로 실제 WORLD validator 의미 검증으로 해석하지 않는다. production source audit은 문서 교체 다섯 경로가 공통 무효화를 commit 전에 호출하는지 확인한다. 현재 실행 결과와 원문 SHA·범위는 각 receipt와 대응 RESULT에 기록한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File out/MarioWorldPrewarm20261004/Run-PrewarmStepContract.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File out/MarioWorldPrewarm20261004/Run-PreparedSubsetCacheContract.ps1
git diff --check
```

제품 검증은 변경된 Client C++와 영향받는 header 소비자의 최소 컴파일을 기준으로 한다. 전체 shader 생성이나 전체 솔루션 재빌드 완료를 전제로 하지 않는다. 실제 실행한 컴파일 target과 그 결과만 RESULT에 남긴다. Client/UI는 에이전트가 실행하지 않으며 Debug 녹화의 hitch 개선과 렌더링 최종 판정은 사용자 화면 확인에 남는다. CPU fixture 성공이나 clone 수량 감소를 실측 프레임 시간 개선으로 바꾸어 서술하지 않는다.
