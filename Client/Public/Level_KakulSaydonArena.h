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
	/* Party roster window (CMainApp): the Server roster and the per-player HP / madness join. */
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
	// Changes whenever a new gate activation is submitted, including the same gate.
	std::uint32_t Get_DebugGateGeneration() const { return m_iNextDebugGateRequestSequence; }
    bool Debug_PrepareCompletePlayResources(const std::vector<std::string>& patternIds,
        const std::vector<std::string>& bundleIds, uint32_t sourceRevision,
        bool& ready, std::string& status, bool wholeRaid = false);
    void Debug_ResetCompletePlayPreparation() { m_CompletePlayPreparation.reset(); }

	bool_t Is_DebugGatePending() const { return m_bDebugStartPending || NO_ACTIVE_DEBUG_GATE != m_iPendingDebugGate; }
	const std::string& Get_DebugGateStatus() const { return m_strDebugGateStatus; }
	/* Debug tuning only: the live body of one arena boss archetype. */
	std::shared_ptr<CNpc> Debug_FindArenaBossNpc(std::string_view archetypeId) const
	{
		return m_Replication.Find_ArenaBossNpc(archetypeId);
	}
#endif

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
    { m_Replication.Set_TargetedCombatPresentationPlayer(player); }
	void Collect_KoukuPresentationViews(std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
		std::vector<KOUKU_CARD_PRESENTATION_VIEW>& cards) const
	{ m_Replication.Collect_KoukuPresentationViews(bosses, cards); }
	void Collect_KoukuMazeTargets(std::vector<KOUKU_MAZE_TARGET_VIEW>& targets) const
	{ m_Replication.Collect_KoukuMazeTargets(targets); }
	bool_t Sample_CompositionCamera(std::string_view shotId, float seconds, const float3_t& offset, std::string_view ownerKey, uint32_t durationMs, bool_t preview);
	bool_t Is_CompositionCameraEnabled() const;
	bool_t Is_CinematicPresentationActive() const;
	bool_t Is_LocalMarioStageActive() const;
	void Trace_CinematicPresentation(std::string_view renderingProfile);
	void Stop_CompositionCamera(bool_t force = false);
	bool_t Try_GetCompositionWorldPivot(std::string_view instanceId, float4x4_t& out,
		std::string_view occurrenceId = {}, std::uint32_t emissionIndex = 0u) const;
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
    bool_t Can_StartCompositionWorld(const std::string& instanceId, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr) const;
	bool_t Try_GetOwnedCompositionWorldPivot(std::uint32_t runEpoch, const std::string& memberId,
		const std::string& sequenceId, const std::string& cueId, float4x4_t& out, std::uint32_t emissionIndex = 0u,
        std::uint32_t patternSequence = 0u) const;
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
	   smoke leaf once; a newly set curse bit queues the centred notice. */
	void Update_MarioBallPresentation(f32_t timeDelta);
	std::string m_strMarioBallLayoutInstance;
	std::uint16_t m_iMarioPoppedBallsSeen = 0u;
	std::uint8_t m_iMarioCurseSeen = 0u;
	std::uint8_t m_iMarioCurseNoticeQueue = 0u;
	std::int32_t m_iMarioCurseNoticeColor = -1;
	f32_t m_fMarioCurseNoticeSeconds = 0.f;
	std::array<bool_t, 3u> m_bMarioBallSmokeFailed = {};
	// Presentation-only launch markers use published world positions and the existing object player.
	struct MARIO_BOMB_EMITTER
	{
		std::uint8_t stage = 0u;
		std::uint32_t seed = 1u, phaseMs = 0u, durationMs = 0u;
		std::vector<std::string> slots;
		std::vector<std::int64_t> births;
		bool_t failed = false;
	};
	bool_t Ready_MarioBombPresentation(std::string& status);
	void Update_MarioBombPresentation(f32_t timeDelta);
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
#ifdef _DEBUG
    struct COMPLETE_PLAY_PREPARATION final
    {
        std::vector<std::string> selectedPatterns, selectedBundles;
        KOUKU_SAYDON_PLAY_RESOURCES resources;
        uint32_t sourceRevision = 0u;
        bool wholeRaid = false;
        uint64_t v1Revision = 0u, v2Generation = 0u, worldRevision = 0u;
        size_t actorIndex = 0u, v2Index = 0u, worldIndex = 0u;
    };
    std::optional<COMPLETE_PLAY_PREPARATION> m_CompletePlayPreparation;
#endif

	bool_t m_bWorldObjectReloadPending = false;
	struct OWNED_WORLD_CUE final
	{
		std::uint32_t runEpoch = 0, patternSequence = 0, startTick = 0, durationMs = 0;
		std::string memberId, cueId, occurrenceId, sequenceId;
		float clockMs = 0.f;
		bool untilDestroyed = false;
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
	/* Index into Get_DebugGates() of the gate whose bosses are raised now;
	   that button stays disabled until another gate or Despawn is chosen. */
	size_t m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	string m_strGatePresentationProfileId;
    size_t m_iGateLightingIndex = NO_ACTIVE_DEBUG_GATE;
    std::shared_ptr<CMapLightPresentationRuntime> m_pGateMapLightPresentation;
    std::uint32_t m_iServerRaidGatePresentationEpoch = 0u;
    std::optional<CMapLightDocument> m_GateMapLightSource;
    std::shared_ptr<CMapLightPresentationRuntime> m_pPendingGateMapLights;
    std::optional<CMapLightDocument> m_PendingGateMapLightSource;

	bool_t m_bSequenceCombatPending = false;
	bool_t m_bSequenceCombatFadeHeld = false;
	std::string m_strDebugGateStatus =
		"Choose a gate. The Server raises its bosses and moves only your player.";
#ifdef _DEBUG
	/* Debug gate command sequence and the accumulated Server replies shown in
	   the F1 arena panel. Session state only; never persisted. */
	bool m_bDebugGazeView = false;
	float m_fDebugGazeHalfAngle = 45.f;
	float m_fDebugGazeDistance = 30.f;
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
#endif
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
	std::uint32_t m_iNextGateRequestSequence = 1u;
	// Attempt once on each playback edge; missing media never retries every frame.
	bool_t m_bReadyTerraceBgmInitialized = false;
	bool_t m_bReadyTerraceBgmWanted = false;
	bool_t m_bLocalSequencePlaybackActive = false;
	bool_t m_bReadyTerraceBgmStarted = false;
	std::uint32_t m_iReadyTerraceObservedRunEpoch = 0u;
	LostArk::Shared::KOUKUSAYDON_RAID_PHASE m_eReadyTerraceObservedPhase = LostArk::Shared::KOUKUSAYDON_RAID_PHASE::INACTIVE;
	bool_t Try_GetReplicatedLocalPlayerPosition(float3_t& outPosition) const;
	void Start_ReadyTerraceBgm();
	void Stop_ReadyTerraceBgm();
	void Update_ReadyTerraceBgm();
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
