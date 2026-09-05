#pragma once

#include "Client_Defines.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "EncounterPatternReference.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#include "MapEffectPresentationRuntime.h"
#include "MapLightPresentationRuntime.h"
#include "PartyInteractionView.h"
#include "PlayerController.h"
#include "WorldPlayerChatBubbleView.h"
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
class CUILayoutRuntime;

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

	static CLevel_ValtanArena* Get_Active() { return s_pActiveInstance; }
#ifdef _DEBUG
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	CPlayerController& Get_DebugPlayerController() { return m_PlayerController; }
	// Applies immediately and remembers this arena's value until process exit.
	bool_t Set_DebugCameraSpeed(f32_t metersPerSecond);
#endif

	/* CGameInstance::Draw_Text submits immediately (SpriteBatch) but the invite
	   popup's art composites later inside CImGuiLayer::EndFrame() -- see
	   CPartyInteractionView::Render_InvitePopupText's own comment. */
	void Render_PartyInviteText()
	{
		m_PartyInteraction.Render_InvitePopupText();
		m_PartyInteraction.Render_ContextMenuText();
	}
	const LostArk::Shared::S2C_PARTY_ROSTER& Get_PartyRoster() const
	{
		return m_Replication.Get_PartyRoster();
	}
	const CReplicatedPlayerHealth& Get_PlayerHealth() const
	{
		return m_Replication.Get_PlayerHealth();
	}
	void Collect_MinimapMarkers(
		CClientReplication::MINIMAP_MARKER_SNAPSHOT& outSnapshot) const
	{
		m_Replication.Collect_MinimapMarkers(outSnapshot);
	}
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const
	{
		return m_pPlayerCommandSink;
	}
	/* Local-only authoring placement. The caller receives a point beside the
	   replicated local player, sampled against that player's Navigation when it
	   is available. If the player presentation has not arrived yet, the primary
	   replicated boss is the visible arena fallback. This never exposes or
	   mutates either authoritative actor. */
	bool_t Try_Get_AuthoringPreviewPlacement(
		float3_t& OutPosition,
		std::string& strOutSource) const;
	/* Workbench authoring must reload the replicated primary Server consumer,
	   not only the Development preview target. These wrappers keep the Level's
	   replication owner as the single route and expose its freshness admission
	   to Complete Play. */
	bool_t Reload_PrimaryValtanPresentationAuthoring(
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus);
	bool_t Reload_PrimaryValtanCombatObjectSoundCues(
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus);
	bool_t Can_Play_PrimaryValtanPresentation(
		const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
		std::string& strOutStatus) const;
	bool_t Get_PrimaryValtanPatternSoundSourceReceipt(
		VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
		std::string& strOutStatus) const;
#ifdef _DEBUG
	struct ARENA_ACTIVE_STATE final
	{
		bool_t bSynchronized = false;
		bool_t bOrdinaryWallsActive = false;
		bool_t bOuterRingActive = false;
		bool_t bThreeOClockFloorActive = false;
		bool_t bNineOClockFloorActive = false;
		uint32_t iOrdinaryGroupCount = 0u;
		uint32_t iOuterRingGroupCount = 0u;
		uint32_t iThreeOClockGroupCount = 0u;
		uint32_t iNineOClockGroupCount = 0u;
		uint32_t iDebrisActorCount = 0u;
		uint32_t iActiveCollisionCount = 0u;
		uint32_t iActiveNavigationRegionCount = 0u;
		uint64_t iNavigationRevision = 0u;
	};

	/* Workbench route. The active Level retains the one request-sequence and
	   bounded retry owner used by the explicit arena-preset controls; callers
	   never send packets or mutate wall visibility locally. */
	bool_t Set_ArenaPreset(
		LostArk::Shared::VALTAN_ARENA_PRESET preset,
		std::string& outStatus);
	const std::string& Get_ArenaAuditionStatus() const
	{
		return m_strAuditionStatus;
	}
	bool_t Is_ArenaPresetRequestPending() const;
	ARENA_ACTIVE_STATE Get_ArenaActiveState() const;
#endif

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
	/* No matching Render_DeadScene() -- migrated to real CUI_Sprite GameObjects on this Level's
	own "Layer_UI" (same reasoning as m_pItemAnnounceView), so CObject_Manager's normal
	Update/Late_Update/Render cycle draws them without an explicit call here. */
	void Update_DeadScene(bool_t isBlockedByRaidClear, f32_t fTimeDelta);
	/* Valtan clear celebration overlay: real EFUI_EPICGATECOMMONCLEAR trace (BgFlash/Emblem
	art + "던전 클리어" headline). Edge-triggers off CCombatHUDViewModel's already-replicated
	primary-boss death latch. The reliable DEAD despawn is the normal terminal edge because the
	Server removes the boss before it builds the following world snapshot, then
	runs a fixed real-derived reveal/hold timeline (EpicGateCommonClearFrame's own
	startFrame=90/holdFrame=296 at the source's 40fps) instead of looping forever. */
	/* No matching Render_RaidClear() -- migrated to real CUI_Sprite GameObjects on this Level's
	own "Layer_UI" (same reasoning as m_pDeadSceneView), so CObject_Manager's normal
	Update/Late_Update/Render cycle draws them without an explicit call here. */
	void Update_RaidClear(f32_t fTimeDelta);
	/* Item acquisition toast: real EFUI_ANNOUNCE frame art (announce_i3e3.dds), one item at a
	time. Diffs CCombatHUDViewModel's own already-replicated inventory snapshot frame-to-frame
	(itemIds present now that weren't in the previous frame) into a FIFO queue, then shows each
	queued item's icon/name for ITEM_ANNOUNCE_HOLD_SECONDS before starting the next. The first
	frame only captures a baseline (m_bItemAnnounceBaselineCaptured) instead of diffing, so
	whatever the player already owns on Level entry never gets announced as newly acquired. */
	/* No matching Render_ItemAnnounce() -- same reasoning as m_pDeadSceneView/m_pRaidClearView
	above: m_pItemAnnounceView's slots are real CUI_Sprite GameObjects added to this Level's own
	layer, so CObject_Manager's normal per-frame Update/Late_Update/Render cycle draws them
	without an explicit call here. */
	void Update_ItemAnnounce(f32_t fTimeDelta);
	/* Starts the overlay's reveal/hold timeline from 0 and fires its one-shot sound cue. Shared by
	Update_RaidClear's real edge-trigger and Update_DebugRaidClearKey's forced trigger so both
	paths play the same cue instead of the debug key silently skipping it. */
	void Trigger_RaidClear();
#ifdef _DEBUG
	// O key: instantly show the Raid Clear overlay (Update_RaidClear), to test it
	// without waiting to kill Valtan for real. Used to instantly kill the local
	// player to test the death screen; that overlay is done now, so the key was
	// repurposed instead of adding a second debug key.
	void Update_DebugRaidClearKey();
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
	void Update_AuditionTransaction();
	bool_t Submit_Audition(
		LostArk::Shared::VALTAN_AUDITION_OPERATION operation,
		uint32_t explicitCommandPayload = 0u);
	struct AUDITION_PENDING_REQUEST final
	{
		uint32_t iSequence = 0u;
		LostArk::Shared::VALTAN_AUDITION_OPERATION eOperation =
			LostArk::Shared::VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET;
		uint32_t iTargetHealthBar = 0u;
		uint64_t iLastSentAtMilliseconds = 0u;
		uint32_t iRetryCount = 0u;

		[[nodiscard]] bool_t Is_Active() const
		{
			return 0u != iSequence;
		}
	};
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
	CPartyInteractionView m_PartyInteraction;
	CWorldPlayerChatBubbleView m_ChatBubbleView;
	CPlayerController m_PlayerController;
	unique_ptr<CUILayoutRuntime> m_pDeadSceneView;
	unique_ptr<CUILayoutRuntime> m_pRaidClearView;
	/* Edge-detect for the boss's replicated eAction (see Update_RaidClear) and the elapsed time
	since that edge -- negative means the overlay is not currently showing. */
	bool_t m_bRaidClearWasBossDead = false;
	f32_t m_fRaidClearElapsedSeconds = -1.f;
	// RaidClear_ReturnButton's own request sequence, same one-writer pattern as
	// CLevel_Bern::m_iNextNpcEntryConfirmSequence.
	uint32_t m_iNextReturnToBernSequence = 1u;
	unique_ptr<CUILayoutRuntime> m_pItemAnnounceView;
	/* See Update_ItemAnnounce's own comment. False until the first frame's inventory snapshot is
	captured as the "already owned" baseline. */
	bool_t m_bItemAnnounceBaselineCaptured = false;
	vector<string> m_ItemAnnounceObservedItemIds;
	vector<string> m_ItemAnnounceQueue;
	string m_strItemAnnounceCurrentItemId;
	/* Negative means no toast is currently showing (queue may still be non-empty, waiting for
	the next Update_ItemAnnounce tick to pop it). */
	f32_t m_fItemAnnounceElapsedSeconds = -1.f;
#ifdef _DEBUG
	// O key: instantly show the Raid Clear overlay, to test it without waiting to kill Valtan.
	bool_t m_bDebugRaidClearKeyDown = false;
	weak_ptr<CTransform> m_pReferenceCameraRestoreTarget;
	bool_t m_bReferenceCameraRestoreFollowRequested = false;
	bool_t m_bReferenceCameraApplied = false;
	bool_t m_bReferenceSpaceHoleVisible = false;
	REFERENCE_CAMERA_VIEW m_eReferenceCameraView =
		REFERENCE_CAMERA_VIEW::NONE;
	uint32_t m_iNextAuditionRequestSequence = 1u;
	AUDITION_PENDING_REQUEST m_PendingAuditionRequest;
	std::string m_strAuditionStatus;
#endif

	static CLevel_ValtanArena* s_pActiveInstance;

public:
	static unique_ptr<CLevel_ValtanArena> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
