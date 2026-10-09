#pragma once

#include "Client_Defines.h"
#include "ArenaCameraProfile.h"
#include "ClientReplication.h"
#include "DeployPropRuntime.h"
#include "Level.h"
#include "MapPlacementRuntime.h"
#ifdef _DEBUG
#include "MapAuthoringHost.h"
#endif
#include "ValtanCinematicCameraDocument.h"

#include "PartyInteractionView.h"
#include "PlayerController.h"
#include "RaidEntryPreviewView.h"
#include "SystemMenuButtonsView.h"
#include "WorldPlayerChatBubbleView.h"
#include "WorldPlayerNameplateView.h"

#include <chrono>
#include <optional>

NS_BEGIN(Engine)
class CTransform;
NS_END

NS_BEGIN(Client)

class CCamera_Free;
class CCharacter;
class CMapLightPresentationRuntime;
class CMapEffectPresentationRuntime;
class CTrigger_Box;
class CInteractKeyPromptView;
class IPlayerCommandSink;

class CLevel_Bern final : public CLevel
#ifdef _DEBUG
	, public IMapAuthoringHost
#endif
{
private:
	CLevel_Bern(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);

public:
	virtual ~CLevel_Bern();

public:
	virtual HRESULT Initialize() override;
	virtual void Update(f32_t fTimeDelta) override;
	virtual HRESULT Render() override;

	/* CGameInstance::Draw_Text submits immediately (SpriteBatch), so every LOA-font label in
	this codebase is drawn from its own pass after CImGuiLayer::EndFrame() -- same reason
	Level_CharacterSelect splits Render_ArenaSpawnLabels() out. m_pValtanEntryView is private
	to this level, so CMainApp reaches it through Get_Active() instead of a second
	CRaidEntryPreviewView of its own. Defined out-of-line in the .cpp (not inline here) so
	files that merely include this header for Get_Active() etc. don't also need
	CRaidEntryPreviewView's full definition just to compile it. */
	void Render_ValtanEntryModalText();
	/* Drives the raid-entry popup's own CUI_Sprite visibility/hover state and hit-testing for
	   one frame; the sprites themselves draw through CObject_Manager's normal render cycle, so
	   the old ImGui foreground-drawlist ordering requirement against the combat HUD is gone.
	   Public (not called from this level's own Render()) because CMainApp owns the call site.
	   Only this level reacts to a true return (real NPC entry command); the debug-only preview
	   in Level_CharacterSelect never does. */
	void Render_ValtanEntryModal();
	/* Same reasoning, for CPartyInteractionView's invite-confirm popup text --
	   see CPartyInteractionView::Render_InvitePopupText's own comment. */
	void Render_PartyInviteText()
	{
		m_PartyInteraction.Render_InvitePopupText();
		m_PartyInteraction.Render_ContextMenuText();
	}
	/* The captions under the bottom-right icon buttons (HUD text layer, called by CMainApp). */
	void Render_SystemMenuText() { m_SystemMenuButtons.Render_Text(); }
	static CLevel_Bern* Get_Active() { return s_pActiveInstance; }
	/* Non-null only while the ship follow pose is the active camera, so CMainApp can hand the
	   ride's fog tuning to the presentation environment and nothing else needs to know about
	   ships. Every other level and the map camera keep the authored scene fog untouched. */
	static const ARENA_SHIP_FOG* Get_ActiveShipFog()
	{
		return (nullptr != s_pActiveInstance && s_pActiveInstance->m_bShipCameraActive) ?
			&s_pActiveInstance->m_FollowCameraProfile.shipFog : nullptr;
	}
	const ARENA_CAMERA_PROFILE& Get_FollowCameraProfile() const
	{ return m_FollowCameraProfile; }
	const std::string& Get_FollowCameraProfileStatus() const
	{ return m_strFollowCameraProfileStatus; }
	bool_t Set_FollowCameraProfile(const ARENA_CAMERA_PROFILE& profile,
		std::string& outStatus);
	/* Main-thread-only, process-session lens override for the Bern entrance cue.
	   Empty uses the authored track. Set accepts finite 10..120 degree vertical FOV;
	   clearing restores that track on the next sample without changing other cameras. */
	static std::optional<f32_t> Get_EntranceCinematicFovYOverride();
	static bool_t Set_EntranceCinematicFovYOverride(f32_t fovYDegrees);
	static void Clear_EntranceCinematicFovYOverride();
	// First key of the active authored cue, or 45 degrees before it is prepared.
	static f32_t Get_EntranceCinematicAuthoredFovYDegrees();
#ifdef _DEBUG
	shared_ptr<CCamera_Free> Get_DebugCamera() const { return m_pCamera; }
	bool_t Request_DebugEntranceReplay();
	const std::string& Get_DebugEntranceReplayStatus() const
	{ return m_strDebugEntranceReplayStatus; }
	/* Map Tool borrows this level's live map the same way the Kouku and Valtan
	   arenas lend theirs. The level keeps ownership; the tool only edits the
	   placements in place. */
	CMapPlacementRuntime& Get_MapAuthoringRuntime() override { return m_MapRuntime; }
	CDeployPropRuntime& Get_MapAuthoringDeploy() { return m_MapAuthoringDeploy; }
	const ComPtr<ID3D11Device>& Get_MapAuthoringDevice() const { return m_pDevice; }
	const ComPtr<ID3D11DeviceContext>& Get_MapAuthoringContext() const { return m_pContext; }
	uint32_t Get_MapAuthoringLevelIndex() const override { return ETOUI(LEVEL::BERN); }
	const char_t* Get_MapAuthoringLabel() const override { return "Bern"; }
	const CMapAssetCatalog& Get_MapAuthoringCatalog() const override { return m_MapRuntime.Get_Catalog(); }
	std::vector<MAP_RUNTIME_PLACED_ENTRY>& Get_MapAuthoringPlacements() override
	{ return m_MapRuntime.Get_MutablePlacements(); }
	std::vector<MAP_RUNTIME_STATIC_BATCH_ENTRY>& Get_MapAuthoringBatches() override
	{ return m_MapRuntime.Get_AuthoringBatches(); }
	CDeployPropRuntime* Get_MapAuthoringDeployRuntime() override { return nullptr; }
	bool_t Can_ChangeMapAuthoringStructure(std::string& outReason) const override;
	void Rebase_MapAuthoringSelfMotions(const std::vector<MAP_PLACEMENT_RECORD>& records) override
	{ m_MapRuntime.Rebase_AuthoringSelfMotions(records); }
	void Set_MapAuthoringActive(bool_t active) override { m_bMapAuthoringActive = active; }
	CWorldSequencePlayer::TARGET_SET Make_MapAuthoringTargets() override;
#endif
	void Drain_ChatLines(std::vector<CClientReplication::CHAT_LINE>& outLines)
	{
		m_Replication.Drain_ChatLines(outLines);
	}
	const LostArk::Shared::S2C_GUIDE_STATE* Get_GuideState() const { return m_Replication.Get_GuideState(); }
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
	/* The replicated local player (nullptr until the entry snapshot spawned it) -- read-only
	   presentation access for the character info window's live portrait. */
	shared_ptr<CCharacter> Get_LocalCharacter() const
	{
		return m_Replication.Get_LocalCharacter();
	}
	const shared_ptr<IPlayerCommandSink>& Get_PlayerCommandSink() const
	{
		return m_pPlayerCommandSink;
	}
	/* The level's own input controller: the vehicle window submits its mount request through
	   it so the Server round trip has one owner (CPlayerController::Request_VehicleRiding). */
	CPlayerController& Get_PlayerController() { return m_PlayerController; }
	/* Out-of-line for the same reason as Render_ValtanEntryModalText() above --
	   keeps CRaidEntryPreviewView's definition out of this header's own
	   compile requirement for unrelated includers. CMainApp reads it so one Esc press
	   closes only this popup. */
	bool_t Is_ValtanEntryModalOpen() const;

private:
	HRESULT Ready_Layer_Camera(
		const wstring_t& strLayerTag,
		const std::string& areaId);

	bool_t Bind_CameraToLocalCharacter();

	/* Loads the Bern authoring document once and keeps only the two known
	Valtan-entry guide NPCs' authored positions (npc.bern.beda.guide,
	npc.bern.aylara) -- the same static, server-uninvolved position lookup
	Ready_DebugLevelChangeTriggers already does for the enabled trigger boxes, just
	not _DEBUG-only since the interaction it drives is a real product path. */
	bool_t Ready_ValtanEntryNpcs(const std::string& areaId);
	/* Right-click hit-test against m_ValtanEntryNpcs using a world-ray-vs-sphere
	pick (CPlayerController::Try_PickWorldRay) so the NPC can be clicked from
	anywhere on screen, not just while already standing next to it. A hit
	suppresses that frame's move command and walks the character to the NPC
	instead (Request_MoveToPoint); Advance_ValtanEntryWalk opens the confirm
	window once the character actually arrives. Runs before
	m_PlayerController.Update() each frame for the same suppression-timing
	reason. */
	void Update_ValtanEntryInteraction();
	/* Polled every frame regardless of this frame's click: once
	m_isWalkingToValtanEntryNpc is set, opens the confirm window as soon as
	the local character's live position is back within interaction range of
	the target NPC. */
	void Advance_ValtanEntryWalk();
	/* 매 프레임 파티 레이드 입장 투표 replication 이벤트를 소비한다. 프롬프트면 수락/거절
	   창을 열고(모달이 안 열려 있어도), 거절/타임아웃/취소 종료면 창을 닫아 Bern에 남는다. */
	void Poll_RaidEntryVote();
	/* Colosseum match queue: the Server refusing the join or dropping the player closes the wait window. */
	void Poll_ColosseumQueueState();
	/* The bottom-right icon buttons: the options window and the way back to character select. */
	void Update_SystemMenuButtons();
	/* Offers the character's saved inventory, purse and honor title to the Server once. */
	void Try_Send_CharacterRestore();

	/* npc.bern.schmidt's authored position (real placement in Data/Worlds/
	LV_BER_BERNCASTLE/Gameplay.world.json, archetype NPC_SCHMIDT), loaded the same
	way Ready_ValtanEntryNpcs loads its own guide NPCs -- kept as a separate
	single-NPC lookup since it drives an unrelated interaction (opens the Item
	Upgrade window, not a level transfer, and has no confirm modal). */
	bool_t Ready_ItemUpgradeNpc(const std::string& areaId);
	/* Same right-click ray-vs-sphere pick pattern as Update_ValtanEntryInteraction,
	against the single Schmidt NPC position. Uses its own edge-detect state
	(m_wasRightMouseDownForItemUpgradeNpcInteract) rather than sharing
	m_wasRightMouseDownForNpcInteract -- both read the same live mouse button
	each frame independently, which is safe since a click can only ever land
	near one of the two NPCs. */
	void Update_ItemUpgradeNpcInteraction();
	/* Polled every frame: once m_isWalkingToItemUpgradeNpc is set, opens the Item
	Upgrade window (via CMainApp::Get_Active()) as soon as the local character's
	live position is back within interaction range of Schmidt -- no confirm
	modal, unlike Advance_ValtanEntryWalk. */
	void Advance_ItemUpgradeNpcWalk();
	/* Bern's service NPCs: the two repair NPCs (npc.bern.src.31 / npc.bern.src.48 -- the
	pair carrying the anvil symbol in Data/UI/WorldMap/WorldMapNpcSymbols.json) and every
	NPC Data/Items/ItemCatalog.json names as running a shop (the potion merchants). Loaded,
	picked and walked to exactly like the Schmidt NPC above; the picked one is remembered by
	placement id the way the Valtan guides are, and arriving opens its window. */
	bool_t Ready_ServiceNpcs(const std::string& areaId);
	void Update_ServiceNpcInteraction();
	void Advance_ServiceNpcWalk();

	/* Ship NPCs (Bern3 harbor): every enabled NPC placement whose archetype starts with NPC_SHIP_.
	Right-click one, walk to it, and the vehicle window opens in its ship-only mode
	(CMainApp::Open_ShipWindow). Same pick and walk pattern as the Item Upgrade NPC above, over a list. */
	bool_t Ready_ShipNpcs(const std::string& areaId);
	void Update_ShipNpcInteraction();
	void Advance_ShipNpcWalk();
	/* While the local player rides a ship the follow camera takes the retail voyage lens
	(EFTable_CameraSetting 1001 zoom steps 1-3): the anchor-volume step near the harbour, the
	open-sea step outside it, the mouse wheel between steps; it returns to the map profile on
	dismount. */
	void Update_ShipCamera(f32_t fTimeDelta);

	/* Optional entrance cinematic: one authored camera cue from
	Data/Encounters/Bern/BernEntranceCamera.json plays exactly once right after
	entry through the same public product sampler the Valtan cinematics use.
	A missing or invalid document isolates the cinematic and never blocks the
	level; gameplay commands stay suppressed by the existing follow-disabled
	contract while the override owns the camera. */
	bool_t Ready_EntranceCinematic();
	void Update_EntranceCinematic(f32_t fTimeDelta);
	void End_EntranceCinematic();

#ifdef _DEBUG
	bool_t Can_DebugEntranceReplay(std::string& outStatus) const;
	void Consume_DebugEntranceReplay();
	bool_t Ready_DebugLevelChangeTriggers(const std::string& areaId);
	/* O opens m_ValtanEntryView without walking to the guide NPC first --
	   debug-only shortcut for iterating on ValtanRaidEntry_Layout.json's visual
	   layout against a real screen instead of re-walking every time. Uses the
	   first authored Valtan-entry NPC's placement id if one is loaded so the
	   real Entrance button still submits a valid Request_ConfirmNpcEntry; falls
	   back to an empty id (Server rejects, panel still previews) if not.
	   Level_CharacterSelect has its own equivalent O-key debug preview, for the
	   same reason but permanently visual-only there (see its own comment). */
	void Update_ValtanEntryDebugPreviewKey();
#endif

private:
	/*베른성 맵 객체들의 생성과 제거는 기존 Map Runtime이 담당한다.
	Network Player 수명과 섞지 않는다.*/
	CMapPlacementRuntime m_MapRuntime;
#ifdef _DEBUG
	/* Stays empty: LV_BER_BERNCASTLE declares no DeployProp source pair, so
	   Stage_DeployProps returns before touching it. Map Tool's runtime attach
	   still needs a real owner because TARGET_SET::Is_Complete() requires one. */
	CDeployPropRuntime m_MapAuthoringDeploy;
#endif
	shared_ptr<CMapLightPresentationRuntime> m_pMapLightPresentation;
	shared_ptr<CMapEffectPresentationRuntime> m_pMapEffectPresentation;
	bool_t m_bMapLightSubmissionFailureReported = false;

	shared_ptr<CCamera_Free> m_pCamera = { nullptr };

	weak_ptr<CCharacter> m_pCameraTarget;
	ARENA_CAMERA_PROFILE m_FollowCameraProfile =
		CArenaCameraProfile::Default(ARENA_CAMERA_MAP::BERN);
	std::string m_strFollowCameraProfileStatus;

	CClientReplication m_Replication;
	CWorldPlayerNameplateView m_PlayerNameplateView;
	CWorldPlayerChatBubbleView m_ChatBubbleView;
	CSystemMenuButtonsView m_SystemMenuButtons;
	/* Set once the character select icon started the trip to the Lobby: this level then stops
	updating so the closing connection is not reported as a loss. */
	bool_t m_bReturningToCharacterSelect = false;
	bool_t m_bCharacterRestoreSent = false;
	std::optional<std::chrono::steady_clock::time_point> m_CharacterRestoreStarted;
	std::vector<REPLICATED_PLAYER_VIEW> m_NameplatePlayers;
	shared_ptr<IPlayerCommandSink> m_pPlayerCommandSink;
	CPartyInteractionView m_PartyInteraction;
	//PlayerController 추가
	CPlayerController m_PlayerController;

	struct VALTAN_ENTRY_NPC
	{
		std::string strPlacementId;
		float3_t vPosition{};
		/* The castle-interior NPC that opens the Colosseum confirm instead of the raid screen. */
		bool_t isColosseum = false;
	};
	std::vector<VALTAN_ENTRY_NPC> m_ValtanEntryNpcs;
	unique_ptr<CRaidEntryPreviewView> m_pValtanEntryView;
	/* Interact prompt ("<place> [G]") and the mooring marker of the authored dock trigger.
	   Presentation only: the Server decides when the box is offered and what G does. */
	unique_ptr<CInteractKeyPromptView> m_pInteractPrompt;
	uint64_t m_iAnchorMarkerHandle = 0u;
	bool_t m_bAnchorMarkerPlaced = false;
	f32_t m_fAnchorMarkerRetrySeconds = 0.f;
	uint32_t m_iAnchorMarkerAttempts = 0u;
	void Update_AnchorMarker(f32_t fTimeDelta);
	void Clear_AnchorMarker();
	bool_t m_isWalkingToValtanEntryNpc = false;
	std::string m_strValtanEntryNpcPlacementId;
	bool_t m_wasRightMouseDownForNpcInteract = false;
	std::uint32_t m_iNextNpcEntryConfirmSequence = 1u;
	bool_t m_bBernBgmStarted = false;

	bool_t m_hasItemUpgradeNpc = false;
	float3_t m_vItemUpgradeNpcPosition{};
	bool_t m_isWalkingToItemUpgradeNpc = false;
	bool_t m_wasRightMouseDownForItemUpgradeNpcInteract = false;
	enum class NPC_SERVICE { REPAIR, SHOP };
	struct SERVICE_NPC
	{
		std::string strPlacementId;
		float3_t vPosition{};
		NPC_SERVICE eService = NPC_SERVICE::REPAIR;
	};
	std::vector<SERVICE_NPC> m_ServiceNpcs;
	bool_t m_isWalkingToServiceNpc = false;
	std::string m_strServiceNpcPlacementId;
	bool_t m_wasRightMouseDownForServiceNpcInteract = false;

	std::vector<float3_t> m_ShipNpcPositions;
	int32_t m_iWalkingToShipNpc = -1;
	bool_t m_wasRightMouseDownForShipNpcInteract = false;
	bool_t m_bShipCameraActive = false;
	/* Eased ship lens in project units; the zoom step is 0-based into shipCamera.zoomSteps. */
	struct SHIP_LENS final
	{
		f32_t distanceMeters = 0.f;
		f32_t pitchDegrees = 0.f;
		f32_t yawDegrees = 0.f;
		f32_t fovXDegrees = 60.f;
		f32_t focusOffsetYMeters = 0.f;
	};
	SHIP_LENS m_ShipLens{};
	uint32_t m_iShipZoomStep = 0u;
	bool_t m_bShipInAnchorVolume = false;

	VALTAN_CINEMATIC_CAMERA_CUE m_EntranceCameraCue;
	bool_t m_hasEntranceCameraCue = false;
	bool_t m_bEntranceCinematicApplied = false;
	bool_t m_bEntranceCinematicDone = false;
	f32_t m_fEntranceCinematicSeconds = 0.f;
	bool_t m_bEntranceRestoreFollowRequested = false;
	bool_t m_wasEscapeDownForEntranceSkip = false;
	weak_ptr<CTransform> m_pEntranceRestoreTarget;

#ifdef _DEBUG
	bool_t m_bDebugEntranceReplayRequested = false;
	bool_t m_bDebugEntranceReplayActive = false;
	std::string m_strDebugEntranceReplayStatus;
	std::vector<shared_ptr<CTrigger_Box>> m_DebugLevelChangeTriggers;
	bool_t m_bMapAuthoringActive = false;
	bool_t m_wasODownForValtanEntryDebugPreview = false;
#endif

	static CLevel_Bern* s_pActiveInstance;

public:
	static unique_ptr<CLevel_Bern> Create(
		ComPtr<ID3D11Device> pDevice,
		ComPtr<ID3D11DeviceContext> pContext);
};

NS_END
