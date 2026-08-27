#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "EncounterPatternReference.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "MapEffectPresentationRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PlayerController.h"
#include "ValtanCinematicCameraController.h"
#include "ValtanCinematicCameraDocument.h"
#include "WorldDestructionDebrisPresentationDocument.h"
#include "WorldDestructionDebrisPresentationRuntime.h"
#include "WorldDestructionProjectionDocument.h"
#include "WorldPlayerNameplateView.h"

#include <array>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class IPlayerCommandSink;
class CMapAssetObject;
class CHUDRuntimeView;

class CLevel_ValtanArena final : public CLevel
{
private:
	CLevel_ValtanArena(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_ValtanArena();

	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Layer_Camera(const wstring_t& strLayerTag);
	bool_t Ready_CinematicCamera();
	bool_t Bind_CameraToLocalCharacter();
	void Update_CinematicCamera(f32_t fTimeDelta);
	bool_t Update_CinematicCameraExitTransition(f32_t fTimeDelta);
	void End_CinematicCameraOverride();
	void End_CinematicCamera();
	enum class RAID_PRELUDE_BGM_STATE : uint8_t
	{
		NONE,
		M01_PROGRESS,
		M04_POST_MINIBOSS
	};
	void Transition_RaidPreludeBgm(RAID_PRELUDE_BGM_STATE nextState);
	void Handle_WorldEntityDespawned(
		std::string_view placementId,
		std::string_view archetypeId);
	void Update_WorldDestructionPresentation(f32_t fTimeDelta);
	bool_t Apply_EncounterPropPresentation();
	/* Death-screen overlay: real deadscene.gfx panel art + revive button. Local
	player only, unlimited revives (Handle_RevivePlayer already gates this to
	VALTAN_ARENA and is free/no-cooldown by design). */
	void Update_DeadScene();
	void Render_DeadScene();
#ifdef _DEBUG
	// O key: instantly kill the local player (Handle_DebugKillSelf), to test
	// the death screen without waiting to die for real.
	void Update_DebugKillSelfKey();
	enum class REFERENCE_CAMERA_VIEW : uint8_t
	{
		NONE,
		TOP_DOWN,
		EXTERIOR
	};
	bool_t Begin_ReferenceCamera(REFERENCE_CAMERA_VIEW view);
	bool_t Set_ReferencePhaseProxyVisible(bool_t visible);
	void Update_ReferenceCamera();
	void End_ReferenceCamera(bool_t toggleFollowRequested);
	const char_t* Get_ReferenceCameraViewName() const;
	/* Debug audition of an authored health-bar pattern. The panel only submits
	typed requests and reports what the Server answered; it never starts a
	pattern or breaks a wall on its own. Reference-view buttons below are a
	separate presentation-only camera aid and never submit gameplay state. */
	void Render_AuditionPanel();
	void Update_AuditionTransaction();
	bool_t Submit_Audition(
		LostArk::Shared::VALTAN_AUDITION_OPERATION operation,
		uint32_t explicitCommandPayload = 0u);
	bool_t Load_AuditionTimeline();
	struct AUDITION_PENDING_REQUEST final
	{
		uint32_t iSequence = 0u;
		LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
			LostArk::Shared::VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		uint32_t iTargetHealthBar = 0u;
		uint64_t iLastSentAtMilliseconds = 0u;
		uint32_t iRetryCount = 0u;

		[[nodiscard]] bool_t Is_Active() const
		{
			return 0u != iSequence;
		}
	};
	struct AUDITION_TIMELINE_ACTION final
	{
		std::string strPatternId;
		uint32_t iRepeat = 0u;
	};
	struct AUDITION_TIMELINE_ROW final
	{
		std::string strRowId;
		uint32_t iCommandId = 0u;
		uint32_t iOrdinal = 0u;
		uint32_t iSectionHealthBar = 0u;
		std::string strEntryType;
		std::vector<AUDITION_TIMELINE_ACTION> PatternActions;
		std::string strArenaState;
		std::string strPropState;
		std::string strDisplayLabel;
	};
	/* A separate developer helper can still cross several authored health bars
	without resetting between them. The selectable fight timeline above it is a
	Server-owned one-row audition and never uses this Client-side queue. */
	struct ENVIRONMENT_TIMELINE_STEP final
	{
		LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
			LostArk::Shared::VALTAN_AUDITION_OPERATION::ARM_HEALTH_BAR;
		uint32_t iTargetHealthBar = 0u;
		bool_t waitForPattern = false;
	};
	void Start_EnvironmentTimeline();
	/* Queues one PLAY_PATTERN step per authored rotation entry, in the order
	the script lists them, so the whole authored order can be watched without
	waiting for the weighted roll to pick each pattern. */
	void Start_AuthoredRotationPlayback(
		const std::vector<std::string>& rotationOrder);
	void Advance_EnvironmentTimeline(bool_t isBossPatternRunning);
#endif

private:
	CMapPlacementRuntime m_MapRuntime;
	CDeployPropRuntime m_DeployRuntime;
	CMapEffectPresentationRuntime m_MapEffectPresentationRuntime;
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;
	shared_ptr<CCamera_Free> m_pCamera = { nullptr };
	weak_ptr<CCharacter> m_pCameraTarget;
	weak_ptr<CTransform> m_pCinematicRestoreTarget;
	bool_t m_bCinematicRestoreFollowRequested = false;
	bool_t m_bCinematicCameraApplied = false;
	uint64_t m_iCinematicCameraOwnerId = 0u;
	CEncounterPatternReference m_ValtanEncounterReference;
	CValtanCinematicCameraDocument m_ValtanCinematicCameraDocument;
	CValtanCinematicCameraController m_ValtanCinematicCameraController;
	CWorldDestructionProjectionDocument m_WorldDestructionProjectionDocument;
	CWorldDestructionDebrisPresentationDocument
		m_WorldDestructionDebrisPresentationDocument;
	CWorldDestructionDebrisPresentationRuntime
		m_WorldDestructionDebrisPresentationRuntime;
	uint64_t m_iObservedWorldDestructionPresentationGeneration = 0u;
	uint32_t m_iObservedEncounterPropEpoch = 0u;
	uint32_t m_iObservedEncounterPropServerTick = 0u;
	/* The shatter is an edge, not a level: a Server sync repeats BREAKING for
	   as long as the slot stays in it. Remembering the slot state version the
	   burst was thrown for fires exactly once per shatter and still catches a
	   later one even if the intervening syncs were never observed. */
	std::array<uint32_t, 4> m_FiredEncounterPropBurstVersions = {
		0u, 0u, 0u, 0u };
	CClientReplication m_Replication;
	RAID_PRELUDE_BGM_STATE m_eRaidPreludeBgmState =
		RAID_PRELUDE_BGM_STATE::NONE;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPlayerController m_PlayerController;
	unique_ptr<CHUDRuntimeView> m_pDeadSceneView;
#ifdef _DEBUG
	// O key: instantly kill the local player, to test the death screen without waiting to die.
	bool_t m_bDebugKillSelfKeyDown = false;
	weak_ptr<CTransform> m_pReferenceCameraRestoreTarget;
	bool_t m_bReferenceCameraRestoreFollowRequested = false;
	bool_t m_bReferenceCameraApplied = false;
	bool_t m_bReferenceSpaceHoleVisible = false;
	REFERENCE_CAMERA_VIEW m_eReferenceCameraView =
		REFERENCE_CAMERA_VIEW::NONE;
	size_t m_iSelectedAuditionBarIndex = 0u;
	/* Index into the authored pattern order the Server publishes in the same
	document order, so the Debug browser can play a NORMAL pattern no health
	bar owns. */
	size_t m_iSelectedAuditionPatternIndex = 0u;
	std::vector<AUDITION_TIMELINE_ROW> m_AuditionTimelineRows;
	size_t m_iSelectedAuditionTimelineRowIndex = 0u;
	bool_t m_bAuditionTimelineLoadAttempted = false;
	std::string m_strAuditionTimelineStatus;
	uint32_t m_iNextAuditionRequestSequence = 1u;
	AUDITION_PENDING_REQUEST m_PendingAuditionRequest;
	std::string m_strAuditionStatus;
	std::vector<ENVIRONMENT_TIMELINE_STEP> m_EnvironmentTimeline;
	size_t m_iEnvironmentTimelineStep = 0u;
	bool_t m_bEnvironmentTimelineWaiting = false;
	bool_t m_bEnvironmentTimelinePatternStarted = false;
#endif

public:
	static unique_ptr<CLevel_ValtanArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
