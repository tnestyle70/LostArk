/* imgui.h defines its own placement-new helper and has to precede the project
headers, which is the same order Level_CharacterSelect.cpp uses. Previously
_DEBUG-only (the audition panel was its only user); the death-screen overlay
below is a real Release-build feature, so the include is no longer guarded. */
#include "imgui.h"

#include "Level_ValtanArena.h"

#include "Camera_Free.h"
#ifdef _DEBUG
#include "CameraTool.h"
#endif
#include "Character.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "UILayoutRuntime.h"
#include "UIInputRouter.h"
#include "ItemCatalog.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"
#include "Valtan.h"

#include "DataJson.h"
#include "DeployPropObject.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_set>

namespace
{
	// Each arena remembers its own chosen speed for this process session.
	f32_t g_ValtanFreeCameraSpeed = CCamera_Free::DEFAULT_ARENA_MOVE_SPEED;
	constexpr std::string_view VALTAN_PILLAR_SET_ID =
		"encounterprop.valtan.four-pillars";
	constexpr std::string_view VALTAN_STAGE_TWO_PLACEMENT_PREFIX =
		"spawn.valtan.stage02.miniboss.";
	constexpr std::string_view VALTAN_STAGE_TWO_ARCHETYPE_ID =
		"MINIBOSS_LUGARU";
	constexpr const wchar_t* VALTAN_BGM_M01_ASSET_ID =
		L"Sound/BGM/Valtan/M01_KeepGoing__992459057.wav";
	constexpr const wchar_t* VALTAN_BGM_M04_ASSET_ID =
		L"Sound/BGM/Valtan/M04_KeepGoing2__106505321.wav";
	constexpr const char* RAID_CLEAR_TEST_MODE_ENV =
		"LOSTARK_RAID_CLEAR_TEST_MODE";

	bool_t Is_RaidClearTestModeEnabled()
	{
		char* value = nullptr;
		size_t valueLength = 0u;
		if (0 != _dupenv_s(&value, &valueLength, RAID_CLEAR_TEST_MODE_ENV))
			return false;
		const bool_t enabled = nullptr != value && 2u == valueLength &&
			'1' == value[0];
		std::free(value);
		return enabled;
	}
	constexpr std::array<std::string_view, 4> VALTAN_PILLAR_SLOT_IDS = {
		"pillar.valtan.slot00", "pillar.valtan.slot01",
		"pillar.valtan.slot02", "pillar.valtan.slot03" };
	/* The Server currently owns the four inner repeatable slots. Keep this
	   slot-to-placement mapping stable when applying an encounter-prop sync. */
	/* The stele bursts instead of toppling, so its twelve pieces leave along
	   the full hemisphere around the upward axis rather than the narrow cone a
	   falling wall slab uses. */
	constexpr f32_t VALTAN_PILLAR_BURST_SPEED_METERS_PER_SECOND = 7.f;
	constexpr f32_t VALTAN_PILLAR_BURST_GRAVITY_SCALE = 2.f;
	constexpr f32_t VALTAN_PILLAR_BURST_LIFETIME_SECONDS = 4.f;
	constexpr f32_t VALTAN_GAMEPLAY_FOLLOW_LOOK_HEIGHT = 1.2f;
	constexpr f32_t VALTAN_GAMEPLAY_FOV_Y_DEGREES = 60.f;
	constexpr std::array<uint64_t, 4> VALTAN_PILLAR_SLOT_PLACEMENT_IDS = {
		14226635865317864635ull,
		14753860598629869201ull,
		16421721927631310369ull,
		13580356114054111471ull };
	/* All eight DEPLOY_ITR_02326 occurrences are encounter props, not permanent
	   arena dressing. Suppress every occurrence before the first rendered frame;
	   later authored trigger work can assign the remaining four stable IDs to
	   their own slots without restoring them at level load. */
	constexpr std::array<uint64_t, 8> VALTAN_ALL_PILLAR_PLACEMENT_IDS = {
		9528847514271106184ull,
		13580356114054111471ull,
		14224868475885080166ull,
		14226635865317864635ull,
		14351557328510348857ull,
		14753860598629869201ull,
		16068006371995901944ull,
		16421721927631310369ull };

	std::filesystem::path Find_ValtanWorldDataFile(
		const std::filesystem::path& fileName)
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
		if (0u == length || length >= std::size(modulePath))
			return {};

		const std::filesystem::path moduleDirectory =
			std::filesystem::path(modulePath).parent_path();
		const std::filesystem::path adjacent = moduleDirectory /
			L"DataFiles" / L"World" / fileName;
		if (std::filesystem::is_regular_file(adjacent))
			return adjacent;

		const std::filesystem::path parent = moduleDirectory.parent_path() /
			L"DataFiles" / L"World" / fileName;
		return std::filesystem::is_regular_file(parent) ? parent : adjacent;
	}

	/* A refused level activation drops the player back to the Lobby with the
	   socket already closed, which looks exactly like a dropped connection.
	   Every refusal therefore hands the Lobby the stage that actually said no,
	   not just E_FAIL. */
	HRESULT Report_InitFailure(
		const char_t* pStage,
		const std::string& status)
	{
		const std::string detail = std::string(pStage) + " " + status;
		OutputDebugStringA((detail + "\n").c_str());
		Client::CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_ACTIVATION_LEVEL_CREATE_FAILED,
			"level-valtan.initialize",
			detail,
			E_FAIL);
		return E_FAIL;
	}

	/* Every slot m_pRaidClearView owns while showing (RaidClear_TitleTextBox is a separate,
	always-Set_SlotVisible(false) marker -- see Update_RaidClear). Shared by Initialize() (hides
	them before this Level's very first Render(), which the engine's activation-frame ordering can
	call before this Level's own first Update() -- otherwise every one of these full-opacity
	default-visible slots, BgFlash included, flashes across the whole screen for that frame) and
	Update_RaidClear's own real per-frame visibility/alpha drive. */
	constexpr const char* const RAIDCLEAR_FADING_SLOTS[] = {
		"RaidClear_BgFlash", "RaidClear_avtive02", "RaidClear_CoreShine",
		"RaidClear_particleLooping", "RaidClear_particleLighting",
		"RaidClear_lineLeft", "RaidClear_lineRight", "RaidClear_Emblem",
	};
}

CLevel_ValtanArena* CLevel_ValtanArena::s_pActiveInstance = nullptr;

CLevel_ValtanArena::CLevel_ValtanArena(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
	s_pActiveInstance = this;
}

CLevel_ValtanArena::~CLevel_ValtanArena()
{
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	CCameraTool::Clear_ActorPreviewContext(
		ETOUI(LEVEL::VALTAN_ARENA));
	End_ReferenceCamera(false);
#endif
	End_CinematicCamera();
	m_MapEffectPresentationRuntime.Clear();
	if (RAID_PRELUDE_BGM_STATE::NONE != m_eRaidPreludeBgmState)
		CGameInstance::Get().Stop_Music();
	m_Replication.Reset();
	m_WorldDestructionDebrisPresentationRuntime.Clear();
	m_WorldDestructionDebrisPresentationDocument.Clear();
	m_WorldDestructionProjectionDocument.Clear();
}

HRESULT CLevel_ValtanArena::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_PartyInteraction.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA));

	const CLIENT_LEVEL_DESCRIPTOR* pEntry =
		CLevelRegistry::Find(LEVEL::VALTAN_ARENA);
	if (nullptr == pEntry || nullptr == pEntry->pMapAreaId ||
		!m_MapRuntime.Load_Area(
			ETOUI(LEVEL::VALTAN_ARENA),
			pEntry->pMapAreaId,
			pEntry->MapLoadScope))
	{
		return Report_InitFailure("[Level_ValtanArena][MapArea]",
			nullptr == pEntry || nullptr == pEntry->pMapAreaId ?
				"Valtan level descriptor has no map area" :
				m_MapRuntime.Get_Status());
	}
#ifdef _DEBUG
	/* A placement that misses its static batch becomes its own draw, so the
	   fallback count is what separates a heavy arena from a broken one. */
	OutputDebugStringA(("[Level_ValtanArena][MapArea] " +
		m_MapRuntime.Get_Status() + "\n").c_str());
#endif
	if (!m_DeployRuntime.Load_Area(
		ETOUI(LEVEL::VALTAN_ARENA),
		pEntry->pMapAreaId))
	{
		m_MapRuntime.Clear();
		return Report_InitFailure("[Level_ValtanArena][DeployProp]",
			m_DeployRuntime.Get_Status());
	}
	/* The authored deploy catalog contains all eight reusable pillars so later
	   triggers can raise the intended set. Their product initial state is
	   HIDDEN, therefore suppress every occurrence before the first rendered
	   Valtan frame rather than waiting for a Server sync to arrive. */
	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> hiddenPillars;
	hiddenPillars.reserve(VALTAN_ALL_PILLAR_PLACEMENT_IDS.size());
	for (const uint64_t placementId : VALTAN_ALL_PILLAR_PLACEMENT_IDS)
		hiddenPillars.emplace_back(placementId, DEPLOY_PROP_STATE::DESPAWNED);
	if (!m_DeployRuntime.Set_States(hiddenPillars))
	{
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure("[Level_ValtanArena][EncounterProps]",
			m_DeployRuntime.Get_Status());
	}
	std::string destructionStatus;
	if (!m_WorldDestructionProjectionDocument.Load(
		Find_ValtanWorldDataFile(
			L"LV_LUT_HEARTRB_ED.worlddestruction.json"), destructionStatus) ||
		m_WorldDestructionProjectionDocument.Get_AreaId() !=
			pEntry->pMapAreaId)
	{
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure("[Level_ValtanArena][WorldDestruction]",
			destructionStatus);
	}
	if (!m_WorldDestructionDebrisPresentationDocument.Load(
		Find_ValtanWorldDataFile(
			L"LV_LUT_HEARTRB_ED.worlddestructionpresentation.json"),
		destructionStatus) ||
		!m_WorldDestructionDebrisPresentationDocument.Validate_Against(
			m_WorldDestructionProjectionDocument, destructionStatus) ||
		m_WorldDestructionDebrisPresentationDocument.Get_AreaId() !=
			pEntry->pMapAreaId)
	{
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure(
			"[Level_ValtanArena][DestructionDebrisDocument]",
			destructionStatus);
	}
	if (!m_WorldDestructionDebrisPresentationRuntime.Initialize(
		ETOUI(LEVEL::VALTAN_ARENA), m_pDevice, m_pContext,
		m_DeployRuntime, destructionStatus))
	{
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		return Report_InitFailure(
			"[Level_ValtanArena][DestructionDebrisRuntime]",
			destructionStatus);
	}
	auto mapLightPresentation = make_shared<CMapLightPresentationRuntime>();
	if (!mapLightPresentation->Load_Runtime(pEntry->pMapAreaId))
	{
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return Report_InitFailure("[Level_ValtanArena][MapLight]",
			mapLightPresentation->Get_Status());
	}
	m_pMapLightPresentation = std::move(mapLightPresentation);
	if (!Ready_CinematicCamera())
	{
		m_MapEffectPresentationRuntime.Clear();
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return E_FAIL;
	}

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
	{
		m_MapEffectPresentationRuntime.Clear();
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return Report_InitFailure("[Level_ValtanArena][Camera]",
			"Camera layer could not be created");
	}

	CClientReplication::DESC replicationDesc{};
	replicationDesc.pDevice = m_pDevice;
	replicationDesc.pContext = m_pContext;
	replicationDesc.iPrototypeLevelIndex =
		ETOUI(LEVEL::VALTAN_ARENA);
	replicationDesc.iLayerLevelIndex =
		ETOUI(LEVEL::VALTAN_ARENA);
	replicationDesc.strMapAreaId = pEntry->pMapAreaId;
	replicationDesc.strPlayerLayerTag =
		TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag =
		TEXT("Layer_WorldEntity");
	replicationDesc.pDeployPropRuntime = &m_DeployRuntime;
	replicationDesc.pWorldDestructionProjection =
		&m_WorldDestructionProjectionDocument;
	replicationDesc.onWorldEntityDespawned =
		[this](const std::string_view placementId,
			const std::string_view archetypeId)
		{
			Handle_WorldEntityDespawned(placementId, archetypeId);
		};
	if (!m_Replication.Initialize(replicationDesc))
	{
		m_MapEffectPresentationRuntime.Clear();
		m_pMapLightPresentation->Clear();
		m_pMapLightPresentation.reset();
		m_WorldDestructionDebrisPresentationRuntime.Clear();
		m_DeployRuntime.Clear();
		m_MapRuntime.Clear();
		m_WorldDestructionDebrisPresentationDocument.Clear();
		m_WorldDestructionProjectionDocument.Clear();
		return Report_InitFailure("[Level_ValtanArena][Replication]",
			"Client replication could not be initialized");
	}

	m_pPlayerCommandSink = make_shared<CNetworkPlayerCommandSink>();
	m_PlayerController.Set_CommandSink(m_pPlayerCommandSink);
	if (!m_PlayerController.Initialize_TargetingPreview(
			ETOUI(LEVEL::VALTAN_ARENA)))
	{
		return Report_InitFailure("[Level_ValtanArena][TargetingPreview]",
			"Ground-target preview object could not be initialized");
	}
	if (!m_PlayerController.Initialize_ClickMoveEffect(
			ETOUI(LEVEL::VALTAN_ARENA)))
	{
		return Report_InitFailure("[Level_ValtanArena][ClickMoveEffect]",
			"Click-move effect object could not be initialized");
	}

	/* A slot's authored default (no Set_SlotVisible call yet) is visible -- so on this Level's
	activation frame, if the engine's own Update-then-Render ordering ends up calling this Level's
	Render() before its first Update() (the transition-frame case that actually happens: the
	previous Level's Update() already ran this frame when Change_Level swaps the active Level, so
	the newly-activated one's Render() runs with zero Update() calls behind it yet), every one of
	these slots -- DeadScene's panel/effect/buttons, RaidClear's screen-covering BgFlash included
	-- would flash at full opacity for that frame before Update_DeadScene/Update_RaidClear ever
	gets to hide them for real. Explicitly hiding them the instant each view loads closes that gap
	regardless of which order those two calls happen to land in on any given frame. */
	m_pDeadSceneView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA), TEXT("Layer_UI"),
		L"UI/DeadScene/DeadSceneUI.json");
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Dim", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_PanelBg", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_WingedArch", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Effect", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveButton", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_SpectateButton", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_SpectateBorder", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_TitleTextMarker", false);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveMessageMarker", false);

	m_pRaidClearView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA), TEXT("Layer_UI"),
		L"UI/RaidClear/RaidClear_Layout.json");
	m_pRaidClearView->Set_SlotVisible("RaidClear_Dim", false);
	for (const char* const szSlotId : RAIDCLEAR_FADING_SLOTS)
		m_pRaidClearView->Set_SlotVisible(szSlotId, false);
	m_pRaidClearView->Set_SlotVisible("RaidClear_TitleTextBox", false);
	m_pRaidClearView->Set_SlotVisible("RaidClear_ReturnButton", false);

	/* First screen migrated off the ImGui interim UI rendering (see
	.md/TJ/08-31/2026-08-31_ImGui_런타임UI_전환_PLAN.md) -- real CUI_Sprite GameObjects on this
	Level's own new "Layer_UI" instead of CHUDRuntimeView's ImGui foreground-drawlist draws.
	Get_SlotRect/Set_SlotVisible/Set_SlotTexture below are unchanged calls; only the type and
	construction differ. */
	m_pItemAnnounceView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA), TEXT("Layer_UI"),
		L"UI/ItemAnnounce/ItemAnnounce_Layout.json");
	m_pItemAnnounceView->Set_SlotVisible("ItemAnnounce_Frame", false);
	m_pItemAnnounceView->Set_SlotVisible("ItemAnnounce_Icon", false);

	Transition_RaidPreludeBgm(RAID_PRELUDE_BGM_STATE::M01_PROGRESS);
	return S_OK;
}

void CLevel_ValtanArena::Transition_RaidPreludeBgm(
	const RAID_PRELUDE_BGM_STATE nextState)
{
	if (m_eRaidPreludeBgmState == nextState)
		return;

	const wchar_t* pAssetId = nullptr;
	switch (nextState)
	{
	case RAID_PRELUDE_BGM_STATE::M01_PROGRESS:
		pAssetId = VALTAN_BGM_M01_ASSET_ID;
		break;
	case RAID_PRELUDE_BGM_STATE::M04_POST_MINIBOSS:
		pAssetId = VALTAN_BGM_M04_ASSET_ID;
		break;
	case RAID_PRELUDE_BGM_STATE::NONE:
		CGameInstance::Get().Stop_Music();
		m_eRaidPreludeBgmState = nextState;
		return;
	default:
		return;
	}

	m_eRaidPreludeBgmState = nextState;
	const std::filesystem::path musicPath =
		CRuntimeAssetRoot::Resolve(pAssetId);
	if (musicPath.empty() || !std::filesystem::is_regular_file(musicPath) ||
		FAILED(CGameInstance::Get().Play_Music(
			musicPath.wstring(), 1.f, true)))
	{
#ifdef _DEBUG
		OutputDebugStringA(
			"[Level_ValtanArena] Prelude BGM transition was isolated because "
			"the runtime WAV could not be played.\n");
#endif
	}
}

void CLevel_ValtanArena::Handle_WorldEntityDespawned(
	const std::string_view placementId,
	const std::string_view archetypeId)
{
	if (RAID_PRELUDE_BGM_STATE::M01_PROGRESS !=
			m_eRaidPreludeBgmState ||
		VALTAN_STAGE_TWO_ARCHETYPE_ID != archetypeId ||
		0u != placementId.rfind(VALTAN_STAGE_TWO_PLACEMENT_PREFIX, 0u))
	{
		return;
	}

	/* stage02.miniboss owns exactly one Lugaru and completes under
	ALL_WAVES_CLEARED. Its reliable Server despawn is therefore the visible
	stage-two completion edge; no Client timer or position guess is involved. */
	Transition_RaidPreludeBgm(
		RAID_PRELUDE_BGM_STATE::M04_POST_MINIBOSS);
}

void CLevel_ValtanArena::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
			LEVEL::VALTAN_ARENA))
	{
#ifdef _DEBUG
		End_ReferenceCamera(false);
#endif
		End_CinematicCamera();
		return;
	}
	if (nullptr != m_pMapLightPresentation &&
		!m_pMapLightPresentation->Submit_Frame() &&
		!m_bMapLightSubmissionFailureReported)
	{
		m_bMapLightSubmissionFailureReported = true;
		OutputDebugStringA(("[Level_ValtanArena][MapLight] " +
			m_pMapLightPresentation->Get_Status() + "\n").c_str());
	}

	const bool_t replicationUpdated = m_Replication.Update();
	if (!replicationUpdated)
	{
		OutputDebugStringA(
			"[Level_ValtanArena] Failed to apply replication event.\n");
		if (m_Replication.Has_FatalWorldDestructionFailure())
		{
			/* This bounce closes the socket and returns to the Lobby without
			going through the load-failure path, so the reason has to be handed
			over explicitly or the Lobby shows nothing at all. */
			std::string presentationStatus;
			if (!m_Replication.Try_Consume_PresentationFailure(
				presentationStatus))
			{
				presentationStatus =
					"World destruction projection rejected the Server sync.";
			}
			CLevelTransitionService::Report_Recovery(
				LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
					CLIENT_REPLICATION_FAILED,
				"level-valtan.world-destruction-sync",
				"[Level_ValtanArena][WorldDestructionSync] " +
					presentationStatus,
				E_FAIL);
#ifdef _DEBUG
			End_ReferenceCamera(false);
#endif
			End_CinematicCamera();
			CNetworkManager::Get().Close_ServerConnection();
			if (!CLevelTransitionService::Request_Load(
				LEVEL::LOBBY,
				"world-destruction-projection-failed"))
			{
				OutputDebugStringA(
					"[Level_ValtanArena] World destruction recovery request was rejected; retrying.\n");
			}
			return;
		}
	}
	if (m_Replication.Has_PendingConnectionLoss())
	{
#ifdef _DEBUG
		End_ReferenceCamera(false);
#endif
		End_CinematicCamera();
		CLevelTransitionService::Report_NetworkRecovery(
			"level-valtan.network-connection-lost",
			"Valtan replication observed a disconnected Server session.");
		CNetworkManager::Get().Close_ServerConnection();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_ValtanArena] Lobby recovery request was rejected; retrying.\n");
		return;
	}
	if (!Apply_EncounterPropPresentation())
	{
		CLevelTransitionService::Report_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::
				CLIENT_REPLICATION_FAILED,
			"level-valtan.encounter-prop-sync",
			"[Level_ValtanArena][EncounterPropSync] " +
				m_DeployRuntime.Get_Status(),
			E_FAIL);
#ifdef _DEBUG
		End_ReferenceCamera(false);
#endif
		End_CinematicCamera();
		CNetworkManager::Get().Close_ServerConnection();
		(void)CLevelTransitionService::Request_Load(
			LEVEL::LOBBY, "encounter-prop-projection-failed");
		return;
	}

#ifdef _DEBUG
	Update_AuditionTransaction();
#endif
	Update_WorldDestructionPresentation(fTimeDelta);
	Bind_CameraToLocalCharacter();
#ifdef _DEBUG
	Update_ReferenceCamera();
#endif
	Update_CinematicCamera(fTimeDelta);
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	bool_t cameraAcceptsGameplay =
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled() &&
		!m_bCinematicCameraApplied;
#ifdef _DEBUG
	cameraAcceptsGameplay = cameraAcceptsGameplay &&
		!m_bReferenceCameraApplied;
#endif
	/* Has to run before m_PlayerController.Update() so a right-click that
	hits another player is not also spent as that frame's move command --
	same reasoning as Level_Bern's own Valtan-entry NPC click. Needs this
	frame's replicated player list, so Collect_PlayerViews moves here
	instead of Render(). */
	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	/* worldInteractionAllowed=false: the right-click-a-player invite context menu
	is Bern-only by design (party formation happens before a Valtan entry, not
	mid-fight) -- right-click-moving past a teammate during combat kept opening
	it by accident. This only disables that trigger; an already-pending incoming
	invite (Try_Consume_PartyInviteReceived, inside Update() itself) is unrelated
	and still shown regardless. */
	/* Raid Clear is the topmost product modal. Update it before any gameplay or
	   lower-priority product interaction so its click cannot also become an
	   attack, movement command, revive, or party action in this frame. */
	Update_RaidClear(fTimeDelta);
	const bool_t isRaidClearActive = m_fRaidClearElapsedSeconds >= 0.f;
	if (!isRaidClearActive && m_PartyInteraction.Update(
		m_Replication, m_pPlayerCommandSink, m_NameplatePlayers,
		false))
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	if (isRaidClearActive)
	{
		CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, true);
		CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, true);
	}
	bool_t debugPlacementEnabled =
		nullptr != m_pCamera && !m_pCamera->Is_FollowRequested() &&
		!m_pCamera->Is_PresentationOverrideActive() &&
		!m_bCinematicCameraApplied && !isRaidClearActive;
#ifdef _DEBUG
	debugPlacementEnabled = debugPlacementEnabled &&
		!m_bReferenceCameraApplied;
#endif
	m_PlayerController.Update(
		cameraAcceptsGameplay && !isRaidClearActive, debugPlacementEnabled);
	Update_DeadScene(isRaidClearActive, fTimeDelta);
	Update_ItemAnnounce(fTimeDelta);
#ifdef _DEBUG
	Update_DebugRaidClearKey();
#endif
}

bool_t CLevel_ValtanArena::Try_Get_AuthoringPreviewPlacement(
	float3_t& OutPosition,
	std::string& strOutSource) const
{
	OutPosition = {};
	strOutSource.clear();

	/* Screen-right keeps the clone next to the actor from the current camera's
	   point of view instead of using world +X. Flattening it prevents a pitched
	   raid camera from moving the model above/below the floor. */
	vector_t vScreenRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	if (nullptr != m_pCamera)
	{
		const shared_ptr<CTransform> pCameraTransform =
			dynamic_pointer_cast<CTransform>(
				m_pCamera->Get_Component(g_strTransformComTag));
		if (nullptr != pCameraTransform)
		{
			vector_t vCandidate = pCameraTransform->Get_State(STATE::RIGHT);
			vCandidate = XMVectorSetY(vCandidate, 0.f);
			vCandidate = XMVectorSetW(vCandidate, 0.f);
			if (XMVectorGetX(XMVector3LengthSq(vCandidate)) > 0.000001f)
				vScreenRight = XMVector3Normalize(vCandidate);
		}
	}

	const shared_ptr<CCharacter> pLocalCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr != pLocalCharacter &&
		nullptr != pLocalCharacter->Get_Transform())
	{
		const vector_t vPlayerPosition =
			pLocalCharacter->Get_Transform()->Get_State(STATE::POSITION);
		constexpr f32_t PREVIEW_OFFSET_METERS = 3.25f;
		const array<f32_t, 2u> Directions = { 1.f, -1.f };
		for (const f32_t fDirection : Directions)
		{
			const vector_t vCandidate = XMVectorSetW(
				vPlayerPosition +
				vScreenRight * (PREVIEW_OFFSET_METERS * fDirection),
				1.f);
			float3_t Candidate{};
			XMStoreFloat3(&Candidate, vCandidate);
			float3_t Sampled{};
			if (pLocalCharacter->Try_SampleTargetGround(
					Candidate.x, Candidate.z, Sampled))
			{
				OutPosition = Sampled;
				strOutSource = fDirection > 0.f ?
					"replicated local player / camera-right / Navigation" :
					"replicated local player / camera-left / Navigation";
				return true;
			}
		}

		/* A missing Client Navigation component must not make the explicitly
		   requested Model View disappear. The replicated player point is still a
		   valid arena anchor; only the optional floor clamp was unavailable. */
		const vector_t vFallback = XMVectorSetW(
			vPlayerPosition + vScreenRight * PREVIEW_OFFSET_METERS, 1.f);
		XMStoreFloat3(&OutPosition, vFallback);
		strOutSource =
			"replicated local player / camera-right / unclamped fallback";
		return true;
	}

	const VALTAN_PRESENTATION_STATE& Boss =
		m_Replication.Get_ValtanPresentationState();
	if (Boss.isValid &&
		std::isfinite(Boss.vPosition.x) &&
		std::isfinite(Boss.vPosition.y) &&
		std::isfinite(Boss.vPosition.z))
	{
		const vector_t vBossPosition = XMLoadFloat3(&Boss.vPosition);
		const vector_t vFallback = XMVectorSetW(
			vBossPosition + vScreenRight * 4.5f, 1.f);
		XMStoreFloat3(&OutPosition, vFallback);
		strOutSource =
			"primary replicated Valtan / camera-right fallback";
		return true;
	}

	strOutSource =
		"waiting for the replicated local player or primary Valtan";
	return false;
}

bool_t CLevel_ValtanArena::Reload_PrimaryValtanPresentationAuthoring(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	return m_Replication.Reload_PrimaryValtanPresentationAuthoring(
		ExpectedRevision, strOutStatus);
}

bool_t CLevel_ValtanArena::Reload_PrimaryValtanCombatObjectSoundCues(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus)
{
	return m_Replication.Reload_PrimaryValtanCombatObjectSoundCues(
		ExpectedRevision, strOutStatus);
}

bool_t CLevel_ValtanArena::Can_Play_PrimaryValtanPresentation(
	const LostArk::Shared::GameplayDataRevision& ExpectedRevision,
	std::string& strOutStatus) const
{
	return m_Replication.Can_Play_PrimaryValtanPresentation(
		ExpectedRevision, strOutStatus);
}

bool_t CLevel_ValtanArena::Get_PrimaryValtanPatternSoundSourceReceipt(
	VALTAN_PATTERN_SOUND_SOURCE_RECEIPT& OutReceipt,
	std::string& strOutStatus) const
{
	return m_Replication.Get_PrimaryValtanPatternSoundSourceReceipt(
		OutReceipt, strOutStatus);
}

#ifdef _DEBUG
namespace
{
	constexpr uint64_t AUDITION_RETRY_INTERVAL_MILLISECONDS = 750u;
	constexpr uint32_t AUDITION_MAX_RETRY_COUNT = 3u;
	/* This owner is outside the uint32 Server entity range, so a Debug reference
	view can never impersonate the owner of an authoritative cinematic cue. */
	constexpr uint64_t VALTAN_REFERENCE_CAMERA_OWNER_ID =
		0x56414C54414E5246ull;
	constexpr std::string_view VALTAN_REFERENCE_PHASE_SOURCE_LEVEL =
		"DEBUG_REFERENCE_VALTAN_PHASE_SPACEHOLE";
	constexpr size_t VALTAN_REFERENCE_PHASE_PLACEMENT_COUNT = 3u;

	struct VALTAN_REFERENCE_CAMERA_POSE final
	{
		float3_t vEye;
		float3_t vLookAt;
		f32_t fFovYDegrees = 60.f;
	};

	/* Stable comparison poses for the two supplied references. The top-down
	shot keeps a small Z offset so LookAt never becomes parallel to world-up. */
	const VALTAN_REFERENCE_CAMERA_POSE VALTAN_REFERENCE_TOP_DOWN = {
		float3_t(156.03f, 132.f, -111.f),
		float3_t(156.03f, 23.f, -122.06f),
		48.f };
	const VALTAN_REFERENCE_CAMERA_POSE VALTAN_REFERENCE_EXTERIOR = {
		float3_t(156.03f, 96.f, -18.f),
		float3_t(156.03f, 23.f, -122.06f),
		54.f };

	uint64_t Get_AuditionMonotonicMilliseconds()
	{
		return static_cast<uint64_t>(GetTickCount64());
	}

	const char_t* Describe_AuditionResult(
		const LostArk::Shared::VALTAN_AUDITION_RESULT result)
	{
		using LostArk::Shared::VALTAN_AUDITION_RESULT;
		switch (result)
		{
		case VALTAN_AUDITION_RESULT::ARMED:
			return "Armed one bar above the target. Press Cross to play it.";
		case VALTAN_AUDITION_RESULT::QUEUED:
			return "The Server queued the requested arena preset.";
		case VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED:
			return "Already handled that request; treating it as confirmed.";
		case VALTAN_AUDITION_RESULT::REJECTED_RELEASE_BUILD:
			return "Release Server: auditions are Debug-only.";
		case VALTAN_AUDITION_RESULT::REJECTED_WRONG_WORLD:
			return "This session is not a member of the Valtan room.";
		case VALTAN_AUDITION_RESULT::REJECTED_NO_BOSS:
			return "No Valtan is active. Enter the encounter trigger first.";
		case VALTAN_AUDITION_RESULT::REJECTED_BOSS_DEAD:
			return "Valtan is dead. Leave and re-enter to reset the room.";
		case VALTAN_AUDITION_RESULT::REJECTED_UNKNOWN_HEALTH_BAR:
			return "That bar carries no authored pattern.";
		case VALTAN_AUDITION_RESULT::REJECTED_PATTERN_UNAVAILABLE:
			return "The Server rejected this arena preset.";
		case VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED:
			return "Arm the same bar before crossing it.";
		case VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED:
			return "Move into engage range and act once, then retry.";
		default:
			return "Unknown audition verdict.";
		}
	}
}

bool_t CLevel_ValtanArena::Begin_ReferenceCamera(
	const REFERENCE_CAMERA_VIEW view)
{
	if (nullptr == m_pCamera || REFERENCE_CAMERA_VIEW::NONE == view ||
		m_bCinematicCameraApplied ||
		m_ValtanCinematicCameraController.Is_Active())
	{
		return false;
	}

	const VALTAN_REFERENCE_CAMERA_POSE* pose = nullptr;
	switch (view)
	{
	case REFERENCE_CAMERA_VIEW::TOP_DOWN:
		pose = &VALTAN_REFERENCE_TOP_DOWN;
		break;
	case REFERENCE_CAMERA_VIEW::EXTERIOR:
		pose = &VALTAN_REFERENCE_EXTERIOR;
		break;
	default:
		return false;
	}
	const bool_t showSpaceHole =
		REFERENCE_CAMERA_VIEW::TOP_DOWN == view;
	if (!Set_ReferencePhaseProxyVisible(showSpaceHole))
		return false;

	if (!m_bReferenceCameraApplied)
	{
		m_bReferenceCameraRestoreFollowRequested =
			m_pCamera->Is_FollowRequested();
		m_pReferenceCameraRestoreTarget = m_pCamera->Get_FollowTarget();
		/* Keep the target bound so the existing F6 shortcut remains meaningful,
		but suspend follow while the fixed presentation pose is active. */
		m_pCamera->Set_FollowEnabled(false);
		if (!m_pCamera->Begin_PresentationOverride(
			VALTAN_REFERENCE_CAMERA_OWNER_ID,
			CCamera::PRESENTATION_PRIORITY::REFERENCE_AUDITION))
		{
			m_pCamera->Set_FollowTarget(
				m_pReferenceCameraRestoreTarget.lock());
			m_pCamera->Set_FollowEnabled(
				m_bReferenceCameraRestoreFollowRequested);
			m_pReferenceCameraRestoreTarget.reset();
			m_bReferenceCameraRestoreFollowRequested = false;
			(void)Set_ReferencePhaseProxyVisible(false);
			return false;
		}
		m_bReferenceCameraApplied = true;
	}

	if (!m_pCamera->Apply_PresentationPose(
		VALTAN_REFERENCE_CAMERA_OWNER_ID,
		pose->vEye, pose->vLookAt, pose->fFovYDegrees))
	{
		End_ReferenceCamera(false);
		return false;
	}
	m_eReferenceCameraView = view;
	return true;
}

bool_t CLevel_ValtanArena::Set_ReferencePhaseProxyVisible(
	const bool_t visible)
{
	const std::string sourceLevel(VALTAN_REFERENCE_PHASE_SOURCE_LEVEL);
	const bool_t applied = visible ?
		m_MapRuntime.Set_DebugSourceLevelVisible(
			sourceLevel, true, VALTAN_REFERENCE_PHASE_PLACEMENT_COUNT) :
		m_MapRuntime.Restore_DebugSourceLevelVisibility(
			sourceLevel, VALTAN_REFERENCE_PHASE_PLACEMENT_COUNT);
	if (!applied)
	{
		OutputDebugStringA(
			"[Level_ValtanArena][ReferenceCamera] Debug-only SpaceHole proxy visibility failed.\n");
		return false;
	}
	m_bReferenceSpaceHoleVisible = visible;
	return true;
}

void CLevel_ValtanArena::Update_ReferenceCamera()
{
	if (!m_bReferenceCameraApplied)
	{
		if (m_bReferenceSpaceHoleVisible)
			(void)Set_ReferencePhaseProxyVisible(false);
		return;
	}
	if (nullptr == m_pCamera)
	{
		if (m_bReferenceSpaceHoleVisible)
			(void)Set_ReferencePhaseProxyVisible(false);
		m_pReferenceCameraRestoreTarget.reset();
		m_bReferenceCameraRestoreFollowRequested = false;
		m_bReferenceCameraApplied = false;
		m_eReferenceCameraView = REFERENCE_CAMERA_VIEW::NONE;
		return;
	}

	/* F6 remains the one follow/free shortcut. While a reference pose owns the
	camera, the same press dismisses it and applies the toggle the user asked for. */
	if (GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantTextInput &&
		!CUIInputRouter::Get().Is_TextInputActive() &&
		CGameInstance::Get().Get_DIKeyPressed(DIK_F6))
	{
		End_ReferenceCamera(true);
	}
}

void CLevel_ValtanArena::End_ReferenceCamera(
	const bool_t toggleFollowRequested)
{
	if (m_bReferenceSpaceHoleVisible)
		(void)Set_ReferencePhaseProxyVisible(false);
	if (!m_bReferenceCameraApplied)
		return;

	const bool_t restoreFollowRequested =
		toggleFollowRequested ?
			!m_bReferenceCameraRestoreFollowRequested :
			m_bReferenceCameraRestoreFollowRequested;
	const shared_ptr<CTransform> restoreTarget =
		m_pReferenceCameraRestoreTarget.lock();
	if (nullptr != m_pCamera)
	{
		(void)m_pCamera->End_PresentationOverride(
			VALTAN_REFERENCE_CAMERA_OWNER_ID);
		m_pCamera->Set_FollowTarget(restoreTarget);
		m_pCamera->Set_FollowEnabled(restoreFollowRequested);
	}
	m_pReferenceCameraRestoreTarget.reset();
	m_bReferenceCameraRestoreFollowRequested = false;
	m_bReferenceCameraApplied = false;
	m_eReferenceCameraView = REFERENCE_CAMERA_VIEW::NONE;
}

const char_t* CLevel_ValtanArena::Get_ReferenceCameraViewName() const
{
	switch (m_eReferenceCameraView)
	{
	case REFERENCE_CAMERA_VIEW::TOP_DOWN:
		return "Top Down";
	case REFERENCE_CAMERA_VIEW::EXTERIOR:
		return "Exterior";
	default:
		return "none";
	}
}

void CLevel_ValtanArena::Update_AuditionTransaction()
{
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT result{};
	while (CNetworkManager::Get().Try_Consume_ValtanAuditionResult(result))
	{
		if (!m_PendingAuditionRequest.Is_Active() ||
			result.iRequestSequence != m_PendingAuditionRequest.iSequence)
		{
			continue;
		}
		if (result.eOperation != m_PendingAuditionRequest.eOperation ||
			result.iTargetHealthBar !=
				m_PendingAuditionRequest.iTargetHealthBar)
		{
			/* A sequence is not enough to identify a verdict. Keep waiting for
			the exact echoed request instead of completing the wrong UI action. */
			m_strAuditionStatus =
				"Ignored a mismatched Server verdict; retrying the exact request.";
			continue;
		}

		m_PendingAuditionRequest = {};
		m_strAuditionStatus = Describe_AuditionResult(result.eResult);
	}

	if (!m_PendingAuditionRequest.Is_Active())
		return;
	const uint64_t now = Get_AuditionMonotonicMilliseconds();
	if (now - m_PendingAuditionRequest.iLastSentAtMilliseconds <
		AUDITION_RETRY_INTERVAL_MILLISECONDS)
	{
		return;
	}

	if (m_PendingAuditionRequest.iRetryCount >= AUDITION_MAX_RETRY_COUNT)
	{
		m_PendingAuditionRequest = {};
		m_strAuditionStatus =
			"Server verdict timed out after three bounded retries.";
		return;
	}

	++m_PendingAuditionRequest.iRetryCount;
	m_PendingAuditionRequest.iLastSentAtMilliseconds = now;
	const bool_t sent = CNetworkManager::Get().Send_ValtanAudition(
		m_PendingAuditionRequest.iSequence,
		m_PendingAuditionRequest.eOperation,
		m_PendingAuditionRequest.iTargetHealthBar);
	m_strAuditionStatus = sent ?
		"Waiting for the Server verdict after a bounded retry..." :
		"The bounded retry could not be sent; it will not retry immediately.";
}

bool_t CLevel_ValtanArena::Submit_Audition(
	const LostArk::Shared::VALTAN_AUDITION_OPERATION operation,
	const uint32_t explicitCommandPayload)
{
	if (nullptr == m_Replication.Get_LocalCharacter())
	{
		m_strAuditionStatus =
			"Wait for the local character snapshot before sending an audition.";
		return false;
	}
	if (m_PendingAuditionRequest.Is_Active())
	{
		m_strAuditionStatus =
			"Another audition transaction must finish before this request.";
		return false;
	}

	/* The old Level-owned Pattern/health-bar/timeline browser was removed. This
	   bounded receipt owner remains solely for the visible Server arena presets;
	   Product Pattern playback must go through ValtanBossTool's stable-ID revision CAS. */
	if (LostArk::Shared::VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET !=
		operation)
	{
		m_strAuditionStatus =
			"Legacy Valtan Pattern audition controls are retired; use Complete Play.";
		return false;
	}
	if (explicitCommandPayload < static_cast<uint32_t>(
			LostArk::Shared::VALTAN_ARENA_PRESET::FRESH) ||
		explicitCommandPayload >= static_cast<uint32_t>(
			LostArk::Shared::VALTAN_ARENA_PRESET::END))
	{
		m_strAuditionStatus = "Arena preset identity is invalid.";
		return false;
	}

	const uint32_t sequence = 0u == m_iNextAuditionRequestSequence ?
		1u : m_iNextAuditionRequestSequence;
	if (!CNetworkManager::Get().Send_ValtanAudition(
		sequence, operation, explicitCommandPayload))
	{
		m_strAuditionStatus = "Could not send the audition request.";
		return false;
	}
	m_iNextAuditionRequestSequence =
		(std::numeric_limits<uint32_t>::max)() == sequence ?
		1u : sequence + 1u;
	m_PendingAuditionRequest.iSequence = sequence;
	m_PendingAuditionRequest.eOperation = operation;
	m_PendingAuditionRequest.iTargetHealthBar = explicitCommandPayload;
	m_PendingAuditionRequest.iLastSentAtMilliseconds =
		Get_AuditionMonotonicMilliseconds();
	m_PendingAuditionRequest.iRetryCount = 0u;
	m_strAuditionStatus = "Waiting for the Server verdict...";
	return true;
}

bool_t CLevel_ValtanArena::Set_ArenaPreset(
	const LostArk::Shared::VALTAN_ARENA_PRESET preset,
	std::string& outStatus)
{
	const bool_t submitted = Submit_Audition(
		LostArk::Shared::VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET,
		static_cast<uint32_t>(preset));
	outStatus = m_strAuditionStatus;
	return submitted;
}

bool_t CLevel_ValtanArena::Is_ArenaPresetRequestPending() const
{
	return m_PendingAuditionRequest.Is_Active() &&
		LostArk::Shared::VALTAN_AUDITION_OPERATION::SET_ARENA_PRESET ==
			m_PendingAuditionRequest.eOperation;
}

CLevel_ValtanArena::ARENA_ACTIVE_STATE
CLevel_ValtanArena::Get_ArenaActiveState() const
{
	ARENA_ACTIVE_STATE snapshot{};
	snapshot.bSynchronized =
		m_Replication.Is_WorldDestructionSynchronized();
	static constexpr std::string_view OUTER_RING_GROUP_PREFIX =
		"destroyable.group.valtan.outerwall109.";
	static constexpr std::string_view THREE_OCLOCK_GROUP_PREFIX =
		"destroyable.group.valtan.floor84.";
	static constexpr std::string_view NINE_OCLOCK_GROUP_PREFIX =
		"destroyable.group.valtan.floor30.";
	uint32_t ordinaryActive = 0u;
	uint32_t outerActive = 0u;
	uint32_t threeActive = 0u;
	uint32_t nineActive = 0u;
	for (const LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& group :
		m_Replication.Get_WorldDestructionGroupStates())
	{
		const std::string_view groupId(group.strGroupId);
		const bool_t active =
			LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT ==
			group.eState;
		if (groupId.starts_with(OUTER_RING_GROUP_PREFIX))
		{
			++snapshot.iOuterRingGroupCount;
			outerActive += active ? 1u : 0u;
		}
		else if (groupId.starts_with(THREE_OCLOCK_GROUP_PREFIX))
		{
			++snapshot.iThreeOClockGroupCount;
			threeActive += active ? 1u : 0u;
		}
		else if (groupId.starts_with(NINE_OCLOCK_GROUP_PREFIX))
		{
			++snapshot.iNineOClockGroupCount;
			nineActive += active ? 1u : 0u;
		}
		else
		{
			++snapshot.iOrdinaryGroupCount;
			ordinaryActive += active ? 1u : 0u;
		}
	}
	snapshot.bOrdinaryWallsActive = 0u != snapshot.iOrdinaryGroupCount &&
		ordinaryActive == snapshot.iOrdinaryGroupCount;
	snapshot.bOuterRingActive = 0u != snapshot.iOuterRingGroupCount &&
		outerActive == snapshot.iOuterRingGroupCount;
	snapshot.bThreeOClockFloorActive =
		0u != snapshot.iThreeOClockGroupCount &&
		threeActive == snapshot.iThreeOClockGroupCount;
	snapshot.bNineOClockFloorActive =
		0u != snapshot.iNineOClockGroupCount &&
		nineActive == snapshot.iNineOClockGroupCount;
	snapshot.iDebrisActorCount = static_cast<uint32_t>(
		m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount());
	const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS& diagnostics =
		m_Replication.Get_WorldDestructionDiagnostics();
	snapshot.iActiveCollisionCount =
		diagnostics.iActiveWallCollisionCount;
	snapshot.iActiveNavigationRegionCount =
		diagnostics.iActiveNavBlockerRegionCount;
	snapshot.iNavigationRevision = diagnostics.iNavigationRevision;
	return snapshot;
}

#endif

void CLevel_ValtanArena::Update_WorldDestructionPresentation(
	const f32_t fTimeDelta)
{
	const uint64_t generation =
		m_Replication.Get_WorldDestructionPresentationGeneration();
	if (generation != m_iObservedWorldDestructionPresentationGeneration)
	{
		if (!m_WorldDestructionDebrisPresentationRuntime.Reset_Presentation())
		{
			OutputDebugStringA((
				"[Level_ValtanArena][DestructionTransition] " +
				m_WorldDestructionDebrisPresentationRuntime.Get_Status() +
				"\n").c_str());
			/* Keep the observed generation unchanged so teardown is retried next
			   frame. No packet from the replacement generation is sampled until
			   the previous transition lane has been restored atomically. */
			return;
		}
		m_iObservedWorldDestructionPresentationGeneration = generation;
	}
	if (m_Replication.Is_WorldDestructionSynchronized())
	{
		std::string transitionStatus;
		if (!m_WorldDestructionDebrisPresentationRuntime.
			Update_SourceTransitions(
				m_WorldDestructionProjectionDocument,
				m_WorldDestructionDebrisPresentationDocument,
				m_Replication.Get_WorldDestructionGroupStates(),
				m_Replication.Get_WorldDestructionServerTick(),
				m_ValtanEncounterReference.Get_FixedTickHz(),
				fTimeDelta,
				transitionStatus))
		{
			/* This layer is cosmetic. Persistent Deploy state, collision,
			   navigation and falling were already committed by replication and
			   must survive a failed root/fade sample. */
			OutputDebugStringA((
				"[Level_ValtanArena][DestructionTransition] " +
				transitionStatus + "\n").c_str());
		}
	}

	/* The 109 impact drains every group's cue on one frame. They are collected
	first so the actor budget can be split evenly, instead of the earliest
	groups spending everything and later ring sectors vanishing silently. */
	std::vector<WORLD_DESTRUCTION_DEBRIS_CUE> pendingCues;
	LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE event;
	while (m_Replication.Try_Consume_WorldDestructionLiveEvent(event))
	{
		const WORLD_DESTRUCTION_DEBRIS_PROFILE* profile =
			m_WorldDestructionDebrisPresentationDocument.Find_Group(
				event.strGroupId);
		if (nullptr == profile)
		{
			/* State-only groups (for example collapsing floor sectors) have no
			debris recipe. Their persistent projection was already committed. */
			continue;
		}
		if (profile->strMutationId != event.strMutationId)
		{
			OutputDebugStringA(
				"[Level_ValtanArena][DestructionDebris] "
				"Ignored a cue whose group/mutation does not match the published presentation.\n");
			continue;
		}
		const WORLD_DESTRUCTION_PROJECTION_GROUP* projectionGroup =
			m_WorldDestructionProjectionDocument.Find_Group(event.strGroupId);
		if (nullptr == projectionGroup ||
			projectionGroup->strMutationId != event.strMutationId)
		{
			OutputDebugStringA(
				"[Level_ValtanArena][DestructionDebris] "
				"Ignored a cue with no joined projection group.\n");
			continue;
		}

		WORLD_DESTRUCTION_DEBRIS_CUE cue;
		cue.groupId = profile->strGroupId;
		cue.eventSequence = event.iEventSequence;
		cue.randomSeed = event.iRandomSeed;
		cue.emitters.reserve(profile->Emitters.size());
		for (const WORLD_DESTRUCTION_DEBRIS_EMITTER& emitter :
			profile->Emitters)
		{
			WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE emitterCue;
			emitterCue.sourceRuntimePlacementId =
				emitter.iSourceRuntimePlacementId;
			emitterCue.suppressionAliasPlacementIds =
				emitter.SuppressionAliasPlacementIds;
			emitterCue.spawnOffset = emitter.vSpawnOffset;
			emitterCue.direction = emitter.vDirection;
			emitterCue.speedMetersPerSecond =
				emitter.fSpeedMetersPerSecond;
			emitterCue.gravityScale = emitter.fGravityScale;
			emitterCue.lifetimeSeconds = emitter.fLifetimeSeconds;
			emitterCue.suppressSource = !projectionGroup->bRemovesGround;
			cue.emitters.push_back(std::move(emitterCue));
		}

		pendingCues.push_back(std::move(cue));
	}

	/* Server event order stays canonical; only the per-cue share is derived. */
	size_t pendingEmitterCount = 0u;
	for (const WORLD_DESTRUCTION_DEBRIS_CUE& pendingCue : pendingCues)
		pendingEmitterCount += pendingCue.emitters.size();
	const uint32_t emitterShare =
		CWorldDestructionDebrisPresentationRuntime::Resolve_CueEmitterShare(
			m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount(),
			pendingCues.size(),
			pendingEmitterCount);
	for (const WORLD_DESTRUCTION_DEBRIS_CUE& pendingCue : pendingCues)
	{
		std::string cueStatus;
		if (!m_WorldDestructionDebrisPresentationRuntime.Play_Cue(
			pendingCue, emitterShare, cueStatus))
		{
			OutputDebugStringA((
				"[Level_ValtanArena][DestructionDebris] " +
				cueStatus + "\n").c_str());
		}
	}

	std::string status;
	if (!m_WorldDestructionDebrisPresentationRuntime.Post_Physics_Update(
		fTimeDelta, status))
	{
		OutputDebugStringA((
			"[Level_ValtanArena][DestructionDebris] " + status + "\n").c_str());
	}
}

bool_t CLevel_ValtanArena::Apply_EncounterPropPresentation()
{
	const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& props =
		m_Replication.Get_EncounterPropState();
	if (props.Slots.empty())
		return true;
	if (props.strPropSetId != VALTAN_PILLAR_SET_ID ||
		props.Slots.size() != VALTAN_PILLAR_SLOT_IDS.size())
	{
		OutputDebugStringA(
			"[Level_ValtanArena][EncounterProps] Unknown prop set or slot count.\n");
		return false;
	}
	if (props.iEncounterEpoch == m_iObservedEncounterPropEpoch &&
		props.iServerTick == m_iObservedEncounterPropServerTick)
	{
		return true;
	}

	std::vector<std::pair<uint64_t, DEPLOY_PROP_STATE>> states;
	states.reserve(props.Slots.size());
	/* One cue per slot. A slot that shatters on its own tick must not be
	   folded into a neighbour that is still standing. */
	std::vector<WORLD_DESTRUCTION_DEBRIS_CUE> burstCues;
	for (size_t index = 0u; index < props.Slots.size(); ++index)
	{
		const LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE& slot =
			props.Slots[index];
		if (slot.strSlotId != VALTAN_PILLAR_SLOT_IDS[index])
		{
			OutputDebugStringA(
				"[Level_ValtanArena][EncounterProps] Slot order or identity is invalid.\n");
			return false;
		}
		DEPLOY_PROP_STATE deployState = DEPLOY_PROP_STATE::INTACT;
		switch (slot.eState)
		{
		case LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN:
			deployState = DEPLOY_PROP_STATE::DESPAWNED;
			break;
		case LostArk::Shared::ENCOUNTER_PROP_STATE::SPAWNING:
		case LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT:
		case LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING:
			deployState = DEPLOY_PROP_STATE::INTACT;
			break;
		default:
			return false;
		}
		/* The shatter is an edge. The Server repeats BREAKING for as long as the
		   slot stays in it, so the burst is keyed to the slot state version and
		   thrown exactly once for it. */
		if (LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING == slot.eState &&
			0u != slot.iStateVersion &&
			m_FiredEncounterPropBurstVersions[index] != slot.iStateVersion)
		{
			WORLD_DESTRUCTION_DEBRIS_CUE burstCue{};
			burstCue.groupId = slot.strSlotId;
			burstCue.eventSequence = slot.iStateVersion;
			burstCue.randomSeed = slot.iOccurrenceSequence;
			WORLD_DESTRUCTION_DEBRIS_EMITTER_CUE emitter{};
			emitter.sourceRuntimePlacementId =
				VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index];
			emitter.direction = float3_t(0.f, 1.f, 0.f);
			emitter.speedMetersPerSecond =
				VALTAN_PILLAR_BURST_SPEED_METERS_PER_SECOND;
			emitter.gravityScale = VALTAN_PILLAR_BURST_GRAVITY_SCALE;
			emitter.lifetimeSeconds = VALTAN_PILLAR_BURST_LIFETIME_SECONDS;
			emitter.spreadDegrees = BURST_DEBRIS_SPREAD_DEGREES;
			burstCue.emitters.push_back(std::move(emitter));
			burstCues.push_back(std::move(burstCue));
			m_FiredEncounterPropBurstVersions[index] = slot.iStateVersion;
		}
		states.emplace_back(
			VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index], deployState);
	}
	if (!m_DeployRuntime.Set_States(states))
		return false;
#ifdef _DEBUG
	/* Every applied sync says what the Server actually sent. Without it a
	mechanic that never fires is completely silent on both sides, which is what
	made the missing stele so hard to place. The states only move on a raise or
	a shatter, so this stays one line per edge rather than one per frame. */
	{
		size_t hiddenCount = 0u;
		size_t intactCount = 0u;
		size_t breakingCount = 0u;
		for (const LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE& slot : props.Slots)
		{
			if (LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN == slot.eState)
				++hiddenCount;
			else if (LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING ==
				slot.eState)
				++breakingCount;
			else
				++intactCount;
		}
		char_t summary[192]{};
		(void)std::snprintf(summary, sizeof(summary),
			"[Level_ValtanArena][EncounterProps] sync epoch %u tick %u: "
			"%zu slots, HIDDEN %zu INTACT %zu BREAKING %zu\n",
			props.iEncounterEpoch, props.iServerTick,
			props.Slots.size(), hiddenCount, intactCount, breakingCount);
		OutputDebugStringA(summary);
	}
	/* The stele is the one prop whose model arrives in a different unit and a
	different up-axis from the map exports, so the raise reports the size it
	actually drew at. A line here answers "is it there and is it the right
	size" without anyone having to judge it by eye. */
	for (size_t index = 0u; index < props.Slots.size(); ++index)
	{
		if (LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT !=
			props.Slots[index].eState)
		{
			continue;
		}
		const shared_ptr<CDeployPropObject> raised =
			m_DeployRuntime.Find(VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index]);
		float3_t center{};
		float3_t halfExtents{};
		if (nullptr == raised || !raised->Get_WorldBounds(center, halfExtents))
			continue;
		char_t line[192]{};
		(void)std::snprintf(line, sizeof(line),
			"[Level_ValtanArena][EncounterProps] %s raised at "
			"(%.2f, %.2f, %.2f) height %.2fm radius %.2fm\n",
			props.Slots[index].strSlotId.c_str(),
			center.x, center.y, center.z,
			halfExtents.y * 2.f,
			(std::max)(halfExtents.x, halfExtents.z));
		OutputDebugStringA(line);
	}
#endif
	for (const WORLD_DESTRUCTION_DEBRIS_CUE& burstCue : burstCues)
	{
		std::string cueStatus;
		/* A stele with no authored twelve-piece recipe is still retired by the
		   Server's own HIDDEN edge, so a refused cue costs the shatter, never the
		   mechanic. */
		if (!m_WorldDestructionDebrisPresentationRuntime.Play_Cue(
			burstCue, 1u, cueStatus))
		{
			OutputDebugStringA((
				"[Level_ValtanArena][EncounterProps] " +
				cueStatus + "\n").c_str());
		}
	}
	m_iObservedEncounterPropEpoch = props.iEncounterEpoch;
	m_iObservedEncounterPropServerTick = props.iServerTick;
	return true;
}

bool_t CLevel_ValtanArena::Ready_CinematicCamera()
{
	std::string status;
	if (!m_ValtanEncounterReference.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanEncounter.json"), status))
	{
		(void)Report_InitFailure(
			"[Level_ValtanArena][EncounterReference]", status);
		return false;
	}
	if (!m_ValtanCinematicCameraDocument.Load(
		CProjectDataRoot::Resolve(
			L"Encounters/Valtan/ValtanCinematicCamera.json"),
		m_ValtanEncounterReference, status) ||
		!m_ValtanCinematicCameraController.Initialize(
			&m_ValtanCinematicCameraDocument,
			m_ValtanEncounterReference.Get_FixedTickHz()))
	{
		m_ValtanCinematicCameraDocument.Clear();
		m_ValtanEncounterReference.Clear();
		(void)Report_InitFailure(
			"[Level_ValtanArena][CinematicCamera]", status);
		return false;
	}
	if (!m_MapEffectPresentationRuntime.Load_Area(
			ETOUI(LEVEL::VALTAN_ARENA),
			m_WorldDestructionProjectionDocument.Get_AreaId(),
			m_DeployRuntime,
			m_WorldDestructionProjectionDocument,
			m_ValtanEncounterReference,
			status))
	{
		m_MapEffectPresentationRuntime.Clear();
		m_ValtanCinematicCameraController.Reset();
		m_ValtanCinematicCameraDocument.Clear();
		m_ValtanEncounterReference.Clear();
		(void)Report_InitFailure(
			"[Level_ValtanArena][MapEffectPresentation]", status);
		return false;
	}
	return true;
}

void CLevel_ValtanArena::Update_CinematicCamera(const f32_t fTimeDelta)
{
	const VALTAN_PRESENTATION_STATE& boss =
		m_Replication.Get_ValtanPresentationState();
	VALTAN_CINEMATIC_CAMERA_INPUT input{};
	/* Death is no longer a reason to stop: it selects the clear shot instead,
	   and the same restore path still runs when that cue finishes, on level exit
	   and on disconnect. */
	input.isBossDead = boss.isValid &&
		LostArk::Shared::WORLD_ENTITY_ACTION::DEAD == boss.eAction;
	input.isValid = boss.isValid;
	input.iNetEntityId = boss.iNetEntityId;
	input.iServerTick = boss.iServerTick;
	input.strPatternId = boss.strPatternId;
	input.strStageActionId = boss.strActionId;
	input.iPatternSequence = boss.iPatternSequence;
	input.iStageIndex = boss.iPatternStageIndex;
	input.iActionStartTick = boss.iActionStartTick;
	input.vBossPosition = boss.vPosition;
	input.fBossYawDegrees = boss.fYawDegrees;
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr != localCharacter)
	{
		const shared_ptr<CTransform> localTransform =
			localCharacter->Get_Transform();
		if (nullptr != localTransform)
		{
			XMStoreFloat3(
				&input.vLocalPlayerPosition,
				localTransform->Get_State(STATE::POSITION));
			input.hasLocalPlayerPosition = true;
		}
	}

#ifdef _DEBUG
	CAMERA_TOOL_ACTOR_PREVIEW_CONTEXT previewContext{};
	previewContext.isValid = input.isValid;
	previewContext.iLevelIndex = ETOUI(LEVEL::VALTAN_ARENA);
	previewContext.vBossPosition = input.vBossPosition;
	previewContext.fBossYawDegrees = input.fBossYawDegrees;
	previewContext.hasLocalPlayerPosition = input.hasLocalPlayerPosition;
	previewContext.vLocalPlayerPosition = input.vLocalPlayerPosition;
	CCameraTool::Publish_ActorPreviewContext(previewContext);
#endif

	/* The level-owned Map Effect consumer samples the same authoritative tuple
	   even when this stage has no camera cue.  It owns neither boss gameplay nor
	   camera state and therefore remains active across camera-tool auditions. */
	m_MapEffectPresentationRuntime.Update_ServerPresentation(boss, fTimeDelta);
	if (nullptr == m_pCamera)
	{
		End_CinematicCameraOverride();
		return;
	}

	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!m_ValtanCinematicCameraController.Update(input, fTimeDelta, pose))
	{
		/* A cue-authored exit handoff retains the same Server cinematic owner
		   until its final submitted pose exactly matches live gameplay follow. */
		if (Update_CinematicCameraExitTransition(fTimeDelta))
			return;
		/* Map Effects have an independent level-owned clock, so a stage without
		   a camera cue restores only the camera presentation owner. */
		End_CinematicCameraOverride();
		return;
	}

#ifdef _DEBUG
	/* A Server-authored cue always preempts the local comparison aid before it
	tries to acquire the single camera presentation owner. */
	End_ReferenceCamera(false);
#endif

	if (!m_bCinematicCameraApplied)
	{
		m_bCinematicRestoreFollowRequested = m_pCamera->Is_FollowRequested();
		m_pCinematicRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
		if (!m_pCamera->Begin_PresentationOverride(
			static_cast<uint64_t>(boss.iNetEntityId),
			CCamera::PRESENTATION_PRIORITY::SERVER_CINEMATIC))
		{
			m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
			m_pCamera->Set_FollowEnabled(
				m_bCinematicRestoreFollowRequested);
			End_CinematicCamera();
			return;
		}
		m_bCinematicCameraApplied = true;
		m_iCinematicCameraOwnerId =
			static_cast<uint64_t>(boss.iNetEntityId);
	}
	else if (nullptr != m_pCamera->Get_FollowTarget())
	{
		m_pCinematicRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
	}

	if (m_iCinematicCameraOwnerId !=
		static_cast<uint64_t>(boss.iNetEntityId))
	{
		End_CinematicCamera();
		return;
	}
	if (!m_pCamera->Apply_PresentationPose(
		m_iCinematicCameraOwnerId,
		pose.vEye, pose.vLookAt, pose.fFovYDegrees))
	{
		End_CinematicCamera();
	}
}

bool_t CLevel_ValtanArena::Update_CinematicCameraExitTransition(
	const f32_t fTimeDelta)
{
	if (!m_bCinematicCameraApplied || nullptr == m_pCamera ||
		0u == m_iCinematicCameraOwnerId ||
		!m_bCinematicRestoreFollowRequested ||
		!m_ValtanCinematicCameraController.Is_ExitTransitionActive())
	{
		return false;
	}
	const shared_ptr<CTransform> followTarget =
		m_pCinematicRestoreTarget.lock();
	if (nullptr == followTarget)
		return false;

	const vector_t targetPosition = followTarget->Get_State(STATE::POSITION);
	const float3_t positionOffset = m_pCamera->Get_PositionOffset();
	VALTAN_CINEMATIC_CAMERA_POSE followPose{};
	XMStoreFloat3(&followPose.vEye, XMVectorSetW(
		targetPosition + XMLoadFloat3(&positionOffset), 1.f));
	XMStoreFloat3(&followPose.vLookAt, XMVectorSetW(
		targetPosition + XMVectorSet(
			0.f, VALTAN_GAMEPLAY_FOLLOW_LOOK_HEIGHT, 0.f, 0.f), 1.f));
	followPose.fFovYDegrees = VALTAN_GAMEPLAY_FOV_Y_DEGREES;

	VALTAN_CINEMATIC_CAMERA_POSE transitionPose{};
	if (!m_ValtanCinematicCameraController.Update_ExitTransition(
		followPose, fTimeDelta, transitionPose) ||
		!m_pCamera->Apply_PresentationPose(
			m_iCinematicCameraOwnerId,
			transitionPose.vEye,
			transitionPose.vLookAt,
			transitionPose.fFovYDegrees))
	{
		return false;
	}
	if (!m_ValtanCinematicCameraController.Is_ExitTransitionActive())
		End_CinematicCameraOverride();
	return true;
}

void CLevel_ValtanArena::End_CinematicCameraOverride()
{
	if (m_bCinematicCameraApplied && nullptr != m_pCamera)
	{
		if (0u != m_iCinematicCameraOwnerId)
			m_pCamera->End_PresentationOverride(m_iCinematicCameraOwnerId);
		m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
		m_pCamera->Set_FollowEnabled(m_bCinematicRestoreFollowRequested);
	}
	m_pCinematicRestoreTarget.reset();
	m_bCinematicRestoreFollowRequested = false;
	m_bCinematicCameraApplied = false;
	m_iCinematicCameraOwnerId = 0u;
	m_ValtanCinematicCameraController.Cancel_ExitTransition();
}

void CLevel_ValtanArena::End_CinematicCamera()
{
	/* Map Effects have their own level lifetime. Camera completion must not
	   cancel a stage-owned world Effect or restore unrelated Deploy surfaces. */
	End_CinematicCameraOverride();
}

HRESULT CLevel_ValtanArena::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_PlayerNameplateView.Render(m_NameplatePlayers);
	m_ChatBubbleView.Render(m_Replication, m_NameplatePlayers);
	m_PartyInteraction.Render(m_pPlayerCommandSink);

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(TEXT("Valtan Arena Map"));
#endif

	return S_OK;
}

void CLevel_ValtanArena::Update_DeadScene(
	const bool_t isBlockedByRaidClear, const f32_t fTimeDelta)
{
	if (nullptr == m_pDeadSceneView)
		return;

	m_pDeadSceneView->Update(fTimeDelta);

	using LostArk::Shared::PLAYER_ACTION_STATE;
	const HUD_PLAYER_STATE& player = CCombatHUDViewModel::Get().Get_Player();
	const bool_t isDead = !isBlockedByRaidClear && player.isValid &&
		PLAYER_ACTION_STATE::DEAD == player.eAction;

	/* Real Render_DeadScene's own whole-screen AddRectFilled(IM_COL32(0,0,0,160)), now a real
	slot (DeadScene_Dim, White1x1 tinted) instead of a raw ImGui draw call. */
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Dim", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_PanelBg", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_WingedArch", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_Effect", isDead);
	m_pDeadSceneView->Set_SlotVisible("DeadScene_ReviveButton", isDead);
	/* Spectate is not wired to any server/client command yet -- these two slots exist only
	so the button and its border can be positioned in the HUD Layout Tool. */
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

#ifdef _DEBUG
void CLevel_ValtanArena::Update_DebugRaidClearKey()
{
	if (ImGui::GetIO().WantTextInput ||
		CUIInputRouter::Get().Is_TextInputActive())
		return;
	const HWND hForeground = GetForegroundWindow();
	DWORD foregroundProcessId = {};
	const bool_t windowFocused = nullptr != hForeground &&
		0 != GetWindowThreadProcessId(hForeground, &foregroundProcessId) &&
		GetCurrentProcessId() == foregroundProcessId;
	const bool_t oDown = windowFocused &&
		0 != (GetAsyncKeyState(0x4F /* VK_O */) & 0x8000);
	/* Forces the overlay's own timeline (Update_RaidClear) to start from 0 without a real dead
	Valtan behind it -- pure Client-local test convenience, no Server command involved. */
	if (oDown && !m_bDebugRaidClearKeyDown)
		Trigger_RaidClear();
	m_bDebugRaidClearKeyDown = oDown;
}
#endif

/* Real EpicGateCommonClearFrame timeline: TweenMax.delayedCall(startFrame/40, ...) holds on a
blank frame before resultMc.gotoAndPlay(startFrame) actually starts the reveal, then
onUpdateFrameCheck() hides it the instant currentFrame reaches totalFrames -- an abrupt cut, not
a fade-out. startFrame=90/holdFrame=296 (result_101's own defaults) at the source's 40fps gives
the two real durations below; this runtime doesn't replay the source's 309-frame shape timeline
frame-for-frame, only its outer show/hold/hide shape. */
namespace
{
	constexpr f32_t RAIDCLEAR_REVEAL_SECONDS = 90.f / 40.f;
	constexpr f32_t RAIDCLEAR_HOLD_SECONDS = 296.f / 40.f;
	constexpr f32_t RAIDCLEAR_TOTAL_SECONDS =
		RAIDCLEAR_REVEAL_SECONDS + RAIDCLEAR_HOLD_SECONDS;
}

void CLevel_ValtanArena::Trigger_RaidClear()
{
	m_fRaidClearElapsedSeconds = 0.f;
	/* Real cue name confirmed in the extracted sound resource pool
	(D:\...\Sound\UI\System\sys_raid_success1__457395004.wav) -- epicgatecommonclear.gfx itself
	carries no embedded sound (Scaleform UI movies play native-triggered cues, not baked audio),
	so this is played from here rather than anywhere inside m_pRaidClearView. */
	const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
		L"Sound/UI/System/sys_raid_success1__457395004.wav");
	CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
}

void CLevel_ValtanArena::Update_RaidClear(f32_t fTimeDelta)
{
	if (nullptr == m_pRaidClearView)
		return;

	/* Reads the un-gated death latch (CCombatHUDViewModel::Get_BossDeadRaw())
	instead of Get_Boss().eAction. CClientReplication raises it from either a raw
	DEAD snapshot or, normally, the reliable DEAD despawn that removes the boss
	before the next snapshot. BossCombat revision validation therefore cannot
	hide the terminal edge from Raid Clear. */
	const bool_t isBossDead = CCombatHUDViewModel::Get().Get_BossDeadRaw();

	/* Edge-trigger only -- no "boss alive again -> hide" branch, so
	Update_DebugRaidClearKey()'s forced trigger below (with no real dead boss behind it) is
	free to run its own timeline out instead of being wiped the very next frame. */
	if (isBossDead && !m_bRaidClearWasBossDead)
		Trigger_RaidClear();
	m_bRaidClearWasBossDead = isBossDead;

	/* Release has no Debug O-key path. An explicit process environment opt-in
	   lets QA open the final Return button immediately in a Release Client; the
	   Server requires the same opt-in to accept a pre-clear test transfer. The
	   default product path remains entirely driven by the authoritative death. */
	if (!isBossDead && m_fRaidClearElapsedSeconds < 0.f &&
		Is_RaidClearTestModeEnabled())
	{
		m_fRaidClearElapsedSeconds = RAIDCLEAR_TOTAL_SECONDS;
		OutputDebugStringA(
			"[Level_ValtanArena] Raid Clear Release test mode enabled.\n");
	}

	if (m_fRaidClearElapsedSeconds >= 0.f)
		m_fRaidClearElapsedSeconds += fTimeDelta;

	const bool_t isShowing = m_fRaidClearElapsedSeconds >= 0.f &&
		m_fRaidClearElapsedSeconds < RAIDCLEAR_TOTAL_SECONDS;
	/* The celebration overlay auto-hides at RAIDCLEAR_TOTAL_SECONDS and never
	resets itself back to -1 afterward (only a fresh Trigger_RaidClear does),
	so this stays true for the rest of this Level's session once reached --
	exactly the "그 자리" moment the button replaces the finished overlay with. */
	const bool_t isAfterRaidClear =
		m_fRaidClearElapsedSeconds >= RAIDCLEAR_TOTAL_SECONDS;

	/* Every real layer traced out of result_101's own 309-frame timeline (see the RESULT doc) --
	the flat background flash, the five className-referenced EFUI_Effect glow/particle flipbooks
	that sit behind/around the crest, and the crest art itself. All fade in/out together on this
	simplified timeline instead of each other's real per-frame stagger. RAIDCLEAR_FADING_SLOTS
	itself lives in the file-scope anonymous namespace near Initialize() (also hides these up
	front, before this Level's first Render()). */
	m_pRaidClearView->Set_SlotVisible("RaidClear_Dim", isShowing);
	for (const char* const szSlotId : RAIDCLEAR_FADING_SLOTS)
		m_pRaidClearView->Set_SlotVisible(szSlotId, isShowing);
	/* Authoring-only placeholder, same split as DeadScene_TitleTextMarker --
	RenderRaidClearText() (CMainApp, after EndFrame()) draws the real text. */
	m_pRaidClearView->Set_SlotVisible("RaidClear_TitleTextBox", false);
	m_pRaidClearView->Set_SlotVisible("RaidClear_ReturnButton", isAfterRaidClear);
	if (isShowing)
	{
		const f32_t fRevealAlpha = (m_fRaidClearElapsedSeconds < RAIDCLEAR_REVEAL_SECONDS) ?
			(m_fRaidClearElapsedSeconds / RAIDCLEAR_REVEAL_SECONDS) : 1.f;
		for (const char* const szSlotId : RAIDCLEAR_FADING_SLOTS)
			m_pRaidClearView->Set_SlotAlpha(szSlotId, fRevealAlpha);
	}
	m_pRaidClearView->Update(fTimeDelta);

	/* "돌아가기" button -- appears once the celebration overlay's own reveal/hold
	timeline finishes and every fading slot has hidden itself, taking that same
	screen position rather than sitting on top of the still-playing overlay.
	Same hover/click hit-test pattern as CLevel_ValtanArena's own DeadScene
	Revive button (CUIInputRouter::Get() + Get_SlotRect), and the same
	one-shot Request_* submission as before. No local hide-on-click --
	CLevelTransitionService's real BERN switch (once the Server accepts the
	transfer) tears this whole Level down anyway. */
	if (isAfterRaidClear && nullptr != m_pPlayerCommandSink)
	{
		f32_t fButtonX = 0.f, fButtonY = 0.f, fButtonWidth = 0.f, fButtonHeight = 0.f;
		if (m_pRaidClearView->Get_SlotRect("RaidClear_ReturnButton",
			fButtonX, fButtonY, fButtonWidth, fButtonHeight))
		{
			CUIInputRouter& Router = CUIInputRouter::Get();
			const f32_t fResolutionWidth = m_pRaidClearView->Get_ResolutionWidth();
			const f32_t fResolutionHeight = m_pRaidClearView->Get_ResolutionHeight();
			const bool_t isButtonHovered = Router.Is_Hovered(
				fButtonX, fButtonY, fButtonWidth, fButtonHeight,
				fResolutionWidth, fResolutionHeight);
			m_pRaidClearView->Set_SlotTexture("RaidClear_ReturnButton", isButtonHovered ?
				"UI/ClassSelect/Common/NormalButtonHover.png" :
				"UI/ClassSelect/Common/NormalButton.png");
			if (isButtonHovered)
			{
				Router.Claim_Mouse_This_Frame();
				if (Router.Is_Clicked(fButtonX, fButtonY, fButtonWidth, fButtonHeight,
					fResolutionWidth, fResolutionHeight))
				{
					CMainApp::Play_UIButtonClickSound();
					m_pPlayerCommandSink->Request_ReturnToBern(
						m_iNextReturnToBernSequence++);
				}
			}
		}
	}

	HUD_RAIDCLEAR_TEXT_RECTS textRects;
	textRects.isValid = isShowing &&
		m_pRaidClearView->Get_SlotRect("RaidClear_TitleTextBox",
			textRects.fTitleX, textRects.fTitleY,
			textRects.fTitleWidth, textRects.fTitleHeight);
	textRects.isButtonValid = isAfterRaidClear &&
		m_pRaidClearView->Get_SlotRect("RaidClear_ReturnButton",
			textRects.fButtonX, textRects.fButtonY,
			textRects.fButtonWidth, textRects.fButtonHeight);
	CCombatHUDViewModel::Get().Set_RaidClearTextRects(textRects);
}

/* No matching Render_RaidClear() -- migrated to real CUI_Sprite GameObjects
that self-render through the normal CRenderer/RENDERGROUP::UI pipeline
(Update_RaidClear above drives their visibility/alpha/texture instead). */

namespace
{
	constexpr f32_t ITEM_ANNOUNCE_HOLD_SECONDS = 2.f;

	/* Standard Hangul syllable-block final-consonant test (Unicode Hangul Syllables block,
	U+AC00..U+D7A3 = 28 trailing-consonant slots per syllable): (codepoint - 0xAC00) % 28 == 0
	means the syllable has no final consonant. A non-Hangul last character (Latin, digit, ...)
	falls back to "를" -- every real item name here ends in a Hangul syllable, so this only
	matters if a future catalog entry doesn't. */
	bool_t Has_HangulFinalConsonant(const wchar_t lastCharacter)
	{
		if (lastCharacter < 0xAC00 || lastCharacter > 0xD7A3)
			return false;
		return 0 != (lastCharacter - 0xAC00) % 28;
	}

	bool_t ConvertUtf8ToWide(const string& strUtf8, wstring& outWide)
	{
		outWide.clear();
		if (strUtf8.empty())
			return false;
		const int iRequiredLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), nullptr, 0);
		if (iRequiredLength <= 0)
			return false;
		outWide.resize(static_cast<size_t>(iRequiredLength));
		return iRequiredLength == MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			strUtf8.data(), static_cast<int>(strUtf8.size()), outWide.data(), iRequiredLength);
	}
}

void CLevel_ValtanArena::Update_ItemAnnounce(f32_t fTimeDelta)
{
	if (nullptr == m_pItemAnnounceView)
		return;

	/* Diff against the previous frame's own observed itemId set (not the previous frame's
	inventory contents wholesale) -- an existing item's quantity going up (e.g. a potion restock)
	must not requeue it, only a itemId this view has never observed before. */
	const vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& currentInventory =
		CCombatHUDViewModel::Get().Get_Inventory().Items;
	if (!m_bItemAnnounceBaselineCaptured)
	{
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : currentInventory)
			m_ItemAnnounceObservedItemIds.push_back(item.strItemId);
		m_bItemAnnounceBaselineCaptured = true;
	}
	else
	{
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : currentInventory)
		{
			if (std::find(m_ItemAnnounceObservedItemIds.begin(),
				m_ItemAnnounceObservedItemIds.end(), item.strItemId) !=
				m_ItemAnnounceObservedItemIds.end())
			{
				continue;
			}
			m_ItemAnnounceObservedItemIds.push_back(item.strItemId);
			m_ItemAnnounceQueue.push_back(item.strItemId);
		}
	}

	if (m_fItemAnnounceElapsedSeconds >= 0.f)
	{
		m_fItemAnnounceElapsedSeconds += fTimeDelta;
		if (m_fItemAnnounceElapsedSeconds >= ITEM_ANNOUNCE_HOLD_SECONDS)
			m_fItemAnnounceElapsedSeconds = -1.f;
	}

	if (m_fItemAnnounceElapsedSeconds < 0.f && !m_ItemAnnounceQueue.empty())
	{
		m_strItemAnnounceCurrentItemId = m_ItemAnnounceQueue.front();
		m_ItemAnnounceQueue.erase(m_ItemAnnounceQueue.begin());
		m_fItemAnnounceElapsedSeconds = 0.f;

		const ITEM_DEFINITION* pDefinition =
			CItemCatalog::Find_ById(m_strItemAnnounceCurrentItemId);
		if (nullptr != pDefinition && !pDefinition->strIconPath.empty())
			m_pItemAnnounceView->Set_SlotTexture("ItemAnnounce_Icon", pDefinition->strIconPath);

		const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
			L"Sound/UI/System/sys_item_itemgetepic1__202768724.wav");
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}

	const bool_t isShowing = m_fItemAnnounceElapsedSeconds >= 0.f;
	m_pItemAnnounceView->Set_SlotVisible("ItemAnnounce_Frame", isShowing);
	m_pItemAnnounceView->Set_SlotVisible("ItemAnnounce_Icon", isShowing);

	HUD_ITEMANNOUNCE_TEXT_RECTS textRects;
	textRects.isValid = isShowing && m_pItemAnnounceView->Get_SlotRect("ItemAnnounce_TextBox",
		textRects.fTextX, textRects.fTextY, textRects.fTextWidth, textRects.fTextHeight);
	if (textRects.isValid)
	{
		const ITEM_DEFINITION* pDefinition =
			CItemCatalog::Find_ById(m_strItemAnnounceCurrentItemId);
		wstring strItemName;
		if (nullptr != pDefinition &&
			ConvertUtf8ToWide(pDefinition->strDisplayName, strItemName))
		{
			// "을 획득하였습니다" / "를 획득하였습니다" -- particle chosen by the item name's
			// last syllable's final consonant (Has_HangulFinalConsonant above). Kept
			// separate from the name (not concatenated into one string) so
			// RenderItemAnnounceText can draw the name in its own grade color and
			// this suffix in plain white.
			textRects.strSuffix = Has_HangulFinalConsonant(strItemName.back()) ?
				L"\xC744 \xD68D\xB4DD\xD558\xC600\xC2B5\xB2C8\xB2E4" :  // "을 획득하였습니다"
				L"\xB97C \xD68D\xB4DD\xD558\xC600\xC2B5\xB2C8\xB2E4";  // "를 획득하였습니다"
			textRects.strItemName = std::move(strItemName);
		}
	}
	CCombatHUDViewModel::Get().Set_ItemAnnounceTextRects(textRects);
}

#ifdef _DEBUG
bool_t CLevel_ValtanArena::Set_DebugCameraSpeed(const f32_t metersPerSecond)
{
	if (nullptr == m_pCamera || !m_pCamera->Set_FreeMoveSpeed(metersPerSecond))
		return false;
	g_ValtanFreeCameraSpeed = metersPerSecond;
	return true;
}
#endif

HRESULT CLevel_ValtanArena::Ready_Layer_Camera(
	const wstring_t& strLayerTag)
{
	const float3_t focus(151.25f, 22.97f, -121.75f);
	const f32_t span = 180.f;

	const f32_t distance = (std::max)(40.f, span * 0.7f);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		focus.x - distance,
		focus.y + distance * 0.65f,
		focus.z - distance);
	cameraDesc.vAt = focus;
	cameraDesc.fFovy = 60.f;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = g_ValtanFreeCameraSpeed;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;

	shared_ptr<CGameObject> gameObject;
	if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
		ETOUI(LEVEL::VALTAN_ARENA),
		TEXT("Prototype_GameObject_Camera_Free"),
		ETOUI(LEVEL::VALTAN_ARENA),
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
			ETOUI(LEVEL::VALTAN_ARENA), strLayerTag, gameObject);
		return E_FAIL;
	}

	return S_OK;
}

bool_t CLevel_ValtanArena::Bind_CameraToLocalCharacter()
{
	if (nullptr == m_pCamera)
		return false;

	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	if (nullptr == localCharacter)
	{
		m_pCameraTarget.reset();
		if (m_bCinematicCameraApplied)
		{
			m_pCinematicRestoreTarget.reset();
			return true;
		}
#ifdef _DEBUG
		if (m_bReferenceCameraApplied)
		{
			m_pReferenceCameraRestoreTarget.reset();
			m_pCamera->Set_FollowTarget(nullptr);
			m_pCamera->Set_FollowEnabled(false);
			return true;
		}
#endif
		m_pCamera->Set_FollowTarget(nullptr);
		m_pCamera->Set_FollowEnabled(false);
		return true;
	}
	if (m_pCameraTarget.lock() == localCharacter)
		return true;

	const shared_ptr<CTransform> transform =
		localCharacter->Get_Transform();
	if (nullptr == transform)
		return false;

	m_pCameraTarget = localCharacter;
	if (m_bCinematicCameraApplied)
	{
		m_pCinematicRestoreTarget = transform;
		return true;
	}
#ifdef _DEBUG
	if (m_bReferenceCameraApplied)
	{
		m_pReferenceCameraRestoreTarget = transform;
		m_pCamera->Set_FollowTarget(transform);
		m_pCamera->Set_FollowEnabled(false);
		return true;
	}
#endif
	m_pCamera->Set_PositionOffset(
		float3_t(0.4f, 7.5f, 4.5f));
	m_pCamera->Set_FollowTarget(transform);
	m_pCamera->Set_FollowEnabled(true);
	return true;
}

unique_ptr<CLevel_ValtanArena> CLevel_ValtanArena::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CLevel_ValtanArena>(
		new CLevel_ValtanArena(pDevice, pContext));
	if (FAILED(instance->Initialize()))
		return nullptr;
	return instance;
}
