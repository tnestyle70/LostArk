/* imgui.h defines its own placement-new helper and has to precede the project
headers, which is the same order Level_CharacterSelect.cpp uses. */
#ifdef _DEBUG
#include "imgui.h"
#endif

#include "Level_ValtanArena.h"

#include "Camera_Free.h"
#include "Character.h"
#include "GameInstance.h"
#include "LevelRegistry.h"
#include "LevelTransitionService.h"
#include "MainApp.h"
#include "NetworkManager.h"
#include "NetworkPlayerCommandSink.h"
#include "ProjectDataRoot.h"
#include "Transform.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <string_view>

namespace
{
	constexpr std::string_view VALTAN_PILLAR_SET_ID =
		"encounterprop.valtan.four-pillars";
	constexpr std::array<std::string_view, 4> VALTAN_PILLAR_SLOT_IDS = {
		"pillar.valtan.slot00", "pillar.valtan.slot01",
		"pillar.valtan.slot02", "pillar.valtan.slot03" };
	/* The Server currently owns the four inner repeatable slots. Keep this
	   slot-to-placement mapping stable when applying an encounter-prop sync. */
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
		Client::CLevelTransitionService::Report_LoadFailure(E_FAIL, detail);
		return E_FAIL;
	}
}

CLevel_ValtanArena::CLevel_ValtanArena(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CLevel { pDevice, pContext }
{
}

CLevel_ValtanArena::~CLevel_ValtanArena()
{
	End_CinematicCamera();
	m_Replication.Reset();
	m_WorldDestructionDebrisPresentationRuntime.Clear();
	m_WorldDestructionDebrisPresentationDocument.Clear();
	m_WorldDestructionProjectionDocument.Clear();
}

HRESULT CLevel_ValtanArena::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

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
	replicationDesc.strPlayerLayerTag =
		TEXT("Layer_Player");
	replicationDesc.strWorldEntityLayerTag =
		TEXT("Layer_WorldEntity");
	replicationDesc.pDeployPropRuntime = &m_DeployRuntime;
	replicationDesc.pWorldDestructionProjection =
		&m_WorldDestructionProjectionDocument;
	if (!m_Replication.Initialize(replicationDesc))
	{
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

	return S_OK;
}

void CLevel_ValtanArena::Update(f32_t fTimeDelta)
{
	__super::Update(fTimeDelta);
	if (SERVER_WORLD_TRANSFER_PUMP_RESULT::NONE !=
		CLevelTransitionService::Pump_ServerApprovedWorldTransfer(
			LEVEL::VALTAN_ARENA))
	{
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

	if (!m_Replication.Update())
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
			CLevelTransitionService::Report_LoadFailure(
				E_FAIL,
				"[Level_ValtanArena][WorldDestructionSync] " +
					presentationStatus);
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
		End_CinematicCamera();
		if (CLevelTransitionService::Request_Load(
			LEVEL::LOBBY,
			"network.connection-lost"))
		{
			m_Replication.Acknowledge_ConnectionLoss();
			return;
		}
		OutputDebugStringA(
			"[Level_ValtanArena] Lobby recovery request was rejected; retrying.\n");
	}
	if (!Apply_EncounterPropPresentation())
	{
		CLevelTransitionService::Report_LoadFailure(
			E_FAIL,
			"[Level_ValtanArena][EncounterPropSync] " +
				m_DeployRuntime.Get_Status());
		End_CinematicCamera();
		CNetworkManager::Get().Close_ServerConnection();
		(void)CLevelTransitionService::Request_Load(
			LEVEL::LOBBY, "encounter-prop-projection-failed");
		return;
	}

#ifdef _DEBUG
	Render_AuditionPanel();
	/* Driven outside the panel body so a collapsed window cannot stall a
	chapter run that is already in flight. */
	{
		const VALTAN_PRESENTATION_STATE& timelineBoss =
			m_Replication.Get_ValtanPresentationState();
		Advance_EnvironmentTimeline(
			timelineBoss.isValid && !timelineBoss.strPatternId.empty());
	}
#endif
	Update_WorldDestructionPresentation(fTimeDelta);
	Bind_CameraToLocalCharacter();
	Update_CinematicCamera(fTimeDelta);
	const shared_ptr<CCharacter> localCharacter =
		m_Replication.Get_LocalCharacter();
	m_PlayerController.Set_LocalCharacter(localCharacter);
	m_PlayerController.Update(
		nullptr != m_pCamera && m_pCamera->Is_FollowEnabled() &&
		!m_bCinematicCameraApplied);
}

#ifdef _DEBUG
namespace
{
	/* The bars this build can audition are exactly the encounter's authored
	HEALTH_BAR thresholds, read from the same document the Server publishes
	from. Nothing here is a second list to keep in step. */
	std::vector<uint32_t> Collect_AuditionHealthBars(
		const Client::CEncounterPatternReference& reference)
	{
		std::vector<uint32_t> bars;
		for (const Client::ENCOUNTER_PATTERN_REFERENCE& pattern :
			reference.Get_Patterns())
		{
			if ("HEALTH_BAR" != pattern.selectionMode ||
				0u == pattern.iTriggerHealthBar)
			{
				continue;
			}
			bars.push_back(pattern.iTriggerHealthBar);
		}
		std::sort(bars.begin(), bars.end(), std::greater<uint32_t>{});
		bars.erase(std::unique(bars.begin(), bars.end()), bars.end());
		return bars;
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
			return "Reset complete. The Server queued the requested pattern or final-arena state.";
		case VALTAN_AUDITION_RESULT::DUPLICATE_IGNORED:
			return "Already handled that request; the boss did not move.";
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
			return "That pattern already fired, or one is still running.";
		case VALTAN_AUDITION_RESULT::REJECTED_NOT_ARMED:
			return "Arm the same bar before crossing it.";
		case VALTAN_AUDITION_RESULT::REJECTED_PLAYER_NOT_ENGAGED:
			return "Move into engage range and act once, then retry.";
		default:
			return "Unknown audition verdict.";
		}
	}
}

void CLevel_ValtanArena::Submit_Audition(
	const LostArk::Shared::VALTAN_AUDITION_OPERATION operation)
{
	/* These operations name an authored mechanic or a Debug state directly,
	rather than a health-bar crossing, so they carry no target bar. */
	const bool_t isBarless =
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE == operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA ==
			operation ||
		LostArk::Shared::VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL ==
			operation;
	const std::vector<uint32_t> bars =
		Collect_AuditionHealthBars(m_ValtanEncounterReference);
	if (!isBarless && m_iSelectedAuditionBarIndex >= bars.size())
	{
		m_strAuditionStatus = "No authored health-bar pattern is selected.";
		return;
	}

	const uint32_t sequence = m_iNextAuditionRequestSequence;
	if (!CNetworkManager::Get().Send_ValtanAudition(
		sequence, operation,
		isBarless ? 0u : bars[m_iSelectedAuditionBarIndex]))
	{
		m_strAuditionStatus = "Could not send the audition request.";
		return;
	}
	++m_iNextAuditionRequestSequence;
	m_iPendingAuditionRequestSequence = sequence;
	m_strAuditionStatus = "Waiting for the Server verdict...";
}

void CLevel_ValtanArena::Start_EnvironmentTimeline()
{
	using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;
	/* Only the first step resets. Every later chapter is an ARM/CROSS pair, so
	the walls the previous chapter broke stay broken exactly as the recording
	shows them accumulating. */
	m_EnvironmentTimeline = {
		{ OPERATION::PLAY_ENTRANCE, 0u, true },
		{ OPERATION::ARM_HEALTH_BAR, 159u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 159u, true },
		{ OPERATION::ARM_HEALTH_BAR, 109u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 109u, true },
		{ OPERATION::ARM_HEALTH_BAR, 100u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 100u, true },
		{ OPERATION::ARM_HEALTH_BAR, 14u, false },
		{ OPERATION::CROSS_HEALTH_BAR, 14u, true } };
	m_iEnvironmentTimelineStep = 0u;
	m_bEnvironmentTimelineWaiting = false;
	m_bEnvironmentTimelinePatternStarted = false;
}

void CLevel_ValtanArena::Advance_EnvironmentTimeline(
	const bool_t isBossPatternRunning)
{
	if (m_EnvironmentTimeline.empty() ||
		0u != m_iPendingAuditionRequestSequence)
	{
		return;
	}
	if (m_bEnvironmentTimelineWaiting)
	{
		/* A queued pattern needs a tick to start, so the step is only finished
		once the Server actually showed it running and then went idle. */
		if (isBossPatternRunning)
		{
			m_bEnvironmentTimelinePatternStarted = true;
			return;
		}
		if (!m_bEnvironmentTimelinePatternStarted)
			return;
		m_bEnvironmentTimelineWaiting = false;
		m_bEnvironmentTimelinePatternStarted = false;
		++m_iEnvironmentTimelineStep;
	}
	if (m_iEnvironmentTimelineStep >= m_EnvironmentTimeline.size())
	{
		m_EnvironmentTimeline.clear();
		m_iEnvironmentTimelineStep = 0u;
		m_strAuditionStatus =
			"Full environment timeline finished. Pillars stay raised: the shatter has no product trigger yet.";
		return;
	}

	const ENVIRONMENT_TIMELINE_STEP& step =
		m_EnvironmentTimeline[m_iEnvironmentTimelineStep];
	if (0u != step.iTargetHealthBar)
	{
		const std::vector<uint32_t> bars =
			Collect_AuditionHealthBars(m_ValtanEncounterReference);
		const auto selected =
			std::find(bars.begin(), bars.end(), step.iTargetHealthBar);
		if (bars.end() == selected)
		{
			m_EnvironmentTimeline.clear();
			m_iEnvironmentTimelineStep = 0u;
			m_strAuditionStatus =
				"Full environment timeline stopped: an authored bar is missing.";
			return;
		}
		m_iSelectedAuditionBarIndex =
			static_cast<size_t>(std::distance(bars.begin(), selected));
	}
	const uint32_t submitted = m_iNextAuditionRequestSequence;
	Submit_Audition(step.eOperation);
	if (submitted == m_iNextAuditionRequestSequence)
	{
		/* The request never left, so the chapter run stops instead of silently
		skipping the rest of the timeline. */
		m_EnvironmentTimeline.clear();
		m_iEnvironmentTimelineStep = 0u;
		return;
	}
	if (step.waitForPattern)
		m_bEnvironmentTimelineWaiting = true;
	else
		++m_iEnvironmentTimelineStep;
}

void CLevel_ValtanArena::Render_AuditionPanel()
{
	LostArk::Shared::S2C_VALTAN_AUDITION_RESULT result{};
	while (CNetworkManager::Get().Try_Consume_ValtanAuditionResult(result))
	{
		if (result.iRequestSequence != m_iPendingAuditionRequestSequence)
			continue;
		m_iPendingAuditionRequestSequence = 0u;
		m_strAuditionStatus = Describe_AuditionResult(result.eResult);
		if (LostArk::Shared::VALTAN_AUDITION_RESULT::ARMED != result.eResult &&
			LostArk::Shared::VALTAN_AUDITION_RESULT::QUEUED != result.eResult &&
			!m_EnvironmentTimeline.empty())
		{
			/* One refused chapter stops the run. Continuing would audition the
			later bars against an environment the Server never produced. */
			m_EnvironmentTimeline.clear();
			m_iEnvironmentTimelineStep = 0u;
			m_bEnvironmentTimelineWaiting = false;
			m_bEnvironmentTimelinePatternStarted = false;
			m_strAuditionStatus += " Full environment timeline stopped.";
		}
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (nullptr != viewport)
	{
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::SetNextWindowPos(
			ImVec2(viewport->WorkPos.x + 24.f, viewport->WorkPos.y + 24.f),
			ImGuiCond_FirstUseEver);
	}
	if (!ImGui::Begin(
		"Valtan Pattern Audition",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}

	const std::vector<uint32_t> bars =
		Collect_AuditionHealthBars(m_ValtanEncounterReference);
	if (bars.empty())
	{
		ImGui::TextDisabled(
			"No authored health-bar patterns were loaded for this encounter.");
		ImGui::End();
		return;
	}
	if (m_iSelectedAuditionBarIndex >= bars.size())
		m_iSelectedAuditionBarIndex = 0u;

	ImGui::TextUnformatted(
		"Server-authoritative repeatable audition.");
	ImGui::TextUnformatted(
		"Each Play resets boss + walls + collision/nav, then runs the real pattern.");
	ImGui::TextColored(
		ImVec4(1.f, 0.55f, 0.25f, 1.f),
		"One click: every Reset + Play activates the boss, places you at the authored bait point and resets walls/collision/nav.");
	ImGui::TextDisabled(
		"No Stage_Boss walk-through and no monster kill is required first.");
	ImGui::TextColored(
		ImVec4(1.f, 0.82f, 0.2f, 1.f),
		"109: completed outer ring only, 30 walls x 12 fragments.");
	ImGui::TextDisabled(
		"TAKEOFF -> DROP -> IMPACT breaks the 8 outer sectors; interior groups stay dormant;");
	ImGui::TextDisabled(
		"collision and nav blockers open on the persistent commit, not on the cue.");
	ImGui::TextColored(
		ImVec4(0.4f, 0.85f, 1.f, 1.f),
		"Attack/159 charge: ordinary walls only; collision/nav opens after the wall disappears.");
	ImGui::TextColored(
		ImVec4(1.f, 0.82f, 0.2f, 1.f),
		"109 outer ring is pattern-only: attack and charge must leave all 30 intact.");
	ImGui::TextDisabled(
		"No jump clip exists in this model, so the Server owns the 109 leap as an authored arc.");

	ImGui::SeparatorText("Live authoritative state");
	const VALTAN_PRESENTATION_STATE& boss =
		m_Replication.Get_ValtanPresentationState();
	if (boss.isValid)
	{
		ImGui::Text("Server tick: %u  Sequence: %u  Stage index: %u",
			boss.iServerTick,
			boss.iPatternSequence,
			boss.iPatternStageIndex);
		ImGui::TextWrapped("Pattern: %s",
			boss.strPatternId.empty() ? "(idle)" : boss.strPatternId.c_str());
		/* The snapshot carries the stage index, not the stage name, so the
		authored encounter resolves the readable stage ID for the panel. */
		const char_t* stageId = "(idle)";
		if (!boss.strPatternId.empty())
		{
			const ENCOUNTER_PATTERN_REFERENCE* pattern =
				m_ValtanEncounterReference.Find_Pattern(boss.strPatternId);
			if (nullptr != pattern &&
				boss.iPatternStageIndex < pattern->stages.size())
			{
				stageId =
					pattern->stages[boss.iPatternStageIndex].stageId.c_str();
			}
		}
		ImGui::TextWrapped("Stage: %s", stageId);
		ImGui::TextWrapped("Action: %s",
			boss.strActionId.empty() ? "(idle)" : boss.strActionId.c_str());
	}
	else
	{
		ImGui::TextDisabled("Waiting for the authoritative Valtan snapshot...");
	}

	/* The 109 collapse owns the outer ring alone, so the panel counts the ring
	separately from the interior groups that must not react to it. The prefix is
	the authored group naming contract in ValtanWorldEvents.json. */
	static constexpr std::string_view OUTER_RING_GROUP_PREFIX =
		"destroyable.group.valtan.outerwall109.";
	size_t outerGroupCount = 0u;
	size_t outerPlacementCount = 0u;
	for (const WORLD_DESTRUCTION_PROJECTION_GROUP& group :
		m_WorldDestructionProjectionDocument.Get_Groups())
	{
		if (!std::string_view(group.strGroupId).starts_with(
			OUTER_RING_GROUP_PREFIX))
		{
			continue;
		}
		++outerGroupCount;
		outerPlacementCount += group.MemberPlacementIds.size();
	}

	/* The arena floor collapses on its own health-bar patterns, so its sectors
	are counted separately instead of being reported as interior walls that
	reacted to the 109 collapse by mistake. */
	static constexpr std::string_view FLOOR_STAGE_A_GROUP_PREFIX =
		"destroyable.group.valtan.floor84.";
	static constexpr std::string_view FLOOR_STAGE_B_GROUP_PREFIX =
		"destroyable.group.valtan.floor30.";
	size_t intactCount = 0u;
	size_t breakingCount = 0u;
	size_t fracturedCount = 0u;
	size_t despawnedCount = 0u;
	size_t interiorReactedCount = 0u;
	size_t floorStageAIntact = 0u;
	size_t floorStageABreaking = 0u;
	size_t floorStageAGone = 0u;
	size_t floorStageBIntact = 0u;
	size_t floorStageBBreaking = 0u;
	size_t floorStageBGone = 0u;
	for (const LostArk::Shared::WORLD_DESTRUCTION_STATE_WIRE& group :
		m_Replication.Get_WorldDestructionGroupStates())
	{
		const std::string_view groupId(group.strGroupId);
		const bool isStageA = groupId.starts_with(FLOOR_STAGE_A_GROUP_PREFIX);
		const bool isStageB = groupId.starts_with(FLOOR_STAGE_B_GROUP_PREFIX);
		if (isStageA || isStageB)
		{
			size_t& intactSlot = isStageA ? floorStageAIntact : floorStageBIntact;
			size_t& breakingSlot =
				isStageA ? floorStageABreaking : floorStageBBreaking;
			size_t& goneSlot = isStageA ? floorStageAGone : floorStageBGone;
			switch (group.eState)
			{
			case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT:
				++intactSlot;
				break;
			case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING:
				++breakingSlot;
				break;
			default:
				++goneSlot;
				break;
			}
			continue;
		}
		if (!groupId.starts_with(OUTER_RING_GROUP_PREFIX))
		{
			if (LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT !=
				group.eState)
			{
				++interiorReactedCount;
			}
			continue;
		}
		switch (group.eState)
		{
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::INTACT:
			++intactCount;
			break;
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::BREAKING:
			++breakingCount;
			break;
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::FRACTURED:
			++fracturedCount;
			break;
		case LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_STATE::DESPAWNED:
			++despawnedCount;
			break;
		default:
			break;
		}
	}
	ImGui::Text(
		"Outer walls: INTACT %zu | BREAKING %zu | FRACTURED %zu | GONE %zu",
		intactCount,
		breakingCount,
		fracturedCount,
		despawnedCount);
	ImGui::Text("109 outer enabled group count: %zu   Outer placement count: %zu",
		outerGroupCount,
		outerPlacementCount);
	ImGui::Text(
		"Floor Stage A (84): INTACT %zu | BREAKING %zu | GONE %zu   (expected 2)",
		floorStageAIntact,
		floorStageABreaking,
		floorStageAGone);
	ImGui::Text(
		"Floor Stage B (30): INTACT %zu | BREAKING %zu | GONE %zu   (expected 4)",
		floorStageBIntact,
		floorStageBBreaking,
		floorStageBGone);
	if (0u != interiorReactedCount)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.35f, 0.35f, 1.f),
			"Interior groups that left INTACT: %zu (the 109 batch must not touch them)",
			interiorReactedCount);
	}
	ImGui::Text("Destruction sync: %s  Epoch: %u  Debris actors: %u/%u",
		m_Replication.Is_WorldDestructionSynchronized() ? "READY" : "WAITING",
		m_Replication.Get_WorldDestructionEncounterEpoch(),
		m_WorldDestructionDebrisPresentationRuntime.Get_ActiveActorCount(),
		CWorldDestructionDebrisPresentationRuntime::MAX_ACTIVE_ACTORS);
	/* Server-owned counters. The Client never derives passage from wall states;
	a wall that is gone while its blocker is still active must stay visible. */
	const LostArk::Shared::WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS& diagnostics =
		m_Replication.Get_WorldDestructionDiagnostics();
	ImGui::Text("Active player-blocking collision boxes: %u   Active nav regions: %u",
		diagnostics.iActiveWallCollisionCount,
		diagnostics.iActiveNavBlockerRegionCount);
	ImGui::Text("Nav revision: %llu   Last destruction event: %llu",
		static_cast<unsigned long long>(diagnostics.iNavigationRevision),
		static_cast<unsigned long long>(diagnostics.iLastEventSequence));
	ImGui::Text("Camera cue: %s   Sky cue: %s",
		m_ValtanCinematicCameraController.Is_Active() ? "ACTIVE" : "none",
		m_ValtanSkyState.isActive ?
			m_ValtanSkyState.strCueId.c_str() : "none");
	if (m_ValtanSkyState.isActive)
	{
		ImGui::Text("  cloud opacity %.2f   aperture %.2f   spin %.1f deg",
			m_ValtanSkyState.fCloudOpacity,
			m_ValtanSkyState.fApertureScale,
			m_ValtanSkyState.fCloudRotationDegrees);
		if (m_ValtanSkyState.strRedCloudAssetId.empty() &&
			m_ValtanSkyState.strBlackApertureAssetId.empty())
		{
			ImGui::TextDisabled(
				"  sky layer assets are not authored yet (effect owner)");
		}
	}
	/* The pillars are the one repeatable encounter prop, so the panel reports
	the live slot states and the occurrence they belong to. */
	const LostArk::Shared::S2C_ENCOUNTER_PROP_SYNC& props =
		m_Replication.Get_EncounterPropState();
	if (props.Slots.empty())
	{
		ImGui::TextDisabled("Pillars: no encounter prop state received yet.");
	}
	else
	{
		size_t hiddenSlots = 0u;
		size_t intactSlots = 0u;
		size_t breakingSlots = 0u;
		uint32_t occurrence = 0u;
		for (const LostArk::Shared::ENCOUNTER_PROP_SLOT_WIRE& slot : props.Slots)
		{
			occurrence = (std::max)(occurrence, slot.iOccurrenceSequence);
			switch (slot.eState)
			{
			case LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT:
				++intactSlots;
				break;
			case LostArk::Shared::ENCOUNTER_PROP_STATE::BREAKING:
				++breakingSlots;
				break;
			case LostArk::Shared::ENCOUNTER_PROP_STATE::HIDDEN:
				++hiddenSlots;
				break;
			default:
				break;
			}
		}
		ImGui::Text(
			"Pillars: %zu slots | HIDDEN %zu | INTACT %zu | BREAKING %zu",
			props.Slots.size(), hiddenSlots, intactSlots, breakingSlots);
		ImGui::Text("  occurrence %u   epoch %u   set %s",
			occurrence, props.iEncounterEpoch, props.strPropSetId.c_str());
	}
	const bool_t isBusy = 0u != m_iPendingAuditionRequestSequence;
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button(
		"Reset + Play Entrance Whirlwind (Front Walls A/B)", ImVec2(300.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_ENTRANCE);
	}
	if (ImGui::Button(
		"Reset + Play 109 Only (Outer Wall 30 x 12)", ImVec2(330.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 109u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	if (ImGui::Button(
		"Reset + Play 84 (Floor Stage A / Outer Rail)", ImVec2(330.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 84u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	if (ImGui::Button(
		"Reset + Play 30 (Floor Stage B / Brick Ring)", ImVec2(330.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 30u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	ImGui::TextDisabled(
		"Floor sectors stay walkable through BREAKING and turn NON-WALKABLE at the DESPAWNED commit tick.");
	if (ImGui::Button(
		"Reset + Play Attack (Down Smash / Ordinary Wall)",
		ImVec2(330.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_WALL_ATTACK);
	}
	if (ImGui::Button(
		"Reset + Play Charge (159 Impact / Nav)", ImVec2(330.f, 0.f)))
	{
		const auto selected = std::find(bars.begin(), bars.end(), 159u);
		if (selected != bars.end())
		{
			m_iSelectedAuditionBarIndex = static_cast<size_t>(
				std::distance(bars.begin(), selected));
			Submit_Audition(
				LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
		}
	}
	if (ImGui::Button(
		"Reset + Remove All Walls (Final Arena View)", ImVec2(330.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::SHOW_FINAL_ARENA);
	}
	ImGui::TextDisabled(
		"Final Arena View uses the Server destruction transaction; wait about 0.3 seconds for BREAKING -> GONE.");
	if (ImGui::Button(
		"Reset + Break Every Wall (Keep Floor)", ImVec2(330.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL);
	}
	ImGui::TextDisabled(
		"Same transaction without the floor, so 84 and 30 can then collapse with nothing standing above them.");
	if (ImGui::Button(
		"Reset + Play Sky + Pillar Cycle", ImVec2(300.f, 0.f)))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_PILLAR_CYCLE);
	}
	if (ImGui::Button(
		"Reset + Play Full Environment Timeline", ImVec2(300.f, 0.f)))
	{
		Start_EnvironmentTimeline();
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Pillar Cycle runs the authored 100-bar pattern: sky cues, the product raise,");
	ImGui::TextDisabled(
		"then the Debug shatter and removal. Press it again for the next cycle.");
	ImGui::TextColored(
		ImVec4(1.f, 0.72f, 0.3f, 1.f),
		"Cycles 2-4 have no product trigger yet: no pattern, stage or binding is authored for them.");
	if (!m_EnvironmentTimeline.empty())
	{
		ImGui::Text("Timeline chapter %zu / %zu",
			m_iEnvironmentTimelineStep + 1u, m_EnvironmentTimeline.size());
	}
	ImGui::SeparatorText("Advanced authored health bar");
	ImGui::SeparatorText("Authored health bar");
	for (size_t index = 0; index < bars.size(); ++index)
	{
		char_t label[32]{};
		(void)snprintf(label, sizeof(label), "%u", bars[index]);
		if (ImGui::RadioButton(label, m_iSelectedAuditionBarIndex == index))
			m_iSelectedAuditionBarIndex = index;
		if (index + 1u < bars.size())
			ImGui::SameLine();
	}

	ImGui::Separator();
	ImGui::BeginDisabled(isBusy);
	if (ImGui::Button("Reset + Play Selected"))
	{
		Submit_Audition(
			LostArk::Shared::VALTAN_AUDITION_OPERATION::PLAY_HEALTH_BAR);
	}
	/* ARM then CROSS is the only pair that leaves the current destruction
	alone, so whatever an earlier chapter already broke stays broken. */
	if (ImGui::Button("Play Selected (Keep Broken)"))
	{
		if (m_iSelectedAuditionBarIndex >= bars.size())
		{
			m_strAuditionStatus =
				"No authored health-bar pattern is selected.";
		}
		else
		{
			using OPERATION = LostArk::Shared::VALTAN_AUDITION_OPERATION;
			const uint32_t selectedBar = bars[m_iSelectedAuditionBarIndex];
			m_EnvironmentTimeline = {
				{ OPERATION::ARM_HEALTH_BAR, selectedBar, false },
				{ OPERATION::CROSS_HEALTH_BAR, selectedBar, true } };
			m_iEnvironmentTimelineStep = 0u;
			m_bEnvironmentTimelineWaiting = false;
			m_bEnvironmentTimelinePatternStarted = false;
		}
	}
	ImGui::EndDisabled();
	ImGui::TextDisabled(
		"Keep Broken replays the selected bar without resetting walls, floor or props.");

	if (!m_strAuditionStatus.empty())
		ImGui::TextWrapped("%s", m_strAuditionStatus.c_str());
	ImGui::End();
}
#endif

void CLevel_ValtanArena::Update_WorldDestructionPresentation(
	const f32_t fTimeDelta)
{
	const uint64_t generation =
		m_Replication.Get_WorldDestructionPresentationGeneration();
	if (generation != m_iObservedWorldDestructionPresentationGeneration)
	{
		m_WorldDestructionDebrisPresentationRuntime.Reset_Presentation();
		m_iObservedWorldDestructionPresentationGeneration = generation;
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
		states.emplace_back(
			VALTAN_PILLAR_SLOT_PLACEMENT_IDS[index], deployState);
	}
	if (!m_DeployRuntime.Set_States(states))
		return false;
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
	return true;
}

void CLevel_ValtanArena::Update_CinematicCamera(const f32_t fTimeDelta)
{
	if (nullptr == m_pCamera)
		return;
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

	/* One authoritative tuple drives both layers, so the sky can never run on a
	   clock of its own. It is resolved before the camera early-outs because the
	   sky is authored on stages that carry no camera cue. */
	m_ValtanSkyState = m_ValtanCinematicCameraController.Resolve_SkyState(input);

	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!m_ValtanCinematicCameraController.Update(input, fTimeDelta, pose))
	{
		End_CinematicCamera();
		return;
	}

	if (!m_bCinematicCameraApplied)
	{
		m_bCinematicRestoreFollowRequested = m_pCamera->Is_FollowRequested();
		m_pCinematicRestoreTarget = m_pCamera->Get_FollowTarget();
		m_pCamera->Set_FollowEnabled(false);
		m_pCamera->Set_FollowTarget(nullptr);
		if (!m_pCamera->Begin_PresentationOverride(
			static_cast<uint64_t>(boss.iNetEntityId)))
		{
			m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
			m_pCamera->Set_FollowEnabled(
				m_bCinematicRestoreFollowRequested);
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

void CLevel_ValtanArena::End_CinematicCamera()
{
	/* Death, disconnect and level exit all land here, and every one of them has
	   to leave the sky exactly as it was found. */
	m_ValtanSkyState = VALTAN_CINEMATIC_SKY_STATE{};
	if (!m_bCinematicCameraApplied || nullptr == m_pCamera)
		return;
	if (0u != m_iCinematicCameraOwnerId)
		m_pCamera->End_PresentationOverride(m_iCinematicCameraOwnerId);
	m_pCamera->Set_FollowTarget(m_pCinematicRestoreTarget.lock());
	m_pCamera->Set_FollowEnabled(m_bCinematicRestoreFollowRequested);
	m_pCinematicRestoreTarget.reset();
	m_bCinematicRestoreFollowRequested = false;
	m_bCinematicCameraApplied = false;
	m_iCinematicCameraOwnerId = 0u;
}

HRESULT CLevel_ValtanArena::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_Replication.Collect_PlayerViews(m_NameplatePlayers);
	m_PlayerNameplateView.Render(m_NameplatePlayers);

#ifdef _DEBUG
	CMainApp::Update_DebugWindowTitleWithFps(TEXT("Valtan Arena Map"));
#endif

	return S_OK;
}

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
	cameraDesc.fSpeedPerSec = (std::max)(20.f, span * 0.08f);
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
