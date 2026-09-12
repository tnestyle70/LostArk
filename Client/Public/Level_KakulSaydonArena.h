#pragma once

#include "Client_Defines.h"
#include "ArenaCameraProfile.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Effect_PresentationService.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PlayerController.h"
#include "StatusEffectTextView.h"
#include "ValtanCinematicCameraDocument.h"
#include "ValtanCinematicCameraController.h"
#include "WorldSequencePlayer.h"

#include <array>
#include <map>
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
{
public:
	void Set_MapLightAuthoringOverride(std::shared_ptr<CMapLightPresentationRuntime> lights) { m_pMapLightAuthoringOverride = std::move(lights); }
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

#ifdef _DEBUG
	void Set_DebugGazeView(bool visible, float halfAngleDegrees, float distanceM)
	{ m_bDebugGazeView = visible; m_fDebugGazeHalfAngle = halfAngleDegrees; m_fDebugGazeDistance = distanceM; }
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	CPlayerController& Get_DebugPlayerController() { return m_PlayerController; }
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
		bool_t playing, uint32_t clockMs, std::string& status);
	void Debug_StopCompositionWorldPreview();
	// Applies immediately and remembers this arena's value until process exit.
	bool_t Set_DebugCameraSpeed(f32_t metersPerSecond);

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
	/* Despawns the previous gate bosses, requests this gate's placements,
	   submits the player teleport, points the HUD and the pattern audition at
	   the gate boss. Every step is a typed Server command; nothing local is
	   spawned or moved. */
	bool_t Debug_ActivateGate(size_t gateIndex, std::string& outStatus);
	bool_t Debug_DespawnArenaBosses(std::string& outStatus);
	void Debug_SetSequenceCombatPending(bool_t pending);
	void Debug_HoldSequenceCombatFade();
	void Debug_RetireGateActivation(const std::string& reason);
	size_t Get_ActiveDebugGate() const { return m_iActiveDebugGate; }
	bool_t Is_DebugGatePending() const { return NO_ACTIVE_DEBUG_GATE != m_iPendingDebugGate; }
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
	void Collect_KoukuPresentationViews(std::vector<KOUKU_BOSS_PRESENTATION_VIEW>& bosses,
		std::vector<KOUKU_CARD_PRESENTATION_VIEW>& cards) const
	{ m_Replication.Collect_KoukuPresentationViews(bosses, cards); }
	void Collect_KoukuMazeTargets(std::vector<KOUKU_MAZE_TARGET_VIEW>& targets) const
	{ m_Replication.Collect_KoukuMazeTargets(targets); }
	bool_t Sample_CompositionCamera(std::string_view shotId, float seconds, const float3_t& offset, std::string_view ownerKey, uint32_t durationMs, bool_t preview);
	void Stop_CompositionCamera(bool_t force = false);
	bool_t Try_GetCompositionWorldPivot(std::string_view instanceId, float4x4_t& out,
		std::string_view occurrenceId = {}, std::uint32_t emissionIndex = 0u) const;
    bool_t Create_CompositionPreviewActor(const KOUKU_SAYDON_COMPOSITION_PATTERN& pattern,
        std::shared_ptr<CNpc>& outActor, std::string& status);
    void Release_CompositionPreviewActor(const std::shared_ptr<CNpc>& actor);
    CWorldSequencePlayer::TARGET_SET Get_CompositionWorldTargets() { return Make_WorldSequenceTargets(); }
	// Authoring inventory reads the placed centre even before any sequence plays.
	bool_t Try_GetWorldSequencePlacementBaseline(const WORLD_SEQUENCE_INSTANCE& instance,
		float3_t& outPosition) const;
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const { return m_pPlayerCommandSink; }
	const CWorldSequenceDocument& Get_WorldSequenceDocument() const { return m_SequencePlayer.Get_Document(); }
	const LostArk::Shared::S2C_KOUKUSAYDON_BUNDLE_STATE& Get_KoukuBundleState() const { return m_Replication.Get_KoukuBundleState(); }
	std::uint32_t Get_PresentationServerTick() const { return m_Replication.Get_LastServerTick(); }
    bool_t Can_StartCompositionWorld(const std::string& instanceId, std::string& status,
        const CWorldSequenceDocument* sourceDocument = nullptr) const;
	bool_t Try_GetOwnedCompositionWorldPivot(std::uint32_t runEpoch, const std::string& memberId,
		const std::string& sequenceId, const std::string& cueId, float4x4_t& out, std::uint32_t emissionIndex = 0u) const;
	void Get_WorldObjectValidationTargets(WORLD_SEQUENCE_PLACEMENT_MAP&, WORLD_SEQUENCE_DEPLOY_MAP&) const;
	bool_t Reload_WorldObjectRuntime(std::string& status);
#ifdef _DEBUG
	bool_t Debug_BeginWorldObjectPreview(const CWorldSequenceDocument&, const std::string& instanceId,
		std::string& status, bool_t previewAtCharacter = true);
	bool_t Debug_SampleWorldObjectPreview(f32_t clockMs, std::string& status);
	void Debug_StopWorldObjectPreview();
#endif
	const std::vector<KAKUL_CAMERA_SHOT>& Get_PublishedCameraShots() const { return m_CameraShots; }
	bool_t Reload_PublishedCameraShots(std::string& outStatus) { return Load_CameraShots(outStatus); }
	bool_t Ensure_CameraShotAuthoring(std::string& outStatus);
	bool_t Reload_CameraShotAuthoring(std::string& outStatus);
	bool_t Create_CameraShot(std::string_view name, std::string& outShotId, std::string& outStatus);
	bool_t Update_CameraShot(const KAKUL_CAMERA_SHOT& shot, std::string& outStatus);
	bool_t Capture_CameraShot(std::string_view shotId, std::string& outStatus);
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
	CWorldSequencePlayer::TARGET_SET Make_WorldSequenceTargets();
	/* The cutscene is one show spread over several instances. Starting the
	   named one starts them all and swaps the arena for the cutscene copy. */
	bool_t Start_PopupBookCutscene(
		const CWorldSequencePlayer::TARGET_SET& targets,
		std::string& outStatus);
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
	void Update_EntranceTriggerMarkers(f32_t deltaSeconds);
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
	void Update_DeadScene(f32_t fTimeDelta);
	const KAKUL_CAMERA_SHOT* Find_ActiveCameraShot(
		const float3_t& vPosition) const;
	void Release_CameraShot();
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Bind_CameraToLocalCharacter();

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
	/* The authored deploy catalog carries both paper levers and both paper
	   stage bridges. A bridge stays DESPAWNED until its lever is pulled, so
	   suppress the bridges before the first rendered frame instead of letting
	   them appear already unfolded. */
	CDeployPropRuntime m_DeployRuntime;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	std::shared_ptr<CMapLightPresentationRuntime> m_pMapLightAuthoringOverride;
	CWorldSequencePlayer m_SequencePlayer;
	bool_t m_bWorldObjectReloadPending = false;
	struct OWNED_WORLD_CUE final
	{
		std::uint32_t runEpoch = 0, startTick = 0, durationMs = 0;
		std::string memberId, cueId, occurrenceId, sequenceId;
		float clockMs = 0.f;
		WORLD_EMISSION_ANCHOR emissionAnchor;
		std::shared_ptr<CWorldSequencePlayer> player;
	};
	WORLD_EMISSION_RESOLVER m_WorldEmissionResolver;
	std::vector<LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY> m_PendingOwnedWorldCues;
	std::map<std::string, OWNED_WORLD_CUE> m_OwnedWorldCues;
	std::set<std::string> m_StoppedWorldOwners;
	std::set<std::string> m_ConsumedWorldCueIds;
	std::uint32_t m_iLatestWorldRunEpoch = 0u;
	void Consume_OwnedWorldCue(const LostArk::Shared::S2C_WORLD_SEQUENCE_PLAY& play,
		const CWorldSequencePlayer::TARGET_SET& targets);
#ifdef _DEBUG
	unique_ptr<CWorldSequencePlayer> m_pWorldObjectPreview;
	std::string m_WorldObjectPreviewInstance;
#endif
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
	std::string m_strFollowCameraProfileStatus;
	CClientReplication m_Replication;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	shared_ptr<IWorldEntityCommandSink> m_pWorldEntityCommandSink;
	CPlayerController m_PlayerController;
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
		std::string cancelledOwnerKey;
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
#ifdef _DEBUG
	/* Debug gate command sequence and the accumulated Server replies shown in
	   the F1 arena panel. Session state only; never persisted. */
	bool m_bDebugGazeView = false;
	float m_fDebugGazeHalfAngle = 45.f;
	float m_fDebugGazeDistance = 30.f;
	std::uint32_t m_iNextDebugGateRequestSequence = 1u;
	/* Index into Get_DebugGates() of the gate whose bosses are raised now;
	   that button stays disabled until another gate or Despawn is chosen. */
	size_t m_iActiveDebugGate = NO_ACTIVE_DEBUG_GATE;
	size_t m_iPendingDebugGate = NO_ACTIVE_DEBUG_GATE;
	std::map<std::string, std::uint64_t> m_DebugGatePendingPlacements;
	bool_t m_bDebugGateFailed = false;
	f32_t m_fDebugGatePendingSeconds = 0.f;
	bool_t m_bSequenceCombatPending = false;
	bool_t m_bSequenceCombatFadeHeld = false;
	std::string m_strDebugGateStatus =
		"Choose a gate. The Server raises its bosses and moves only your player.";
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
	void Debug_Play_ClearThenMvp();
	void Debug_Show_MvpResult();
	void Debug_Hide_MvpResult();
	bool_t Debug_Is_MvpResultVisible() const;

public:
	static unique_ptr<CLevel_KakulSaydonArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
