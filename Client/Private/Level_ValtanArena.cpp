#include <WinSock2.h>
#include <dinput.h>
/* imgui.h defines its own placement-new helper and has to precede the project
headers, which is the same order Level_CharacterSelect.cpp uses. Previously
_DEBUG-only (the audition panel was its only user); the death-screen overlay
below is a real Release-build feature, so the include is no longer guarded. */
#include "imgui.h"
#include "UITextOcclusion.h"

#include "Level_ValtanArena.h"

#include "Camera_Free.h"
#ifdef _DEBUG
#include "CameraTool.h"
#endif
#include "Character.h"
#include "CombatHUDViewModel.h"
#include "GameInstance.h"
#include "HUDRuntimeView.h"
#include "MvpAwardCatalog.h"
#include "MvpResultView.h"
#include "UILayoutRuntime.h"
#include "UIInputRouter.h"
#include "ItemCatalog.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "MapAssetRenderUtils.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "NetworkWorldEntityCommandSink.h"
#include "ProjectDataRoot.h"
#include "RuntimeAssetRoot.h"
#include "Transform.h"
#include "Valtan.h"
#include "ActorCatalog.h"
#include "Effect_Catalog.h"
#include "Effect_PresentationService.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "ValtanPatternTree.h"
#include <set>
#include "WorldGameplayDocument.h"

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
#ifdef _DEBUG
	std::string destructionStatus;
	(void)Debug_StopActionWorkbenchDestruction(destructionStatus);
	Debug_StopActionWorkbenchCinematic();
	m_ActionWorkbenchCinematicPlayer.Clear();
#endif
	if (this == s_pActiveInstance)
		s_pActiveInstance = nullptr;
#ifdef _DEBUG
	CCameraTool::Clear_ActorPreviewContext(
		ETOUI(LEVEL::VALTAN_ARENA));
	End_ReferenceCamera(false);
#endif
	End_CinematicCamera();
	m_SourceCinematicPlayer.Clear();
	Clear_TriggerMarkers();
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
	m_ChatBubbleView.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA));
	m_InteractKeyPrompt.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA),
		"LV_LUT_HEARTRB_ED");

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
#ifdef _DEBUG
	replicationDesc.beforeWorldDestructionProjection = [this](std::string& status)
	{
		const bool active = !m_strWorkbenchDestructionPattern.empty();
		if (!Debug_StopActionWorkbenchDestruction(status)) return false;
		if (active)
			m_strWorkbenchDestructionFailure =
				"Wall preview stopped for Server world update; restart local Play when the arena is idle.";
		return true;
	};
#endif
	replicationDesc.onWorldEntityDespawned =
		[this](const std::string_view placementId,
			const std::string_view archetypeId)
		{
			Handle_WorldEntityDespawned(placementId, archetypeId);
		};
	Ready_SourceCinematics();
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
#ifdef _DEBUG
    m_pWorldEntityCommandSink = make_shared<CNetworkWorldEntityCommandSink>();
#endif
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

	/* Built hidden; Update_RaidClear shows it the instant the clear mark ends. */
	m_pMvpResultView = std::make_unique<CMvpResultView>(
		m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA));
	m_GateProgressView.Initialize(m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA));
	/* GameMsg tip.name.scene_group_index_name_contents_commanderraid_37051_1. */
	m_GateProgressView.Set_Raid(L"\xB9C8\xC218\xAD70\xB2E8\xC7A5 \xBC1C\xD0C4",
		CMvpAwardCatalog::Get().Find_DifficultyText("normal"), 1u);
	m_GateProgressView.Set_Progress(1u, 0u);
	/* A missing world document only hides the decoration; the level stays playable. */
	(void)Load_TriggerMarkers();

	/* First screen migrated off the ImGui interim UI rendering (see
	.md/TJ/08-31/2026-08-31_ImGui_런타임UI_전환_PLAN.md) -- real CUI_Sprite GameObjects on this
	Level's own new "Layer_UI" instead of CHUDRuntimeView's ImGui foreground-drawlist draws.
	Get_SlotRect/Set_SlotVisible/Set_SlotTexture below are unchanged calls; only the type and
	construction differ. */
	m_pItemAnnounceView = std::make_unique<CUILayoutRuntime>(
		m_pDevice, m_pContext, ETOUI(LEVEL::VALTAN_ARENA), TEXT("Layer_UI"),
		L"UI/ItemAnnounce/ItemAnnounce_Layout.json");
	m_pItemAnnounceView->Set_AllSlotsVisible(false);
	{
		f32_t fWidth = 0.f, fHeight = 0.f;
		(void)m_pItemAnnounceView->Get_SlotRect("ItemAnnounce_Band",
			m_fItemAnnounceBandX, m_fItemAnnounceBandY, fWidth, fHeight);
	}

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
#ifdef _DEBUG
    if (placementId == "boss.valtan.center" && m_bDebugValtanDespawnPending)
    {
        End_CinematicCamera();
        m_bDebugValtanDespawnPending = false;
        m_iDebugValtanCommandStartedMs = 0u;
        m_LastSourceCinematicInput = {};
        m_SourceDeathInput = {};
        m_bSourceDeathStarted = false;
        m_bSourceDeathFinished = true;
        m_strDebugValtanBossCommandStatus = "Server despawned Valtan; Play Pattern can prepare it again.";
    }
#endif
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
    Update_DebugValtanBossCommand();
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
	m_InteractKeyPrompt.Update(fTimeDelta, m_Replication.Get_LocalCharacter(),
		CCombatHUDViewModel::Get().Get_InteractPromptTriggerId(),
		!Is_MvpResultVisible());
	Update_TriggerMarkerClocks(fTimeDelta);
	/* worldInteractionAllowed=false: the right-click-a-player invite context menu
	is Bern-only by design (party formation happens before a Valtan entry, not
	mid-fight) -- right-click-moving past a teammate during combat kept opening
	it by accident. This only disables that trigger; an already-pending incoming
	invite (Try_Consume_PartyInviteReceived, inside Update() itself) is unrelated
	and still shown regardless. */
	/* Raid Clear is the topmost product modal. Update it before any gameplay or
	   lower-priority product interaction so its click cannot also become an
	   attack, movement command, revive, or party action in this frame. */
	{
		LostArk::Shared::S2C_RAID_MVP_RESULT MvpResult{};
		while (nullptr != m_pPlayerCommandSink && m_pPlayerCommandSink->Consume_RaidMvpResult(MvpResult))
		{
			m_RaidMvpResult = std::move(MvpResult);
			m_bHasRaidMvpResult = true;
			m_bRaidMvpResultFresh = true;
		}
	}
	Update_RaidClear(fTimeDelta);
	if (nullptr != m_pMvpResultView)
	{
		/* The award page has no characters of its own; each panel draws the player it
		   names (Show_MvpResult), as on KoukuSaydon. */
		for (size_t iStageSlot = 0; iStageSlot < 4u; ++iStageSlot)
			m_pMvpResultView->Set_StageCharacter(
				iStageSlot, m_MvpStageCharacters[iStageSlot].lock());
		m_pMvpResultView->Update(fTimeDelta);
	}
	m_GateProgressView.Set_Progress(1u, m_fRaidClearElapsedSeconds >= 0.f ? 1u : 0u);
	/* One gate and no Server gate progress here: the only panel button is exit, once the
	   award page has closed (the same trip as the clear screen's own return button). */
	m_GateProgressView.Set_Button(
		m_bRaidClearReturnAvailable ? CRaidGateProgressView::BUTTON::EXIT : CRaidGateProgressView::BUTTON::NONE, true);
	if (CRaidGateProgressView::INTENT::EXIT == m_GateProgressView.Update(fTimeDelta) &&
		nullptr != m_pPlayerCommandSink)
		m_pPlayerCommandSink->Request_ReturnToBern(m_iNextReturnToBernSequence++);
	const bool_t isRaidClearActive = m_fRaidClearElapsedSeconds >= 0.f;
	m_PartyInteraction.Register_TextOccluders();
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
	f32_t& fOutYawDegrees,
	std::string& strOutSource) const
{
	OutPosition = {};
	fOutYawDegrees = 0.f;
	strOutSource.clear();

	static constexpr std::string_view AREA_ID = "LV_LUT_HEARTRB_ED";
	static constexpr std::string_view BOSS_PLACEMENT_ID =
		"boss.valtan.center";
	CWorldGameplayDocument document;
	std::string loadStatus;
	if (!document.Load(
			CProjectDataRoot::Resolve(
				std::filesystem::path("Worlds") /
				std::string(AREA_ID) / "Gameplay.world.json"),
			std::string(AREA_ID), loadStatus))
	{
		strOutSource = "canonical Valtan gameplay placement load failed: " +
			loadStatus;
		return false;
	}
	const WORLD_GAMEPLAY_PLACEMENT* const placement =
		document.Find(std::string(BOSS_PLACEMENT_ID));
	if (nullptr == placement || WORLD_PLACEMENT_KIND::BOSS != placement->eKind ||
		placement->archetypeId != "BOSS_VALTAN" ||
		!std::isfinite(placement->position.x) ||
		!std::isfinite(placement->position.y) ||
		!std::isfinite(placement->position.z) ||
		!std::isfinite(placement->yawDegrees))
	{
		strOutSource =
			"canonical boss.valtan.center placement is missing or invalid";
		return false;
	}
	OutPosition = placement->position;
	fOutYawDegrees = placement->yawDegrees;
	strOutSource =
		"Gameplay.world.json / boss.valtan.center / Server audition transform";
	return true;
}

#ifdef _DEBUG
bool_t CLevel_ValtanArena::Debug_SampleActionWorkbenchCinematic(
	const std::string_view patternId,
	const uint32_t patternClockMs,
	const bool_t paused,
	const shared_ptr<CValtan>& previewBoss,
	std::string& status)
{
	struct SOURCE_CINEMATIC_PREVIEW final
	{
		std::string_view patternId;
		std::string_view suffix;
		std::string_view firstStageId;
		uint32_t stageOffsetMs;
	};
	static constexpr std::array<SOURCE_CINEMATIC_PREVIEW, 5u> PREVIEWS = {{
		{ "VALTAN_ENTRANCE_CINEMATIC", "entrance", "ESTABLISH", 0u },
		{ "VALTAN_ARENA_BREAK_109", "phase2", "IMPACT_HOLD", 600u },
		{ "VALTAN_SIX_PIZZA_106", "roar", "STEP_04", 0u },
		{ "VALTAN_TRASH", "trash", "STEP_05", 0u },
		{ "VALTAN_GHOST_DEATH_AUDITION", "finale", "STEP_01", 0u },
	}};
	const auto preview = std::find_if(PREVIEWS.begin(), PREVIEWS.end(),
		[&](const SOURCE_CINEMATIC_PREVIEW& value)
		{ return value.patternId == patternId; });
	if (preview == PREVIEWS.end())
	{
		Debug_StopActionWorkbenchCinematic();
		return true;
	}
	if (!previewBoss || !previewBoss->Is_LocalPatternAuthoringPreview() ||
		nullptr == previewBoss->Get_Transform())
	{
		status = "Action Workbench cinematic requires its non-authoritative Valtan arena clone.";
		Debug_StopActionWorkbenchCinematic();
		return false;
	}
	if (!m_strSourceCinematic.empty())
	{
		status = "Action Workbench cinematic yielded to the active Server source cinematic.";
		Debug_StopActionWorkbenchCinematic();
		return false;
	}
	if (!m_bSourceCinematicsReady)
	{
		if (m_bSourceCinematicsPreparationPending)
			Ready_SourceCinematics();
		if (!m_bSourceCinematicsReady)
		{
			status = "Action Workbench cinematic source is unavailable: " +
				m_strSourceCinematicPreparationStatus;
			Debug_StopActionWorkbenchCinematic();
			return false;
		}
	}
	auto targets = SourceCinematicTargets();
	// This editor sample runs after MainApp's normal Effect spawn commit,
	// like Map Tool. Admit only our new world roots for this same-frame seek.
	targets.bCommitWorldRootEffectsAfterSpawn = true;
	if (m_ActionWorkbenchCinematicPlayer.Get_Document().Get_AreaId().empty())
	{
		/* Ready_SourceCinematics consumes the one Loader-prepared Area when it
		   admits m_SourceCinematicPlayer.  Trying Load_PreparedArea again here
		   therefore always failed and silently left the Workbench on its ordinary
		   Pattern model while the original world-space camera kept playing.  Admit
		   an independent player from that already validated document instead. */
		std::string admissionStatus;
		if (!m_ActionWorkbenchCinematicPlayer.Set_Document(
				m_SourceCinematicPlayer.Get_Document(), targets, admissionStatus))
		{
			status = "Action Workbench cinematic document is unavailable: " +
				admissionStatus;
			Debug_StopActionWorkbenchCinematic();
			return false;
		}
	}

	const std::string instanceId =
		"world.sequence.instance.valtan.source-preview." +
		std::string(preview->suffix);
	const CWorldSequenceDocument& document =
		m_ActionWorkbenchCinematicPlayer.Get_Document();
	const WORLD_SEQUENCE_INSTANCE* instance = document.Find_Instance(instanceId);
	const WORLD_SEQUENCE_TEMPLATE* sequence = nullptr == instance ? nullptr :
		document.Find_Template(instance->templateId);
	const WORLD_SEQUENCE_TRACK* actorTrack = nullptr;
	if (nullptr != sequence)
	{
		const auto found = std::find_if(sequence->tracks.begin(), sequence->tracks.end(),
			[](const WORLD_SEQUENCE_TRACK& track) { return track.slotId == "actor"; });
		if (found != sequence->tracks.end()) actorTrack = &*found;
	}
	const ENCOUNTER_PATTERN_REFERENCE* pattern =
		m_ValtanEncounterReference.Find_Pattern(std::string(patternId));
	const ENCOUNTER_STAGE_REFERENCE* firstStage = nullptr;
	if (nullptr != pattern)
	{
		const auto found = std::find_if(pattern->stages.begin(), pattern->stages.end(),
			[&](const ENCOUNTER_STAGE_REFERENCE& stage)
			{ return stage.stageId == preview->firstStageId; });
		if (found != pattern->stages.end()) firstStage = &*found;
	}
	if (nullptr == instance || nullptr == sequence || nullptr == actorTrack ||
		actorTrack->keys.empty() || nullptr == firstStage)
	{
		status = "Action Workbench cinematic source contract is incomplete: " +
			std::string(patternId) + ".";
		Debug_StopActionWorkbenchCinematic();
		return false;
	}
	const uint64_t sourceStartMs =
		static_cast<uint64_t>(firstStage->iStartOffsetMs) +
		preview->stageOffsetMs;
	// A source cinematic owns only its authored interval. Previously the
	// clamped final pose hid the ordinary Pattern actor through every later
	// stage (including the pizza attack after the roar).
	if (patternClockMs < sourceStartMs ||
		static_cast<uint64_t>(patternClockMs) >= sourceStartMs + sequence->durationMs)
	{
		Debug_StopActionWorkbenchCinematic();
		return true;
	}

	const shared_ptr<CValtan> previousBoss =
		m_pActionWorkbenchCinematicBoss.lock();
	if (m_strActionWorkbenchCinematicPatternId != patternId ||
		previousBoss != previewBoss)
	{
		/* Match editor.cutscene.valtan.entrance exactly.  The Map Tool row owns
		   only the main and colorless Valtan instances; Actor64 is separate source
		   extraction data and must not be injected into this comparison preview. */
		constexpr std::array<std::string_view, 1u> entranceCompanions = {
			"entrance.colorless" };
		if (!m_ActionWorkbenchCinematicPlayer.Prepare_InstanceResources(
				instanceId, targets) ||
			!m_ActionWorkbenchCinematicPlayer.Prewarm_ObjectInstances(
				instanceId, preview->suffix == "entrance" ? 2u : 1u, targets))
		{
			status = "Action Workbench cinematic resource preparation failed: " +
				m_ActionWorkbenchCinematicPlayer.Get_Status();
			Debug_StopActionWorkbenchCinematic();
			return false;
		}
		if (preview->suffix == "entrance")
		{
			for (const std::string_view suffix : entranceCompanions)
			{
				const std::string companionId =
					"world.sequence.instance.valtan.source-preview." +
					std::string(suffix);
				if (!m_ActionWorkbenchCinematicPlayer.Prepare_InstanceResources(
						companionId, targets) ||
					!m_ActionWorkbenchCinematicPlayer.Prewarm_ObjectInstances(
						companionId, 2u, targets))
				{
					status = "Action Workbench entrance companion preparation failed: " +
						m_ActionWorkbenchCinematicPlayer.Get_Status();
					Debug_StopActionWorkbenchCinematic();
					return false;
				}
			}
		}
		Debug_StopActionWorkbenchCinematic();
		m_strActionWorkbenchCinematicPatternId.assign(patternId);
		m_pActionWorkbenchCinematicBoss = previewBoss;
		m_ActionWorkbenchCinematicRestoreWorld =
			*previewBoss->Get_Transform()->Get_WorldMatrixPtr();
		m_bActionWorkbenchCinematicRestoreValid = true;
	}

	const f32_t sourceClockMs = patternClockMs <= sourceStartMs ? 0.f :
		static_cast<f32_t>((std::min)(
			static_cast<uint64_t>(patternClockMs) - sourceStartMs,
			static_cast<uint64_t>(sequence->durationMs)));
	const WORLD_SEQUENCE_TRANSFORM_KEY sample =
		CWorldSequencePlayer::Sample_Track(*sequence, *actorTrack, sourceClockMs);
	constexpr std::array<std::string_view, 1u> entranceCompanions = {
		"entrance.colorless" };
	if (patternClockMs < sourceStartMs &&
		m_bActionWorkbenchCinematicSourcePlaying)
	{
		m_ActionWorkbenchCinematicPlayer.Stop_All(targets, true);
		m_bActionWorkbenchCinematicSourcePlaying = false;
	}
	if (patternClockMs >= sourceStartMs &&
		!m_bActionWorkbenchCinematicSourcePlaying)
	{
		m_ActionWorkbenchCinematicPlayer.Set_Paused(paused);
		bool_t played = m_ActionWorkbenchCinematicPlayer.Play(instanceId, targets);
		if (played && preview->suffix == "entrance")
		{
			for (const std::string_view suffix : entranceCompanions)
			{
				const std::string companionId =
					"world.sequence.instance.valtan.source-preview." +
					std::string(suffix);
				if (!m_ActionWorkbenchCinematicPlayer.Play(companionId, targets))
				{
					played = false;
					break;
				}
			}
		}
		if (!played)
		{
			status = "Action Workbench cinematic play failed: " +
				m_ActionWorkbenchCinematicPlayer.Get_Status();
			Debug_StopActionWorkbenchCinematic();
			return false;
		}
		m_bActionWorkbenchCinematicSourcePlaying = true;
	}
	if (m_bActionWorkbenchCinematicSourcePlaying)
	{
		m_ActionWorkbenchCinematicPlayer.Set_Paused(paused);
		bool_t sampled = m_ActionWorkbenchCinematicPlayer.Seek_InstanceToMs(
			instanceId, sourceClockMs, targets, false);
		if (sampled && preview->suffix == "entrance")
		{
			for (const std::string_view suffix : entranceCompanions)
			{
				const std::string companionId =
					"world.sequence.instance.valtan.source-preview." +
					std::string(suffix);
				if (!m_ActionWorkbenchCinematicPlayer.Seek_InstanceToMs(
						companionId, sourceClockMs, targets, false))
				{
					sampled = false;
					break;
				}
			}
		}
		if (!sampled)
		{
			status = "Action Workbench cinematic sample failed: " +
				m_ActionWorkbenchCinematicPlayer.Get_Status();
			Debug_StopActionWorkbenchCinematic();
			return false;
		}
	}
	const matrix_t restoreWorld =
		XMLoadFloat4x4(&m_ActionWorkbenchCinematicRestoreWorld);
	const float3_t scale(
		XMVectorGetX(XMVector3Length(restoreWorld.r[0])),
		XMVectorGetX(XMVector3Length(restoreWorld.r[1])),
		XMVectorGetX(XMVector3Length(restoreWorld.r[2])));
	const vector_t quaternion = XMQuaternionNormalize(
		XMLoadFloat4(&sample.rotationQuaternion));
	matrix_t world = XMMatrixRotationQuaternion(quaternion);
	world.r[0] *= scale.x * sample.scaleMultiplier.x;
	world.r[1] *= scale.y * sample.scaleMultiplier.y;
	world.r[2] *= scale.z * sample.scaleMultiplier.z;
	world.r[3] = XMVectorSet(
		instance->position.x + sample.positionOffset.x,
		instance->position.y + sample.positionOffset.y,
		instance->position.z + sample.positionOffset.z,
		1.f);
	const shared_ptr<CTransform> transform = previewBoss->Get_Transform();
	transform->Set_State(STATE::RIGHT, world.r[0]);
	transform->Set_State(STATE::UP, world.r[1]);
	transform->Set_State(STATE::LOOK, world.r[2]);
	transform->Set_State(STATE::POSITION, world.r[3]);
	/* The isolated source player renders the exact cinematic body, animation,
	   attachments and effects.  The local Pattern clone remains only as the
	   non-authoritative tracking root consumed by Stage camera cues. */
	previewBoss->Set_CinematicPresentationSuppressed(
		m_bActionWorkbenchCinematicSourcePlaying);
	status = "Source cinematic: " + std::string(preview->suffix) +
		" | actor / effects / sound / subtitles: shared source clock " +
		std::to_string(static_cast<uint32_t>(sourceClockMs)) +
		" ms" + (paused ? " (paused)" : "");
	return true;
}

bool_t CLevel_ValtanArena::Debug_PrepareActionWorkbenchDestruction(
	const VALTAN_PATTERN_VIEW& pattern, std::string& status)
{
	// Only this cinematic's deterministic stage events are supported here;
	// collision-triggered destruction still requires Server Exact playback.
	if (pattern.strPatternId != "VALTAN_ARENA_BREAK_109") return true;
	CWorldDestructionDocument source;
	if (!source.Load(CProjectDataRoot::Resolve(
		L"Encounters/Valtan/ValtanWorldEvents.json"),
		"LV_LUT_HEARTRB_ED", "ENCOUNTER_VALTAN", status)) return false;
	DESTRUCTION_SIMULATION_PROFILE profile;
	profile.profileId = "preview.workbench.valtan.arena-break-109";
	profile.groupId = "preview.group.workbench.valtan.arena-break-109";
	profile.isPreviewGroundEnabled = false;
	uint64_t durationMs = 0u;
	for (const auto& stage : pattern.Stages) durationMs += stage.iDurationMs;
	if (durationMs == 0u || durationMs > 60000u)
	{
		status = "Wall preview requires a pattern duration within 60 seconds.";
		return false;
	}
	profile.fDurationSeconds = static_cast<float>(durationMs) * 0.001f;
	CWorldDestructionDocument combined;
	combined.Reset_Empty();
	if (!combined.Add_Group(profile.groupId, status)) return false;
	std::set<std::string> groups;
	for (const auto& binding : source.Get_Bindings())
	{
		if (!binding.isEnabled || binding.patternId != pattern.strPatternId ||
			binding.eTriggerKind != DESTRUCTION_TRIGGER_KIND::STAGE_ENTER) continue;
		uint32_t stageStartMs = 0u;
		const VALTAN_STAGE_VIEW* matchedStage = nullptr;
		for (const auto& stage : pattern.Stages)
		{
			if (stage.strStageId == binding.stageId) { matchedStage = &stage; break; }
			stageStartMs += stage.iDurationMs;
		}
		const auto* mutation = source.Find_Mutation(binding.mutationId);
		const auto* group = mutation ? source.Find_Group(mutation->groupId) : nullptr;
		const auto* debris = group ?
			m_WorldDestructionDebrisPresentationDocument.Find_Group(group->groupId) : nullptr;
		const auto* projection = group ?
			m_WorldDestructionProjectionDocument.Find_Group(group->groupId) : nullptr;
		if (!matchedStage || !mutation || !group || !debris || !projection ||
			projection->bRemovesGround || debris->strMutationId != mutation->mutationId ||
			projection->strMutationId != mutation->mutationId ||
			!groups.insert(group->groupId).second ||
			stageStartMs + binding.iOffsetMs >= durationMs)
		{
			status = "Wall preview binding/profile join failed: " + binding.bindingId;
			return false;
		}
		std::set<uint64_t> publishedMembers(projection->MemberPlacementIds.begin(),
			projection->MemberPlacementIds.end());
		publishedMembers.insert(projection->SuppressionAliasPlacementIds.begin(),
			projection->SuppressionAliasPlacementIds.end());
		if (publishedMembers != std::set<uint64_t>(
			group->memberPlacementIds.begin(), group->memberPlacementIds.end()))
		{
			status = "Wall projection is stale relative to WorldEvents: " + group->groupId;
			return false;
		}
		std::set<uint64_t> projectedMembers;
		for (const auto& emitter : debris->Emitters)
		{
			DESTRUCTION_SIMULATION_ELEMENT element;
			element.elementId = binding.bindingId + "." +
				std::to_string(emitter.iSourceRuntimePlacementId);
			element.sourceRuntimePlacementId = emitter.iSourceRuntimePlacementId;
			element.suppressionAliasPlacementIds = emitter.SuppressionAliasPlacementIds;
			element.vSpawnOffset = emitter.vSpawnOffset;
			element.vDirection = emitter.vDirection;
			element.fSpeedMetersPerSecond = emitter.fSpeedMetersPerSecond;
			element.fGravityScale = emitter.fGravityScale;
			element.fLifetimeSeconds = emitter.fLifetimeSeconds;
			element.Trigger.eKind = DESTRUCTION_SIMULATION_TRIGGER_KIND::TIMELINE_TIME;
			element.Trigger.fTimeSeconds = (stageStartMs + binding.iOffsetMs) * 0.001f;
			std::vector<uint64_t> members = emitter.SuppressionAliasPlacementIds;
			members.push_back(emitter.iSourceRuntimePlacementId);
			for (uint64_t member : members)
			{
				if (!projectedMembers.insert(member).second ||
					!combined.Add_Member(profile.groupId, member, status))
				{
					status = "Duplicate wall preview member: " + std::to_string(member);
					return false;
				}
			}
			profile.Elements.push_back(std::move(element));
		}
		if (projectedMembers != std::set<uint64_t>(
			group->memberPlacementIds.begin(), group->memberPlacementIds.end()))
		{
			status = "Wall preview members differ from WorldEvents: " + group->groupId;
			return false;
		}
	}
	if (profile.Elements.empty() ||
		!CDestructionSimulationDocument::Validate_Profile(profile, status))
	{
		if (status.empty()) status = "No wall events were found for this pattern.";
		return false;
	}
	// ImGui transport only stages data; MainApp consumes it after typed commands.
	m_ActionWorkbenchDestructionGroup = std::move(combined);
	m_PendingWorkbenchDestruction = std::move(profile);
	m_strWorkbenchDestructionPattern = pattern.strPatternId;
	m_strWorkbenchDestructionFailure.clear();
	return true;
}

bool_t CLevel_ValtanArena::Debug_StopActionWorkbenchDestruction(std::string& status)
{
	m_ActionWorkbenchDestruction.Clear();
	if (m_ActionWorkbenchDestruction.Get_Runtime().Is_Staged())
	{
		status = m_ActionWorkbenchDestruction.Get_Runtime().Get_Status();
		return false;
	}
	m_PendingWorkbenchDestruction.reset();
	m_strWorkbenchDestructionPattern.clear();
	m_strWorkbenchDestructionFailure.clear();
	return true;
}

bool_t CLevel_ValtanArena::Debug_SampleActionWorkbenchDestruction(
	const std::string_view patternId, const uint32_t clockMs, std::string& status)
{
	if (patternId != "VALTAN_ARENA_BREAK_109")
		return Debug_StopActionWorkbenchDestruction(status);
	if (!m_strWorkbenchDestructionFailure.empty())
	{
		status = m_strWorkbenchDestructionFailure;
		return false;
	}
	auto fail = [&](std::string failure)
	{
		std::string cleanup;
		if (!Debug_StopActionWorkbenchDestruction(cleanup)) failure += "; " + cleanup;
		m_strWorkbenchDestructionFailure = std::move(failure);
		status = m_strWorkbenchDestructionFailure;
		return false;
	};
	if (!m_strSourceCinematic.empty() ||
		m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount() != 0u)
		return fail("Local wall preview yielded to active Server destruction/cinematic playback.");
	if (m_strWorkbenchDestructionPattern != patternId)
		return fail("Wall preview is not prepared; restart local Play.");
	if (m_PendingWorkbenchDestruction)
	{
		m_ActionWorkbenchDestruction.Clear();
		if (m_ActionWorkbenchDestruction.Get_Runtime().Is_Staged())
			return fail(m_ActionWorkbenchDestruction.Get_Runtime().Get_Status());
		m_ActionWorkbenchDestruction.Request_StageProfile(
			*m_PendingWorkbenchDestruction, m_PendingWorkbenchDestruction->groupId,
			m_ActionWorkbenchDestructionGroup, m_DeployRuntime, ETOUI(LEVEL::VALTAN_ARENA));
		m_PendingWorkbenchDestruction.reset();
	}
	if (!m_ActionWorkbenchDestruction.Sample_ExternalTime(clockMs * 0.001f, status))
		return fail(status);
	return true;
}

void CLevel_ValtanArena::Debug_StopActionWorkbenchCinematic()
{
	if (m_strActionWorkbenchCinematicPatternId.empty() &&
		m_pActionWorkbenchCinematicBoss.expired() &&
		!m_bActionWorkbenchCinematicRestoreValid &&
		!m_bActionWorkbenchCinematicSourcePlaying)
		return;
	m_ActionWorkbenchCinematicPlayer.Stop_All(
		SourceCinematicTargets(), true);
	if (const shared_ptr<CValtan> boss =
			m_pActionWorkbenchCinematicBoss.lock())
	{
		boss->Set_CinematicPresentationSuppressed(false);
		if (m_bActionWorkbenchCinematicRestoreValid && boss->Get_Transform())
		{
			const matrix_t restore =
				XMLoadFloat4x4(&m_ActionWorkbenchCinematicRestoreWorld);
			boss->Get_Transform()->Set_State(STATE::RIGHT, restore.r[0]);
			boss->Get_Transform()->Set_State(STATE::UP, restore.r[1]);
			boss->Get_Transform()->Set_State(STATE::LOOK, restore.r[2]);
			boss->Get_Transform()->Set_State(STATE::POSITION, restore.r[3]);
		}
	}
	m_strActionWorkbenchCinematicPatternId.clear();
	m_pActionWorkbenchCinematicBoss.reset();
	m_bActionWorkbenchCinematicRestoreValid = false;
	m_bActionWorkbenchCinematicSourcePlaying = false;
}
#endif

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

bool_t CLevel_ValtanArena::Has_DebugValtanBoss() const
{
    return nullptr != m_Replication.Find_PrimaryValtanPresentation();
}

std::string CLevel_ValtanArena::Get_DebugValtanPresentationDiagnostic() const
{
    const auto primary = m_Replication.Find_PrimaryValtanPresentation();
    return primary ? primary->Get_PresentationDiagnostic() : "No replicated primary Valtan presentation.";
}

void CLevel_ValtanArena::Update_DebugValtanBossCommand()
{
    using namespace LostArk::Shared;
    S2C_WORLD_ENTITY_SPAWN_RESULT result{};
    uint64_t token = 0u;
    while (CNetworkManager::Get().Try_Consume_WorldEntitySpawnResult(result, &token))
    {
        if (!m_iDebugValtanSpawnToken || token != m_iDebugValtanSpawnToken ||
            result.strPlacementId != "boss.valtan.center") continue;
        if (result.eResult == WORLD_ENTITY_SPAWN_RESULT::REJECTED)
        {
            m_iDebugValtanSpawnToken = 0u;
            m_iDebugValtanCommandStartedMs = 0u;
            m_bDebugValtanCommandFailed = true;
            m_strDebugValtanBossCommandStatus = "Server rejected the Valtan spawn request.";
        }
    }
    if (m_iDebugValtanSpawnToken && Has_DebugValtanBoss())
    {
        m_iDebugValtanSpawnToken = 0u;
        m_iDebugValtanCommandStartedMs = 0u;
        m_strDebugValtanBossCommandStatus = "Valtan Server spawn and primary presentation are ready.";
    }
    if (Is_DebugValtanBossCommandPending() &&
        GetTickCount64() - m_iDebugValtanCommandStartedMs > 10000u)
    {
        m_iDebugValtanSpawnToken = 0u;
        m_bDebugValtanDespawnPending = false;
        m_iDebugValtanCommandStartedMs = 0u;
        m_bDebugValtanCommandFailed = true;
        m_strDebugValtanBossCommandStatus = "Valtan command timed out before its replicated result; retry the command.";
    }
}

bool_t CLevel_ValtanArena::Debug_DespawnValtanBoss(std::string& status)
{
    Update_DebugValtanBossCommand();
    if (Is_DebugValtanBossCommandPending())
    { status = "Wait for the pending Valtan spawn/despawn command."; return false; }
    if (!Has_DebugValtanBoss())
    { status = m_strDebugValtanBossCommandStatus = "Valtan is already despawned."; return true; }
    if (!m_pWorldEntityCommandSink || !m_iNextDebugValtanRequestSequence)
    { status = m_strDebugValtanBossCommandStatus = "Valtan command sink or sequence is unavailable."; return false; }
    const auto sequence = m_iNextDebugValtanRequestSequence;
    if (!m_pWorldEntityCommandSink->Request_DespawnAllWorldEntities(sequence))
    { status = m_strDebugValtanBossCommandStatus = "Valtan despawn request could not be sent."; return false; }
    m_iNextDebugValtanRequestSequence = sequence == UINT32_MAX ? 0u : sequence + 1u;
    m_bDebugValtanDespawnPending = true;
    m_bDebugValtanCommandFailed = false;
    m_iDebugValtanCommandStartedMs = GetTickCount64();
    status = m_strDebugValtanBossCommandStatus = "Waiting for Server Valtan and dependent despawns.";
    return true;
}

bool_t CLevel_ValtanArena::Debug_EnsureValtanBossForPlay(bool_t& ready, std::string& status, bool_t retryFailed)
{
    ready = false;
    Update_DebugValtanBossCommand();
    if (retryFailed) m_bDebugValtanCommandFailed = false;
    if (m_bDebugValtanCommandFailed)
    { status = m_strDebugValtanBossCommandStatus; return false; }
    if (m_bDebugValtanDespawnPending)
    { status = m_strDebugValtanBossCommandStatus; return true; }
    if (Has_DebugValtanBoss())
    { ready = true; status = "The replicated Valtan presentation is available."; return true; }
    if (m_iDebugValtanSpawnToken)
    { status = m_strDebugValtanBossCommandStatus; return true; }
    if (!m_pWorldEntityCommandSink ||
        !m_pWorldEntityCommandSink->Request_SpawnWorldEntity("boss.valtan.center", &m_iDebugValtanSpawnToken))
    { status = m_strDebugValtanBossCommandStatus = "Valtan spawn request could not be sent."; return false; }
    m_iDebugValtanCommandStartedMs = GetTickCount64();
    status = m_strDebugValtanBossCommandStatus = "Preparing Valtan: waiting for Server spawn and primary presentation.";
    return true;
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
	if (const auto* pattern = m_ValtanEncounterReference.Find_Pattern(input.strPatternId);
		pattern && input.iStageIndex < pattern->stages.size())
		input.strStageId = pattern->stages[input.iStageIndex].stageId;
	input.vBossPosition = boss.vPosition;
	input.fBossYawDegrees = boss.fYawDegrees;
    std::vector<BOSS_STAGE_CAMERA_SAMPLE> cameraInvocations;
    input.hasStageCameraInvocations = !input.isBossDead &&
        CValtan::Get_ActiveStageCameraInvocations(input.strStageActionId, cameraInvocations);
    if (input.hasStageCameraInvocations)
    {
        float age = 0.f;
        if (CActionPresentationTimeline::Try_ResolveActionAgeSeconds(input.iServerTick,
                input.iActionStartTick, static_cast<float>(m_ValtanEncounterReference.Get_FixedTickHz()), age))
            for (const auto& invocation : cameraInvocations)
                if (age * 1000.f >= invocation.iStartMs &&
                    age * 1000.f < double(invocation.iStartMs) + invocation.iDurationMs)
                {
                    input.strInvokedCameraCueId = invocation.strCueId;
                    input.iCameraStartOffsetMs = invocation.iStartMs;
                    input.iCameraDurationMs = invocation.iDurationMs;
                }
    }

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

    m_SourceCinematicPlayer.Update_SoundTails(fTimeDelta);
	Prepare_SourceCinematicInput(input);
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	const bool_t hasCameraPose = m_ValtanCinematicCameraController.Update(input, fTimeDelta, pose);
	// Camera cuts may end before the source actor, FX and dialogue. They share
	// the Server action origin, not the camera's ownership lifetime.
	if (!Update_SourceCinematic(input))
	{
		End_CinematicCamera();
		return;
	}
	if (!hasCameraPose)
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
			static_cast<uint64_t>(input.iNetEntityId),
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
			static_cast<uint64_t>(input.iNetEntityId);
	}
	else if (nullptr != m_pCamera->Get_FollowTarget())
	{
		m_pCinematicRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
	}

	if (m_iCinematicCameraOwnerId !=
		static_cast<uint64_t>(input.iNetEntityId))
	{
		End_CinematicCamera();
		return;
	}
	if (!(pose.hasUp ? m_pCamera->Apply_PresentationPoseWithUp(
        m_iCinematicCameraOwnerId, pose.vEye, pose.vLookAt, pose.vUp, pose.fFovYDegrees) :
        m_pCamera->Apply_PresentationPose(m_iCinematicCameraOwnerId,
            pose.vEye, pose.vLookAt, pose.fFovYDegrees)))
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
	const float3_t positionOffset = m_FollowCameraProfile.positionOffset;
	const float3_t lookOffset = CArenaCameraProfile::LookOffset(m_FollowCameraProfile);
	VALTAN_CINEMATIC_CAMERA_POSE followPose{};
	XMStoreFloat3(&followPose.vEye, XMVectorSetW(
		targetPosition + XMLoadFloat3(&positionOffset), 1.f));
	XMStoreFloat3(&followPose.vLookAt, XMVectorSetW(
		targetPosition + XMLoadFloat3(&lookOffset), 1.f));
	followPose.fFovYDegrees = m_FollowCameraProfile.fovYDegrees;
	const auto rotation = m_FollowCameraProfile.rotationDegrees;
	const auto basis = XMMatrixRotationRollPitchYaw(XMConvertToRadians(rotation.x),
		XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
	XMStoreFloat3(&followPose.vUp, basis.r[1]);
	followPose.hasUp = true;

	VALTAN_CINEMATIC_CAMERA_POSE transitionPose{};
	if (!m_ValtanCinematicCameraController.Update_ExitTransition(
		followPose, fTimeDelta, transitionPose) ||
		!m_pCamera->Apply_PresentationPoseWithUp(
			m_iCinematicCameraOwnerId,
			transitionPose.vEye,
			transitionPose.vLookAt,
			transitionPose.vUp,
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
	Stop_SourceCinematic();
	/* Map Effects have their own level lifetime. Camera completion must not
	   cancel a stage-owned world Effect or restore unrelated Deploy surfaces. */
	End_CinematicCameraOverride();
}

HRESULT CLevel_ValtanArena::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	/* The award page is a full-screen modal: no world text at all while it is up. */
	if (!Is_MvpResultVisible())
	{
		m_PlayerNameplateView.Render(m_NameplatePlayers, &m_Replication.Get_PartyRoster());
		m_ChatBubbleView.Render(m_Replication, m_NameplatePlayers);
	}
	m_InteractKeyPrompt.Render_Text();
	m_PartyInteraction.Render(m_pPlayerCommandSink);
	/* Award page labels over everything else this Level draws; its image layers are
	   CUI_Sprite objects on Layer_UI and need no call. */
	m_GateProgressView.Render_Text();
	if (nullptr != m_pMvpResultView)
	{
		CUITextLayerScope PageText(UI_TEXT_LAYER::PAGE);
		m_pMvpResultView->Render();
	}

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(TEXT("Valtan Arena Map"));
#endif

	return S_OK;
}

bool_t CLevel_ValtanArena::Load_TriggerMarkers()
{
	/* Same document the Server publishes from, read the way KoukuSaydon reads its own:
	   the marker sits on the exact trigger box centre, nothing is copied or guessed. */
	static constexpr const char* VALTAN_AREA_ID = "LV_LUT_HEARTRB_ED";
	/* Same ids the Server uses for the boss entry: stepping on Stage_Boss starts the boss
	   and carries the player to the Stage_Boss_ArenaEntry box, where they land. */
	static constexpr const char* BOSS_ENTRY_TRIGGER_ID = "Stage_Boss";
	static constexpr const char* BOSS_ARRIVAL_TRIGGER_ID = "Stage_Boss_ArenaEntry";
	CWorldGameplayDocument document;
	std::string status;
	if (!document.Load(CProjectDataRoot::Resolve(std::filesystem::path("Worlds") /
		std::string(VALTAN_AREA_ID) / "Gameplay.world.json"),
		std::string(VALTAN_AREA_ID), status))
	{
		OutputDebugStringA(("[ValtanTriggerMarker] " + status + "\n").c_str());
		return false;
	}
	std::vector<TRIGGER_MARKER> staged;
	for (const WORLD_GAMEPLAY_PLACEMENT& placement : document.Get_Placements())
	{
		if (WORLD_PLACEMENT_KIND::TRIGGER_BOX != placement.eKind || !placement.isEnabled)
			continue;
		/* The marker sits on the box the player steps on to go somewhere: every player-move
		   box except an arrival box, plus the boss entry box. Stage_Boss is an
		   activateEncounter, but firing it also moves the player to Stage_Boss_ArenaEntry,
		   so Stage_Boss is the departure and ArenaEntry is where they land. Wave
		   activations and a box authored at a move's landing (Stage_MiniBoss_Spawn sits
		   exactly on the Stage_MiniBoss destination) are arrivals, so none of those show
		   the marker. */
		if (1u != placement.triggerEvents.size())
			continue;
		const bool bossEntry = BOSS_ENTRY_TRIGGER_ID == placement.placementId;
		const bool departureMove =
			WORLD_TRIGGER_EVENT_KIND::MOVE_PLAYER == placement.triggerEvents.front().eKind &&
			BOSS_ARRIVAL_TRIGGER_ID != placement.placementId;
		if (!bossEntry && !departureMove)
			continue;
		TRIGGER_MARKER marker;
		marker.placementId = placement.placementId;
		XMStoreFloat4x4(&marker.rootWorld, XMMatrixTranslation(
			placement.position.x, placement.position.y, placement.position.z));
		staged.push_back(std::move(marker));
	}
	Clear_TriggerMarkers();
	m_TriggerMarkers = std::move(staged);
	return true;
}

void CLevel_ValtanArena::Clear_TriggerMarkers()
{
	for (auto& marker : m_TriggerMarkers)
		CEffectPresentationService::Stop_WorldRoot(marker.handle);
	m_TriggerMarkers.clear();
}

void CLevel_ValtanArena::Update_TriggerMarkerClocks(const f32_t deltaSeconds)
{
	if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.f)
		return;
	for (auto& marker : m_TriggerMarkers)
	{
		if (marker.retired)
			continue;
		if (marker.clockStarted)
			marker.seconds = std::fmod(marker.seconds + deltaSeconds, 7.f);
		marker.clockStarted = true;
	}
}

void CLevel_ValtanArena::Submit_TriggerMarkers()
{
	if (CGameInstance::Get().Get_CurrentLevelID() != ETOUI(LEVEL::VALTAN_ARENA))
		return;
	MAP_CAMERA_CULL_SNAPSHOT camera;
	const bool_t cameraValid = CMapAssetRenderUtils::Capture_CameraCullSnapshot(camera);
	for (auto& marker : m_TriggerMarkers)
	{
		if (marker.retired)
			continue;
		// The fixed move_destination asset fits an 8m sphere, including its
		// source velocity/lifetime, billboard size, camera offset and mesh.
		// The outer band retains live history during small camera reversals.
		MAP_FRUSTUM_CULLING_POLICY policy;
		policy.baseMargin = marker.active ? 32.f : 16.f;
		MAP_FRUSTUM_RUNTIME_STATE cullState;
		MAP_FRUSTUM_CULL_DECISION decision;
		const float3_t center{ marker.rootWorld._41, marker.rootWorld._42, marker.rootWorld._43 };
		const bool_t visible = !cameraValid ||
			!CMapAssetRenderUtils::Evaluate_FrustumVisibility(policy, camera, {}, {},
				0u, center, 8.f, cullState, decision) || decision.shouldRender;
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
			// Reuse the Loader's prepared target and commit only this marker.
			EFFECT_LEVEL_PLACEMENT_SPAWN_DESC desc;
			desc.iLevelIndex = ETOUI(LEVEL::VALTAN_ARENA);
			desc.strPlacementId = "valtan.trigger." + marker.placementId;
			desc.strEffectAssetId = "effect.world.move_destination";
			desc.RootWorld = marker.rootWorld;
			desc.bExternallySampled = true;
			std::string status;
			if (!CEffectPresentationService::Spawn_LevelPlacement(desc, marker.handle, status))
			{
				marker.retired = true;
				OutputDebugStringA(("[ValtanTriggerMarker] " + marker.placementId +
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
			OutputDebugStringA(("[ValtanTriggerMarker] Sample/submission failed: " +
				marker.placementId + ": " + CEffectPresentationService::Get_Status() + "\n").c_str());
			continue;
		}
		marker.active = true;
	}
}

void CLevel_ValtanArena::Render_MvpPortraits()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Render_Portraits();
}

bool_t CLevel_ValtanArena::Is_MvpResultVisible() const
{
	return nullptr != m_pMvpResultView && m_pMvpResultView->Is_Visible();
}

void CLevel_ValtanArena::Hide_MvpResult()
{
	if (nullptr != m_pMvpResultView)
		m_pMvpResultView->Hide();
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

namespace
{
	/* EFTable_ZoneEpicGate.GroupId 101 = Valtan, gate 1, normal; an eight-player raid
	   takes the eight-player cutoffs. */
	constexpr int32_t VALTAN_RAID_GROUP_ID = 101;
	constexpr const char* VALTAN_DIFFICULTY_ID = "normal";
	constexpr int32_t VALTAN_MVP_PARTY_SIZE = 8;
}

void CLevel_ValtanArena::Show_MvpResult(const bool_t bReplayLast)
{
	if (nullptr == m_pMvpResultView)
		return;
	for (weak_ptr<CCharacter>& pStaged : m_MvpStageCharacters)
		pStaged.reset();
	if (m_bHasRaidMvpResult && (m_bRaidMvpResultFresh || bReplayLast))
	{
		vector<LostArk::Shared::PLAYER_ID> StagePlayerIds;
		const MVP_RESULT_DATA Data = CMvpAwardCatalog::Get().Build_ServerPage(
			VALTAN_RAID_GROUP_ID, VALTAN_DIFFICULTY_ID, VALTAN_MVP_PARTY_SIZE,
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
		m_pMvpResultView->Show(Data);
		return;
	}
#ifdef _DEBUG
	/* The O-key test clear has no Server result behind it. */
	m_MvpStageCharacters[0] = m_Replication.Get_LocalCharacter();
	m_pMvpResultView->Show(CMvpAwardCatalog::Get().Build_PreviewPage(
		VALTAN_RAID_GROUP_ID, 1, VALTAN_DIFFICULTY_ID, VALTAN_MVP_PARTY_SIZE));
#endif
}

void CLevel_ValtanArena::Trigger_RaidClear()
{
	m_fRaidClearElapsedSeconds = 0.f;
	m_bRaidClearCuePlayed = false;
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
    const bool_t waitForFinale = m_bSourceDeathStarted && !m_bSourceDeathFinished;
    if (isBossDead && !m_bRaidClearWasBossDead && !waitForFinale)
        Trigger_RaidClear();
    if (!waitForFinale) m_bRaidClearWasBossDead = isBossDead;

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

	const f32_t fPreviousElapsedSeconds = m_fRaidClearElapsedSeconds;
	if (m_fRaidClearElapsedSeconds >= 0.f)
		m_fRaidClearElapsedSeconds += fTimeDelta;
	/* The award page starts the instant the clear mark cuts, as on KoukuSaydon. A
	   crossing, not a latch: the test-mode jump above lands past the end and skips it. */
	if (nullptr != m_pMvpResultView &&
		fPreviousElapsedSeconds >= 0.f && fPreviousElapsedSeconds < RAIDCLEAR_TOTAL_SECONDS &&
		m_fRaidClearElapsedSeconds >= RAIDCLEAR_TOTAL_SECONDS)
	{
		Show_MvpResult(false);
	}
	const bool_t isMvpVisible = nullptr != m_pMvpResultView && m_pMvpResultView->Is_Visible();

	/* Real cue name confirmed in the extracted sound resource pool
	(Sound/UI/System/sys_raid_success1__457395004.wav) -- epicgatecommonclear.gfx itself
	carries no embedded sound (Scaleform UI movies play native-triggered cues, not baked
	audio), so this is played from here rather than anywhere inside m_pRaidClearView.

	It waits for the reveal instead of firing with the clock: the overlay's first
	RAIDCLEAR_REVEAL_SECONDS are empty, so starting the 5.77s cue at zero ran it well
	ahead of the picture. The test-mode branch above jumps straight to
	RAIDCLEAR_TOTAL_SECONDS, which is why this is a latch rather than a crossing -- that
	jump must not fire a cue for a celebration that is already over. */
	if (!m_bRaidClearCuePlayed && m_fRaidClearElapsedSeconds >= RAIDCLEAR_REVEAL_SECONDS &&
		m_fRaidClearElapsedSeconds < RAIDCLEAR_TOTAL_SECONDS)
	{
		m_bRaidClearCuePlayed = true;
		const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
			L"Sound/UI/System/sys_raid_success1__457395004.wav");
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}

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
	/* The clear screen's own return button is gone: the gate progress panel's exit button
	   (top left) takes the same trip once the award page has closed. */
	m_pRaidClearView->Set_SlotVisible("RaidClear_ReturnButton", false);
	m_bRaidClearReturnAvailable = isAfterRaidClear && !isMvpVisible;
	if (isShowing)
	{
		const f32_t fRevealAlpha = (m_fRaidClearElapsedSeconds < RAIDCLEAR_REVEAL_SECONDS) ?
			(m_fRaidClearElapsedSeconds / RAIDCLEAR_REVEAL_SECONDS) : 1.f;
		for (const char* const szSlotId : RAIDCLEAR_FADING_SLOTS)
			m_pRaidClearView->Set_SlotAlpha(szSlotId, fRevealAlpha);
	}
	m_pRaidClearView->Update(fTimeDelta);

	HUD_RAIDCLEAR_TEXT_RECTS textRects;
	textRects.isValid = isShowing &&
		m_pRaidClearView->Get_SlotRect("RaidClear_TitleTextBox",
			textRects.fTitleX, textRects.fTitleY,
			textRects.fTitleWidth, textRects.fTitleHeight);
	textRects.isButtonValid = false;
	CCombatHUDViewModel::Get().Set_RaidClearTextRects(textRects);
}

/* No matching Render_RaidClear() -- migrated to real CUI_Sprite GameObjects
that self-render through the normal CRenderer/RENDERGROUP::UI pipeline
(Update_RaidClear above drives their visibility/alpha/texture instead). */

namespace
{
	/* announce.gfx ItemSlotAnnounceListItem (sprite 360) runs at 40 fps. Frames 1..10 bring the
	band, slot frame and icon from alpha 0 to 1 while the band slides 10 stage px right; frames
	1..16 do the same for the text line. The quality row carries no colour transform, so it is up
	from frame 1. Frame 29 dispatches "complete"; the host's `time` then holds the item and
	BaseAnnounceListItem::initComplete fades it out over 0.35 s. The list fades a new item in over
	0.15 s (DefaultAnnounceListCtrl::addListItem). The hold is the one value the movie does not
	carry; 2 s is the project's choice. */
	constexpr f32_t ITEM_ANNOUNCE_FPS = 40.f;
	constexpr f32_t ITEM_ANNOUNCE_INTRO_SECONDS = 29.f / ITEM_ANNOUNCE_FPS;
	constexpr f32_t ITEM_ANNOUNCE_CHROME_IN_FRAMES = 9.f;
	constexpr f32_t ITEM_ANNOUNCE_TEXT_IN_FRAMES = 15.f;
	constexpr f32_t ITEM_ANNOUNCE_SLIDE_REF_PX = 10.f * 2.f / 3.f;
	constexpr f32_t ITEM_ANNOUNCE_HOLD_SECONDS = 2.f;
	constexpr f32_t ITEM_ANNOUNCE_LIST_FADE_IN_SECONDS = 0.15f;
	constexpr f32_t ITEM_ANNOUNCE_FADE_OUT_SECONDS = 0.35f;
	constexpr f32_t ITEM_ANNOUNCE_TOTAL_SECONDS =
		ITEM_ANNOUNCE_INTRO_SECONDS + ITEM_ANNOUNCE_HOLD_SECONDS + ITEM_ANNOUNCE_FADE_OUT_SECONDS;

	/* qualityProgress.colorList "cut,frame,colour": the gauge shows the frame of the highest cut
	the value reaches and the value text takes its colour. */
	struct ITEM_ANNOUNCE_QUALITY_TIER { int32_t iCut; int32_t iFrame; uint32_t iColor; };
	constexpr ITEM_ANNOUNCE_QUALITY_TIER ITEM_ANNOUNCE_QUALITY_TIERS[] = {
		{ 0, 1, 0x686660u }, { 1, 1, 0xff6000u }, { 10, 2, 0xffd200u }, { 30, 3, 0x91fe02u },
		{ 70, 4, 0x00b5ffu }, { 90, 5, 0xce43fcu }, { 100, 6, 0xfe9600u } };

	const ITEM_ANNOUNCE_QUALITY_TIER& Item_AnnounceQualityTier(const int32_t iQuality)
	{
		const ITEM_ANNOUNCE_QUALITY_TIER* pTier = &ITEM_ANNOUNCE_QUALITY_TIERS[0];
		for (const ITEM_ANNOUNCE_QUALITY_TIER& Tier : ITEM_ANNOUNCE_QUALITY_TIERS)
			if (iQuality >= Tier.iCut)
				pTier = &Tier;
		return *pTier;
	}

	/* Light sweep over the icon: sprite 360 depth 7 (masked by depth 6) lives on frames 10..28;
	build_item_announce_ui.py bakes one image per frame. */
	constexpr int32_t ITEM_ANNOUNCE_SWEEP_FIRST_FRAME = 10;
	constexpr int32_t ITEM_ANNOUNCE_SWEEP_LAST_FRAME = 28;

	constexpr const char* ITEM_ANNOUNCE_ART_SLOTS[] = {
		"ItemAnnounce_Band", "ItemAnnounce_SlotFrame", "ItemAnnounce_Icon", "ItemAnnounce_Sweep",
		"ItemAnnounce_QualityPlate", "ItemAnnounce_QualityWell", "ItemAnnounce_QualityGauge" };

	/* The project keeps no item quality; the announce shows a fixed 70..90 per item id so the
	row reads like retail's and never changes between two pickups of the same item. */
	int32_t Item_AnnounceQuality(const string& strItemId)
	{
		uint32_t iHash = 2166136261u;
		for (const unsigned char c : strItemId)
			iHash = (iHash ^ c) * 16777619u;
		return 70 + static_cast<int32_t>(iHash % 21u);
	}

	/* Item grade colours as GameMsg writes them: tooltip / setup-pass / recommendation strings
	colour 고대 #E3C7A1, 유물 #FF6000, 전설 #FE9600, 영웅 #CE43FC, 희귀 #00B5FF, 고급 #91FE02. */
	uint32_t Item_GradeRgb(const string& strGrade)
	{
		if ("ancient" == strGrade) return 0xe3c7a1u;
		if ("relic" == strGrade) return 0xff6000u;
		if ("legend" == strGrade) return 0xfe9600u;
		if ("epic" == strGrade) return 0xce43fcu;
		if ("rare" == strGrade) return 0x00b5ffu;
		if ("uncommon" == strGrade) return 0x91fe02u;
		return 0xffffffu;
	}

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

	/* The award page owns the screen: the announce draws nothing under it and its clock waits,
	so the items still queued when the page opened are shown once it closes instead of running
	out of time behind it. */
	if (Is_MvpResultVisible())
	{
		for (const char* pSlot : ITEM_ANNOUNCE_ART_SLOTS)
			m_pItemAnnounceView->Set_SlotVisible(pSlot, false);
		CCombatHUDViewModel::Get().Set_ItemAnnounceTextRects(HUD_ITEMANNOUNCE_TEXT_RECTS{});
		return;
	}

	if (m_fItemAnnounceElapsedSeconds >= 0.f)
	{
		m_fItemAnnounceElapsedSeconds += fTimeDelta;
		if (m_fItemAnnounceElapsedSeconds >= ITEM_ANNOUNCE_TOTAL_SECONDS)
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
		/* Quality gauge: the target mask reveals quality/100 of the track, whose frame follows
		the colour tier the value reaches. */
		const int32_t iQuality = Item_AnnounceQuality(m_strItemAnnounceCurrentItemId);
		m_pItemAnnounceView->Set_SlotTexture("ItemAnnounce_QualityGauge",
			"UI/ItemAnnounce/ItemAnnounce_QualityGauge_" +
			std::to_string(Item_AnnounceQualityTier(iQuality).iFrame) + ".png");
		m_pItemAnnounceView->Set_SlotFillRatio("ItemAnnounce_QualityGauge", iQuality / 100.f);
		m_iItemAnnounceSweepFrame = 0;

		const std::filesystem::path soundPath = CRuntimeAssetRoot::Resolve(
			L"Sound/UI/System/sys_item_itemgetepic1__202768724.wav");
		CGameInstance::Get().Play_Sound(soundPath.wstring(), 1.f);
	}

	/* The award page owns the screen: the announce keeps its own clock running underneath but
	draws nothing, the way retail's list sits behind that window. */
	const bool_t isShowing = m_fItemAnnounceElapsedSeconds >= 0.f;
	HUD_ITEMANNOUNCE_TEXT_RECTS textRects;
	if (!isShowing)
	{
		for (const char* pSlot : ITEM_ANNOUNCE_ART_SLOTS)
			m_pItemAnnounceView->Set_SlotVisible(pSlot, false);
		CCombatHUDViewModel::Get().Set_ItemAnnounceTextRects(textRects);
		return;
	}

	/* The retail timeline: see the ITEM_ANNOUNCE_* constants. */
	const f32_t fElapsed = m_fItemAnnounceElapsedSeconds;
	const f32_t fFrame = fElapsed * ITEM_ANNOUNCE_FPS;
	const f32_t fChromeIn = (std::min)(fFrame / ITEM_ANNOUNCE_CHROME_IN_FRAMES, 1.f);
	const f32_t fTextIn = (std::min)(fFrame / ITEM_ANNOUNCE_TEXT_IN_FRAMES, 1.f);
	const f32_t fListIn = (std::min)(fElapsed / ITEM_ANNOUNCE_LIST_FADE_IN_SECONDS, 1.f);
	const f32_t fFadeOut = 1.f - (std::clamp)((fElapsed - ITEM_ANNOUNCE_INTRO_SECONDS -
		ITEM_ANNOUNCE_HOLD_SECONDS) / ITEM_ANNOUNCE_FADE_OUT_SECONDS, 0.f, 1.f);
	const f32_t fItemAlpha = fListIn * fFadeOut;

	for (const char* pSlot : ITEM_ANNOUNCE_ART_SLOTS)
		m_pItemAnnounceView->Set_SlotVisible(pSlot, true);
	m_pItemAnnounceView->Set_SlotPosition("ItemAnnounce_Band",
		m_fItemAnnounceBandX - (1.f - fChromeIn) * ITEM_ANNOUNCE_SLIDE_REF_PX, m_fItemAnnounceBandY);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_Band", fChromeIn * fItemAlpha);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_SlotFrame", fChromeIn * fItemAlpha);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_Icon", fChromeIn * fItemAlpha);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_QualityPlate", fItemAlpha);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_QualityWell", fItemAlpha);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_QualityGauge", fItemAlpha);

	/* Timeline frame numbers start at 1 on the item's first update. */
	const int32_t iTimelineFrame = static_cast<int32_t>(fFrame) + 1;
	const bool_t bSweeping = iTimelineFrame >= ITEM_ANNOUNCE_SWEEP_FIRST_FRAME &&
		iTimelineFrame <= ITEM_ANNOUNCE_SWEEP_LAST_FRAME;
	if (bSweeping && iTimelineFrame != m_iItemAnnounceSweepFrame)
	{
		m_iItemAnnounceSweepFrame = iTimelineFrame;
		m_pItemAnnounceView->Set_SlotTexture("ItemAnnounce_Sweep",
			"UI/ItemAnnounce/ItemAnnounce_Sweep_" + std::to_string(iTimelineFrame) + ".png");
	}
	m_pItemAnnounceView->Set_SlotVisible("ItemAnnounce_Sweep", bSweeping);
	m_pItemAnnounceView->Set_SlotAlpha("ItemAnnounce_Sweep", fItemAlpha);

	const int32_t iQuality = Item_AnnounceQuality(m_strItemAnnounceCurrentItemId);
	const ITEM_ANNOUNCE_QUALITY_TIER* pTier = &Item_AnnounceQualityTier(iQuality);

	textRects.isValid = m_pItemAnnounceView->Get_SlotRect("ItemAnnounce_TextBox",
		textRects.fTextX, textRects.fTextY, textRects.fTextWidth, textRects.fTextHeight) &&
		m_pItemAnnounceView->Get_SlotRect("ItemAnnounce_QualityLabelBox",
			textRects.fQualityLabelX, textRects.fQualityLabelY,
			textRects.fQualityLabelWidth, textRects.fQualityLabelHeight) &&
		m_pItemAnnounceView->Get_SlotRect("ItemAnnounce_QualityValueBox",
			textRects.fQualityValueX, textRects.fQualityValueY,
			textRects.fQualityValueWidth, textRects.fQualityValueHeight);
	if (textRects.isValid)
	{
		/* The text line slides in 10 stage px with its own fade. */
		textRects.fTextX -= (1.f - fTextIn) * ITEM_ANNOUNCE_SLIDE_REF_PX;
		textRects.fTextAlpha = fTextIn * fItemAlpha;
		textRects.fQualityAlpha = fItemAlpha;
		textRects.strQualityValue = std::to_wstring(iQuality);
		textRects.iQualityRgb = pTier->iColor;
		const ITEM_DEFINITION* pDefinition =
			CItemCatalog::Find_ById(m_strItemAnnounceCurrentItemId);
		wstring strItemName;
		if (nullptr != pDefinition &&
			ConvertUtf8ToWide(pDefinition->strDisplayName, strItemName))
		{
			// GameMsg sys.common.item_get "{0}을 획득하였습니다." -- the particle follows the
			// item name's last syllable (Has_HangulFinalConsonant above). Kept separate from the
			// name so RenderItemAnnounceText can draw the name in its grade colour.
			textRects.strSuffix = Has_HangulFinalConsonant(strItemName.back()) ?
				L"\xC744 \xD68D\xB4DD\xD558\xC600\xC2B5\xB2C8\xB2E4." :  // "을 획득하였습니다."
				L"\xB97C \xD68D\xB4DD\xD558\xC600\xC2B5\xB2C8\xB2E4.";  // "를 획득하였습니다."
			textRects.strItemName = std::move(strItemName);
			textRects.iNameRgb = Item_GradeRgb(pDefinition->strGrade);
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
	if (!CArenaCameraProfile::Load(ARENA_CAMERA_MAP::VALTAN,
		m_FollowCameraProfile, m_strFollowCameraProfileStatus))
	{
		OutputDebugStringA(("[Level_ValtanArena][FollowCamera] " +
			m_strFollowCameraProfileStatus + "\n").c_str());
	}
	float3_t focus(151.25f, 22.97f, -121.75f);
	LostArk::Shared::S2C_PLAYER_SPAWNED approvedSpawn{};
	if (CNetworkManager::Get().Try_Get_LocalSpawn(approvedSpawn))
		focus = float3_t(approvedSpawn.fPositionX, approvedSpawn.fPositionY, approvedSpawn.fPositionZ);
	const f32_t span = 180.f;
	const float3_t positionOffset = m_FollowCameraProfile.positionOffset;
	const float3_t lookOffset = CArenaCameraProfile::LookOffset(m_FollowCameraProfile);
	CCamera_Free::CAMERA_FREE_DESC cameraDesc{};
	cameraDesc.vEye = float3_t(
		focus.x + positionOffset.x,
		focus.y + positionOffset.y,
		focus.z + positionOffset.z);
	cameraDesc.vAt = float3_t(
		focus.x + lookOffset.x,
		focus.y + lookOffset.y,
		focus.z + lookOffset.z);
	cameraDesc.fFovy = m_FollowCameraProfile.fovYDegrees;
	cameraDesc.fNear = 0.1f;
	cameraDesc.fFar = (std::max)(2000.f, span * 8.f);
	cameraDesc.fSpeedPerSec = g_ValtanFreeCameraSpeed;
	cameraDesc.fRotationPerSec = 90.f;
	cameraDesc.fMouseSensor = 0.1f;
	cameraDesc.vPositionOffset = positionOffset;
	cameraDesc.vLookOffset = lookOffset;
	cameraDesc.fFollowResponse = m_FollowCameraProfile.followResponse;
	cameraDesc.fFollowRollDegrees = m_FollowCameraProfile.rotationDegrees.z;

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

bool_t CLevel_ValtanArena::Set_FollowCameraProfile(
	const ARENA_CAMERA_PROFILE& profile,
	std::string& outStatus)
{
	if (!CArenaCameraProfile::Validate(profile, outStatus))
		return false;
	if (nullptr == m_pCamera || !m_pCamera->Set_FollowPose(
		profile.positionOffset, CArenaCameraProfile::LookOffset(profile),
		profile.rotationDegrees.z, profile.fovYDegrees, profile.followResponse))
	{
		outStatus = "The active follow camera could not apply these settings.";
		return false;
	}
	m_FollowCameraProfile = profile;
	if (const auto character = Get_LocalCharacter())
		CCharacter::Set_MapPresentationSizeProfile(profile);
	outStatus = "Applied to this map's follow camera. Save to keep these settings.";
	m_strFollowCameraProfileStatus = outStatus;
	return true;
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
	CCharacter::Set_MapPresentationSizeProfile(m_FollowCameraProfile);
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

CWorldSequencePlayer::TARGET_SET CLevel_ValtanArena::SourceCinematicTargets()
{
    CWorldSequencePlayer::TARGET_SET targets;
    targets.levelIndex = ETOUI(LEVEL::VALTAN_ARENA);
    targets.pCatalog = &m_MapRuntime.Get_Catalog();
    targets.pPlacements = &m_MapRuntime.Get_MutablePlacements();
    targets.pDeployRuntime = &m_DeployRuntime;
    targets.device = m_pDevice;
    targets.context = m_pContext;
    return targets;
}


bool_t CLevel_ValtanArena::Debug_PrepareCompletePlayResources(
    const VALTAN_PATTERN_VIEW& pattern, bool_t& ready, std::string& status)
{
    ready = false;
    if (pattern.strPatternId.empty()) { status = "Complete Play has no selected pattern."; return false; }
    auto& v2Catalog = CEffectV2Catalog::Get();
    if (!m_CompletePlayPreparation || m_CompletePlayPreparation->patternId != pattern.strPatternId)
    {
        COMPLETE_PLAY_PREPARATION staged;
        staged.patternId = pattern.strPatternId;
        std::set<std::string> v1;
        std::set<std::pair<std::string,std::string>> v2;
        std::set<std::string> actions, clips;
        for (const auto& stage : pattern.Stages)
        {
            actions.insert(stage.strActionId);
            clips.insert(stage.RuntimeClipNames.begin(), stage.RuntimeClipNames.end());
            if (!stage.strRuntimeClipName.empty()) clips.insert(stage.strRuntimeClipName);
            // Server-authoritative Valtan always consumes V0. Material V1 is
            // an optional local audition lane and must not block this command.
            for (const auto& cue : stage.ProductCues) v1.insert(cue.strEffectAssetId);
            for (const auto& object : stage.CombatObjectEffects)
            {
                const auto* visual = CActorCatalog::Find_BossCombatObjectVisual("BOSS_VALTAN",
                    object.strCombatObjectArchetypeId, object.strClientVisualId);
                if (!visual) { status = "Complete Play combat visual is unavailable: " + object.strCombatObjectArchetypeId; return false; }
                if (visual->activeEffectKind == BOSS_COMBAT_OBJECT_ACTIVE_EFFECT_KIND::EFFECT_V1)
                    v1.insert(visual->effectAssetId);
                else v2.emplace("GROUP", visual->effectV2Group.groupId);
                if (!visual->hitEffectAssetId.empty()) v1.insert(visual->hitEffectAssetId);
            }
        }
        // Both primary and ghost use these catalog attachments; the existing
        // preparation cache makes already resident defaults a cheap probe.
        for (const auto& actor : CActorCatalog::Get_Bosses())
            if (actor.clientPresentationId == "boss.valtan.client.v1")
                for (const auto& effect : actor.defaultParticles) v1.insert(effect.effectAssetId);
        auto snapshot = v2Catalog.Get_RuntimeSnapshot();
        if ((!snapshot || !snapshot->Is_Ready()) && !v2Catalog.Reload_BossValtanRuntime(status)) return false;
        snapshot = v2Catalog.Get_RuntimeSnapshot();
        if (!snapshot || !snapshot->Is_Ready()) { status = "Complete Play Valtan V2 runtime bindings are unavailable."; return false; }
        for (const auto& binding : snapshot->Get_BossValtanBindings())
            if (binding.strPatternId == pattern.strPatternId ||
                (binding.strPatternId.empty() && (actions.contains(binding.strActionId) || clips.contains(binding.strClip))))
                v2.emplace(binding.eResourceKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF", binding.strResourceId);
        const auto addCinema = [&](const char* suffix) {
            staged.worldIds.emplace_back(std::string("world.sequence.instance.valtan.source-preview.") + suffix);
        };
        if (pattern.strPatternId == "VALTAN_ENTRANCE_CINEMATIC")
            for (const char* suffix : {"entrance","entrance.colorless"}) addCinema(suffix);
        else if (pattern.strPatternId == "VALTAN_TRASH") addCinema("trash");
        else if (pattern.strPatternId == "VALTAN_SIX_PIZZA_106") addCinema("roar");
        else if (pattern.strPatternId == "VALTAN_GHOST_DEATH_AUDITION") addCinema("finale");
        else if (pattern.strPatternId == "VALTAN_ARENA_BREAK_109") addCinema("phase2");
        const auto& world = m_SourceCinematicPlayer.Get_Document();
        if (!staged.worldIds.empty() && !m_bSourceCinematicsReady && m_bSourceCinematicsPreparationPending) Ready_SourceCinematics();
        if (!staged.worldIds.empty() && !m_bSourceCinematicsReady)
        { status = "Complete Play source cinematic assembly: " + m_strSourceCinematicPreparationStatus; return m_bSourceCinematicsPreparationPending; }
        for (const auto& id : staged.worldIds)
        {
            const auto* instance = world.Find_Instance(id);
            const auto* sequence = instance ? world.Find_Template(instance->templateId) : nullptr;
            if (!instance || !instance->enabled || !sequence)
            { status = "Complete Play source cinematic instance is unavailable: " + id; return false; }
            for (const auto& effect : sequence->effectTracks)
                if (effect.resourceKind == "V1_EFFECT") v1.insert(effect.resourceId);
                else v2.emplace(effect.resourceKind,effect.resourceId);
        }
        v1.erase("");
        staged.v1Ids.assign(v1.begin(),v1.end()); staged.v2Ids.assign(v2.begin(),v2.end());
        std::vector<std::string> registered;
        if (!staged.v1Ids.empty() && !CEffectPresentationService::Queue_ProductTargets_Priority(staged.v1Ids,registered,status)) return false;
        staged.v1Revision = CEffectCatalog::Get_RuntimeRevision();
        staged.v2Generation = CEffectV2Runtime::Cache_Generation();
        staged.v2Revision = snapshot->Get_Revision(); staged.worldRevision = world.Get_Revision();
        staged.v2Snapshot = std::move(snapshot);
        m_CompletePlayPreparation = std::move(staged);
    }
    auto& pending = *m_CompletePlayPreparation;
    const auto snapshot = v2Catalog.Get_RuntimeSnapshot();
    if (pending.v1Revision != CEffectCatalog::Get_RuntimeRevision() ||
        pending.v2Generation != CEffectV2Runtime::Cache_Generation() || !snapshot ||
        pending.v2Revision != snapshot->Get_Revision() ||
        pending.worldRevision != m_SourceCinematicPlayer.Get_Document().Get_Revision())
    { status = "Complete Play resource generation changed while preparing; select Play again for the new saved generation."; return false; }
    const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(pending.v1Ids);
    if (!pending.v1Ids.empty() && (!probe.strBlockingFailure.empty() || probe.iFailedCount || probe.iUnavailableCount))
    {
        status = "Complete Play Effect preparation failed; no Server start was sent. " + probe.strBlockingFailure;
        for (const auto& id : pending.v1Ids)
        {
            const auto reason = CEffectPresentationService::Get_ProductCuePreparationFailure(id);
            if (!reason.empty()) status += " " + id + ": " + reason;
        }
        if (probe.iUnavailableCount) status += " Unavailable=" + std::to_string(probe.iUnavailableCount);
        return false;
    }
    status = "Preparing Valtan Complete Play: V1 " + std::to_string(probe.iPreparedCount) + "/" + std::to_string(pending.v1Ids.size()) +
        ", V2 " + std::to_string(pending.v2Index) + "/" + std::to_string(pending.v2Ids.size()) +
        ", WORLD " + std::to_string(pending.worldIndex) + "/" + std::to_string(pending.worldIds.size()) + ". Server playback has not started.";
    if (pending.v2Index < pending.v2Ids.size())
    {
        const auto& [kind,id] = pending.v2Ids[pending.v2Index];
        auto resourceSnapshot = pending.v2Snapshot;
        if ((kind == "GROUP" && !resourceSnapshot->Find_Group(id)) ||
            (kind == "LEAF" && !resourceSnapshot->Find_Document(id)))
            if (!v2Catalog.Load_ResourceSnapshot(kind == "GROUP" ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF,
                id, resourceSnapshot, status)) return false;
        EFFECT_V2_GROUP group;
        if (kind == "GROUP")
        {
            const auto* found = resourceSnapshot->Find_Group(id);
            if (!found) { status = "Complete Play V2 group is unavailable: " + id; return false; }
            group = *found;
        }
        else if (kind == "LEAF")
        {
            group.strGroupId = id; EFFECT_V2_GROUP_CHILD child;
            child.strChildId = "valtan.complete-play.leaf"; child.strResourceId = child.strEffectId = id;
            group.Children.push_back(std::move(child));
        }
        else { status = "Complete Play V2 resource kind is invalid: " + kind; return false; }
        if (!CEffectV2Runtime::Prewarm_Group(group,resourceSnapshot,m_pDevice,m_pContext))
        { status = "Complete Play V2 preparation failed: " + id + "; " + CEffectV2Runtime::Last_Error(); return false; }
        ++pending.v2Index; return true;
    }
    if (!pending.v1Ids.empty() && (!probe.bCatalogRevisionCurrent || !probe.bSettled ||
        probe.iPreparedCount != pending.v1Ids.size())) return true;
    if (pending.worldIndex < pending.worldIds.size())
    {
        const auto& id = pending.worldIds[pending.worldIndex];
        if (!m_SourceCinematicPlayer.Prepare_InstanceResources(id,SourceCinematicTargets()))
        { status = "Complete Play WORLD preparation failed: " + id + "; " + m_SourceCinematicPlayer.Get_Status(); return false; }
        ++pending.worldIndex; return true;
    }
    ready = true; status = "Valtan Complete Play resources are fully prepared."; return true;
}

void CLevel_ValtanArena::Ready_SourceCinematics()
{
    if (m_bSourceCinematicsReady) return;
    m_bSourceCinematicsPreparationPending = false;
    const auto targets = SourceCinematicTargets();
    if (m_SourceCinematicPlayer.Get_Document().Get_AreaId().empty() &&
        !m_SourceCinematicPlayer.Load_PreparedArea("LV_LUT_HEARTRB_ED", targets))
    {
        m_strSourceCinematicPreparationStatus = m_SourceCinematicPlayer.Get_Status();
        OutputDebugStringA(("[ValtanSourceCinema] " + m_strSourceCinematicPreparationStatus + "\n").c_str());
        return;
    }
    constexpr std::array<const char*, 6> suffixes = { "entrance", "entrance.colorless", "trash", "roar", "finale", "phase2" };
    std::vector<std::string> effects;
    const auto& document = m_SourceCinematicPlayer.Get_Document();
    for (const char* suffix : suffixes)
    {
        const std::string id = std::string("world.sequence.instance.valtan.source-preview.") + suffix;
        const auto* instance = document.Find_Instance(id);
        const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
        if (!instance || !instance->enabled || !sequence)
        { m_strSourceCinematicPreparationStatus = "Source cinematic instance is absent or disabled: " + id; return; }
        for (const auto& effect : sequence->effectTracks)
            if (effect.resourceKind == "V1_EFFECT") effects.push_back(effect.resourceId);
    }
    std::sort(effects.begin(), effects.end());
    effects.erase(std::unique(effects.begin(), effects.end()), effects.end());
    if (!effects.empty())
    {
        std::vector<std::string> registered;
        if (!CEffectPresentationService::Queue_ProductTargets_Priority(effects, registered,
            m_strSourceCinematicPreparationStatus)) return;
        const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(effects);
        if (probe.iFailedCount || probe.iUnavailableCount || !probe.strBlockingFailure.empty())
        {
            m_strSourceCinematicPreparationStatus = "Source cinematic effect preparation failed: " + probe.strBlockingFailure;
            for (const auto& effect : effects)
            {
                const auto failure = CEffectPresentationService::Get_ProductCuePreparationFailure(effect);
                if (!failure.empty()) m_strSourceCinematicPreparationStatus += " " + effect + ": " + failure;
            }
            return;
        }
        if (!probe.bCatalogRevisionCurrent || !probe.bSettled || probe.iPreparedCount != effects.size())
        {
            // A normal asynchronous prepare is not an invalid scene. Keep its
            // admitted document/pools and retry against the same action clock.
            m_bSourceCinematicsPreparationPending = true;
            m_strSourceCinematicPreparationStatus = "Preparing source cinematic effects " +
                std::to_string(probe.iPreparedCount) + "/" + std::to_string(effects.size());
            return;
        }
    }
    for (const char* suffix : suffixes)
    {
        const std::string id = std::string("world.sequence.instance.valtan.source-preview.") + suffix;
        if (!m_SourceCinematicPlayer.Prepare_InstanceResources(id, targets) ||
            !m_SourceCinematicPlayer.Prewarm_ObjectInstances(id, std::string_view(suffix).starts_with("entrance") ? 2u : 1u, targets))
        {
            // Preserve the exact resource failure for the Workbench. Clear()
            // would discard both the admitted scene and its actionable status.
            m_strSourceCinematicPreparationStatus = id + " / " + m_SourceCinematicPlayer.Get_Status();
            OutputDebugStringA(("[ValtanSourceCinema] " + m_strSourceCinematicPreparationStatus + "\n").c_str());
            return;
        }
    }
    m_bSourceCinematicsReady = true;
    m_strSourceCinematicPreparationStatus = "Source cinematic models, clips and effects are prepared.";
}

void CLevel_ValtanArena::Prepare_SourceCinematicInput(VALTAN_CINEMATIC_CAMERA_INPUT& input)
{
    if (!m_bSourceCinematicsReady && m_bSourceCinematicsPreparationPending) Ready_SourceCinematics();
    if (!m_bSourceCinematicsReady) return;
    if (input.isValid && input.iNetEntityId && !input.isBossDead)
    {
        m_LastSourceCinematicInput = input;
        if (const auto primary = m_Replication.Find_PrimaryValtanPresentation())
            m_pSourceCinematicBoss = primary;
    }
    // Reliable DEAD despawn may retire the replicated registry before a DEAD
    // snapshot arrives. Retain only its presentation identity, never gameplay.
    const bool_t dead = input.isBossDead || CCombatHUDViewModel::Get().Get_BossDeadRaw();
    if (dead && !m_bSourceDeathStarted && m_LastSourceCinematicInput.isValid)
    {
        Stop_SourceCinematic();
        m_SourceDeathInput = m_LastSourceCinematicInput;
        m_SourceDeathInput.isBossDead = true;
        m_SourceDeathInput.iPatternSequence = 0u;
        m_SourceDeathInput.iActionStartTick = m_SourceDeathInput.iServerTick;
        m_SourceDeathInput.hasStageCameraInvocations = false;
        m_SourceDeathInput.strStageId.clear();
        m_bSourceDeathStarted = true;
        m_bSourceDeathFinished = false;
    }
    if (m_bSourceDeathStarted && !m_bSourceDeathFinished)
        input = m_SourceDeathInput;
}

bool_t CLevel_ValtanArena::Update_SourceCinematic(const VALTAN_CINEMATIC_CAMERA_INPUT& input)
{
    if (!input.isValid || !input.iNetEntityId || !input.iServerTick || !input.iActionStartTick)
    {
        Stop_SourceCinematic();
        return true;
    }
    f32_t stageAgeSeconds = 0.f;
    if (m_ValtanCinematicCameraController.Is_Active())
        stageAgeSeconds = m_ValtanCinematicCameraController.Get_ElapsedSeconds() +
            (input.isBossDead ? 0.f : input.iCameraStartOffsetMs * .001f);
    else if (input.isBossDead)
    {
        Stop_SourceCinematic(true);
        return true;
    }
    else if (!CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
        input.iServerTick, input.iActionStartTick,
        static_cast<f32_t>(m_ValtanEncounterReference.Get_FixedTickHz()), stageAgeSeconds))
    {
        Stop_SourceCinematic();
        return false;
    }
    std::string selected;
    f32_t sourceOffsetMs = 0.f;
    if (input.isBossDead && m_bSourceDeathStarted && !m_bSourceDeathFinished)
        selected = "finale";
    else if (input.strPatternId == "VALTAN_GHOST_DEATH_AUDITION")
        selected = "finale";
    else if (input.strPatternId == "VALTAN_ENTRANCE_CINEMATIC")
        selected = "entrance";
    else if (input.strPatternId == "VALTAN_ARENA_BREAK_109" &&
        (input.strStageId == "WIDE_REVEAL" || input.strStageId == "RECOVERY" ||
         (input.strStageId == "IMPACT_HOLD" && stageAgeSeconds >= 0.6f)))
        selected = "phase2";
    else if (input.strPatternId == "VALTAN_TRASH" &&
        (input.strStageActionId == "valtan.sequence.center-trash-rush-if.step-05" ||
         input.strStageActionId == "valtan.sequence.center-trash-rush-if.step-06"))
        selected = "trash";
    else if (input.strPatternId == "VALTAN_SIX_PIZZA_106" &&
        (input.strStageId == "STEP_04" || input.strStageId == "STEP_05"))
        selected = "roar";
    if (selected.empty())
    {
        Stop_SourceCinematic(true);
        return true;
    }
    if (!m_bSourceCinematicsReady && m_bSourceCinematicsPreparationPending) Ready_SourceCinematics();
    if (!m_bSourceCinematicsReady) return false;
    if (!input.isBossDead)
    {
        const auto* pattern = m_ValtanEncounterReference.Find_Pattern(input.strPatternId);
        if (!pattern || input.iStageIndex >= pattern->stages.size()) return false;
        sourceOffsetMs = static_cast<f32_t>(pattern->stages[input.iStageIndex].iStartOffsetMs);
        if (selected == "phase2")
            sourceOffsetMs -= 2600.f; // Original420629: Att_Battle_12_03+400ms -> Event_02.
        if (selected == "trash" || selected == "roar")
        {
            const auto first = std::find_if(pattern->stages.begin(), pattern->stages.end(),
                [&](const auto& stage) { return stage.stageId == (selected == "trash" ? "STEP_05" : "STEP_04"); });
            if (first == pattern->stages.end()) return false;
            sourceOffsetMs -= static_cast<f32_t>(first->iStartOffsetMs);
        }
    }
    const auto targets = SourceCinematicTargets();
    const std::string prefix = "world.sequence.instance.valtan.source-preview.";
    f32_t timeMs = sourceOffsetMs + stageAgeSeconds * 1000.f;
    const bool changed = selected != m_strSourceCinematic ||
        m_iSourceCinematicSequence != input.iPatternSequence || m_iSourceCinematicEntity != input.iNetEntityId;
    // Returning from a smoothed camera clock to the last Server snapshot can
    // otherwise rewind a few milliseconds and re-seek the playing WAV. Only
    // bound the same product occurrence; editor scrubbing remains reversible.
    f32_t previousSourceMs = 0.f;
    if (!changed && m_SourceCinematicPlayer.Try_GetElapsedMs(prefix + selected, previousSourceMs))
        timeMs = (std::max)(timeMs, previousSourceMs);
    const auto& sourceDocument = m_SourceCinematicPlayer.Get_Document();
    const auto* sourceInstance = sourceDocument.Find_Instance(prefix + selected);
    const auto* sourceSequence = sourceInstance ? sourceDocument.Find_Template(sourceInstance->templateId) : nullptr;
    if (!sourceSequence) return false;
    if (timeMs < 0.f || timeMs >= static_cast<f32_t>(sourceSequence->durationMs))
    {
        Stop_SourceCinematic(true);
        return true;
    }
    constexpr std::array<std::string_view, 1> entranceCompanions = { "entrance.colorless" };
    if (changed)
    {
        if (!m_strSourceCinematic.empty()) Stop_SourceCinematic();
        bool played = m_SourceCinematicPlayer.Play(prefix + selected, targets);
        if (played && selected == "entrance")
            for (const auto suffix : entranceCompanions)
                if (!m_SourceCinematicPlayer.Play(prefix + std::string(suffix), targets)) { played = false; break; }
        if (!played)
        {
            m_SourceCinematicPlayer.Stop_All(targets, true);
            OutputDebugStringA(("[ValtanSourceCinema] play failed: " + m_SourceCinematicPlayer.Get_Status() + "\n").c_str());
            if (selected == "finale") m_bSourceDeathFinished = true;
            return false;
        }
        m_strSourceCinematic = selected;
        m_iSourceCinematicSequence = input.iPatternSequence;
        m_iSourceCinematicEntity = input.iNetEntityId;
        if (const auto primary = m_pSourceCinematicBoss.lock())
            primary->Set_CinematicPresentationSuppressed(true);
    }
    // Use the camera's smoothed action age while a cut is active; after its
    // return, the same Server action timestamps finish the remaining actors/FX.
    bool sampled = m_SourceCinematicPlayer.Seek_InstanceToMs(prefix + selected, timeMs, targets, false);
    if (sampled && selected == "entrance")
        for (const auto suffix : entranceCompanions)
            if (!m_SourceCinematicPlayer.Seek_InstanceToMs(prefix + std::string(suffix), timeMs, targets, false)) { sampled = false; break; }
    if (!sampled)
    {
        OutputDebugStringA(("[ValtanSourceCinema] sample failed: " + m_SourceCinematicPlayer.Get_Status() + "\n").c_str());
        Stop_SourceCinematic();
        return false;
    }
    return true;
}

void CLevel_ValtanArena::Stop_SourceCinematic(const bool_t preserveSoundTail)
{
    if (m_strSourceCinematic.empty())
    {
        if (!preserveSoundTail) m_SourceCinematicPlayer.Stop_All(SourceCinematicTargets(), true);
        return;
    }
    if (m_strSourceCinematic == "finale") m_bSourceDeathFinished = true;
    m_SourceCinematicPlayer.Stop_All(SourceCinematicTargets(), true, preserveSoundTail);
    if (const auto primary = m_pSourceCinematicBoss.lock())
        primary->Set_CinematicPresentationSuppressed(false);
    m_strSourceCinematic.clear();
    m_iSourceCinematicSequence = 0u;
    m_iSourceCinematicEntity = 0u;
}
